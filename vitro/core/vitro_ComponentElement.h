namespace vitro {

/** Base class for Components based elements

    This is a base class for all the elements which are also
    UI Components.
*/
class ComponentElement : public LayoutElement
{
public:

    /** Helper class used to listen to mouse events.

        Received mouse events get forwarded to the associated element.
    */
    class MouseEventsProxy : public juce::MouseListener
    {
    public:
        MouseEventsProxy() = delete;
        explicit MouseEventsProxy(ComponentElement& el);

        // juce::MouseListener
        //void mouseMove(const juce::MouseEvent &event) override;
        void mouseEnter(const juce::MouseEvent &event) override;
        void mouseExit(const juce::MouseEvent &event) override;
        void mouseDown(const juce::MouseEvent &event) override;
        //void mouseDrag(const juce::MouseEvent &event) override;
        void mouseUp(const juce::MouseEvent &event) override;
        //void mouseDoubleClick(const juce::MouseEvent &event) override;

    private:
        ComponentElement& element;
        JUCE_DECLARE_NON_COPYABLE(MouseEventsProxy)
    };

    friend class MouseEventsProxy;

    //==========================================================================

    using Ptr = std::shared_ptr<ComponentElement>;

    static JSClassID jsClassID;

    ComponentElement(const juce::Identifier& tag, Context& ctx);

    // vitro::Element
    bool isComponentElement() const override { return true; }

    JSClassID getJSClassID() const override { return ComponentElement::jsClassID; }

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
    void initialize() override;
    void reconcileElement() override;

    // Mouse events forwarded to this component.
    virtual void handleMouseEnter(const juce::MouseEvent&) {};
    virtual void handleMouseExit(const juce::MouseEvent&) {};
    virtual void handleMouseDown(const juce::MouseEvent&) {};
    virtual void handleMouseUp(const juce::MouseEvent&) {};

private:

    // Proxy to intercept mouse events and forward them to this component element.
    std::unique_ptr<MouseEventsProxy> mouseEventsProxy{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentElement)
};

} // namespace vitro
