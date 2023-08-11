namespace vitro {

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void fillTextEditorBackground(juce::Graphics& g,
                                  int width, int height,
                                  juce::TextEditor& textEditor) override;

    void drawTextEditorOutline(juce::Graphics& g,
                               int width, int height,
                               juce::TextEditor& textEditor) override;
};

} // namespace vitro
