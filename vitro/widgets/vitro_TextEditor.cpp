namespace vitro {

JSClassID TextEditor::jsClassID = 0;

const Identifier TextEditor::tag("TextEditor");

TextEditor::TextEditor(Context& ctx)
    : ComponentElement(TextEditor::tag, ctx)
{
    registerStyleProperty(attr::css::multiline);
    registerStyleProperty(attr::css::password_character);
    registerStyleProperty(attr::css::text_color);
    registerStyleProperty(attr::css::empty_text_color);
    registerStyleProperty(attr::css::background_color);
    registerStyleProperty(attr::css::highlight_color);
    registerStyleProperty(attr::css::highlight_text_color);
    registerStyleProperty(attr::css::border_color);
    registerStyleProperty(attr::css::focused_border_color);
    registerStyleProperty(attr::css::shadow_color);

    addListener(this);
}

void TextEditor::update()
{
    ComponentElement::update();

    // readonly
    if (auto&& [changed, val]{ getAttributeChanged(attr::readonly) }; changed)
        setReadOnly(bool(val));

    // multiline
    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::multiline) }; changed) {
        bool shouldMultiline{ prop.isVoid() ? false : (bool)prop };
        bool shouldWordWrap{ false };

        // wrap
        auto& wrap{ getStyleProperty(attr::css::wrap) };
        shouldWordWrap = wrap.isVoid() ? false : (bool)wrap;

        setReturnKeyStartsNewLine(shouldMultiline);
        setMultiLine(shouldMultiline, shouldWordWrap);
    }

    // password-character
    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::password_character) }; changed) {
        const auto str{ prop.toString() };

        if (str.isNotEmpty())
            setPasswordCharacter(str[0]);
    }

    setColourFromStyleProperty(juce::TextEditor::backgroundColourId,      attr::css::background_color);
    setColourFromStyleProperty(juce::TextEditor::textColourId,            attr::css::text_color);
    setColourFromStyleProperty(juce::TextEditor::highlightColourId,       attr::css::highlight_color);
    setColourFromStyleProperty(juce::TextEditor::highlightedTextColourId, attr::css::highlight_text_color);
    setColourFromStyleProperty(juce::TextEditor::outlineColourId,         attr::css::border_color);
    setColourFromStyleProperty(juce::TextEditor::focusedOutlineColourId,  attr::css::focused_border_color);
    setColourFromStyleProperty(juce::TextEditor::shadowColourId,          attr::css::shadow_color);

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::empty_text_color) }; changed) {
        emptyTextColour = prop.isVoid() ? findColour(juce::TextEditor::textColourId)
                                        : parseColourFromString(prop.toString());
    }

    if (auto&& [changed, val]{ getAttributeChanged(attr::text)}; changed)
        juce::TextEditor::setText(val, NotificationType::dontSendNotification);

    if (auto&& [changed, val]{ getAttributeChanged(attr::emptytext)}; changed)
        juce::TextEditor::setTextToShowWhenEmpty(val, emptyTextColour);
}

void TextEditor::textEditorTextChanged(juce::TextEditor&)
{
    const juce::var value{ getText() };
    setAttribute(attr::text, value, false); // @note Here we do not notify to prevent updates when typing
    evaluateAttributeScript(attr::onchange, value);
}

void TextEditor::textEditorReturnKeyPressed(juce::TextEditor&)
{
    const juce::var value{ getText() };
    evaluateAttributeScript(attr::onenter, value);
}

} // namespace vitro
