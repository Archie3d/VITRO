namespace vitro {

JSClassID Label::jsClassID = 0;

const Identifier Label::tag("Label");

Label::Label(Context& ctx)
    : ComponentElement(Label::tag, ctx)
{
    registerStyleProperty(attr::css::color);
    registerStyleProperty(attr::css::background_color);
    registerStyleProperty(attr::css::border_color);
    registerStyleProperty(attr::css::text_align);
    registerStyleProperty(attr::css::font_family);
    registerStyleProperty(attr::css::font_style);
    registerStyleProperty(attr::css::font_size);
    registerStyleProperty(attr::css::font_kerning);
}

void Label::update()
{
    ComponentElement::update();

    auto v{ getAttribute(attr::text) };
    setText(v.toString(), NotificationType::dontSendNotification);

    setColourFromStyleProperty(juce::Label::textColourId,       attr::css::color);
    setColourFromStyleProperty(juce::Label::backgroundColourId, attr::css::background_color);
    setColourFromStyleProperty(juce::Label::outlineColourId,    attr::css::border_color);

    auto labelFont{ getFont() };
    populateFontFromStyleProperties( labelFont );
    setFont(labelFont);

    // text-align
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::text_align) }; changed) {
        juce::Label::setJustificationType(prop.isVoid() ? Justification::left
                                                        : parseJustificationFromString(prop.toString()));
    }
}

} // namespace vitro
