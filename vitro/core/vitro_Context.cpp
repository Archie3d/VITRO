namespace vitro {

// This literal is used to inject the js::Context pointer to the
// JavaScript global scope so that it can be retrieved then.
const char* const contextInternalLiteral {"__contetx__"};

//==============================================================================

/** JavaScript function wrapper

    This is a helper class used to capture a JS function to
    be called later. The function object gets copied and
    then released upon the destruction of this object.
*/
class JSFunctionWrapper
{
public:
    JSFunctionWrapper(JSContext* context, JSValueConst val)
        : ctx{ context },
          func{ JS_DupValue(ctx, val) }
    {
    }

    ~JSFunctionWrapper()
    {
        JS_FreeValue(ctx, func);
    }

    void call()
    {
        auto res{ JS_Call(ctx, func, JS_NULL, 0, nullptr) };
        JS_FreeValue(ctx, res); // Release value if function returns any.
    }

private:
    JSContext* ctx;
    JSValue func;
};

/** Helper class to handle async callbacks and timers. */
class TimerPool final
{
public:

    struct Invoker : private juce::Timer
    {
        Invoker(TimerPool& pool, int milliseconds, std::function<void()> f)
            : timerPool{ pool },
              func{ f }
        {
            if (func)
                startTimer (milliseconds);
        }

        ~Invoker()
        {
            stopTimer();
        }

        // juce::Timer
        void timerCallback() override
        {
            func();
            timerPool.removeInvoker(this); // this object is now deleted!
        }

        TimerPool& timerPool;
        std::function<void()> func;

        JUCE_DECLARE_NON_COPYABLE (Invoker)
    };

    //------------------------------------------------------

    TimerPool()
    {}

    ~TimerPool() = default;

    void callAfterDelay(int milliseconds, std::function<void()> f)
    {
        auto inv{ std::make_shared<Invoker>(*this, milliseconds, f) };
        invokers.push_back(inv);
    }

private:

    void removeInvoker(Invoker* ptr)
    {
        auto it{ invokers.begin() };

        while (it != invokers.end()) {
            if (it->get() == ptr) {
                it = invokers.erase(it);
                return;
            }
            ++it;
        }
    }

    std::list<std::shared_ptr<Invoker>> invokers{};
};

//==============================================================================

// Print JavaScript object as string to stderr
static void jsDumpObj(JSContext* ctx, JSValueConst val)
{
    if (const char* str = JS_ToCString(ctx, val)) {
        std::cerr << str << '\n';
        JS_FreeCString(ctx, str);
    } else {
        std::cerr << "[exception]\n";
    }
}

// Helper function used to dump JavaScript error trace to stderr.
static void jsDumpError(JSContext* ctx, JSValueConst exception)
{
    if (JS_IsError(ctx, exception) || JS_IsException(exception)) {
        auto message = JS_GetPropertyStr(ctx, exception, "message");

        if (!JS_IsUndefined(message)) {
            if (auto* str = JS_ToCString(ctx, message)) {
                std::cerr << str << '\n';
                JS_FreeCString(ctx, str);
            }
        }

        JS_FreeValue(ctx, message);

        auto stack{ JS_GetPropertyStr(ctx, exception, "stack") };

        if (!JS_IsUndefined(stack))
            jsDumpObj(ctx, stack);

        JS_FreeValue(ctx, stack);
    }
}

//==============================================================================

// Print console.log() message via juce::Logger.
static JSValue js_console_log(JSContext* ctx, [[maybe_unused]] JSValueConst self, int argc, JSValueConst* argv)
{
    String juceString{};


    for (int i = 0; i < argc; i++) {
        if (i != 0)
            juceString += ' ';

        size_t len{};
        const char* str{ JS_ToCStringLen(ctx, &len, argv[i]) };

        if (!str)
            return JS_EXCEPTION;

        juceString += String::fromUTF8(str, (int)len);
        JS_FreeCString(ctx, str);
    }

    juce::Logger::writeToLog(juceString);

    return JS_UNDEFINED;
}

static JSValue js_setTimeout(JSContext* ctx, [[maybe_unused]] JSValueConst this_val, int argc, JSValueConst* argv)
{
    if (argc != 2)
        return JS_EXCEPTION;

    if (!JS_IsFunction(ctx, argv[0]))
        return JS_EXCEPTION;

    if (JS_VALUE_GET_TAG(argv[1]) != JS_TAG_INT)
        return JS_EXCEPTION;

    auto* context{ Context::getContextFromJSContext(ctx) };

    if (!context)
        return JS_EXCEPTION;

    int delay{};
    JS_ToInt32(ctx, &delay, argv[1]);

    auto funcWrapper{ std::make_shared<JSFunctionWrapper>(ctx, argv[0]) };

    auto callback = [f = funcWrapper]() {
        f->call();
    };

    context->callAfterDelay(delay, callback);

    return JS_UNDEFINED;
}

//==============================================================================

struct Context::Impl final
{
    Context& self;

    Loader loader{};
    Stylesheet stylesheet{};
    ElementsFactory elementsFactory;

    std::unique_ptr<JSRuntime, void(*)(JSRuntime*)> jsRuntime;
    std::unique_ptr<JSContext, void(*)(JSContext*)> jsContext;

    std::unique_ptr<TimerPool> timerPool;

    Impl(Context& ctx)
        : self{ ctx },
          elementsFactory(ctx),
          jsRuntime(JS_NewRuntime(), JS_FreeRuntime),
          jsContext(JS_NewContext(jsRuntime.get()), JS_FreeContext),
          timerPool{ std::make_unique<TimerPool>() }
    {
        exposeGlobals();
    }

    ~Impl()
    {
        // Make sure to delete all the pending timers first.
        timerPool.reset();
    }

    void initialize()
    {
        elementsFactory.registerDefaultElements();
    }

    JSValue eval(StringRef script, StringRef fileName)
    {
        const int evalFlags{ JS_DetectModule(script, script.length()) ? JS_EVAL_TYPE_MODULE : JS_EVAL_TYPE_GLOBAL };

      	if ((evalFlags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE) {
            JSValue val{ JS_Eval(jsContext.get(), script, script.length(), fileName, evalFlags | JS_EVAL_FLAG_COMPILE_ONLY) };

            if (!JS_IsException(val)) {
                /*
                constexpr int useRealPath = 1;
                constexpr int isMain = 1;
                js_module_set_import_meta(jsContext.get(), val, useRealPath, isMain);
                */
                val = JS_EvalFunction(jsContext.get(), val);
            }

            return val;
        }

        return JS_Eval(jsContext.get(), script, script.length(), "", evalFlags);
    }

    JSValue evalThis(JSValueConst thisObj, StringRef script, StringRef fileName)
    {
        const int evalFlags{ JS_EVAL_TYPE_GLOBAL };

        return JS_EvalThis(jsContext.get(), thisObj, script, script.length(), fileName, evalFlags);
    }

    void dumpError()
    {
        auto* ctx{ jsContext.get() };

        JSValue exception{ JS_GetException(ctx) };
        jsDumpError(ctx, exception);
        JS_FreeValue(ctx, exception);
    }

    JSValue getGlobalJSObject()
    {
        return JS_GetGlobalObject(jsContext.get());
    }

    void setGlobalJSNative(StringRef name, void* ptr)
    {
        auto* ctx{ jsContext.get() };
        auto obj{ JS_NewObject(ctx) };
        JS_SetOpaque(obj, ptr);

        auto globalObj{ JS_GetGlobalObject(ctx) };
        JS_SetPropertyStr(ctx, globalObj, name, obj);

        JS_FreeValue(ctx, globalObj);
    }

    void* getGlobalJSNative(StringRef name)
    {
        void* ptr{ nullptr };
        auto* ctx{ jsContext.get() };
        auto globalObj{ JS_GetGlobalObject(ctx) };
        auto obj{ JS_GetPropertyStr(ctx, globalObj, name) };

        if (JS_VALUE_GET_TAG(obj) == JS_TAG_OBJECT)
            JS_GetClassID (obj, &ptr);

        JS_FreeValue(ctx, obj);
        JS_FreeValue(ctx, globalObj);

        return ptr;
    }

    void reset()
    {
        jsContext.reset(JS_NewContext(jsRuntime.get()));
        timerPool = std::make_unique<TimerPool>();

        exposeGlobals();
    }

    void callAfterDelay(int milliseconds, const std::function<void()>& f)
    {
        timerPool->callAfterDelay(milliseconds, f);
    }

    // Inject global objects into JS context
    void exposeGlobals()
    {
        auto* ctx{ jsContext.get() };
        auto global{ JS_GetGlobalObject(ctx) };

        /* Embed this context object */
        {
            auto context{ JS_NewObject(ctx) };
            JS_SetOpaque(context, &self);
            JS_SetPropertyStr(ctx, global, contextInternalLiteral, context);
        }

        /* console.log */
        {
            auto console{ JS_NewObject(ctx) };
            JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, js_console_log, "log", 1));
            JS_SetPropertyStr(ctx, global, "console", console);
        }

        /* setTimeout */
        {
            auto setTimeout{ JS_NewCFunction(ctx, js_setTimeout, "setTimeout", 2) };
            JS_SetPropertyStr(ctx, global, "setTimeout", setTimeout);
        }

        JS_FreeValue(ctx, global);
    }
};

