namespace vitro {

namespace attr {

// Some attributes that need to be available from within a header

extern const juce::Identifier text;
extern const juce::Identifier toggle;
extern const juce::Identifier active;
extern const juce::Identifier radiogroup;

extern const juce::Identifier onclick;

/** Tells whether an attribute is a volatile one.

    Volatile attributes store transient dynamic values or event handlers
    and do not represent element's data.

    For example: hover, onload, onclick, etc. 
*/
bool isVolatile(const juce::Identifier& attr);

} // namespace attr

} // namespace vitro
