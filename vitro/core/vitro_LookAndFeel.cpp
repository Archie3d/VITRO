namespace vitro {

LookAndFeel::LookAndFeel()
{
}

void LookAndFeel::drawButtonBackground(Graphics& g,
                                       juce::Button& button,
                                       const Colour& backgroundColour,
                                       bool shouldDrawButtonAsHighlighted,
                                       bool shouldDrawButtonAsDown)
{
    float cornerSize{ vitro::TextButton::defaultBorderRadius };
    float borderWidth{ vitro::TextButton::defaultBorderWidth };

    bool hasGradientBackground{ false };
    juce::ColourGradient gradient{};

    if (auto* element{ dynamic_cast<vitro::TextButton*>(&button) }) {
        cornerSize = element->getBorderRadius();
        borderWidth = element->getBorderWidth();

        hasGradientBackground = element->hasGradientBackground();

        if (hasGradientBackground)
            gradient = element->getBackgroundColourGradient();
    }

    // The following is copied from juce::LookAndFeel_V4
    const auto bounds{ button.getLocalBounds().toFloat().reduced(0.5f, 0.5f) };

    auto baseColour{ backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
                                     .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f)} ;

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

    if (hasGradientBackground)
        g.setGradientFill(gradient);
    else
        g.setColour(baseColour);

    const auto flatOnLeft  { button.isConnectedOnLeft() };
    const auto flatOnRight { button.isConnectedOnRight() };
    const auto flatOnTop   { button.isConnectedOnTop() };
    const auto flatOnBottom{ button.isConnectedOnBottom() };

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom) {
        Path path{};
        path.addRoundedRectangle(bounds.getX(), bounds.getY(),
                                 bounds.getWidth(), bounds.getHeight(),
                                 cornerSize, cornerSize,
                                 !(flatOnLeft  || flatOnTop),
                                 !(flatOnRight || flatOnTop),
                                 !(flatOnLeft  || flatOnBottom),
                                 !(flatOnRight || flatOnBottom));

        g.fillPath(path);

        g.setColour(button.findColour(ComboBox::outlineColourId));
        g.strokePath(path, PathStrokeType(borderWidth));
    
    } else {
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(button.findColour(ComboBox::outlineColourId));
        g.drawRoundedRectangle(bounds, cornerSize, borderWidth);
    }
}

} // namespace vitro
