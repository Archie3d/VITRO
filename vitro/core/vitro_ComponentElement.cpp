namespace vitro {

JSClassID ComponentElement::jsClassID = 0;

ComponentElement::ComponentElement(const Identifier& tag, Context& ctx)
    : LayoutElement(tag, ctx)
{
}

ComponentElement::Ptr ComponentElement::getParentComponentElement()
{
    Element::Ptr p{ parent.lock() };

    while (p != nullptr) {
        if (p->isComponentElement())
            return std::dynamic_pointer_cast<ComponentElement>(p);

        p = p->getParentElement();
    }

    return nullptr;
}

void ComponentElement::updateComponentBoundsToLayoutNode()
{
    if (auto* component{ getComponent() }) {
        auto targetBounds{ getLayoutElementBounds().toNearestInt() };

        DBG("<" << getTag() << "> (" << targetBounds.getX() << ", " << targetBounds.getY() << ", " <<
            targetBounds.getWidth() << ", " << targetBounds.getHeight() << ")");

        component->setBounds(targetBounds);
    }
}

void ComponentElement::registerJSPrototype(JSContext* ctx, JSValue prototype)
{
    LayoutElement::registerJSPrototype(ctx, prototype);
}

void ComponentElement::reconcileElement()
{
    LayoutElement::reconcileElement();

    juce::Component* thisComponent{ getComponent() };
    jassert(thisComponent != nullptr);

    if (parent.lock() == nullptr) {
        if (auto owner{ thisComponent->getParentComponent() })
            owner->removeChildComponent(thisComponent);
    } else {
        if (thisComponent->getParentComponent() == nullptr) {
            if (auto parentComponentElement{ getParentComponentElement() })
                parentComponentElement->getComponent()->addAndMakeVisible(thisComponent);
        }
    }
}

} // namespace vitro
