namespace vitro {

/** Code editing widget.

    This UI element wraps both juce::CodeEditorComponent and juce::CodeDocument.

    Attributes:
        text
        readonly

    Style properties:
        color
        background-color
        highlight-color
        line-number-color
        line-number-background-color
        syntax-error-color
        syntax-comment-color
        syntax-keyword-color
        syntax-operator-color
        syntax-identifier-color
        syntax-integer-color
        syntax-float-color
        syntax-string-color
        syntax-bracket-color
        syntax-punctuation-color
*/
class CodeEditor : public vitro::ComponentElement,
                   public juce::Component,
                   private juce::CodeDocument::Listener
{
public:

    const static juce::Identifier tag;  // <Label>

    static JSClassID jsClassID;

    CodeEditor(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::CodeEditor::jsClassID; }

    juce::Component* getComponent() override { return this; }

    // juce::Component
    void resized() override;

    juce::CodeDocument& getDocument() { return document; }
    const juce::CodeDocument& getDocument() const { return document; }

protected:

    // vitro::Element
    void update() override;

private:

    /** Helper class that allows swithing code syntax without recreating the editor. */
    class Tokeniser final : public juce::CodeTokeniser
    {
    public:
        Tokeniser(CodeEditor& elem);

        void setSyntax(const juce::String& syntax);

        int readNextToken(juce::CodeDocument::Iterator& source) override;
        juce::CodeEditorComponent::ColourScheme getDefaultColourScheme() override;

        const static std::map<juce::Identifier, juce::String> cssToSyntaxMap;

    private:
        CodeEditor& codeEditor;
        std::unique_ptr<juce::CodeTokeniser> tokeniser{};
    };

    // Copy content of the document to "text" attribute
    void copyDocumentToTextAttribute();

    // juce::CodeDocument::Listener
    void codeDocumentTextInserted(const juce::String& newText, int insertIndex) override;
    void codeDocumentTextDeleted(int startIndex, int endIndex) override;


    juce::CodeDocument document;
    Tokeniser tokeniser;
    juce::CodeEditorComponent editor;
};

} // namespace vitro
