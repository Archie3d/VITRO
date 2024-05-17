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

/** Convert Mouse event to var object.

    This function is used to pass mouse events to JS context.
*/
juce::var mouseEventToVar(const juce::MouseEvent& event);

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

//==============================================================================

namespace js {

/** Convert JSValue to juce::var */
juce::var JSValueToVar(JSContext* ctx, JSValueConst val);

/** Convert juce::var to JSValue */
JSValue varToJSValue(JSContext* ctx, const juce::var& val);

/** JavaScript function wrapper.

    This is a JS function wrapper so that functions can be
    stored in juce::var (and hence in Element attributes ValueTree).
*/
class Function final : public juce::ReferenceCountedObject
{
public:
    Function(JSContext* ctx, JSValueConst val);
    ~Function();

    JSValue getJSValue() { return jsFunc; }

    /** Call this function with a list of juce::var arguments. */
    template <typename ...Ts>
    juce::var call(Ts&&... args)
    {
        std::vector<JSValue> jsArgs;

        ([this, &jsArgs](auto& arg) {
            juce::var v(arg);
            jsArgs.push_back(varToJSValue(jsCtx, v));
        } (args), ...);

        auto ret{ JS_Call(jsCtx, jsFunc, JS_NULL, int(jsArgs.size()), jsArgs.data()) };
        juce::var retVar{ JSValueToVar(jsCtx, ret) };
        JS_FreeValue(jsCtx, ret);

        for (auto& jsArg : jsArgs)
            JS_FreeValue(jsCtx, jsArg);

        return retVar;
    }

    /** Call this function for given 'this' object. */
    template <typename ...Ts>
    juce::var callThis(JSValue thisObj, Ts&&... args)
    {
        std::vector<JSValue> jsArgs;

        ([this, &jsArgs](auto& arg) {
            juce::var v(arg);
            jsArgs.push_back(varToJSValue(jsCtx, v));
        } (args), ...);

        auto ret{ JS_Call(jsCtx, jsFunc, thisObj, int(jsArgs.size()), jsArgs.data()) };
        juce::var retVar{ JSValueToVar(jsCtx, ret) };
        JS_FreeValue(jsCtx, ret);

        for (auto& jsArg : jsArgs)
            JS_FreeValue(jsCtx, jsArg);

        return retVar;
    }

private:
    JSContext* jsCtx{};
    JSValue jsFunc{ JS_UNDEFINED };
};

} // namespace js

} // namespace vitro
