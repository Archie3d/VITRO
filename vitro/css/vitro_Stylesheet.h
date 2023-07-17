namespace vitro {

namespace css {

/** CSS Style selector.

    A selector is composed of tag, class, id, and attributes.
    All of these elements are optional. A selector matches to
    a ValueTree object when corresponding non-empty tag/class/id
    values match to those from the ValueTree, and all the attributes
    match as well.
*/
class Selector final
{
public:

    /** Attribute matching part of a CSS selector. */
    struct Attribute
    {
        enum class Operator
        {
            None,
            Defined,    // [attr]
            IsTrue,     // internal for :attr syntax
            Equals,     // [attr="value"]
            Contains,   // [attr*="value"]
            Prefix,     // [attr|="value"]
            Begins,     // [attr^="value"]
            Ends        // [attr$="value"]
        };

        Attribute(const juce::Identifier& argName, Operator argOp, const juce::String& argValue)
            : name{ argName },
              op{ argOp },
              value{ argValue }
        {
        }

        /** Returns true if the attribute name is empty or matching operator is none. */
        bool isEmpty() const;

        /** Match this attribute selector to the property from the ValueTree. */
        bool match(const juce::ValueTree& tree) const;

        /** Returns string representation of this attribute matcher. Useful for debugging. */
        juce::String toString() const;

        juce::Identifier name{};
        Operator op{ Operator::None };
        juce::String value{};
    };

    //==========================================================================

    Selector(const juce::StringRef argTag = "", const juce::StringRef argClass = "", const juce::StringRef argId = "");

    void addAttribute(const Attribute& attr);
    void addAttribute(Attribute&& attr);

    bool matchTag(const juce::String& argTag) const;
    bool matchClasses(const juce::StringArray& argClassArray) const;
    bool matchId(const juce::String& argId) const;
    bool matchAttributes(const juce::ValueTree& tree) const;

    bool match(const juce::String& argTag, const juce::StringArray& argClassArray, const juce::String& argId) const;
    bool match(const juce::String& argTag, const juce::StringArray& argClassArray, const juce::String& argId, const juce::ValueTree& tree) const;
    bool match(const juce::ValueTree& tree) const;

    /** Tells whether the other selector is more important.

        Multiple selectors may match the tag/class/is/attributes tuple, however only
        one of the selectors should be chosen to derive the style properties from.
        This method compares the matching criteria of this and the other selector
        and returns true if the other selector's matching conditions are tighter.

        Note that sometimes it is not possible to clearly decide a selector's priority,
        in such case any of the matching selectors can be used.
    */
    bool isOtherMoreImportant(const Selector& other) const;

    const juce::Array<Attribute>& getAttributes() const { return attributes; }

    /** Selector's string representation. Useful for debugging. */
    juce::String toString() const;

private:
    juce::String tag{};
    juce::String clazz{};
    juce::String id{};

    juce::Array<Attribute> attributes{};
};

//==============================================================================

/** A single style set.

    A style is a collection of the named properties grouped
    under a set of selectors. A style can be queried for a perticular
    property if one of the style's selectors matches the querying
    tag/class/id/attributes tuple.
*/
class Style final
{
public:
    Style();

    void addSelector(const Selector& selector);
    void addSelector(Selector&& selector);

    bool hasProperty(const juce::Identifier& name) const;
    void setProperty(const juce::Identifier& name, const juce::var& value);
    void setProperty(const juce::Identifier& name, juce::var&& value);

    const juce::var& getProperty(const juce::Identifier& name) const;
    const juce::var& operator[] (const juce::Identifier& name) const;

    /** Match any of the selectors of this style.

        @returns true if any of the selectors of this style matches the
                 tag/class/id/attributes tuple passed in the arguments.
    */
    bool match(const juce::String& tag, const juce::StringArray& classes, const juce::String& id, const juce::ValueTree& tree) const;

    bool match(const juce::ValueTree& tree) const;

    /** Check whether the other style should take priority.

        @return true if the most important selector of the other style given by the agument
                should take priority over the most important selector of this style.
    */
    bool isOtherMoreImportant(const Style& style) const;

private:
    const Selector* getMostImportantSelector() const;

    juce::Array<Selector> selectors{};
    juce::NamedValueSet properties{};
};

} // namespace css

//==============================================================================

/** Single stylesheet.

    A stylesheet is a collection of styles. Style properties
    can be queried from the stylesheet providing a tuple of
    tag/class/id/attribute.

    The stylesheet parser implements syntactic extension allowing
    macro definitions to be used in style properties:

        $macroName: macroValue;

    which can then be used in styles:

        {
            property: $macroName;
        }
*/
class Stylesheet
{
public:
    Stylesheet();

    /** Remove all styles from this stylesheet. */
    void clear();

    void addStyle(const css::Style& style);
    void addStyle(css::Style&& style);

    const juce::var& getProperty(const juce::Identifier& name,
                                 const juce::String& tag,
                                 const juce::StringArray& classes,
                                 const juce::String& id,
                                 const juce::ValueTree& tree) const;

    const juce::var& getProperty(const juce::Identifier& name, const juce::ValueTree& tree) const;

    const juce::NamedValueSet& getMacroDefinitions() const { return macroDefinitions; }
    juce::NamedValueSet& getMacroDefinitions() { return macroDefinitions; }

    /** Function to load referenced stylesheets.

        When there is an import statement encountered in the stylesheet
        being parsed, the parser will invoke this function to load
        the references stylesheet as a string.
    */
    using ImportFunction = std::function<juce::String(const juce::String&)>;

    /** Parse CSS string and populate this stylesheet.

        This method will add styles discovered in the CSS being parsed
        to this stylesheet. All existing styles will be preserved. Should the entire
        stylesheet needs to be replaced it must be cleared first via @ref clear() call.

        @note The CSS parser is rather relaxed. It will not report any errors and will
              populate only the styles it was able to parse correctly.

        @note The parser will ignore commented text. C-style comments are used.

        @note The parser will substitute all the encountered macros.
    */
    void populateFromString(const juce::String& text, const ImportFunction& importFunction = {});

    void populateFromVar(const juce::var& val, const ImportFunction& inportFunction = {});

private:

    juce::NamedValueSet macroDefinitions{};
    juce::Array<css::Style> styles{};
};

} // namespace vitro
