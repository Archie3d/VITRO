namespace vitro {

JSClassID ScrollArea::jsClassID = 0;

const Identifier ScrollArea::tag("ScrollArea");

ScrollArea::ScrollArea(Context& ctx)
    : ComponentElement(ScrollArea::tag, ctx)
{
    setViewedComponent(&container, false);

    registerStyleProperty(attr::css::thumb_color);
    registerStyleProperty(attr::css::vertical_scrollbar);
    registerStyleProperty(attr::css::horizontal_scrollbar);
    registerStyleProperty(attr::css::scrollbar_thickness);
}

void ScrollArea::resized()
{
    if (!children.empty()) {
        // Scroll area expect only one child component,
        // if there are others, they will be ignored since we do not
        // know how to lay them out.
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

    setColourFromStyleProperty(getVerticalScrollBar(), juce::ScrollBar::thumbColourId, attr::css::thumb_color);
    setColourFromStyleProperty(getHorizontalScrollBar(), juce::ScrollBar::thumbColourId, attr::css::thumb_color);

    bool showVeticalScrollbar{ isVerticalScrollBarShown() };
    bool showHorizontalScrollbar{ isHorizontalScrollBarShown() };
    bool showScrollbarChanged{ false };

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::vertical_scrollbar) }; changed) {
        showVeticalScrollbar = prop;
        showScrollbarChanged = true;
    }

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::horizontal_scrollbar) }; changed) {
        showHorizontalScrollbar = prop;
        showScrollbarChanged = true;
    }

    if (showScrollbarChanged)
        setScrollBarsShown(showVeticalScrollbar, showHorizontalScrollbar);

    // scrollbar-thickness
    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::scrollbar_thickness) }; changed)
        setScrollBarThickness(prop.isVoid() ? 8 : (int)prop);

}

} // namespace Label