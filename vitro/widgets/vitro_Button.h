namespace vitro {

/** Generic button element.

    This class defines a generic button element and
    must be firther specialized.

    Attributes:
        text
        toggle
        radiogroup
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
        // @todo Evaluate attribute onclick
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
    void update() override;
};

} // namespace vitro
