namespace vitro {

/** Styled element.

    This class defines an element that can have a style
    applied to it. All layout and component elements are
    derived from this one.

    @see LayoutElement
    @see ComponentElement
*/
class StyledElement : public Element
{
public:

    StyledElement(const juce::Identifier& tag, Context& ctx);

    bool isStyledElement() const override { return true; }

    /** Assign style attribute value.

        This method allows assigning the local style of the
        element via its style attribute.
    */
    void setStyleAttribute(const juce::var& value);

    /** Capture style properties.

        This method should be used to capture element's style
        properties from the local and global stylesheets.

        The method must be called on element attributes update
        and on global stylesheet change.
    */
    void updateStyleProperties();

    /** Return element's style property.

        This method returns a style property value captured
        for the current state of the element.
    */
    const juce::var& getStyleProperty(const juce::Identifier& name) const;

protected:

    /** Register element's style property.

        A styled element must register its style properties. These
        properties will then be captured by the @ref updateStyleProperties method
        for further access.
    */
    void registerStyleProperty(const juce::Identifier& name, const juce::var& value = {});

private:

    // Local stylesheet applicable to this element only.
    Stylesheet localStylesheet{};

    // List of style properties this element cares about.
    juce::NamedValueSet styleProperties{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StyledElement)
};

} // namespace vitro
