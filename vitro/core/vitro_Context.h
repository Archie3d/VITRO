namespace vitro {

class ElementsFactory;
class Loader;

#ifndef VITRO_USE_INTERNAL_QUICKJS
JSClassID JS_GetClassID(JSValueConst obj, void** ppopaque) {
  JSObject *p;
  if (JS_VALUE_GET_TAG(obj) != JS_TAG_OBJECT)
    return 0;
  p = JS_VALUE_GET_OBJ(obj);
  if(ppopaque)
    *ppopaque = p->u.opaque;
  return p->class_id;
}
#endif

/** UI Context.

    UI Context aggregates global UI functionalityes like
    the stylesheet, elements factory, resources loader.
*/
class Context final
{
public:

    /** Construct the context.

        This will initialize the new elements factory, stylesheet and JavaScript context.
    */
    Context();

    /** Destroy the context.

        @note When deleting the context there must not be any elements left, otherwise
              they may attemp to access the deleted context.
    */
    ~Context();

    const Loader& getLoader() const;
    Loader& getLoader();

    const Stylesheet& getStylesheet() const;
    Stylesheet& getStylesheet();

    const LookAndFeel& getLookAndFeel() const;
    LookAndFeel& getLookAndFeel();

    const ElementsFactory& getElementsFactory() const;
    ElementsFactory& getElementsFactory();


    JSRuntime* getJSRuntime();
    JSContext* getJSContext();

    /** Register JavaScript class prototype for the elements.

        The class must have a static member jsClassID, which will be
        set to the new class ID value by this class.
    */
    template<class T>
    void registerJSClass(const char* className)
    {
        if (T::jsClassID == 0)
            JS_NewClassID(&T::jsClassID);

        if (!JS_IsRegisteredClass(getJSRuntime(), T::jsClassID)) {
            const JSClassDef def {
                className,
                [](JSRuntime*, JSValue value) {
                    if (auto* ref{ static_cast<typename Element::JSObjectRef*>(JS_GetOpaque(value, T::jsClassID)) })
                        delete ref;
                },
                nullptr,
                nullptr,
                nullptr
            };

            [[maybe_unused]] const auto res = JS_NewClass(getJSRuntime(), T::jsClassID, &def);
            jassert (res == 0);
        }

        auto* ctx{ getJSContext() };

        auto proto{ JS_NewObject(ctx) };
        T::registerJSPrototype(ctx, proto);
        JS_SetClassProto(ctx, T::jsClassID, proto);
    }

    /** Returns inner reference object. */
    template<class T>
    static typename Element::JSObjectRef* getJSObjectRef(JSValue obj)
    {
        void* opaque{ nullptr };
        const JSClassID classID{ JS_GetClassID(obj, &opaque) };
        juce::ignoreUnused(classID);

        // @todo We have to check that classID is among registered ones

        if (/* classID == T::jsClassID && */opaque != nullptr)
            return static_cast<typename Element::JSObjectRef*>(opaque);

        return nullptr;
    }

    template<class T>
    static typename std::shared_ptr<T> getJSNativeObject(JSValue obj)
    {
        if (auto* ref{ getJSObjectRef<T>(obj) }) {
            if (auto elementPtr{ ref->element.lock() }) {
                return std::dynamic_pointer_cast<T>(elementPtr);
            }
        }

        return nullptr;
    }

    /** Evaluate JavaScript code.

        This method evaluates the passed JavaScript code string.
        The returned value is the result of evaluation (if applicable).
    */
    JSValue eval(juce::StringRef script, juce::StringRef fileName = "");

    /** Evaluate JavaScript with a given object context. */
    JSValue evalThis(JSValue thisObj, juce::StringRef script, juce::StringRef fileName = "");

    void dumpError();

    /** Returns global JavaScript object (scope). */
    JSValue getGlobalJSObject();

    /** Store a native pointer in JS global scope. */
    void setGlobalJSNative(juce::StringRef name, void* ptr);

    /** Retrieve a native pointer from JS global scope. */
    void* getGlobalJSNative(juce::StringRef name);

    /** Load a JavaScript model. */
    juce::String loadJSModule(const juce::String& moduleName);

    /** Reset the context.

        This will the script context.
    */
    void reset();

    /** Invoke a function after a delay.

        This method implements a function delayed invocation by
        starting a timer. This is used to implement the setTimeout
        global function.
    */
    void callAfterDelay(int milliseconds, const std::function<void()>& f);

    /** Retrieve the this Context object from the JSContext. */
    static Context* getContextFromJSContext(JSContext* ctx);

private:

    struct Impl;
    std::unique_ptr<Impl> d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Context)
};

} // namespace vitro
