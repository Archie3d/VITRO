#include "yoga/YGNode.h"

namespace vitro {

//==============================================================================

Element::JSObjectRef::JSObjectRef(const Element::Ptr& el)
    : element{ el }
{
}

Element::JSObjectRef::~JSObjectRef()
{
    if (auto el{ element.lock() }) {
        el->context.getElementsFactory().removeStashedElement(el);
    }
}

//==============================================================================
// Helpers used to create elements from XML.

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

const Identifier Element::tag("Element");

JSClassID Element::jsClassID = 0;

Element::Element(Context& ctx)
    : Element(Element::tag, ctx)
{
}

Element::Element(const juce::Identifier& elementTag, Context& ctx)
    : valueTree(elementTag),
      context{ ctx }
{
    updatePending = true;
    valueTree.addListener(this);
}

Element::~Element()
{
    inDestructor = true;

    if (JS_VALUE_GET_TAG(jsValue) != JS_TAG_UNINITIALIZED) {
        JS_GetClassID(jsValue, nullptr);

        JS_FreeValue(context.getJSContext(), jsValue);
    }
};

void Element::populateFromXml(const XmlElement& xmlElement)
{
    removeAllChildElements();

    if (hasInnerXml()) {
        // Forward XML if element has a special way to handle it.
        forwardXmlElement(xmlElement);
    } else {
        const Identifier elementTag{ xmlElement.getTagName() };

        if (elementTag == View::tag) {
            auto ptr{ shared_from_this() };
            copyElementAttributesFromXml(ptr, xmlElement);
            populateChildElementsFromXml(context, ptr, xmlElement);
        }
    }

    // Evaluate onload attribute script recursively
    evaluateOnLoadScript(true);

    // Trigger the elements tree update
    forceUpdate();
}

void Element::populateFromXmlString(const String& xmlString)
{
    if (auto xml{ XmlDocument::parse(xmlString) })
        populateFromXml(*xml);
    else
        removeAllChildElements();
}

void Element::populateFromXmlResource(const String& location)
{
    if (auto xml{ context.getLoader().loadXML(location) })
        populateFromXml(*xml);
    else
        removeAllChildElements();
}

std::unique_ptr<XmlElement> Element::createXml() const
{
    auto xml{ std::make_unique<XmlElement>(valueTree.getType().toString()) };

    // Copy attributes
    for (int i = 0; i < valueTree.getNumProperties(); ++i) {
        auto attrName{ valueTree.getPropertyName(i) };

        // Skip volatile attributes
        if (!attr::isVolatile(attrName)) {
            const var& attrValue{ valueTree.getProperty(attrName) };

            if (!attrValue.isVoid() && !attrValue.isUndefined() && !attrValue.isObject())
                xml->setAttribute(attrName, attrValue.toString());
        }
    }

    for (auto&& child : children) {
        if (auto childXml{ child->createXml() })
            xml->addChildElement(childXml.release());
    }

    return xml;
}

juce::Identifier Element::getTag() const
{
    return valueTree.getType();
}

String Element::getId() const
{
    return valueTree.getProperty(attr::id);
}

void Element::setId(const String& id)
{
    setAttribute(attr::id, id);
}

Element::Ptr Element::getParentElement() const
{
    return parent.lock();
}

Element::Ptr Element::getTopLevelElement()
{
    if (inDestructor) {
        // Elements tree is currently being destroyed,
        // we should avoid querying elements pointers.
        return nullptr;
    }

    auto parentPtr{ parent.lock() };

    if (parentPtr == nullptr)
        return shared_from_this();

    return parentPtr->getTopLevelElement();
}

Element::Ptr Element::getElementById(const juce::String& id)
{
    if (getId() == id)
        return shared_from_this();

    for (auto&& child : children) {
        if (auto elem{ child->getElementById(id) })
            return elem;
    }

    return nullptr;
}

void Element::addChildElement(const Element::Ptr& element)
{
    jassert(element != nullptr);

    element->parent = shared_from_this();
    children.push_back(element);

    valueTree.appendChild(element->valueTree, nullptr);

    element->reconcileElementTree();
    element->unstash();

    numberOfChildrenChanged();
}

void Element::removeChildElement(const Element::Ptr& element)
{
    jassert(element != nullptr);

    element->elementIsAboutToBeRemoved();
    element->notifyChildrenAboutToBeRemoved();

    element->parent.reset();

    element->reconcileElementTree();

    valueTree.removeChild(element->valueTree, nullptr);

    children.erase(std::remove(children.begin(), children.end(), element), children.end());

    // @note When we remove an element it is still possible for a JS object to
    //       keep a reference to it. We stash the element to let it be removed later
    //       when there are no more references to it.
    element->stash();

    numberOfChildrenChanged();
}

void Element::removeAllChildElements()
{
    valueTree.removeAllChildren(nullptr);

    for (auto&& child : children) {
        child->elementIsAboutToBeRemoved();
        child->notifyChildrenAboutToBeRemoved();
        child->parent.reset();
        child->reconcileElementTree();
        valueTree.removeChild(child->valueTree, nullptr);
        child->stash();
    }

    // This will delete the removed elements.
    // @note We do not perform reconcile calls here, since the removed
    //       elements will be deleted anyways, which will also remove them
    //       from corresponding internal trees (layout or component).
    children.clear();

    numberOfChildrenChanged();
}

void Element::replaceChildElements(const std::vector<Element::Ptr>& newChildren)
{
    std::vector<bool> retainedChildren(children.size());

    for (size_t i = 0; i < children.size(); ++i) {
        bool retain{ false };
        auto& child{ children.at(i) };

        for (auto& newChild : newChildren) {
            if (child == newChild) {
                retain = true;
                break;
            }
        }

        retainedChildren[i] = retain;
    }

    for (size_t i = 0; i < children.size(); ++i) {
        auto& child{ children.at(i) };

        if (!retainedChildren[i]) {
            child->elementIsAboutToBeRemoved();
            child->notifyChildrenAboutToBeRemoved();
            child->stash();
        }

        child->parent.reset();
        child->reconcileElementTree();
        valueTree.removeChild(child->valueTree, nullptr);
    }

    children.clear();

    for (auto& newChild : newChildren) {
        if (newChild != nullptr) {
            newChild->parent = shared_from_this();
            children.push_back(newChild);
            valueTree.appendChild(newChild->valueTree, nullptr);
            newChild->reconcileElementTree();
            newChild->unstash();
        }
    }

    numberOfChildrenChanged();
}

void Element::setAttribute(const Identifier& name, const var& value, bool notify)
{
    if (isStyledElement() && name == attr::style) {
        if (auto* styledElement{ dynamic_cast<StyledElement*>(this) })
            styledElement->setStyleAttribute(value);
    }

    if (notify) {
        valueTree.setProperty(name, value, nullptr);
    } else {
        if (const var* property{ valueTree.getPropertyPointer(name) }) {
            var* ptr = const_cast<var*>(property);
            *ptr = value;
        } else {
            valueTree.setPropertyExcludingListener(this, name, value, nullptr);
        }
    }
}

const var& Element::getAttribute (const Identifier& name) const
{
    return valueTree[name];
}

bool Element::hasAttribute (const Identifier& name) const
{
    return valueTree.hasProperty(name);
}

void Element::updateElementIfNeeded()
{
    if (updatePending) {
        if (isStyledElement()) {
            if (auto* styleElement{ dynamic_cast<StyledElement*>(this) })
                styleElement->updateStyleProperties();
        }

        update();
        updatePending = false;

        changedAttributes.clear();
    }

    updateChildren();
}

void Element::forceUpdate()
{
    updatePending = true;
    updateElementIfNeeded();
}

void Element::updateChildren()
{
    for (auto&& child : children)
        child->updateElementIfNeeded();
}

void Element::registerJSPrototype(JSContext* jsCtx, JSValue prototype)
{
    registerJSProperty(jsCtx, prototype, "tagName",       &js_getTagName);
    registerJSProperty(jsCtx, prototype, "id",            &js_getId, &js_setId);
    registerJSProperty(jsCtx, prototype, "class",         &js_getClass, &js_setClass);
    registerJSProperty(jsCtx, prototype, "style",         &js_getStyle, &js_setStyle);
    registerJSProperty(jsCtx, prototype, "innerXml",      &js_getInnerXml, &js_setInnerXml);
    registerJSProperty(jsCtx, prototype, "parentElement", &js_getParentElement);
    registerJSProperty(jsCtx, prototype, "children",      &js_getChildren);
    registerJSProperty(jsCtx, prototype, "attributes",    &js_getAttributes);

    registerJSMethod(jsCtx, prototype, "getAttribute",    &js_getAttribute);
    registerJSMethod(jsCtx, prototype, "setAttribute",    &js_setAttribute);
    registerJSMethod(jsCtx, prototype, "hasAttribute",    &js_hasAttribute);
    registerJSMethod(jsCtx, prototype, "getElementById",  &js_getElementById);
    registerJSMethod(jsCtx, prototype, "appendChild",     &js_appendChild);
    registerJSMethod(jsCtx, prototype, "removeChild",     &js_removeChild);
    registerJSMethod(jsCtx, prototype, "replaceChildren", &js_replaceChildren);
}

void Element::stash()
{
    context.getElementsFactory().stashElement(shared_from_this());
}

void Element::unstash()
{
    context.getElementsFactory().removeStashedElement(shared_from_this());
}

JSValue Element::duplicateJSValue()
{
    return JS_DupValue(context.getJSContext(), jsValue);
}

int Element::getJSValueRefCount() const
{
    if (JS_VALUE_HAS_REF_COUNT(jsValue)) {
        JSRefCountHeader *p = (JSRefCountHeader *)JS_VALUE_GET_PTR(jsValue);
        return p->ref_count;
    }

    return -1;
}

void Element::evaluateOnLoadScript(bool recursive)
{
    if (recursive) {
        for (auto&& child : children)
            child->evaluateOnLoadScript(recursive);
    }

    evaluateAttributeScript(attr::onload);
}

void Element::setUpdateHook(const HookFunc& func)
{
    updateHook = func;
}

void Element::initialize()
{
    initJSValue();
}

void Element::update()
{
    if (updateHook)
        updateHook(shared_from_this());
}

std::pair<bool, const var&> Element::getAttributeChanged(const Identifier& attr) const
{
    const bool changed{ changedAttributes.find(attr) != changedAttributes.end() };

    return {changed, getAttribute(attr) };
}

void Element::notifyChildrenAboutToBeRemoved()
{
    for (auto&& child : children) {
        child->elementIsAboutToBeRemoved();
        child->notifyChildrenAboutToBeRemoved();
    }
}

void Element::reconcileElementTree()
{
    reconcileElement();

    for (auto&& child : children) {
        child->reconcileElementTree();
    }
}

void Element::forEachChild(const std::function<void(const Element::Ptr&)>& func, bool recursive)
{
    for (auto&& child : children) {
        func(child);
    }

    if (recursive) {
        for (auto&& child : children) {
            child->forEachChild(func, true);
        }
    }
}

void Element::evaluateAttributeScript(const Identifier& attr, const juce::var& data)
{
    const auto val{ getAttribute(attr) };

    if (val.isVoid())
        return;

    auto* jsCtx{ context.getJSContext() };

    if (val.isObject()) {
        if (auto* func{ dynamic_cast<js::Function*>(val.getObject()) }) {
            if (data.isVoid())
                func->callThis(jsValue);
            else
                func->callThis(jsValue, data);
        }
    } else {
        auto res{ context.evalThis(jsValue, val.toString()) };

        if (JS_IsException(res)) {
            DBG("Exception thrown when evaluating " << attr.toString() << " of <" << getTag().toString() << ">");
            jsDumpError(jsCtx, res);
        }

        if (JS_VALUE_GET_TAG(res) != JS_TAG_UNDEFINED)
            JS_FreeValue(jsCtx, res);
    }
}

void Element::registerConstructor(JSContext* jsCtx, JSValue proto, StringRef name, JSCFunction func, int numArgs)
{
    JSValue clazz{ JS_NewCFunction2(jsCtx, func, name.text, numArgs, JS_CFUNC_constructor, 0) };

    auto global{ JS_GetGlobalObject(jsCtx) };
    JS_DefinePropertyValueStr(jsCtx, global, name.text, JS_DupValue(jsCtx, clazz),
                              JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    JS_SetConstructor(jsCtx, clazz, proto);
    JS_FreeValue(jsCtx, global);
    JS_FreeValue(jsCtx, clazz);
}

void Element::registerJSMethod(JSContext* jsCtx, JSValue proto, StringRef name, JSCFunction func)
{
    JS_SetPropertyStr(jsCtx, proto, name.text, JS_NewCFunction(jsCtx, func, name, 0));
}

void Element::registerJSProperty(JSContext* jsCtx, JSValue proto, StringRef name, JSGetter getter, JSSetter setter)
{
    auto aProperty{ JS_NewAtom(jsCtx, name.text) };
    int flags{ JS_PROP_CONFIGURABLE };

    if (getter != nullptr)
        flags |= JS_PROP_ENUMERABLE;

    if (setter != nullptr)
        flags |= JS_PROP_WRITABLE;

    auto get{ getter != nullptr ? JS_NewCFunction2(jsCtx, (JSCFunction*)getter, "<get>", 0, JS_CFUNC_getter, 0) : JS_UNDEFINED };
    auto set{ setter != nullptr ? JS_NewCFunction2(jsCtx, (JSCFunction*)setter, "<set>", 1, JS_CFUNC_setter, 0) : JS_UNDEFINED };

    JS_DefinePropertyGetSet(jsCtx, proto, aProperty,
        get,
        set,
        flags
    );

    JS_FreeAtom(jsCtx, aProperty);
}

void Element::initJSValue()
{
    jsValue = JS_NewObjectClass(context.getJSContext(), getJSClassID());
    JS_SetOpaque(jsValue, new JSObjectRef(shared_from_this()));
}

void Element::triggerUpdate()
{
    updatePending = true;

    if (auto root{ getTopLevelElement() })
        root->update();
}

void Element::valueTreePropertyChanged(ValueTree&, const Identifier& changedAttr)
{
    changedAttributes.insert(changedAttr);

    triggerUpdate();
}

void Element::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&)
{
    triggerUpdate();
}

void Element::valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int)
{
    triggerUpdate();
}

