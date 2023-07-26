namespace vitro {

JSClassID CodeEditor::jsClassID = 0;

const Identifier CodeEditor::tag("CodeEditor");

//==============================================================================

// Mapping CSS properties to the syntax highlighter colour keys.
const std::map<juce::Identifier, juce::String> CodeEditor::Tokeniser::cssToSyntaxMap {
    { Identifier("syntax-error-color"),       "Error" },
    { Identifier("syntax-comment-color"),     "Comment" },
    { Identifier("syntax-keyword-color"),     "Keyword" },
    { Identifier("syntax-operator-color"),    "Operator" },
    { Identifier("syntax-identifier-color"),  "Identifier" },
    { Identifier("syntax-integer-color"),     "Integer" },
    { Identifier("syntax-float-color"),       "Float" },
    { Identifier("syntax-string-color"),      "String" },
    { Identifier("syntax-bracket-color"),     "Bracket" },
    { Identifier("syntax-punctuation-color"), "Punctuation" }
};

CodeEditor::Tokeniser::Tokeniser(CodeEditor& elem)
    : codeEditor{ elem }
{
}

void CodeEditor::Tokeniser::setSyntax(const String& syntax)
{
    const auto syn{ syntax.trim().toLowerCase() };

    if (syn == "xml")
        tokeniser = std::make_unique<juce::XmlTokeniser>();
    else if (syn == "lua")
        tokeniser = std::make_unique<juce::LuaTokeniser>();
    else if (syn == "cpp")
        tokeniser = std::make_unique<juce::CPlusPlusCodeTokeniser>();
    else
        tokeniser.reset();
}

int CodeEditor::Tokeniser::readNextToken(juce::CodeDocument::Iterator& source)
{
    if (tokeniser != nullptr)
        return tokeniser->readNextToken(source);

    source.skipToEndOfLine();

    return 0;
}

juce::CodeEditorComponent::ColourScheme CodeEditor::Tokeniser::getDefaultColourScheme()
{
    const static juce::CodeEditorComponent::ColourScheme defaultColourScheme{};

    auto colourScheme{ tokeniser == nullptr ? defaultColourScheme : tokeniser->getDefaultColourScheme() };

    // Copy syntax colours from CSS style properties
    for (auto it = std::cbegin(cssToSyntaxMap); it != std::cend(cssToSyntaxMap); ++it) {
        if (auto prop{ codeEditor.getStyleProperty(it->first) }; !prop.isVoid())
            colourScheme.set(it->second, parseColourFromString(prop.toString()));
    }

    return colourScheme;
}

//==============================================================================

CodeEditor::CodeEditor(Context& ctx)
    : ComponentElement(CodeEditor::tag, ctx),
      document{},
      tokeniser(*this),
      editor(document, &tokeniser)
{
    registerStyleProperty(attr::css::color);
    registerStyleProperty(attr::css::background_color);
    registerStyleProperty(attr::css::highlight_color);
    registerStyleProperty(attr::css::line_number_color);
    registerStyleProperty(attr::css::line_number_background_color);

    for (auto it = std::cbegin(Tokeniser::cssToSyntaxMap); it != std::cend(Tokeniser::cssToSyntaxMap); ++it)
        registerStyleProperty(it->first);

    document.addListener(this);
    addAndMakeVisible(editor);
}

void CodeEditor::resized()
{
    editor.setBounds(getLocalBounds());
}

void CodeEditor::update()
{
    ComponentElement::update();

    if (auto&& [changed, val]{ getAttributeChanged(attr::readonly) }; changed)
        editor.setReadOnly(bool(val));

    if (auto&& [changed, val]{ getAttributeChanged(attr::syntax) }; changed) {
        tokeniser.setSyntax(val.toString());
        editor.setColourScheme(tokeniser.getDefaultColourScheme());
    }

    if (auto&& [changed, val]{ getAttributeChanged(attr::text) }; changed)
        document.replaceAllContent(val.toString());

    setColourFromStyleProperty(editor, CodeEditorComponent::defaultTextColourId,    attr::css::color);
    setColourFromStyleProperty(editor, CodeEditorComponent::backgroundColourId,     attr::css::background_color);
    setColourFromStyleProperty(editor, CodeEditorComponent::highlightColourId,      attr::css::highlight_color);
    setColourFromStyleProperty(editor, CodeEditorComponent::lineNumberTextId,       attr::css::line_number_color);
    setColourFromStyleProperty(editor, CodeEditorComponent::lineNumberBackgroundId, attr::css::line_number_background_color);
}

void CodeEditor::copyDocumentToTextAttribute()
{
    setAttribute(attr::text, document.getAllContent(), false);
    evaluateAttributeScript(attr::onchange);
}

void CodeEditor::codeDocumentTextInserted(const String&, int)
{
    copyDocumentToTextAttribute();
}

void CodeEditor::codeDocumentTextDeleted(int, int)
{
    copyDocumentToTextAttribute();
}

} // namespace vitro
