#include "yoga/YGNode.h"

namespace vitro {

//==============================================================================

Element::Element(const juce::Identifier& tag, Context& ctx)
    : valueTree(tag),
      context{ ctx }
{
}

Element::~Element() = default;

juce::Identifier Element::getTag() const
{
    return valueTree.getType();
}

String Element::getId() const
{
    return valueTree.getProperty(attr::id);
}

Element* Element::getParentElement() const
{
    return parent;
}

Element* Element::getTopLevelElement()
{
    if (parent == nullptr)
        return this;

    return parent->getTopLevelElement();
}

Element* Element::getElementById(const juce::String& id) const
{
    if (getId() == id)
        return const_cast<Element*>(this);

    for (auto* child : children) {
        if (auto* elem{ child->getElementById(id) })
            return elem;
    }

    return nullptr;
}

void Element::addChildElement(Element* element)
{
    jassert(element != nullptr);

    element->parent = this;
    children.add(element);

    valueTree.appendChild(element->valueTree, nullptr);

    element->reconcileElementTree();

    numberOfChildrenChanged();
}

void Element::removeChildElement(Element* element, bool deleteObject)
{
    jassert(element != nullptr);

    if (deleteObject) {
        element->elementIsAboutToBeRemoved();
        element->notifyChildrenAboutToBeRemoved();
    }

    element->parent = nullptr;

    element->reconcileElementTree();

    valueTree.removeChild(element->valueTree, nullptr);

    children.removeObject(element, deleteObject);

    numberOfChildrenChanged();
}

void Element::removeAllChildElements()
{
    valueTree.removeAllChildren(nullptr);

    for (auto* child : children) {
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
    for (auto* child : children)
        child->updateElementIfNeeded();
}

std::pair<bool, const var&> Element::getAttributeChanged(const Identifier& attr) const
{
    const bool changed{ changedAttributes.find(attr) != changedAttributes.end() };

    return {changed, getAttribute(attr) };
}

void Element::notifyChildrenAboutToBeRemoved()
{
    for (auto* child : children) {
        child->elementIsAboutToBeRemoved();
        child->notifyChildrenAboutToBeRemoved();
    }
}

void Element::reconcileElementTree()
{
    reconcileElement();

    for (auto* child : children) {
        child->reconcileElementTree();
    }
}

void Element::forEachChild(const std::function<void(Element*)>& func, bool recursive)
{
    for (auto* child : children) {
        func(child);
    }

    if (recursive) {
        for (auto* child : children) {
            child->forEachChild(func, true);
        }
    }
}

void Element::valueTreePropertyChanged(ValueTree&, const Identifier& changedAttr)
{
    updatePending = true;
    changedAttributes.insert(changedAttr);

    // Trigger update on the root element.
    if (auto* root{ getTopLevelElement() })
        root->update();
}

} // namespace vitro