//==============================================================================

JSValue Element::js_getTagName(JSContext* jsCtx, JSValueConst self)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const auto elementTag{ element->getTag().toString() };
        return JS_NewStringLen(jsCtx, elementTag.toRawUTF8(), elementTag.length());
    }

    return JS_UNDEFINED;
}

JSValue Element::js_getId(JSContext* jsCtx, JSValueConst self)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const auto id{ element->getId() };
        return JS_NewStringLen(jsCtx, id.toRawUTF8(), id.length());
    }

    return JS_UNDEFINED;
}

JSValue Element::js_setId(JSContext* jsCtx, JSValueConst self, JSValueConst val)
{
    if (JS_IsString(val))
        return JS_ThrowTypeError(jsCtx, "id attribute expects a string value");

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        if (const auto* str{ JS_ToCString(jsCtx, val) }) {
            element->setId(String::fromUTF8(str));
            JS_FreeCString(jsCtx, str);
        }
    }

    return JS_UNDEFINED;
}

JSValue Element::js_getClass(JSContext* jsCtx, JSValueConst self)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const auto clazz{ element->getAttribute(attr::clazz)};
        return js::varToJSValue(jsCtx, clazz);
    }

    return JS_UNDEFINED;
}

JSValue Element::js_setClass(JSContext* jsCtx, JSValueConst self, JSValueConst val)
{
    if (!JS_IsString(val) && !JS_IsObject(val))
        return JS_ThrowTypeError(jsCtx, "class attribute expects a string or dictionary value");

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const var clazz{ js::JSValueToVar(jsCtx, val) };
        element->setAttribute(attr::clazz, clazz);
    }

    return JS_UNDEFINED;
}

