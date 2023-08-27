namespace vitro {

JSClassID MidiKeyboard::jsClassID = 0;

const Identifier MidiKeyboard::tag("MidiKeyboard");

MidiKeyboard::MidiKeyboard(Context& ctx)
    : ComponentElement(MidiKeyboard::tag, ctx),
      juce::MidiKeyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    registerStyleProperty(attr::css::orientation);
}

void MidiKeyboard::drawWhiteNote(int midiNoteNumber,
                                 juce::Graphics& g, juce::Rectangle<float> area,
                                 bool isDown, bool isOver,
                                 juce::Colour lineColour, juce::Colour textColour)
{
    juce::MidiKeyboardComponent::drawWhiteNote(midiNoteNumber, g, area, isDown, isOver, lineColour, textColour);
}

void MidiKeyboard::drawBlackNote(int midiNoteNumber,
                                 juce::Graphics& g, juce::Rectangle<float> area,
                                 bool isDown, bool isOver,
                                 juce::Colour noteFillColour)
{
    juce::MidiKeyboardComponent::drawBlackNote(midiNoteNumber, g, area, isDown, isOver, noteFillColour);
}

void MidiKeyboard::update()
{
    ComponentElement::update();

    // orientation
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::orientation) }; changed) {
        const String orientation{ prop.toString().toLowerCase() };

        if (orientation == "horizontal")
            setOrientation(MidiKeyboardComponent::horizontalKeyboard);
        else if (orientation == "vertical left")
            setOrientation(MidiKeyboardComponent::verticalKeyboardFacingLeft);
        else if (orientation == "vertical right")
            setOrientation(MidiKeyboardComponent::verticalKeyboardFacingRight);
    }

}

} // namespace vitro
