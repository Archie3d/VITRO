namespace vitro {

static void copyElementAttributesFromXml(const Element::Ptr& element, const XmlElement& xmlElement)
{
    for (int i = 0; i < xmlElement.getNumAttributes(); ++i) {
        const auto& name{ xmlElement.getAttributeName(i) };
        const auto& value{ xmlElement.getAttributeValue(i) };

        element->setAttribute(name, value);
    }
}

static Element::Ptr createElementFromXml(Context& ctx, const XmlElement& xmlElement);

static void populateChildElementsFromXml(Context& ctx, const Element::Ptr& element, const XmlElement& xmlElement)
{
    jassert(element != nullptr);

    for (auto* child : xmlElement.getChildIterator()) {
        if (auto childElement{ createElementFromXml(ctx, *child) })
            element->addChildElement(childElement);
        else
            DBG("Unable to create element for <" << child->getTagName() << ">");
    }
}

static Element::Ptr createElementFromXml(Context& ctx, const XmlElement& xmlElement)
{
    if (xmlElement.isTextElement())
        return nullptr;

    const Identifier tag{ xmlElement.getTagName() };

    auto element{ ctx.getElementsFactory().createElement(tag) };

    if (element != nullptr) {
        copyElementAttributesFromXml(element, xmlElement);

        if (element->hasInnerXml()) {
            // This element manages its inner XML. We don't need to parse the XML tree
            // further but forward the current node to the element.
            element->forwardXmlElement(xmlElement);
        } else {
            populateChildElementsFromXml(ctx, element, xmlElement);
        }

        // Call element's onload script.
        // @note The element is not yet attached to its parent at this point
        element->evaluateOnLoadScript();
    }

    return element;
}

//==============================================================================

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

void View::populateFromXml(const XmlElement& xmlElement)
{
    removeAllChildElements();

    const Identifier tag{ xmlElement.getTagName() };

    if (tag != View::tag)
        return;

    auto ptr{ shared_from_this() };
    copyElementAttributesFromXml(ptr, xmlElement);
    populateChildElementsFromXml(context, ptr, xmlElement);

    // Evaluate onload attribute script recursively
    evaluateOnLoadScript(true);

    // Trigger the elements tree update
    forceUpdate();
}

void View::populateFromXmlString(const String& xmlString)
{
    if (auto xml{ XmlDocument::parse(xmlString) })
        populateFromXml(*xml);
    else
        removeAllChildElements();
}

void View::populateFromXmlResource(const String& location)
{
    if (auto xml{ context.getLoader().loadXML(location) })
        populateFromXml(*xml);
    else
        removeAllChildElements();
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

} // namespace vitro
