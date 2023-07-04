namespace vitro {

ComponentElement::ComponentElement(const Identifier& tag, Context& ctx)
    : LayoutElement(tag, ctx)
{
}

void ComponentElement::updateComponentBoundsToLayoutNode()
{
    if (auto* component{ getComponent() }) {
        auto targetBounds{ getLayoutElementBounds().toNearestInt() };

        component->setBounds(targetBounds);
    }
}

} // namespace vitro
