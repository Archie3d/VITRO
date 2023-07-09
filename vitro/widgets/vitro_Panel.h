namespace vitro {

/** A simple panel widgets.

    Panel widget corresponds directly to juce::Component. It can
    be used to group other widgets and can be decorated with a background colour.

    @note Currently when background-image is set it will override the background-color property.

    Style attributes:
        background-color
        background-image
        border-color
        border-width
        border-radius
*/
class Panel : public vitro::ComponentElement,
              public juce::Component
{
public:

    const static juce::Identifier tag;  // <Panel>

    static JSClassID jsClassID;

    Panel(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::Panel::jsClassID; }

    juce::Component* getComponent() override { return this; }

    // juce::Component
    void resized() override;
    void paint(juce::Graphics& g) override;

protected:

    // vitro::Element
    void update() override;

private:

    juce::Image backgroundImage{};

    std::optional<juce::Colour> backgroundColour{};

    std::optional<vitro::Gradient> gradient{};
    juce::ColourGradient colourGradient{};

    std::optional<juce::Colour> borderColour{};

    float borderRadius{ 0.0f };
    float borderWidth{ 1.0f };
};

} // namespace vitro
