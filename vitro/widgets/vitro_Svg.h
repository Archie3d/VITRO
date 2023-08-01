namespace vitro {

/** An SVG drawable element.

    Attributes:
        src
        scale
*/
class Svg : public ComponentElementWithBackground,
            public juce::Component
{
public:

    const static juce::Identifier tag;  // <Svg>

    static JSClassID jsClassID;

    Svg(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::Svg::jsClassID; }

    juce::Component* getComponent() override { return this; }

    bool hasInnerXml() const override { return true; }

    void forwardXmlElement(const juce::XmlElement& xml) override;

    // juce::Component
    void paint(juce::Graphics& g) override;
    void resized() override;

protected:

    // vitro::Element
    void update() override;

    void populateFromXml(const juce::XmlElement& svg);
    void populateFromXmlResource(const juce::String& location);
    void updateScaleTransform();

    float scale{ 1.0f };
    juce::AffineTransform scaleTransform{};
    std::unique_ptr<juce::Drawable> drawable{};
};

} // namespace vitro
