namespace vitro {

CustomMidiKeyboard::CustomMidiKeyboard(juce::MidiKeyboardState& state,
                                       juce::MidiKeyboardComponent::Orientation orientation)
    : juce::MidiKeyboardComponent(state, orientation)
{
}

void CustomMidiKeyboard::drawWhiteNote(int midiNoteNumber,
                                       juce::Graphics& g, juce::Rectangle<float> area,
                                       bool isDown, bool isOver,
                                       juce::Colour lineColour, juce::Colour textColour)
{
    juce::MidiKeyboardComponent::drawWhiteNote(midiNoteNumber, g, area, isDown, isOver, lineColour, textColour);
}

void CustomMidiKeyboard::drawBlackNote(int midiNoteNumber,
                                       juce::Graphics& g, juce::Rectangle<float> area,
                                       bool isDown, bool isOver,
                                       juce::Colour noteFillColour)
{
    juce::MidiKeyboardComponent::drawBlackNote(midiNoteNumber, g, area, isDown, isOver, noteFillColour);
}

//==============================================================================

JSClassID MidiKeyboard::jsClassID = 0;

const Identifier MidiKeyboard::tag("MidiKeyboard");

MidiKeyboard::MidiKeyboard(Context& ctx)
    : ComponentElement(MidiKeyboard::tag, ctx),
      keyboardState{},
      keyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    registerStyleProperty(attr::css::orientation);

    addAndMakeVisible(keyboard);
}

void MidiKeyboard::resized()
{
    keyboard.setBounds(getLocalBounds());
}

void MidiKeyboard::update()
{
    ComponentElement::update();

    // orientation
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::orientation) }; changed) {
        const String orientation{ prop.toString().toLowerCase() };

        if (orientation == "horizontal")
            keyboard.setOrientation(MidiKeyboardComponent::horizontalKeyboard);
        else if (orientation == "vertical left")
            keyboard.setOrientation(MidiKeyboardComponent::verticalKeyboardFacingLeft);
        else if (orientation == "vertical right")
            keyboard.setOrientation(MidiKeyboardComponent::verticalKeyboardFacingRight);
    }

}

} // namespace vitro
