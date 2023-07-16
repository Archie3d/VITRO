namespace vitro {

JSClassID Panel::jsClassID = 0;

const Identifier Panel::tag("Panel");

Panel::Panel(Context& ctx)
    : Panel(Panel::tag, ctx)
{
}

Panel::Panel(const Identifier& tag, Context& ctx)
    : ComponentElement(tag, ctx),
      juce::Component()
{
    registerStyleProperty(attr::css::background_color);
    registerStyleProperty(attr::css::background_image);
    registerStyleProperty(attr::css::border_color);
    registerStyleProperty(attr::css::border_radius);
    registerStyleProperty(attr::css::border_width);
}

void Panel::resized()
{
    if (gradient)
        colourGradient = gradient->getColourGradient(getWidth(), getHeight());
}

void Panel::paint(Graphics& g)
{
    if (backgroundImage.isValid()) {
        if (borderRadius > 0) {
            g.saveState();
            Path path{};
            path.addRoundedRectangle(0.0f, 0.0f, (float)getWidth(), (float)getHeight(), borderRadius);
            g.reduceClipRegion(path);
            g.drawImage(backgroundImage, 0, 0, getWidth(), getHeight(),
                                         0, 0, backgroundImage.getWidth(), backgroundImage.getHeight());
            g.restoreState();
        } else {
            g.drawImage(backgroundImage, 0, 0, getWidth(), getHeight(),
                                         0, 0, backgroundImage.getWidth(), backgroundImage.getHeight());
        }
    } else if (gradient) {
        g.setGradientFill(colourGradient);
    } else if (backgroundColour) {
        g.setColour(*backgroundColour);
    }

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

void Panel::update()
{
    ComponentElement::update();

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

    // background-image
    if (const auto&& [changed, prop]{ getStylePropertyChanged(attr::css::background_image) }; changed) {
            backgroundImage = prop.isVoid() ? juce::Image()
                                            : context.getLoader().loadImage(prop.toString());
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
