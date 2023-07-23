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

const Identifier Element::tag("Element");

JSClassID Element::jsClassID = 0;

Element::Element(Context& ctx)
    : Element(Element::tag, ctx)
{
}

Element::Element(const juce::Identifier& tag, Context& ctx)
    : valueTree(tag),
      context{ ctx }
{
    updatePending = true;
    valueTree.addListener(this);
}

Element::~Element()
{
    inDestructor = true;

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
        child->stash();
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
    registerJSProperty(jsCtx, prototype, "tagName",       &js_getTagName);
    registerJSProperty(jsCtx, prototype, "id",            &js_getId, &js_setId);
    registerJSProperty(jsCtx, prototype, "class",         &js_getClass, &js_setClass);
    registerJSProperty(jsCtx, prototype, "style",         &js_getStyle, &js_setStyle);
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

        if (JS_IsException(res))
            jsDumpError(jsCtx, res);

        if (res != JS_UNDEFINED)
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

JSValue Element::js_getClass(JSContext* jsCtx, JSValueConst self)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        const auto clazz{ element->getAttribute(attr::clazz).toString() };
        return JS_NewStringLen(jsCtx, clazz.toRawUTF8(), clazz.length());
    }

    return JS_UNDEFINED;
}

JSValue Element::js_setClass(JSContext* jsCtx, JSValueConst self, JSValueConst val)
{
    if (auto element{ Context::getJSNativeObject<Element>(self) }) {
        if (JS_IsString(val)) {
            const auto* str{ JS_ToCString(jsCtx, val) };
            element->setAttribute(attr::clazz, String::fromUTF8(str));
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
        element->removeAllChildElements();

        if (argc > 0) {
            int length{};
            JS_ToInt32(ctx, &length, JS_GetPropertyStr(ctx, arg[0], "length"));

            for (int i = 0; i < length; ++i) {
                JSValue item{ JS_GetPropertyUint32(ctx, arg[0], static_cast<uint32_t>(i)) };

                if (auto childElement{ Context::getJSNativeObject<Element>(item) })
                    element->addChildElement(childElement);

                JS_FreeValue(ctx, item);
            }
        }
    }

    return JS_UNDEFINED;
}

} // namespace vitro
