namespace vitro {

/** CSS styelsheet parser.

    This class implements a parser for a subset
    of CSS syntax. It is used internaly by the Stylesheet class.
*/
class CSSParser final
{
public:

    /** Callback function to handle CSS imports. */
    using ImportFunction = std::function<juce::String(const juce::String&)>;

    CSSParser() = delete;
    CSSParser(Stylesheet& argStylesheet);
    ~CSSParser();

    /** Assign an import function. */
    void setImportFunction(const ImportFunction& func);

    /** Add new styled from the parsed string. */
    void fromString(const juce::String& text);

private:

    bool parseStyleOrMacroDefinition(css::Style& style, juce::NamedValueSet& macros);

    struct Context;
    std::unique_ptr<Context> ctx;
    ImportFunction importFunction;

    Stylesheet& stylesheet;
};

} // namespace vitro
