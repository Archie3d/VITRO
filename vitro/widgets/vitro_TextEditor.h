namespace vitro {

/** Text editing widget.

    This UI element is equivalent to juce::TextEditor

    Attributes:
        text
        emptytext
        readonly

    Style properties:
        multiline
        wrap
        password-character
        text-color
        empty-text-color
        background-color
        highlight-color
        highlight-text-color
        border-color
        border-radius
        border-width
        focused-border-color
        shadow-color
*/
class TextEditor : public vitro::ComponentElement,
                   public juce::TextEditor,
                   private juce::TextEditor::Listener
{
public:

    const static juce::Identifier tag;  // <TextEditor>

    static JSClassID jsClassID;

    TextEditor(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::TextEditor::jsClassID; }

    juce::Component* getComponent() override { return this; }

    float getBorderRadius() const { return borderRadius; }
    float getBorderWidth() const { return borderWidth; }

    static constexpr float defaultBorderRadius = 0.0f;
    static constexpr float defaultBorderWidth = 2.0f;

protected:

    // vitro::Element
    void update() override;

    // juce::TextEditor::Listener
    void textEditorTextChanged(juce::TextEditor&) override;
    void textEditorReturnKeyPressed(juce::TextEditor&) override;

    juce::Colour emptyTextColour{};

    float borderRadius{ defaultBorderRadius };
    float borderWidth{ defaultBorderWidth };
};

} // namespace vitro
