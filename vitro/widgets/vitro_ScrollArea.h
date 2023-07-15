namespace vitro {

/** Container for scrollable content.

    This UI element implements a viewport with its single direct child
    as a scrollable content.

    @note This element may have only one direct child,
          all other children will be ignored.
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

    // juce::Component
    void resized() override;

protected:

    // vitro::Element
    void update() override;

private:

    // Scrollable component
    juce::Component container;

};

} // namespace vitro
