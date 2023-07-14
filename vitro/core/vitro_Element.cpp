#include "yoga/YGNode.h"

namespace vitro {

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

JSClassID Element::jsClassID = 0;

Element::Element(const juce::Identifier& tag, Context& ctx)
    : valueTree(tag),
      context{ ctx }
{
    updatePending = true;
    valueTree.addListener(this);
}

Element::~Element()
{
    if (jsValue != JS_UNINITIALIZED) {
        const auto classID{ JS_GetClassID(jsValue, nullptr) };

        JS_FreeValue(context.getJSContext(), jsValue);
    }
};

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
    }

    // This will delete the removed elements.
    // @note We do not perform reconcile calls here, since the removed
    //       elements will be deleted anyways, which will also remove them
    //       from corresponding internal trees (layout or component).
    children.clear();

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
        if (const var* property{ valueTree.getPropertyPointer (name) }) {
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
    registerJSProperty(jsCtx, prototype, "tagName", &js_getTagName);
    registerJSProperty(jsCtx, prototype, "id",      &js_getId,    &js_setId);
    registerJSProperty(jsCtx, prototype, "style",   &js_getStyle, &js_setStyle);
}

void Element::stash()
{
    context.getElementsFactory().stashElement(shared_from_this());
}

void Element::unstash()
{
    context.getElementsFactory().removeStashedElement(shared_from_this());
}

int Element::getJSValueRefCount() const
{
    if (JS_VALUE_HAS_REF_COUNT(jsValue)) {
        JSRefCountHeader *p = (JSRefCountHeader *)JS_VALUE_GET_PTR(jsValue);
        return p->ref_count;
    }

    return -1;
}

void Element::initialize()
{
    initJSValue();
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

void Element::valueTreePropertyChanged(ValueTree&, const Identifier& changedAttr)
{
    updatePending = true;
    changedAttributes.insert(changedAttr);

    // Trigger update on the root element.
    if (auto root{ getTopLevelElement() })
        root->update();
}

//==============================================================================

JSValue Element::js_getTagName(JSContext* jsCtx, JSValueConst self)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const auto tag{ element->getTag().toString() };
        return JS_NewStringLen(jsCtx, tag.toRawUTF8(), tag.length());
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
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        if (JS_IsString(val)) {
            const auto* str{ JS_ToCString(jsCtx, val) };
            element->setId(String::fromUTF8(str));
            JS_FreeCString(jsCtx, str);
        }
    }

    return JS_UNDEFINED;
}

JSValue Element::js_getStyle(JSContext* jsCtx, JSValueConst self)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const auto style{ element->getAttribute(attr::style).toString() };
        return JS_NewStringLen(jsCtx, style.toRawUTF8(), style.length());
    }

    return JS_UNDEFINED;
}

JSValue Element::js_setStyle(JSContext* jsCtx, JSValueConst self, JSValueConst val)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        if (JS_IsString(val)) {
            const auto* str{ JS_ToCString(jsCtx, val) };
            element->setAttribute(attr::style, String::fromUTF8(str));
            JS_FreeCString(jsCtx, str);
        }
    }

    return JS_UNDEFINED;
}

} // namespace vitro
