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

    static JSClassID jsClassID;

    StyledElement(const juce::Identifier& tag, Context& ctx);

    // vitro::Element
    bool isStyledElement() const override { return true; }

    virtual JSClassID getJSClassID() const { return StyledElement::jsClassID; }

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

    /** Return style property value and its change flag.

        This is a helper method that tells whether a style property has changed
        since the last update, along with the current value itself.
    */
    std::pair<bool, const juce::var&> getStylePropertyChanged(const juce::Identifier& name) const;

    static void registerJSPrototype(JSContext* ctx, JSValue prototype);

protected:

    /** Register element's style property.

        A styled element must register its style properties. These
        properties will then be captured by the @ref updateStyleProperties method
        for further access.
    */
    void registerStyleProperty(const juce::Identifier& name, const juce::var& value = {});

    /** Tell whether a style property has changed since the last update.

        This method queries a set of changed properties since the last update
        and tells whether a given one is on the list. The set of changed properties gets
        cleared before each update, and then it gets repopulated by comparing the stylesheet
        with the locally stored style properties.
    */
    bool isStylePropertyChanged(const juce::Identifier& name) const;

private:

    // Local stylesheet applicable to this element only.
    Stylesheet localStylesheet{};

    // List of style properties this element cares about.
    juce::NamedValueSet styleProperties{};

    // List of style properties changes since the last update.
    std::set<juce::Identifier> changedStyleProperties{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StyledElement)
};

} // namespace vitro
