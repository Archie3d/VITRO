namespace vitro {

class ComponentElement : public LayoutElement
{
public:

    ComponentElement(const juce::Identifier& tag, Context& ctx);

    // vitro::Element
    bool isComponentElement() const { return true; }

    /** Return component of this element.

        Derived elements must return the pointer to the component that
        represent this element.
    */
    virtual juce::Component* getComponent() = 0;

    ComponentElement* getParentComponentElement();

    /** Update comonent bounds to the layout.

        This will reposition this component to the layout bounds calculated.

        @see LayoutElement::updateLayout
        @see LayoutElement::recalculateLayout
    */
    void updateComponentBoundsToLayoutNode();

protected:

    // vitro::Element
    void reconcileElement() override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentElement)
};

} // namespace vitro
