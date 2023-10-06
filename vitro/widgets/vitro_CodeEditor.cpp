namespace vitro {

JSClassID CodeEditor::jsClassID = 0;

const Identifier CodeEditor::tag("CodeEditor");

//==============================================================================

struct JsTokeniserFunctions
{
    /** The token values returned by this tokeniser. */
    enum TokenType
    {
        tokenType_error = 0,
        tokenType_comment,
        tokenType_keyword,
        tokenType_operator,
        tokenType_identifier,
        tokenType_integer,
        tokenType_float,
        tokenType_string,
        tokenType_bracket,
        tokenType_punctuation
    };

    static bool isReservedKeyword(juce::String::CharPointerType token, const int tokenLength) noexcept
    {
        static const char* const keywords2Char[] =
            { "if", "do", "in", nullptr };

        static const char* const keywords3Char[] =
            { "var", "for", "new", "let", "try", nullptr };

        static const char* const keywords4Char[] =
            { "null", "true", "else", "case", "class", "this", nullptr };

        static const char* const keywords5Char[] =
            { "false", "until", "while", "break", "const", "catch", "throw", nullptr };

        static const char* const keywords6Char[] =
            { "return", "switch", "delete", "typeof", nullptr };

        static const char* const keywordsOther[] =
            { "function", nullptr };

        const char* const* k;

        switch (tokenLength)
        {
            case 2: k = keywords2Char; break;
            case 3: k = keywords3Char; break;
            case 4: k = keywords4Char; break;
            case 5: k = keywords5Char; break;
            case 6: k = keywords6Char; break;

            default:
                if (tokenLength < 2 || tokenLength > 16)
                    return false;

                k = keywordsOther;
                break;
        }

        for (int i = 0; k[i] != nullptr; ++i) {
            if (token.compare(CharPointer_ASCII(k[i])) == 0)
                return true;
        }

        return false;
    }

    static bool isIdentifierBody(const juce_wchar c) noexcept
    {
        return CharacterFunctions::isLetterOrDigit (c)
        || c == '_' || c == '@' || c == '$';
    }

    template <typename Iterator>
    static int parseIdentifier (Iterator& source) noexcept
    {
        int tokenLength = 0;
        String::CharPointerType::CharType possibleIdentifier[100];
        String::CharPointerType possible (possibleIdentifier);

        while (isIdentifierBody(source.peekNextChar())) {
            auto c = source.nextChar();

            if (tokenLength < 20)
                possible.write (c);

            ++tokenLength;
        }

        if (tokenLength > 1 && tokenLength <= 16) {
            possible.writeNull();

            if (isReservedKeyword (String::CharPointerType (possibleIdentifier), tokenLength))
                return tokenType_keyword;
        }

        return tokenType_identifier;
    }

    template <typename Iterator>
    static int readNextToken (Iterator& source)
    {
        source.skipWhitespace();

        auto firstChar = source.peekNextChar();

        switch (firstChar)
        {
            case 0: break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case '.':
            {
                auto result = CppTokeniserFunctions::parseNumber(source);

                if (result == tokenType_error) {
                    source.skip();

                    if (firstChar == '.')
                        return tokenType_punctuation;
                }

                return result;
            }

            case ',':
            case ';':
            case ':':
                source.skip();
                return tokenType_punctuation;

            case '(': case ')':
            case '{': case '}':
            case '[': case ']':
                source.skip();
                return tokenType_bracket;

            case '"':
            case '\'':
                CppTokeniserFunctions::skipQuotedString(source);
                return tokenType_string;

            case '+':
                source.skip();
                CppTokeniserFunctions::skipIfNextCharMatches(source, '+', '=');
                return tokenType_operator;

            case '-':
            {
                source.skip();
                auto result = CppTokeniserFunctions::parseNumber(source);

                if (result == tokenType_error) {
                    CppTokeniserFunctions::skipIfNextCharMatches(source, '-', '=');
                    return tokenType_operator;
                }

                return result;
            }

            case '/':
                source.skip();

                if (source.peekNextChar() == '/') {
                    source.skipToEndOfLine();
                    return tokenType_comment;
                }

                if (source.peekNextChar() == '*') {
                    CppTokeniserFunctions::skipComment(source);
                    return tokenType_comment;
                }

                CppTokeniserFunctions::skipIfNextCharMatches(source, '=');
                return tokenType_operator;

            case '*': case '%':
            case '=': case '!':
                source.skip();
                CppTokeniserFunctions::skipIfNextCharMatches(source, '=');
                return tokenType_operator;

            case '?':
            case '~':
                source.skip();
                return tokenType_operator;

            case '<': case '>':
            case '|': case '&': case '^':
                source.skip();
                CppTokeniserFunctions::skipIfNextCharMatches(source, firstChar);
                CppTokeniserFunctions::skipIfNextCharMatches(source, '=');
                return tokenType_operator;

            default:
                if (CppTokeniserFunctions::isIdentifierStart(firstChar) || firstChar == '$')
                    return parseIdentifier(source);

                source.skip();
                break;
        }

        return tokenType_error;
    }
};

class JsTokeniser : public juce::CodeTokeniser
{
public:
    JsTokeniser() {}
    ~JsTokeniser() = default;

    int readNextToken(CodeDocument::Iterator& source) override
    {
        return JsTokeniserFunctions::readNextToken(source);
    }

    CodeEditorComponent::ColourScheme getDefaultColourScheme() override
    {
        static const CodeEditorComponent::ColourScheme::TokenType types[] =
        {
            // Light colour scheme
            { "Error",          Colour (0xffcc0000) },
            { "Comment",        Colour (0xff3c3c3c) },
            { "Keyword",        Colour (0xff0000cc) },
            { "Operator",       Colour (0xff225500) },
            { "Identifier",     Colour (0xff000000) },
            { "Integer",        Colour (0xff003333) },
            { "Float",          Colour (0xff006633) },
            { "String",         Colour (0xffcc00cc) },
            { "Bracket",        Colour (0xff996600) },
            { "Punctuation",    Colour (0xff004400) }

            // These are tuned to a dark colour scheme
            /*
            { "Error",          Colour (0xffff0000) },
            { "Comment",        Colour (0xff546e6f) },
            { "Keyword",        Colour (0xffc792ea) },
            { "Operator",       Colour (0xff89ddff) },
            { "Identifier",     Colour (0xffeefff1) },
            { "Integer",        Colour (0xfff8c365) },
            { "Float",          Colour (0xfff78c6c) },
            { "String",         Colour (0xffc3e883) },
            { "Bracket",        Colour (0xff89ddff) },
            { "Punctuation",    Colour (0xff89ddff) }
            */
        };

        CodeEditorComponent::ColourScheme cs;

        for (auto& t : types)
            cs.set (t.name, Colour (t.colour));

        return cs;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JsTokeniser)
};

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
    else if (syn == "js")
        tokeniser = std::make_unique<JsTokeniser>();
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
    registerStyleProperty(attr::css::scrollbar_thickness);

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

    // scrollbar-thickness
    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::scrollbar_thickness) }; changed)
        editor.setScrollbarThickness(prop.isVoid() ? 8 : (int)prop);
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
