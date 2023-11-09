namespace vitro {

JSClassID ScrollArea::jsClassID = 0;

const Identifier ScrollArea::tag("ScrollArea");

namespace {
    constexpr int kScrollBarShowTimeMs = 1500;
    constexpr int kScrollBarFadeOutTimeMs = 500;
    constexpr int kScrollBarSingleStepSize = 16;
}

//==============================================================================

ScrollArea::Updater::Updater(ScrollArea& parent)
    : scrollArea{ parent }
{
}

void ScrollArea::Updater::scrollBarMoved(juce::ScrollBar* bar, double newRangeStart)
{
    scrollArea.customScrollBarMoved(bar, newRangeStart);
}

//==============================================================================

void ScrollArea::Container::childBoundsChanged(juce::Component* child)
{
    setSize(child->getWidth(), child->getHeight());
}

//==============================================================================

ScrollArea::ScrollArea(Context& ctx)
    : ComponentElement(ScrollArea::tag, ctx),
      scrollUpdater{ *this }
{
    // Hide the viewport scroll bars but allow scrolling
    setScrollBarsShown(false, false, true, true);

    addChildComponent(horizontalScrollBar);
    addChildComponent(verticalScrollBar);

    horizontalScrollBar.addListener(&scrollUpdater);
    verticalScrollBar.addListener(&scrollUpdater);

    setViewedComponent(&container, false);

    addMouseListener(this, true);

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
    {
        horizontalScrollBar.setVisible(showHorizontalScrollbar);
        verticalScrollBar.setVisible(showVeticalScrollbar);
    }

    // scrollbar-thickness
    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::scrollbar_thickness) }; changed)
    {
        // We store thickness in the native scrollbars, but they are not visible.
        // The thickness will be retrieved then via getScrollBarThickness().
        setScrollBarThickness(prop.isVoid() ? 8 : (int)prop);
    }

    updateScrollBars();
}

void ScrollArea::resized()
{
    juce::Viewport::resized();
    updateScrollBars();
}

void ScrollArea::visibleAreaChanged(const juce::Rectangle<int>& newVisibleArea)
{
    updateScrollBars();
    triggerShowScrollBars();
}

void ScrollArea::mouseMove(const juce::MouseEvent& e)
{
    mousePosition = e.getEventRelativeTo(this).getPosition();
    updateScrollBarsVisibility();
}

void ScrollArea::timerCallback()
{
    stopTimer();
    updateScrollBarsVisibility();
}

void ScrollArea::updateScrollBars()
{
    const auto viewportBounds{ getBounds() };
    const auto scrollBarWidth{ getScrollBarThickness() };
    const auto contentBounds{ getViewedComponent()->getBounds() };

    bool rangeLimitChanged = horizontalScrollBar.getRangeLimit().getEnd() != contentBounds.getWidth();
    
    horizontalScrollBar.setBounds(0, viewportBounds.getHeight() - scrollBarWidth, viewportBounds.getWidth(), scrollBarWidth);
    horizontalScrollBar.setRangeLimits(0.0, contentBounds.getWidth());
    horizontalScrollBar.setCurrentRange(getViewPositionX(), viewportBounds.getWidth());
    horizontalScrollBar.setSingleStepSize(kScrollBarSingleStepSize);
    horizontalScrollBar.cancelPendingUpdate();

    rangeLimitChanged = rangeLimitChanged || verticalScrollBar.getRangeLimit().getEnd() != contentBounds.getHeight();
    
    verticalScrollBar.setBounds(viewportBounds.getWidth() - scrollBarWidth, 0, scrollBarWidth, viewportBounds.getHeight());
    verticalScrollBar.setRangeLimits(0.0, contentBounds.getHeight());
    verticalScrollBar.setCurrentRange(getViewPositionY(), viewportBounds.getHeight());
    verticalScrollBar.setSingleStepSize(kScrollBarSingleStepSize);
    verticalScrollBar.cancelPendingUpdate();
    
    if (rangeLimitChanged)
        triggerShowScrollBars();
}

void ScrollArea::triggerShowScrollBars()
{
    startTimer(kScrollBarShowTimeMs);
    updateScrollBarsVisibility();
}

void ScrollArea::customScrollBarMoved(juce::ScrollBar* bar, double newRangeStart)
{
    int newRangeStartInt{ juce::roundToInt(newRangeStart) };

    if (bar == &horizontalScrollBar) {
        setViewPosition(newRangeStartInt, getViewPositionY());
    } else if (bar == &verticalScrollBar) {
        setViewPosition(getViewPositionX(), newRangeStartInt);
    }
}

void ScrollArea::updateScrollBarsVisibility()
{
    for (auto* bar : { &horizontalScrollBar, &verticalScrollBar }) {
        const bool currentlyVisible{ bar->isVisible() };
        //const bool shouldBeVisible{ isTimerRunning() || bar->getBounds().contains(mousePosition) };
        
        const bool shouldBeVisible{ isTimerRunning() };
        
        if (currentlyVisible && !shouldBeVisible) {
            animator.fadeOut(bar, kScrollBarFadeOutTimeMs);
        } else {
            bar->setVisible(shouldBeVisible);
            bar->setAlpha(shouldBeVisible ? 1.0f : 0.0f);
        }
    }
}

} // namespace Label
