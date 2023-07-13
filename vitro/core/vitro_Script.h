namespace vitro {

/** Script element.

    This is a non-visual element that contains JavaScript.

    Attributes:
        src
*/
class Script : public Element
{
public:

    const static juce::Identifier tag;  // <script>

    static JSClassID jsClassID;

    Script(Context& ctx);

    // vitro::Element
    bool hasInnerXml() const override { return true; }

    void forwardXmlElement(const juce::XmlElement& xml) override;

protected:

    // vitro::Element
    void update() override;

};

} // namespace vitro