JSValue Element::js_getStyle(JSContext* jsCtx, JSValueConst self)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const auto style{ element->getAttribute(attr::style) };
        return js::varToJSValue(jsCtx, style);
    }

    return JS_UNDEFINED;
}

JSValue Element::js_setStyle(JSContext* jsCtx, JSValueConst self, JSValueConst val)
{
    if (!JS_IsString(val) && !JS_IsObject(val))
        return JS_ThrowTypeError(jsCtx, "style attribute expects a string or dictionary value");

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const var style{ js::JSValueToVar(jsCtx, val) };
        element->setAttribute(attr::style, style);
    }

    return JS_UNDEFINED;
}

JSValue Element::js_getInnerXml(JSContext* jsCtx, JSValueConst self)
{
    const static auto xmlTextFormat{ XmlElement::TextFormat().withoutHeader() };

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        if (auto xml{ element->createXml() }) {
            const auto str{ xml->toString(xmlTextFormat).trim() };
            return JS_NewStringLen(jsCtx, str.toRawUTF8(), str.length());
        }
    }

    return JS_UNDEFINED;
}

JSValue Element::js_setInnerXml(JSContext* jsCtx, JSValueConst self, JSValueConst val)
{
    if (JS_IsString(val))
        return JS_ThrowTypeError(jsCtx, "innerXml attribute expects a string value");

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        if (const auto* str{ JS_ToCString(jsCtx, val) }) {
            element->populateFromXmlString(String::fromUTF8(str));
            JS_FreeCString(jsCtx, str);
        }
    }

    return JS_UNDEFINED;
}

