namespace vitro {

ComponentElementWithBackground::ComponentElementWithBackground(const Identifier& tag, Context& ctx)
    : ComponentElement(tag, ctx)
{
    registerStyleProperty(attr::css::background_color);
    registerStyleProperty(attr::css::background_image);
    registerStyleProperty(attr::css::border_color);
    registerStyleProperty(attr::css::border_radius);
    registerStyleProperty(attr::css::border_width);
}

void ComponentElementWithBackground::updateGradientToComponentSize()
{
    if (gradient) {
        if (auto* comp{ getComponent() }) {
            colourGradient = gradient->getColourGradient(comp->getWidth(), comp->getHeight());
        }
    }
}

void ComponentElementWithBackground::paintBackground(Graphics& g)
{
    auto* comp{ getComponent() };

    if (comp == nullptr)
        return;

    const auto width{ comp->getWidth() };
    const auto height{ comp->getHeight() };

    if (backgroundImage.isValid()) {
        if (borderRadius > 0) {
            g.saveState();
            Path path{};
            path.addRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, borderRadius);
            g.reduceClipRegion(path);
            g.drawImage(backgroundImage, 0, 0, width, height,
                                         0, 0, backgroundImage.getWidth(), backgroundImage.getHeight());
            g.restoreState();
        } else {
            g.drawImage(backgroundImage, 0, 0, width, height,
                                         0, 0, backgroundImage.getWidth(), backgroundImage.getHeight());
        }
    } else if (gradient) {
        g.setGradientFill(colourGradient);
    } else if (backgroundColour) {
        g.setColour(*backgroundColour);
    }

    if (borderRadius > 0.0f)
        g.fillRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, borderRadius);
    else
        g.fillRect(0, 0, width, height);

    if (borderColour && (!backgroundColour || *backgroundColour != *borderColour)) {
        g.setColour(*borderColour);

        if (borderRadius > 0.0f) {
            const float offset{ 0.5f * borderWidth };

            g.drawRoundedRectangle(offset, offset,
                float(width) - borderWidth, float(height) - borderWidth,
                borderRadius, borderWidth);
        } else {
            g.drawRect(comp->getLocalBounds(), (int)borderWidth);
        }
    }
}

void ComponentElementWithBackground::update()
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

                if (const auto* comp{ getComponent() }) {
                    colourGradient = gradient->getColourGradient(comp->getWidth(), comp->getHeight());
                }

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
