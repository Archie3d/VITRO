namespace vitro {

template <typename T>
static bool isWhiteChar(const T t)
{
    return t == ' ' || t == '\t' || t == '\r' || t == '\n';
}

template <typename T>
static bool isAlphaOrUnderscore(const T t)
{
    return (t >= 'a' && t <= 'z') || (t >= 'A' && t <= 'Z') || t == '_';
}

template <typename T>
static bool isAlphaOrUnderscoreOrHyphen(const T t)
{
    return isAlphaOrUnderscore(t) || t == '-';
}

template <typename T>
static bool isDigit(const T t)
{
    return t >= '0' && t <= '9';
}

//==============================================================================

/** @internal

    Parsing context.
    Implements parsing of CSS syntax elements.
*/
struct CSSParser::Context
{
    Stylesheet& stylesheet;
    String source;
    int length;
    int pos;
    int col;

    Context(Stylesheet& ss, const String& src)
        : stylesheet{ ss },
          source{ src },
          length{ source.length() },
          pos{ 0 }
    {
    }

    bool isOver() const { return pos >= length; }

    void skipSpaces()
    {
        for (; pos < length && isWhiteChar(source[pos]); ++pos);
    }

    void skipToNextLine()
    {
        for (; pos < length && source[pos] != '\n' && source[pos] != '\r'; ++pos);

        if (isOver()) return;

        const auto first{ source[pos++] };

        if (isOver()) return;

        // Handle \r\n or \n\r sequences
        if ((first == '\r' && source[pos] == '\n') || (first == '\n' && source[pos] == '\r'))
            ++pos;
    }

    bool skipInlineComment()
    {
        skipSpaces();

        if (pos < length - 2 && source[pos] == '/' && source[pos + 1] == '/') {
            skipToNextLine();
            return true;
        }

        return false;
    }

    bool skipBlockComment()
    {
        skipSpaces();

        if (pos < length - 2 && source[pos] == '/' && source[pos + 1] == '*') {
            pos += 2;

            for (; pos < length - 2; ++pos) {
                if (source[pos] == '*' && source[pos + 1] == '/') {
                    pos += 2;
                    break;
                }
            }

            return true;
        }

        return false;
    }

    void skipSpacesAndComments()
    {
        while (pos < length && (skipInlineComment() || skipBlockComment()));
    }

    bool expect(const String& atom)
    {
        const auto nextPos{ pos + atom.length() };

        if (nextPos > length)
            return false;

        if (source.substring(pos, nextPos) != atom)
            return false;

        if (nextPos == length) {
            pos = nextPos;
            return true;
        }

        if (!isWhiteChar(source[nextPos]))
            return false;

        pos = nextPos;

        return true;
    }

    bool parseIdentifier(String& name)
    {
        skipSpacesAndComments();

        if (isOver())
            return false;

        if (!isAlphaOrUnderscore(source[pos]))
            return false;

        const auto startPos{ pos };

        ++pos;

        for (; pos < length && (isAlphaOrUnderscoreOrHyphen(source[pos]) || isDigit(source[pos])); ++pos);

        name = source.substring(startPos, pos);

        return true;
    }

    bool parsePropertyName(String& name)
    {
        skipSpacesAndComments();

        if (isOver())
            return false;

        if (!isAlphaOrUnderscoreOrHyphen(source[pos]))
            return false;

        const auto startPos {pos};

        ++pos;

        for (; pos < length && (isAlphaOrUnderscoreOrHyphen(source[pos]) || isDigit(source[pos])); ++pos);

        name = source.substring(startPos, pos);

        return true;
    }

    bool parseStringValue(String& value)
    {
        skipSpacesAndComments();

        if (isOver())
            return false;

        const auto quote {source[pos]};

        if (quote != '"' && quote != '\'')
            return false;

        ++pos;
        value.clear();

        for (; pos < length && source[pos] != quote; value += source[pos++]);

        if (source[pos] != quote)
            return false;

        ++pos;

        return true;
    }

    bool parseSelectorTagClassId(String& tag, String& clazz, String& id)
    {
        skipSpacesAndComments();

        if (isOver())
            return false;

        // tag
        if (isAlphaOrUnderscore(source[pos])) {
            if (!parseIdentifier(tag))
                return false;
        }

        // .class
        if (source[pos] == '.') {
            ++pos;

            if (!parseIdentifier(clazz))
                return false;
        }

        // #id
        if (source[pos] == '#') {
            ++pos;

            if (!parseIdentifier(id))
                return false;
        }

        return true;
    }

    bool parseAttribute(String& attr, css::Selector::Attribute::Operator& op, String& value)
    {
        skipSpacesAndComments();

        attr.clear();
        op = css::Selector::Attribute::Operator::None;
        value.clear();

        if (isOver())
            return false;

        const auto openingChar{ source[pos] };

        if (openingChar != '[' && openingChar != ':')
            return false;

        ++pos;

        if (!parseIdentifier(attr))
            return false;

        if (openingChar == '[') {
            // [attr <op> "value"]
            if (!parseAttributeOperator(op))
                return false;

            if (op != css::Selector::Attribute::Operator::None) {
                if (!parseStringValue(value))
                    return false;
            }

            skipSpacesAndComments();

            if (isOver())
                return false;

            if (source[pos] != ']')
                return false;

            ++pos;

        } else {
            // :attr is equivalent to [attr="true"]
            op = css::Selector::Attribute::Operator::IsTrue;
            value = "true";
        }

        return true;
    }

    bool parseAttributeOperator(css::Selector::Attribute::Operator& op)
    {
        skipBlockComment();

        op = css::Selector::Attribute::Operator::None;

        if (isOver())
            return false;

        String opStr;

        auto c{ source[pos] };

        while (pos < length && (c == '=' || c == '*' || c == '^' || c == '|' || c == '$')) {
            opStr += c;
            c = source[++pos];
        }

        if (opStr.isEmpty())
            op = css::Selector::Attribute::Operator::Defined;
        else if (opStr == "=")
            op = css::Selector::Attribute::Operator::Equals;
        else if (opStr == "*=")
            op = css::Selector::Attribute::Operator::Contains;
        else if (opStr == "|=")
            op = css::Selector::Attribute::Operator::Prefix;
        else if (opStr == "^=")
            op = css::Selector::Attribute::Operator::Begins;
        else if (opStr == "$=")
            op = css::Selector::Attribute::Operator::Ends;
        else
            return false;

        return true;
    }

    bool parseProperty(String& name, String& value)
    {
        skipSpacesAndComments();

        if (!parsePropertyName(name))
            return false;

        skipSpacesAndComments();

        if (isOver())
            return false;

        if (source[pos] != ':')
            return false;

        ++pos;
        skipSpacesAndComments();

        if (isOver())
            return false;

        bool endOfLine{ false };

        value.clear();

        bool quoted{ source[pos] == '"' || source[pos] == '\'' };
        const auto quote{ source[pos] };

        if (quoted) {
            ++pos;

            while (!isOver()) {
                const auto c{ source[pos++] };

                if (c == quote)
                    break;
                else if (c == '\r' || c == '\n')
                    return false;
                else
                    value += c;
            }

            skipSpaces();

        } else {

            while ((!isOver()) && (!endOfLine)) {
                auto c { source[pos] };

                if (c == ';')
                    break;

                if (c == '\r' || c == '\n')
                    return false;

                value += c;
                ++pos;
            }
        }

        if (isOver())
            return false;

        if (source[pos] != ';')
            return false;

        ++pos;

        return true;
    }

    bool parseImport(String& importName)
    {
        skipSpacesAndComments();

        if (!expect("@import"))
            return false;

        if (! parseStringValue(importName))
            return false;

        skipSpacesAndComments();

        if (source[pos] != ';')
            return false;

        return true;
    }

    bool parseMacroDefinition(String& name, String& value)
    {
        skipSpacesAndComments();

        if (source[pos] != '$')
            return false;

        ++pos;

        return parseProperty(name, value);
    }

};

//==============================================================================

static String substituteMacros(const String& value, const juce::NamedValueSet& macros)
{
    if (value.isEmpty())
        return value;

    String result{ value };
    bool keepGoing{ true };
    int recursionLevel{ 5 };

    while (keepGoing && recursionLevel > 0) {
        keepGoing = false;
        --recursionLevel;

        const int macroBegin{ result.indexOfChar('$') };

        if (macroBegin >= 0 && macroBegin < result.length() - 1) {
            keepGoing = true;
            int macroEnd {macroBegin + 1};

            while (macroEnd < result.length() && isAlphaOrUnderscoreOrHyphen(result[macroEnd]))
                ++macroEnd;

            const String macro{ result.substring (macroBegin + 1, macroEnd) };
            const String subst{ macros.contains (macro) ? macros[macro].toString() : "" };
            result = result.substring(0, macroBegin) + subst + result.substring(macroEnd);
        }
    }

    return result;
}

//==============================================================================

CSSParser::CSSParser(Stylesheet& argStylesheet)
    : ctx{ nullptr },
      stylesheet{ argStylesheet },
      importFunction{}
{
}

CSSParser::~CSSParser() = default;

void CSSParser::setImportFunction(const ImportFunction& func)
{
    importFunction = func;
}

void CSSParser::fromString(const String& text)
{
    ctx = std::make_unique<Context>(stylesheet, text);

    while (! ctx->isOver()) {
        css::Style style{};

        if (!parseStyleOrMacroDefinition(style, stylesheet.getMacroDefinitions()))
            break;

        stylesheet.addStyle(std::move(style));
    }
}

bool CSSParser::parseStyleOrMacroDefinition(css::Style& style, juce::NamedValueSet& macros)
{
    ctx->skipSpacesAndComments();

    if (ctx->isOver())
        return false;

    bool keepGoing{ true };

    while (keepGoing) {
        keepGoing = false;

        // @import
        {
            String importName;

            if (ctx->parseImport(importName)) {
                keepGoing = true;

                if (importFunction) {
                    const auto importedSource{ importFunction(importName) };

                    if (importedSource.isNotEmpty()) {
                        ctx->source = importedSource + "\n" + ctx->source.substring(ctx->pos);
                        ctx->pos = 0;
                        ctx->length = ctx->source.length();
                    }
                }

                continue;
            }
        }

        // $macro
        {
            String macroName{};
            String macroValue{};

            if (ctx->parseMacroDefinition(macroName, macroValue))
            {
                keepGoing = true;
                macros.set(macroName, macroValue);
                continue;
            }
        }

        // tag.class.id[attr],..
        {
            String tag{};
            String clazz{};
            String id{};
            css::Selector selector{};

            if (ctx->parseSelectorTagClassId(tag, clazz, id))
                selector = css::Selector(tag, clazz, id);

            String attr, value;
            css::Selector::Attribute::Operator op;

            while (ctx->parseAttribute(attr, op, value))
                selector.addAttribute(std::move(css::Selector::Attribute(attr, op, value)));

            style.addSelector(selector);

            ctx->skipSpacesAndComments();

            if ((!ctx->isOver()) && (ctx->source[ctx->pos] == ',')) {
                keepGoing = true;
                ctx->pos += 1;
            }
        }
    }

    // Style properties section
    ctx->skipSpacesAndComments();

    if (ctx->isOver())
        return false;

    if (ctx->source[ctx->pos] != '{')
        return false;

    ctx->pos += 1;

    keepGoing = true;

    while (keepGoing) {
        keepGoing = false;
        ctx->skipSpacesAndComments();

        if (ctx->isOver())
            return false;

        if (ctx->source[ctx->pos] == '}') {
            ctx->pos += 1;
        } else {
            if (ctx->expect("@extend")) {
                // Parse extend selector
                ctx->skipSpacesAndComments();

                String tag{};
                String clazz{};
                String id{};
                css::Selector extendSelector{};

                if (ctx->parseSelectorTagClassId(tag, clazz, id))
                    extendSelector = css::Selector(tag, clazz, id);

                ctx->skipSpacesAndComments();

                if (!ctx->expect(";"))
                    return false;

                style.addExtendSelector(extendSelector);

                keepGoing = true;

            } else {
                // Parse key: value property.
                String name{};
                String value{};

                if (!ctx->parseProperty(name, value))
                    return false;

                value = substituteMacros(value, macros);
                style.setProperty(name, value);
                keepGoing = true;
            }
        }
    }

    return true;
}

} // namespace vitro
