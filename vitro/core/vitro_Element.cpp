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

void Element::registerJSPrototype(JSContext* ctx, JSValue prototype)
{
    // @todo
}

void Element::stash()
{
    context.getElementsFactory().stashElement(shared_from_this());
}

void Element::unstash()
{
    context.getElementsFactory().removeStashedElement(shared_from_this());
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

} // namespace vitro
