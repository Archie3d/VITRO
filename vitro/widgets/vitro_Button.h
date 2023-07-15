namespace vitro {

/** Generic button element.

    This class defines a generic button element and
    must be firther specialized.

    Attributes:
        text
        toggle
        radiogroup

    Callback attributes:
        onclick
*/
template <class ButtonClass>
class Button : public vitro::ComponentElement,
               public ButtonClass,
               private juce::Button::Listener
{
public:

    Button(const juce::Identifier& tag, Context& ctx)
        : ComponentElement(tag, ctx),
          ButtonClass("")
    {
        ButtonClass::addListener(this);
    }

    juce::Component* getComponent() override { return this; }

    // vitro::Element
    void update() override
    {
        ComponentElement::update();

        if (auto&& [changed, val]{ getAttributeChanged(attr::text) }; changed)
            ButtonClass::setButtonText(val.toString());

        if (auto&& [changed, val]{ getAttributeChanged(attr::toggle) }; changed)
            ButtonClass::setToggleState(val, juce::dontSendNotification);

        if (auto&& [changed, val]{ getAttributeChanged(attr::radiogroup) }; changed)
            ButtonClass::setRadioGroupId(val);
    }

    // juce::Button::Listener
    void buttonClicked(juce::Button*) override
    {
        evaluateAttributeScript(attr::onclick);
    }

    void buttonStateChanged(juce::Button*) override
    {
        setAttribute(attr::active, ButtonClass::isDown());
        setAttribute(attr::toggle, ButtonClass::getToggleStateValue());
    }
};

//==============================================================================

/** Text button.

    This element is a text button, which is equivalent to juce::TextButton.

    Style properties:
        toggle
        trigger-down
        text-color-on
        text-color-off
        background-color
        background-color-on
        border-color
*/
class TextButton : public vitro::Button<juce::TextButton>
{
public:
    using ButtonBase = vitro::Button<juce::TextButton>;

    const static juce::Identifier tag;  // <TextButton>

    static JSClassID jsClassID;

    TextButton(Context& ctx);

    // juce::Element
    JSClassID getJSClassID() const override { return vitro::TextButton::jsClassID; }

    void update() override;
};

//==============================================================================

/** Toggle button.

    This element is a toggle button, which is equivalent to juce::ToggleButton.

    Style properties:
        color
        tick-color
        tick-disabled-color
*/

class ToggleButton : public vitro::Button<juce::ToggleButton>
{
public:
    using ButtonBase = vitro::Button<juce::ToggleButton>;

    const static juce::Identifier tag;  // <ToggleButton>

    static JSClassID jsClassID;

    ToggleButton(Context& ctx);

    // juce::Element
    JSClassID getJSClassID() const override { return vitro::ToggleButton::jsClassID; }

    void update() override;
};

//==============================================================================

// @internal
// Shim class to align DrawableButton constructor with other Button types.
class JuceDrawableButton : public juce::DrawableButton
{
public:
    JuceDrawableButton(const juce::String& buttonName)
        : juce::DrawableButton(buttonName, juce::DrawableButton::ButtonStyle::ImageRaw)
    {}
};

/** Drawable button.

    This element is a drawable button defined by a set of SVG images.
    It is equivalent to juce::DrawableButton.

    Style properties:
        color
        text-color-on
        background-color
        background-color-on
        button-style
        image-normal
        image-over
        image-down
        image-disabled
        image-normal-on
        image-over-on
        image-down-on
        image-disabled-on
*/
class DrawableButton : public vitro::Button<JuceDrawableButton>
{
public:
    using ButtonBase = vitro::Button<JuceDrawableButton>;

    const static juce::Identifier tag;  // <DrawableButton>

    static JSClassID jsClassID;

    DrawableButton(Context& ctx);

    // juce::Element
    JSClassID getJSClassID() const override { return vitro::DrawableButton::jsClassID; }

    void update() override;

private:
    static std::unique_ptr<juce::Drawable> loadDrawable(Loader& loader, const juce::String& location);
};

} // namespace vitro
