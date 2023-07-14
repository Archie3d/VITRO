namespace vitro {

JSClassID Slider::jsClassID = 0;

const Identifier Slider::tag("Slider");

Slider::Slider(Context& ctx)
    : ComponentElement(Slider::tag, ctx)
{
    registerStyleProperty(attr::css::background_color);
    registerStyleProperty(attr::css::thumb_color);
    registerStyleProperty(attr::css::track_color);
    registerStyleProperty(attr::css::fill_color);
    registerStyleProperty(attr::css::border_color);
    registerStyleProperty(attr::css::text_box_color);
    registerStyleProperty(attr::css::text_box_background_color);
    registerStyleProperty(attr::css::text_box_highlight_color);
    registerStyleProperty(attr::css::text_box_border_color);
    registerStyleProperty(attr::css::popup_color);
    registerStyleProperty(attr::css::slider_style);
    registerStyleProperty(attr::css::text_box_position);
    registerStyleProperty(attr::css::text_box_read_only);
    registerStyleProperty(attr::css::text_box_width);
    registerStyleProperty(attr::css::text_box_height);

    addListener(this);
}

void Slider::update()
{
    ComponentElement::update();

    setColourFromStyleProperty(juce::Slider::backgroundColourId,          attr::css::background_color);
    setColourFromStyleProperty(juce::Slider::thumbColourId,               attr::css::thumb_color);
    setColourFromStyleProperty(juce::Slider::trackColourId,               attr::css::track_color);
    setColourFromStyleProperty(juce::Slider::rotarySliderFillColourId,    attr::css::fill_color);
    setColourFromStyleProperty(juce::Slider::rotarySliderOutlineColourId, attr::css::border_color);
    setColourFromStyleProperty(juce::Slider::textBoxTextColourId,         attr::css::text_box_color);
    setColourFromStyleProperty(juce::Slider::textBoxBackgroundColourId,   attr::css::text_box_background_color);
    setColourFromStyleProperty(juce::Slider::textBoxHighlightColourId,    attr::css::text_box_highlight_color);
    setColourFromStyleProperty(juce::Slider::textBoxOutlineColourId,      attr::css::text_box_border_color);
    setColourFromStyleProperty(juce::TooltipWindow::textColourId,         attr::css::popup_color);

    updateStyle();
    updateTextBox();
    updateRange();

    if (auto&& [changed, val]{ getAttributeChanged(attr::decimals) }; changed)
        setNumDecimalPlacesToDisplay(val.isVoid() ? 7 : (int)val);

    if (auto&& [changed, val]{ getAttributeChanged(attr::value) }; changed && !val.isVoid())
        juce::Slider::setValue(val, juce::dontSendNotification);
}

void Slider::sliderValueChanged(juce::Slider*)
{
    setAttribute(attr::value, juce::Slider::getValue());

    // @todo Evaluate JS onvaluechange
}

void Slider::updateStyle()
{
    if (const auto&& [changed, val]{ getStylePropertyChanged(attr::css::slider_style) }; changed) {
        if (val.isVoid()) {
            setSliderStyle(LinearHorizontal); // Default slider style
        } else {
            const auto str{ val.toString().trim().toLowerCase() };
            StringArray tokens{};
            tokens.addTokens(str, false);

            bool horizontal{ false };
            bool vertical{ false };
            bool bar{ false };
            bool rotary{ false };

            for (const auto& t : tokens) {
                horizontal = horizontal || (t == "horizontal");
                vertical   = vertical   || (t == "vertical");
                rotary     = rotary     || (t == "rotary");
                bar        = bar        || (t == "bar");
            }

            if (rotary) {
                setSliderStyle(Rotary);
            } else if (bar) {
                if (vertical)
                    setSliderStyle(LinearBarVertical);
                else if (horizontal)
                    setSliderStyle(LinearBar);
            } else {
                if (vertical)
                    setSliderStyle(LinearVertical);
                else if (horizontal)
                    setSliderStyle(LinearHorizontal);

            }
        }
    }
}

void Slider::updateTextBox()
{
    auto textBoxPosition{ getTextBoxPosition() };
    bool textBoxReadOnly{ !isTextBoxEditable() };
    int textBoxWidth{ getTextBoxWidth() };
    int textBoxHeight{ getTextBoxHeight() };
    bool textBoxStyleChanged{ false };

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::text_box_position) }; changed && !prop.isVoid()) {
        const auto str{ prop.toString().trim().toLowerCase() };
        textBoxPosition = parseSliderTextBoxPositionFromString(str);
        textBoxStyleChanged = true;
    }

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::text_box_read_only) }; changed && !prop.isVoid()) {
        textBoxReadOnly = bool(prop);
        textBoxStyleChanged = true;
    }

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::text_box_width) }; changed && !prop.isVoid()) {
        textBoxWidth = int(prop);
        textBoxStyleChanged = true;
    }

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::text_box_height) }; changed && !prop.isVoid()) {
        textBoxHeight = int(prop);
        textBoxStyleChanged = true;
    }

    if (textBoxStyleChanged)
        setTextBoxStyle(textBoxPosition, textBoxReadOnly, textBoxWidth, textBoxHeight);
}

void Slider::updateRange()
{
    auto rangeMin{ getMinimum() };
    auto rangeMax{ getMaximum() };
    auto rangeStep{ getInterval() };
    bool rangeChanged{ false };

    if (auto&& [changed, val]{ getAttributeChanged(attr::min)}; changed && !val.isVoid()) {
        rangeMin = val;
        rangeChanged = true;
    }

    if (auto&& [changed, val]{ getAttributeChanged(attr::max)}; changed && !val.isVoid()) {
        rangeMax = val;
        rangeChanged = true;
    }

    if (auto&& [changed, val]{ getAttributeChanged(attr::step)}; changed && !val.isVoid()) {
        rangeStep = val;
        rangeChanged = true;
    }

    if (rangeChanged)
        setRange(rangeMin, rangeMax, rangeStep);
}

} // namespace vitro
