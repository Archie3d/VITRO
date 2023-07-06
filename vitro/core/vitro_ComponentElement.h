namespace vitro {

/** Base class for Components based elements

    This is a base class for all the elements which are also
    UI Components.
*/
class ComponentElement : public LayoutElement
{
public:

    using Ptr = std::shared_ptr<ComponentElement>;

    static JSClassID jsClassID;

    ComponentElement(const juce::Identifier& tag, Context& ctx);

    // vitro::Element
    bool isComponentElement() const { return true; }

    virtual JSClassID getJSClassID() const { return ComponentElement::jsClassID; }

    /** Return component of this element.

        Derived elements must return the pointer to the component that
        represent this element.
    */
    virtual juce::Component* getComponent() = 0;

    /** Return parent component element from the tree.

        This method travels the elements tree up from this element
        and returns the first component element found.
    */
    ComponentElement::Ptr getParentComponentElement();

    /** Update comonent bounds to the layout.

        This will reposition this component to the layout bounds calculated.

        @see LayoutElement::updateLayout
        @see LayoutElement::recalculateLayout
    */
    void updateComponentBoundsToLayoutNode();

    static void registerJSPrototype(JSContext* ctx, JSValue prototype);

protected:

    // vitro::Element
    void reconcileElement() override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentElement)
};

} // namespace vitro
