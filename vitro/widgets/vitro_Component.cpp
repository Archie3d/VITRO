namespace vitro {

JSClassID Component::jsClassID = 0;

const Identifier Component::tag("Component");

Component::Component(Context& ctx)
    : ComponentElement(Component::tag, ctx),
      juce::Component()
{
    registerStyleProperty(attr::css::background_color);
    registerStyleProperty(attr::css::border_color);
    registerStyleProperty(attr::css::border_radius);
    registerStyleProperty(attr::css::border_width);
}

void Component::resized()
{
    if (gradient)
        colourGradient = gradient->getColourGradient(getWidth(), getHeight());
}

void Component::paint(Graphics& g)
{
    if (gradient)
        g.setGradientFill(colourGradient);
    else if (backgroundColour)
        g.setColour(*backgroundColour);


    if (borderRadius > 0.0f)
        g.fillRoundedRectangle(0.0f, 0.0f, (float)getWidth(), (float)getHeight(), borderRadius);
    else
        g.fillRect(0, 0, getWidth(), getHeight());

    if (borderColour && (!backgroundColour || *backgroundColour != *borderColour)) {
        g.setColour(*borderColour);

        if (borderRadius > 0.0f) {
            const float offset{ 0.5f * borderWidth };

            g.drawRoundedRectangle(offset, offset,
                float(getWidth()) - borderWidth, float(getHeight()) - borderWidth,
                borderRadius, borderWidth);
        } else {
            g.drawRect(getLocalBounds(), (int)borderWidth);
        }
    }
}

void Component::update()
{
    // background-color
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::background_color) }; changed) {
        if (prop.isVoid()) {
            // Background colour has been removed
            backgroundColour.reset();
            gradient.reset();
        } else {
            const String str{ prop.toString() };

            if (Gradient::isPotentiallyGradientString(str)) {
                gradient = Gradient::fromString(str);
                colourGradient = gradient->getColourGradient(getWidth(), getHeight());

                backgroundColour.reset();
            } else {
                backgroundColour = parseColourFromString(str);
                gradient.reset();
            }
        }
    }

    // border-color
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::border_color) }; changed) {
        if (prop.isVoid())
            borderColour.reset();
        else
            borderColour = parseColourFromString(prop.toString());
    }

    // border-radius
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::border_radius) }; changed)
        borderRadius = prop;

    // border-width
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::border_width) }; changed)
        borderWidth = prop;
}

} // namespace vitro
