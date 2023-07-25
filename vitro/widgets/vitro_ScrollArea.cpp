namespace vitro {

JSClassID ScrollArea::jsClassID = 0;

const Identifier ScrollArea::tag("ScrollArea");

//==============================================================================

void ScrollArea::Container::childBoundsChanged(juce::Component* child)
{
    setSize(child->getWidth(), child->getHeight());
}

//==============================================================================

ScrollArea::ScrollArea(Context& ctx)
    : ComponentElement(ScrollArea::tag, ctx)
{
    setViewedComponent(&container, false);

    registerStyleProperty(attr::css::thumb_color);
    registerStyleProperty(attr::css::vertical_scrollbar);
    registerStyleProperty(attr::css::horizontal_scrollbar);
    registerStyleProperty(attr::css::scrollbar_thickness);
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