namespace vitro {

class CustomMidiKeyboard : public juce::MidiKeyboardComponent
{
public:

    CustomMidiKeyboard(juce::MidiKeyboardState& state,
                       juce::MidiKeyboardComponent::Orientation orientation);

protected:

    // juce::MidiKeyboardComponent
    void drawWhiteNote(int midiNoteNumber,
                       juce::Graphics& g, juce::Rectangle<float> area,
                       bool isDown, bool isOver,
                       juce::Colour lineColour, juce::Colour textColour) override;

    void drawBlackNote(int midiNoteNumber,
                       juce::Graphics& g, juce::Rectangle<float> area,
                       bool isDown, bool isOver,
                       juce::Colour noteFillColour) override;

};

/** MIDI keyboard.

    This widget corresponds to juce::MidiKeyboardComponent.

    Style properties:
        orientation

*/
class MidiKeyboard : public vitro::ComponentElement,
                     public juce::Component
{
public:

    const static juce::Identifier tag;  // <MidiKeyboard>

    static JSClassID jsClassID;

    MidiKeyboard(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::MidiKeyboard::jsClassID; }

    juce::Component* getComponent() override { return this; }

    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

    // juce::Component
    void resized() override;

protected:

    // vitro::Element
    void update() override;

private:

    juce::MidiKeyboardState keyboardState{};
    CustomMidiKeyboard keyboard;

};

} // namespace vitro
