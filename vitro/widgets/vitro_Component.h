namespace vitro {

class Component : public vitro::ComponentElement,
                  public juce::Component
{
public:
    Component(const juce::Identifier& tag, Context& ctx);

    juce::Component* getComponent() override { return this; }

    // juce::Component
    void resized() override;
    void paint(juce::Graphics& g) override;

protected:

    // vitro::Element
    void update() override;

private:

    std::optional<juce::Colour> backgroundColour{};

    std::optional<vitro::Gradient> gradient{};
    juce::ColourGradient colourGradient{};

    std::optional<juce::Colour> borderColour{};

    float borderRadius{ 0.0f };
    float borderWidth{ 1.0f };
};

} // namespace vitro
