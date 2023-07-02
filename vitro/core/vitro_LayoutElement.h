namespace vitro {

class LayoutElement : public StyledElement
{
public:

    LayoutElement();

    bool isLayoutElement() const { return true; }

    juce::Rectangle<float> getLayoutElementBounds() const;

private:

    struct Layout;
    std::unique_ptr<Layout> layout;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayoutElement)
};

} // namespace vitro
