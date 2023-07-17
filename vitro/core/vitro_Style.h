namespace vitro {

/** Style element.

    This is a non-visual element that contains CSS style definitions.

    Attributes:
        src
*/
class Style : public Element
{
public:

    const static juce::Identifier tag;  // <style>

    static JSClassID jsClassID;

    Style(Context& ctx);

    // vitro::Element
    JSClassID getJSClassID() const override { return vitro::Style::jsClassID; }

    bool hasInnerXml() const override { return true; }

    void forwardXmlElement(const juce::XmlElement& xml) override;

protected:

    // vitro::Element
    void update() override;

};

} // namespace vitro