//==============================================================================

Context::Context()
    : d{ std::make_unique<Impl>(*this) }
{
    d->initialize();
}

Context::~Context()
{
    // We must clear all the stashed elements before deleting the
    // context implementation, since the elements deletion will
    // attempt to access the context (freeing JSValues).
    d->elementsFactory.clearStashedElements();
}

const Loader& Context::getLoader() const
{
    return d->loader;
}

Loader& Context::getLoader()
{
    return d->loader;
}

const Stylesheet& Context::getStylesheet() const
{
    return d->stylesheet;
}

Stylesheet& Context::getStylesheet()
{
    return d->stylesheet;
}

const ElementsFactory& Context::getElementsFactory() const
{
    return d->elementsFactory;
}

ElementsFactory& Context::getElementsFactory()
{
    return d->elementsFactory;
}

JSRuntime* Context::getJSRuntime()
{
    return d->jsRuntime.get();
}

JSContext* Context::getJSContext()
{
    return d->jsContext.get();
}

JSValue Context::eval(StringRef script, StringRef fileName)
{
    return d->eval(script, fileName);
}

JSValue Context::evalThis(JSValue thisObj, StringRef script, StringRef fileName)
{
    return d->evalThis(thisObj, script, fileName);
}

void Context::dumpError()
{
    d->dumpError();
}

JSValue Context::getGlobalJSObject()
{
    return d->getGlobalJSObject();
}

void Context::setGlobalJSNative(StringRef name, void* ptr)
{
    d->setGlobalJSNative(name, ptr);
}

void* Context::getGlobalJSNative(StringRef name)
{
    return d->getGlobalJSNative(name);
}

void Context::reset()
{
    d->reset();
}

void Context::callAfterDelay(int milliseconds, const std::function<void()>& f)
{
    d->callAfterDelay(milliseconds, f);
}

Context* Context::getContextFromJSContext(JSContext* ctx)
{
    Context* contextPtr{ nullptr };
    auto global{ JS_GetGlobalObject(ctx) };
    auto obj{ JS_GetPropertyStr(ctx, global, contextInternalLiteral) };

    void* opaque{ nullptr };
    JS_GetClassID(obj, &opaque);
    JS_FreeValue(ctx, obj);

    if (opaque != nullptr)
        contextPtr = reinterpret_cast<Context*>(opaque);

    JS_FreeValue(ctx, global);

    return contextPtr;

}

} // namespace vitro
