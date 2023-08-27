namespace vitro {

/** MIDI keyboard.

    This widget corresponds to juce::MidiKeyboardComponent.

    Style properties:
        orientation

*/
class MidiKeyboard : public vitro::ComponentElement,
                     public juce::MidiKeyboardComponent
{
public:

    const static juce::Identifier tag;  // <MidiKeyboard>

    static JSClassID jsClassID;

    MidiKeyboard(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::MidiKeyboard::jsClassID; }

    juce::Component* getComponent() override { return this; }

    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

    // juce::MidiKeyboardComponent
    void drawWhiteNote(int midiNoteNumber,
                       juce::Graphics& g, juce::Rectangle<float> area,
                       bool isDown, bool isOver,
                       juce::Colour lineColour, juce::Colour textColour) override;

    void drawBlackNote(int midiNoteNumber,
                       juce::Graphics& g, juce::Rectangle<float> area,
                       bool isDown, bool isOver,
                       juce::Colour noteFillColour) override;


protected:

    // vitro::Element
    void update() override;

private:

    juce::MidiKeyboardState keyboardState{};

};

} // namespace vitro
