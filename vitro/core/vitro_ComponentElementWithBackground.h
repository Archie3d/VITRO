namespace vitro {

/** Component with a painted background.

    This is an helper class for components that can have have styled background.

    @see Panel, View

    @note Currently when background-image is set it will override the background-color property.

    Style attributes:
        background-color
        background-image
        background-image-tile
        border-color
        border-width
        border-radius
*/
class ComponentElementWithBackground : public vitro::ComponentElement
{
public:

    ComponentElementWithBackground(const juce::Identifier& tag, Context& ctx);

protected:

    void updateGradientToComponentSize();
    void paintBackground(juce::Graphics& g);

    // vitro::Element
    void update() override;

private:

    juce::Image backgroundImage{};
    bool backgroundImageTile{ false };

    std::optional<juce::Colour> backgroundColour{};

    std::optional<vitro::Gradient> gradient{};
    juce::ColourGradient colourGradient{};

    std::optional<juce::Colour> borderColour{};

    float borderRadius{ 0.0f };
    float borderWidth{ 1.0f };
};


} // namespace vitro