JSValue Element::js_getAttribute(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg)
{
    if (argc != 1)
        return JS_ThrowSyntaxError(ctx, "getAttribute expects a single argument - attribute's name");

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const auto* str{ JS_ToCString(ctx, arg[0]) };
        const auto attrName{ String::fromUTF8(str) };
        JS_FreeCString(ctx, str);
        const juce::var& val{ element->getAttribute(attrName) };
        return js::varToJSValue(ctx, val);
    }

    return JS_UNDEFINED;
}

JSValue Element::js_setAttribute(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg)
{
    if (argc != 2)
        return JS_ThrowSyntaxError(ctx, "setAttribute expects two arguments: attribute's name and value");

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const auto* str{ JS_ToCString(ctx, arg[0]) };
        const auto attrName{ String::fromUTF8(str) };
        JS_FreeCString(ctx, str);
        const auto val{ js::JSValueToVar(ctx, arg[1]) };
        element->setAttribute(attrName, val);
    }

    return JS_UNDEFINED;
}

JSValue Element::js_hasAttribute(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg)
{
    if (argc != 1)
        return JS_ThrowSyntaxError(ctx, "hasAttribute expects a single argument - attribute name");

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const auto* str{ JS_ToCString(ctx, arg[0]) };
        const auto name{ String::fromUTF8(str) };
        JS_FreeCString(ctx, str);

        if (element->hasAttribute(name))
            return JS_TRUE;

        return JS_FALSE;
    }

    return JS_UNDEFINED;
}

