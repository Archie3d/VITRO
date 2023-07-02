namespace vitro {

class ComponentElement : public LayoutElement
{
public:

    ComponentElement();

    bool isComponentElement() const { return true; }

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentElement)
};

} // namespace vitro
