namespace vitro {

JSClassID TextButton::jsClassID = 0;

const Identifier TextButton::tag("TextButton");

TextButton::TextButton(Context& ctx)
    : ButtonBase(TextButton::tag, ctx)
{
    registerStyleProperty(attr::css::toggle);
    registerStyleProperty(attr::css::trigger_down);
    registerStyleProperty(attr::css::text_color_on);
    registerStyleProperty(attr::css::text_color_off);
    registerStyleProperty(attr::css::background_color);
    registerStyleProperty(attr::css::background_color_on);
    registerStyleProperty(attr::css::border_color);
}

void TextButton::update()
{
    ButtonBase::update();

    // toggle
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::toggle) }; !prop.isVoid())
        juce::TextButton::setClickingTogglesState(prop);

    // trigger-down
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::trigger_down) }; !prop.isVoid())
        juce::TextButton::setTriggeredOnMouseDown(prop);

    setColourFromStyleProperty(juce::TextButton::textColourOnId,   attr::css::text_color_on);
    setColourFromStyleProperty(juce::TextButton::textColourOffId,  attr::css::text_color_off);
    setColourFromStyleProperty(juce::TextButton::buttonColourId,   attr::css::background_color);
    setColourFromStyleProperty(juce::TextButton::buttonOnColourId, attr::css::background_color_on);
    setColourFromStyleProperty(juce::ComboBox::outlineColourId,    attr::css::border_color);
}

} // namespace vitro
