namespace vitro {

class ComponentElement : public LayoutElement
{
public:

    ComponentElement(const juce::Identifier& tag, Context& ctx);

    bool isComponentElement() const { return true; }

    virtual juce::Component* getComponent() = 0;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentElement)
};

} // namespace vitro
