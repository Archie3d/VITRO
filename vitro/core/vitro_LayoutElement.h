namespace vitro {

/** Element that participate in the UI layout

    This class defines an element that will be positioned
    by the layout engine.
*/
class LayoutElement : public StyledElement
{
public:

    LayoutElement(const juce::Identifier& tag, Context& ctx);

    bool isLayoutElement() const override { return true; }

    juce::Rectangle<float> getLayoutElementBounds() const;

    LayoutElement* getParentLayoutElement();

protected:

    // vitro::Element
    void numberOfChildrenChanged() override;

    void reconcileElement() override;

private:

    struct Layout;
    std::unique_ptr<Layout> layout;

    // Flag indicating the children composition has been changed.
    // This is used when reporting the layout change, since adding or
    // removing children most likely causes the layout changes.
    bool childrenChanged{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayoutElement)
};

} // namespace vitro
