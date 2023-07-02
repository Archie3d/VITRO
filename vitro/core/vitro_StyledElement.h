namespace vitro {

class StyledElement : public Element
{
public:

    class Style : private juce::PropertySet
    {
    public:
        Style();

        // @todo Here we need to add a method to capture element-specific
        //       style properties from the global (and local) stylesheets.
        //       Once captured, all style queries should go via this class
        //       instead of matching CSS selectors.

    private:
        void propertyChanged() override;
    };


    StyledElement();
    StyledElement(const juce::Identifier& tag);

    bool isStyledElement() const override { return true; }

    const Style& getStyle() const { return style; }

    virtual void recalculateStyle() {}

private:

    Style style{};
};

} // namespace vitro
