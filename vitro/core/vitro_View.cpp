namespace vitro {

const Identifier View::tag("View");

JSClassID View::jsClassID = 0;

View::View(Context& ctx)
    : ComponentElementWithBackground(View::tag, ctx)
{
}

View::~View()
{
    inDestructor = true;

    // We must remove all the children in order for them to be notified
    // that they are about to be deleted. This is important for some
    // element to release resources and perform cleaning up.
    removeAllChildElements();
}

void View::initialize()
{
    ComponentElement::initialize();

    exposeToJS();
}

void View::exposeToJS()
{
    auto jsCtx{ context.getJSContext() };
    auto global{ JS_GetGlobalObject(jsCtx) };
    JS_SetPropertyStr(jsCtx, global, "view", JS_DupValue(jsCtx, jsValue));
    JS_FreeValue(jsCtx, global);
}

void View::registerJSPrototype(JSContext* jsCtx, JSValue prototype)
{
    ComponentElement::registerJSPrototype(jsCtx, prototype);

    registerJSMethod(jsCtx, prototype, "createElement", &js_createElement);
    registerJSMethod(jsCtx, prototype, "isDragAndDropActive", &js_isDragAndDropActive);
}

void View::resized()
{
    recalculateLayoutToCurrentBounds();
    updateGradientToComponentSize();
}

void View::paint(Graphics& g)
{
    paintBackground(g);
}

void View::update()
{
    ComponentElementWithBackground::update();

    triggerAsyncUpdate();
}

void View::handleAsyncUpdate()
{
    updateEverything();
}

void View::updateEverything()
{
    if (updateLayout())
        recalculateLayoutToCurrentBounds();

    updateElementIfNeeded();

    updateChildren();

    // Housekeeping: removing unreferenced elements from the stash.
    context.getElementsFactory().clearUnreferencedStashedElements();

    repaint();
}

void View::recalculateLayoutToCurrentBounds()
{
    recalculateLayout(static_cast<float>(getWidth()), static_cast<float>(getHeight()));
}

//==============================================================================

JSValue View::js_createElement(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg)
{
    if (argc != 1)
        return JS_ThrowSyntaxError(ctx, "createElement expects a single argument - element tag name");;

    if (auto view{ Context::getJSNativeObject<View>(self) }) {
        const auto* str{ JS_ToCString(ctx, arg[0]) };
        const auto tag{ String::fromUTF8(str) };
        JS_FreeCString(ctx, str);

        if (auto child{ view->context.getElementsFactory().createElement(tag) }) {
            child->stash();
            return child->duplicateJSValue();
        }

        return JS_NULL;
    }

    return JS_UNDEFINED;
}

JSValue View::js_isDragAndDropActive(JSContext* ctx, JSValueConst self, int, JSValueConst*)
{
    if (auto view{ Context::getJSNativeObject<View>(self) })
        return view->isDragAndDropActive() ? JS_TRUE : JS_FALSE;

    return JS_FALSE;
}

} // namespace vitro
