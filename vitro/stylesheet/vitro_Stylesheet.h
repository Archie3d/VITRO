namespace vitro {

namespace css {

class Selector
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

        juce::String toString() const;

        juce::Identifier name{};
        Operator op{ Operator::None };
        juce::String value{};
    };

    //==========================================================================

    Selector(const juce::StringRef argTag = "", const juce::StringRef argClass = "", const juce::StringRef argId = "");

private:
    juce::String tag{};
    juce::String clazz{};
    juce::String id{};

    juce::Array<Attribute> attributes{};
};

} // namespace css

} // namespace vitro
