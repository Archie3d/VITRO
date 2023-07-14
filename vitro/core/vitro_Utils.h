namespace vitro {

/** Parse strig into a colour.

    The input string must be a known colour name or
    a hexadecimal colour representation started with the '#' symbol.
*/
juce::Colour parseColourFromString(const juce::String& str);

/** Parse string as a justification.

    This function understands the following inputs:
        left, right, top, bottom, center, justify
*/
juce::Justification parseJustificationFromString(const juce::String& str);

/** Parse string as a drawable button style.

    This function accepts the following strings:
        fitted, raw, default, above, background, original, stretched
*/
juce::DrawableButton::ButtonStyle parseDrawableButtonStyleFromString(const juce::String& str);

/** Parse string as slider's text box position.

    Acceptable values:
        none, left, right, above, below
*/
juce::Slider::TextEntryBoxPosition parseSliderTextBoxPositionFromString(const juce::String& str);

/** Linear or radial gradient.

    This class hold a gradient description and provides means
    to parse a gradience CSS string.
*/
class Gradient
{
public:
    struct ColourPoint
    {
        float distance;
        juce::Colour colour;
    };

    Gradient();

    juce::ColourGradient getColourGradient(int width, int height) const;

    bool isRadial() const  { return radial; }
    void setRadial(bool shouldBeRadial) { radial = shouldBeRadial; }

    float getAngle() const { return angle; }
    void setAngle(float a) { angle = a;}

    void addColourPoint(float pointDistance, const juce::Colour& pointColour);
    int getColourPointsCount() const { return static_cast<int> (colourPoints.size()); }
    const ColourPoint& getColourPoint (int index) const;

    static bool isPotentiallyGradientString (const juce::String& str);
    static Gradient fromString(const juce::String& str);

private:
    float angle{ 0.0f };
    bool radial{ false };

    std::vector<ColourPoint> colourPoints{};
};

} // namespace vitro
