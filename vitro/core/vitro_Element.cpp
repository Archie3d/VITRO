#include "yoga/YGNode.h"

namespace vitro {

Element::Element()
    : valueTree{}
{
}

Element::Element(const juce::Identifier& tag)
    : valueTree(tag)
{
}

Element::~Element() = default;

juce::Identifier Element::getTag() const
{
    return valueTree.getType();
}

Element* Element::getParentElement() const
{
    return parent;
}

Element* Element::getTopLevelElement() const
{
    if (parent == nullptr)
        return this;

    return parent->getTopLevelElement();
}

void Element::addChildElement(Element* element)
{
    jassert(element != nullptr);

    element->parent = this;
    children.add(element);

    valueTree.appendChild(element->valueTree, nullptr);
}

void Element::removeChildElement(Element* element, bool deleteObject)
{
    jassert(element != nullptr);

    element->parent = nullptr;

    valueTree.removeChild(element->valueTree, nullptr);

    children.removeObject(element, deleteObject);
}

void Element::removeAllChildElements()
{
    valueTree.removeAllChildren(nullptr);
    children.clear();
}

} // namespace vitro
