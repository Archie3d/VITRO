namespace vitro {

struct Context::Impl
{
    Context& self;

    Loader loader{};
    Stylesheet stylesheet{};
    ElementsFactory elementsFactory;

    std::unique_ptr<JSRuntime, void(*)(JSRuntime*)> jsRuntime;
    std::unique_ptr<JSContext, void(*)(JSContext*)> jsContext;

    Impl(Context& ctx)
        : self{ ctx },
          elementsFactory(ctx),
          jsRuntime(JS_NewRuntime(), JS_FreeRuntime),
          jsContext(JS_NewContext(jsRuntime.get()), JS_FreeContext)
    {
        initialize();
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
    }
};

//==============================================================================

Context::Context()
    : d{ std::make_unique<Impl>(*this) }
{
    d->initialize();
}

Context::~Context() = default;

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

} // namespace vitro
