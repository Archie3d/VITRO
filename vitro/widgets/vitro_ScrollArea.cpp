namespace vitro {

JSClassID ScrollArea::jsClassID = 0;

const Identifier ScrollArea::tag("ScrollArea");

ScrollArea::ScrollArea(Context& ctx)
    : ComponentElement(ScrollArea::tag, ctx)
{
    setViewedComponent(&container, false);
}

void ScrollArea::resized()
{
    if (!children.empty()) {
        auto child{ children[0] };
        if (child->isLayoutElement()) {
            if (auto childLayout{ std::dynamic_pointer_cast<LayoutElement>(child) }) {
                auto bounds{ childLayout->getLayoutElementBounds() };
                auto intBounds{ bounds.toNearestInt() };
                container.setSize(intBounds.getWidth(), intBounds.getHeight());
            }
        }
    }
}

void ScrollArea::update()
{
    ComponentElement::update();
}

} // namespace Label