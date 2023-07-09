namespace vitro {

JSClassID ComponentElement::jsClassID = 0;

//==============================================================================

ComponentElement::MouseEventsProxy::MouseEventsProxy(ComponentElement& el)
    : element {el}
{
    if (auto* component{ element.getComponent() })
        component->addMouseListener(this, true);
}

void ComponentElement::MouseEventsProxy::mouseEnter(const MouseEvent& event)
{
    element.setAttribute(attr::hover, true);
    element.handleMouseEnter(event);
}

void ComponentElement::MouseEventsProxy::mouseExit(const MouseEvent& event)
{
    element.setAttribute(attr::hover, false);
    element.handleMouseExit(event);
}

void ComponentElement::MouseEventsProxy::mouseDown (const MouseEvent& event)
{
    element.setAttribute(attr::active, true);
    element.handleMouseDown(event);
}

void ComponentElement::MouseEventsProxy::mouseUp (const MouseEvent& event)
{
    element.setAttribute (attr::active, false);
    element.handleMouseUp(event);
}

//==============================================================================

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

void ComponentElement::initialize()
{
    LayoutElement::initialize();

    mouseEventsProxy = std::make_unique<MouseEventsProxy>(*this);
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
