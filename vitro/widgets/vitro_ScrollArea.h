namespace vitro {

/** Container for scrollable content.

    This UI element implements a viewport with its single direct child
    as a scrollable content.

    @note This element may have only one direct child,
          all other children will be ignored.

    Style properties:
        thumb-color
        vertical-scrollbar
        horizontal-scrollbar
        scrollbar-thickness
*/
class ScrollArea : public vitro::ComponentElement,
                   public juce::Viewport,
                   private juce::Timer
{
public:

    const static juce::Identifier tag;  // <ScrollArea>

    static JSClassID jsClassID;

    ScrollArea(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::ScrollArea::jsClassID; }

    juce::Component* getComponent() override { return this; }

    juce::Component* getContainerComponent() override { return &container; }

    // juce::Viewport
    void resized() override;
    void visibleAreaChanged(const juce::Rectangle<int>& newVisibleArea) override;
    void mouseMove(const juce::MouseEvent& e) override;

protected:

    // vitro::Element
    void update() override;

private:

    class Updater : public juce::ScrollBar::Listener
    {
    public:
        Updater(ScrollArea& parent);

        void scrollBarMoved(juce::ScrollBar* bar, double newRangeStart) override;
    private:

        ScrollArea& scrollArea;
    };

    friend class Updater;

    // Component with the s scrollable content.
    // This component automatically adjusts its size to its child element.
    class Container : public juce::Component
    {
    public:
        // juce::Component
        void childBoundsChanged(juce::Component* child) override;
    };

    void updateScrollBars();

    void triggerShowScrollBars();

    // juce::Timer
    void timerCallback() override;

    // @internal
    // To be called by the Updater
    void customScrollBarMoved(juce::ScrollBar* bar, double newRangeStart);

    void updateScrollBarsVisibility();

    Updater scrollUpdater;

    // Scrollable component
    Container container;

    juce::ScrollBar horizontalScrollBar{ false };
    juce::ScrollBar verticalScrollBar{ true };

    // Animator to fade out the scroll bars
    juce::ComponentAnimator animator{};

    juce::Point<int> mousePosition{};
};

} // namespace vitro
