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
    JSClassID getJSClassID() const override { return vitro::Script::jsClassID; }

    bool hasInnerXml() const override { return true; }

    void forwardXmlElement(const juce::XmlElement& xml) override;

protected:

    // vitro::Element
    void update() override;

private:

    void evaluate(const juce::String& script);

    bool evaluatedOnce{};
    size_t scriptHash{};

};

} // namespace vitro
