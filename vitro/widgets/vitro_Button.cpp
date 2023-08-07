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
    registerStyleProperty(attr::css::border_radius);
    registerStyleProperty(attr::css::border_width);
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

    // background-color
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::background_color) }; changed) {
        if (prop.isVoid()) {
            // Background colour has been removed
            gradient.reset();
        } else {
            const String str{ prop.toString() };

            if (Gradient::isPotentiallyGradientString(str)) {
                gradient = Gradient::fromString(str);
                updateGradientToComponentSize();
            } else {
                setColourFromStyleProperty(juce::TextButton::buttonColourId,   attr::css::background_color);
                gradient.reset();
            }
        }
    }

    setColourFromStyleProperty(juce::TextButton::textColourOnId,   attr::css::text_color_on);
    setColourFromStyleProperty(juce::TextButton::textColourOffId,  attr::css::text_color_off);
    // @note We use custom LAF draw for the background
    //setColourFromStyleProperty(juce::TextButton::buttonColourId,   attr::css::background_color);
    setColourFromStyleProperty(juce::TextButton::buttonOnColourId, attr::css::background_color_on);
    setColourFromStyleProperty(juce::ComboBox::outlineColourId,    attr::css::border_color);

    // border-radius
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::border_radius) }; changed)
        borderRadius = prop;

    // border-width
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::border_width) }; changed)
        borderWidth = prop;
}

void TextButton::resized()
{
    ButtonBase::resized();

    updateGradientToComponentSize();
}

bool TextButton::hasGradientBackground() const
{
    return static_cast<bool>(gradient);
}

juce::ColourGradient TextButton::getBackgroundColourGradient() const
{
    return colourGradient;
}

void TextButton::updateGradientToComponentSize()
{
    if (gradient)
        colourGradient = gradient->getColourGradient(getWidth(), getHeight());
}

//==============================================================================

JSClassID ToggleButton::jsClassID = 0;

const Identifier ToggleButton::tag("ToggleButton");

ToggleButton::ToggleButton(Context& ctx)
    : ButtonBase(ToggleButton::tag, ctx)
{
    registerStyleProperty(attr::css::color);
    registerStyleProperty(attr::css::tick_color);
    registerStyleProperty(attr::css::tick_disabled_color);
}

void ToggleButton::update()
{
    ButtonBase::update();

    setColourFromStyleProperty(juce::ToggleButton::textColourId,         attr::css::color);
    setColourFromStyleProperty(juce::ToggleButton::tickColourId,         attr::css::tick_color);
    setColourFromStyleProperty(juce::ToggleButton::tickDisabledColourId, attr::css::tick_disabled_color);
}

//==============================================================================

JSClassID DrawableButton::jsClassID = 0;

const Identifier DrawableButton::tag("DrawableButton");

std::unique_ptr<juce::Drawable> DrawableButton::loadDrawable(Loader& loader, const String& location)
{
    if (auto xml{ loader.loadXML(location) })
        return juce::Drawable::createFromSVG(*xml);

    return nullptr;
}

DrawableButton::DrawableButton(Context& ctx)
    : ButtonBase(DrawableButton::tag, ctx)
{
    registerStyleProperty(attr::css::button_style);
    registerStyleProperty(attr::css::color);
    registerStyleProperty(attr::css::text_color_on);
    registerStyleProperty(attr::css::background_color);
    registerStyleProperty(attr::css::background_color_on);
    registerStyleProperty(attr::css::image_normal);
    registerStyleProperty(attr::css::image_over);
    registerStyleProperty(attr::css::image_down);
    registerStyleProperty(attr::css::image_disabled);
    registerStyleProperty(attr::css::image_normal_on);
    registerStyleProperty(attr::css::image_over_on);
    registerStyleProperty(attr::css::image_down_on);
    registerStyleProperty(attr::css::image_disabled_on);
}

void DrawableButton::update()
{
    ButtonBase::update();

    if (const auto&& [changed, val]{ getAttributeChanged(attr::text)}; changed)
        juce::DrawableButton::setButtonText(val.toString());

    setColourFromStyleProperty(juce::DrawableButton::textColourId,         attr::css::color);
    setColourFromStyleProperty(juce::DrawableButton::textColourOnId,       attr::css::text_color_on);
    setColourFromStyleProperty(juce::DrawableButton::backgroundColourId,   attr::css::background_color);
    setColourFromStyleProperty(juce::DrawableButton::backgroundOnColourId, attr::css::background_color_on);

    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::button_style) }; changed) {
        juce::DrawableButton::setButtonStyle(prop.isVoid() ? juce::DrawableButton::ButtonStyle::ImageRaw
                                                           : parseDrawableButtonStyleFromString(prop.toString()));
    }

    std::unique_ptr<juce::Drawable> normal{};
    std::unique_ptr<juce::Drawable> over{};
    std::unique_ptr<juce::Drawable> down{};
    std::unique_ptr<juce::Drawable> disabled{};
    std::unique_ptr<juce::Drawable> normalOn{};
    std::unique_ptr<juce::Drawable> overOn{};
    std::unique_ptr<juce::Drawable> downOn{};
    std::unique_ptr<juce::Drawable> disabledOn{};

    auto& loader{ context.getLoader() };

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::image_normal) }; changed)
        normal = loadDrawable(loader, prop.toString());

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::image_over) }; changed)
        over = loadDrawable(loader, prop.toString());

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::image_down) }; changed)
        down = loadDrawable(loader, prop.toString());

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::image_disabled) }; changed)
        disabled = loadDrawable(loader, prop.toString());

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::image_normal_on) }; changed)
        normalOn = loadDrawable(loader, prop.toString());

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::image_over_on) }; changed)
        overOn = loadDrawable(loader, prop.toString());

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::image_down_on) }; changed)
        downOn = loadDrawable(loader, prop.toString());

    if (auto&& [changed, prop]{getStylePropertyChanged(attr::css::image_disabled_on) }; changed)
        disabledOn = loadDrawable (loader, prop.toString());

    if (normal != nullptr) {
        juce::DrawableButton::setImages(normal.get(), over.get(), down.get(), disabled.get(),
                                        normalOn.get(), overOn.get(), downOn.get(), disabledOn.get());
    }
}

} // namespace vitro
