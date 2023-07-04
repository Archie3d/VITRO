namespace vitro {

ComponentElement::ComponentElement(const Identifier& tag, Context& ctx)
    : LayoutElement(tag, ctx)
{
}

ComponentElement* ComponentElement::getParentComponentElement()
{
    Element* p{ parent };

    while (p != nullptr) {
        if (p->isComponentElement())
            return dynamic_cast<ComponentElement*>(p);

        p = p->getParentElement();
    }

    return nullptr;
}

void ComponentElement::updateComponentBoundsToLayoutNode()
{
    if (auto* component{ getComponent() }) {
        auto targetBounds{ getLayoutElementBounds().toNearestInt() };

        component->setBounds(targetBounds);
    }
}

void ComponentElement::reconcileElement()
{
    LayoutElement::reconcileElement();

    Component* thisComponent{ getComponent() };
    jassert(thisComponent != nullptr);

    if (parent == nullptr) {
        if (auto* owner{ thisComponent->getParentComponent() })
            owner->removeChildComponent(thisComponent);
    } else {
        if (thisComponent->getParentComponent() == nullptr) {
            if (auto* parentComponentElement{ getParentComponentElement() })
                parentComponentElement->getComponent()->addAndMakeVisible(thisComponent);
        }
    }
}

} // namespace vitro
