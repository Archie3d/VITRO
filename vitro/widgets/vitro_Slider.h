namespace vitro {

/** A slider.

    This UI element is equivalent to juce::Slider.

    Attributes:
        value
        min
        max
        step

    Callback attributes:
        onchange

    Style properties:
        slider-style
        background-color
        thumb-color
        track-color
        fill-color
        border-color
        popup-color
        text-box-color
        text-box-background-color
        text-box-highlight-color
        text-box-border-color
        text-box-position
        text-box-read-only
        text-box-width
        text-box-height
*/
class Slider : public vitro::ComponentElement,
               public juce::Slider,
               private juce::Slider::Listener
{
public:

    const static juce::Identifier tag;  // <Slider>

    static JSClassID jsClassID;

    Slider(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::Label::jsClassID; }

    juce::Component* getComponent() override { return this; }

protected:

    // vitro::Element
    void update() override;

    // juce::Slider::Listener
    void sliderValueChanged(juce::Slider*) override;

    void updateStyle();
    void updateTextBox();
    void updateRange();

};

} // namespace vitro
