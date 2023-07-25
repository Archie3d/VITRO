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
                   public juce::Viewport
{
public:

    const static juce::Identifier tag;  // <ScrollArea>

    static JSClassID jsClassID;

    ScrollArea(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::ScrollArea::jsClassID; }

    juce::Component* getComponent() override { return this; }

    juce::Component* getContainerComponent() override { return &container; }

protected:

    // vitro::Element
    void update() override;

private:

    // Component with the s scrollable content.
    // This component automatically adjusts its size to its child element.
    class Container : public juce::Component
    {
    public:
        // juce::Component
        void childBoundsChanged(juce::Component* child) override;
    };

    // Scrollable component
    Container container;

};

} // namespace vitro
