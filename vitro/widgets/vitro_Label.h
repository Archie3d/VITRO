namespace vitro {

/** Text label.

    This widget corresponds to juce::Label.

    Attributes:
        text    Label text

    Style properties:
        color
        background-color
        border-color
        text-align
        font-family
        font-style
        font-size
        font-kerning
*/
class Label : public vitro::ComponentElement,
              public juce::Label
{
public:

    const static juce::Identifier tag;  // <Label>

    static JSClassID jsClassID;

    Label(Context& ctx);

    juce::Component* getComponent() override { return this; }

protected:

    // vitro::Element
    void update() override;
};

} // namespace vitro
