namespace vitro {

Colour parseColourFromString(const String& str) {
    if (str.isEmpty())
        return {};

    auto namedColour{ Colours::findColourForName(str, Colour (0x00000000)) };

    if (namedColour.getARGB() != 0)
        return namedColour;

    if (str.startsWithChar('#'))
    {
        auto hexStr{ str.substring(1) };

        if (hexStr.length() == 6)
            hexStr = "ff" + hexStr;

        return Colour::fromString(hexStr);
    }

    return {};
}

//==============================================================================

Justification parseJustificationFromString(const String& str)
{
    const static std::map<String, Justification::Flags> justMap {
        { "left",    Justification::left },
        { "right",   Justification::right },
        { "top",     Justification::top },
        { "bottom",  Justification::bottom },
        { "center",  Justification::centred },
        { "justify", Justification::horizontallyJustified }
    };

    const auto it = justMap.find(str.trim().toLowerCase());

    if (it == justMap.end())
        return Justification(Justification::left);

    return Justification(it->second);
}

//==============================================================================

juce::DrawableButton::ButtonStyle parseDrawableButtonStyle(const String& str)
{
    const static std::map<String, juce::DrawableButton::ButtonStyle> styleMap {
        { "fitted",     juce::DrawableButton::ButtonStyle::ImageFitted },
        { "raw",        juce::DrawableButton::ButtonStyle::ImageRaw},
        { "default",    juce::DrawableButton::ButtonStyle::ImageRaw},
        { "above",      juce::DrawableButton::ButtonStyle::ImageAboveTextLabel },
        { "background", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground },
        { "original",   juce::DrawableButton::ButtonStyle::ImageOnButtonBackgroundOriginalSize },
        { "stretched",  juce::DrawableButton::ButtonStyle::ImageStretched }
    };

    const auto it = styleMap.find(str.trim().toLowerCase());

    if (it == styleMap.end())
        return juce::DrawableButton::ButtonStyle::ImageRaw;

    return it->second;
}

//==============================================================================

class GradientParser
{
public:
    GradientParser(Gradient& gradientToPopulate, const String& stringToParse)
        : gradient{ gradientToPopulate }
        , str{ stringToParse }
        , strLength{ str.length() }
        , pos{ 0 }
    {
        parse();
    }

    void parse()
    {
        if (expect("radial-gradient"))
            gradient.setRadial (true);
        else if (!expect("linear-gradient"))
            return;

        skipSpaces();

        if (!expect("("))
            return;

        skipSpaces();

        // Directional angle followed by "mandatory "deg" unit.
        const int angle{ parseInteger() };

        skipSpaces();

        if (!expect("deg"))
            return;

        gradient.setAngle(juce::MathConstants<float>::pi * static_cast<float> (angle) / 180.0f);

        skipSpaces();

        // Colour point pairs: colour distance%
        while (pos < strLength) {
            if (!expect (","))
                break;

            if (pos >= strLength)
                return;

            const auto colour{ parseColour() };
            skipSpaces();
            const int distance{ parseInteger() };

            if (!expect("%"))
                break;

            gradient.addColourPoint(distance * 0.01f, colour);

            skipSpaces();

            if (expect(")"))
                break;
        }
    }

private:

    void skipSpaces()
    {
        while (pos < strLength && (str[pos] == ' ' || str[pos] == '\t'))
            ++pos;
    }

    bool expect(const String& expectedString)
    {
        const auto expectedLength{ expectedString.length() };

        if (strLength - pos < expectedLength)
            return false;

        if (const auto actualString{ str.substring (pos, pos + expectedLength) }; actualString == expectedString) {
            pos += expectedLength;
            return true;
        }

        return false;
    }

    int parseInteger()
    {
        if (pos >= strLength)
            return 0;

        int sign{ 1 };
        int value{ 0 };

        if (str[pos] == '-') {
            sign = -1;
            ++pos;
        }

        while (pos < strLength) {
            const auto c {str[pos]};

            if (c < '0' || c >'9')
                break;

            const int digit{ static_cast<int>(c - '0') };
            value = value * 10 + digit;

            ++pos;
        }

        return value * sign;
    }

    Colour parseColour()
    {
        String colourStr;

        skipSpaces();

        while (pos < strLength) {
            const auto c{ str[pos] };

            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '#')) {
                colourStr += c;
                ++pos;
            } else {
                break;
            }
        }

        return parseColourFromString(colourStr);
    }

    Gradient& gradient;
    const String& str;
    const int strLength;
    int pos;
};

//==============================================================================

Gradient::Gradient()
{
}

template <typename T>
static void clampToWidthHeight(juce::Point<T>& point, T width, T height)
{
    point.x = jlimit(0.0f, width, point.x);
    point.y = jlimit(0.0f, height, point.y);
}

ColourGradient Gradient::getColourGradient(int width, int height) const
{
    const float fWidth{ static_cast<float>(jmax (1, width)) };
    const float fHeight{ static_cast<float> (jmax (1, height)) };

    juce::Point<float> centre(0.5f * fWidth, 0.5f * fHeight);
    float d { 0.5f * sqrt(fWidth * fWidth + fHeight * fHeight) };

    // Directional vector
    const juce::Point<float> direction(sinf (angle), cosf (angle));
    juce::Point<float> fromPoint(centre.x - d * direction.x, centre.y + d * direction.y);
    juce::Point<float> toPoint(centre.x + d * direction.x, centre.y - d * direction.y);

    clampToWidthHeight(fromPoint, fWidth, fHeight);
    clampToWidthHeight(toPoint, fWidth, fHeight);

    const float dx{ toPoint.x - fromPoint.x };
    const float dy{ toPoint.y - fromPoint.y };

    // Update the actual length of the gradient
    d = sqrt(dx * dx + dy * dy);

    juce::Point<float> p1;
    juce::Point<float> p2;
    juce::Colour colour1;
    juce::Colour colour2;

    if (colourPoints.size() > 0) {
        const auto& point1{ colourPoints.front() };
        colour1 = point1.colour;
        p1.x = fromPoint.x + point1.distance * d * direction.x;
        p1.y = fromPoint.y - point1.distance * d * direction.y;

        const auto& point2{ colourPoints.back() };
        colour2 = point2.colour;
        p2.x = fromPoint.x + point2.distance * d * direction.x;
        p2.y = fromPoint.y - point2.distance * d * direction.y;
    }

    juce::ColourGradient gradient(colour1, p1, colour2, p2, radial);

    if (colourPoints.size() > 2) {
        for (size_t i = 1; i < colourPoints.size() - 1; ++i) {
            const auto& point{ colourPoints.at (i) };
            gradient.addColour(point.distance, point.colour);
        }
    }

    return gradient;
}

void Gradient::addColourPoint(float pointDistance, const juce::Colour& pointColour)
{
    colourPoints.push_back({ pointDistance, pointColour });
}

const Gradient::ColourPoint& Gradient::getColourPoint(int index) const
{
    jassert(isPositiveAndBelow(index, colourPoints.size()));
    return colourPoints.at(static_cast<size_t>(index));
}

bool Gradient::isPotentiallyGradientString(const juce::String& str)
{
    return str.startsWith("linear-gradient") || str.startsWith("radial-gradient");
}

Gradient Gradient::fromString(const String& str)
{
    Gradient gradient;
    GradientParser parser(gradient, str);

    return gradient;
}

} // namespace vitro