JSValue Element::js_getAttributes(JSContext* ctx, JSValueConst self)
{
    Array<juce::var> attrs{};

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        auto& valueTree {element->valueTree };

        for (int i = 0; i < valueTree.getNumProperties(); ++i) {
            juce::DynamicObject::Ptr obj{ new juce::DynamicObject() };
            const auto name{ valueTree.getPropertyName(i) };
            obj->setProperty("name", name.toString());
            obj->setProperty("value", valueTree.getProperty(name));
            attrs.add(juce::var{ obj });
        }
    }

    return js::varToJSValue(ctx, juce::var(attrs));
}

JSValue Element::js_getParentElement(JSContext* ctx, JSValueConst self)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        if (auto parent{ element->getParentElement() })
            return parent->duplicateJSValue();

        return JS_NULL;
    }

    return JS_UNDEFINED;
}

JSValue Element::js_getChildren(JSContext* ctx, JSValueConst self)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        auto jsArr{ JS_NewArray(ctx) };

        int index{ 0 };

        element->forEachChild([&](const Element::Ptr& child) {
            JS_SetPropertyUint32(ctx, jsArr, index++, child->duplicateJSValue());
        }, false);

        return jsArr;
    }

    return JS_UNDEFINED;
}

JSValue Element::js_getElementById(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg)
{
    if (argc != 1)
        return JS_ThrowSyntaxError(ctx, "getElementById expects a single argument - element's id");

    if (auto element{ Context::getJSNativeObject<Element>(self)}) {
        const auto* str{ JS_ToCString(ctx, arg[0]) };
        const auto id{ String::fromUTF8(str) };
        JS_FreeCString(ctx, str);

        if (auto elementWithId{ element->getElementById(id) })
            return elementWithId->duplicateJSValue();
    }

    return JS_NULL;
}

JSValue Element::js_appendChild([[maybe_unused]] JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg)
{
    if (argc != 1)
        return JS_ThrowSyntaxError(ctx, "appendChild expects a single argument");

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        if (auto childElem{ Context::getJSNativeObject<Element>(arg[0]) })
            element->addChildElement(childElem);
    }

    return JS_UNDEFINED;
}

JSValue Element::js_removeChild([[maybe_unused]] JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg)
{
    if (argc != 1)
        return JS_ThrowSyntaxError(ctx, "removeChild expects a single argument");;

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        if (auto childElem{ Context::getJSNativeObject<Element>(arg[0]) })
            element->removeChildElement(childElem);
    }

    return JS_UNDEFINED;
}

JSValue Element::js_replaceChildren([[maybe_unused]] JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg)
{
    if (argc > 1)
        return JS_ThrowSyntaxError(ctx, "replaceChildren expects none or a single argument");

    if (argc > 0) {
        if (!JS_IsArray(ctx, arg[0]))
            return JS_ThrowTypeError(ctx, "replaceChildren expects an argument of an array type");
    }

    if (auto element{ Context::getJSNativeObject<Element>(self) }) {

        if (argc > 0) {
            int length{};
            JS_ToInt32(ctx, &length, JS_GetPropertyStr(ctx, arg[0], "length"));

            std::vector<Element::Ptr> newChildren((size_t)length);

            for (int i = 0; i < length; ++i) {
                JSValue item{ JS_GetPropertyUint32(ctx, arg[0], static_cast<uint32_t>(i)) };

                if (auto childElement{ Context::getJSNativeObject<Element>(item) })
                    newChildren[i] = childElement;

                JS_FreeValue(ctx, item);
            }

            element->replaceChildElements(newChildren);
        }
    }

    return JS_UNDEFINED;
}

} // namespace vitro
