namespace vitro {

/** Base class for Components based elements

    This is a base class for all the elements which are also
    UI Components.

    @note JUCE shadow works properly only for rectangular components.
          When using rounded corners the shadow will not paint properly.

    Attributes:
        visible
        enabled

    Style properties:
        alpha
        click-through
        cursor
        shadow-color
        shadow-radius
        shadow-offset-x
        shadow-offset-y
*/
class ComponentElement : public LayoutElement,
                         public juce::DragAndDropTarget,
                         private juce::ComponentListener
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
        void mouseMove(const juce::MouseEvent &event) override;
        void mouseEnter(const juce::MouseEvent &event) override;
        void mouseExit(const juce::MouseEvent &event) override;
        void mouseDown(const juce::MouseEvent &event) override;
        void mouseDrag(const juce::MouseEvent &event) override;
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

    ComponentElement(const juce::Identifier& elementTag, Context& ctx);

    // vitro::Element
    bool isComponentElement() const override { return true; }

    JSClassID getJSClassID() const override { return ComponentElement::jsClassID; }

    /** Return component of this element.

        Derived elements must return the pointer to the component that
        represent this element.
    */
    virtual juce::Component* getComponent() = 0;

    /** Returns a container component.

        This method is used to indicate container components, like
        scroll areas or windows.
    */
    virtual juce::Component* getContainerComponent() { return getComponent(); }

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

    // juce::DragAndDropTarget
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails &dragSourceDetails) override;

    bool isComponentElementDraggable() const { return draggable; }

    bool doesComponentElementAccepDrop() const { return acceptDrop; }

    static void registerJSPrototype(JSContext* ctx, JSValue prototype);

protected:

    /** Helper to set component's colour from a style property. */
    void setColourFromStyleProperty(juce::Component& component, int colourId, const juce::Identifier& propertyName);
    void setColourFromStyleProperty(int colourId, const juce::Identifier& propertyName);

    /** Helper to set component's font from style property. */
    void populateFontFromStyleProperties(juce::Font& font);

    /** Helper to update component's mouse curtsor. */
    void setMouseCursorFromStyleProperties();

    // vitro::Element
    void initialize() override;
    void update() override;
    void reconcileElement() override;

    // Mouse events forwarded to this component.
    virtual void handleMouseMove(const juce::MouseEvent&);
    virtual void handleMouseEnter(const juce::MouseEvent&);
    virtual void handleMouseExit(const juce::MouseEvent&);
    virtual void handleMouseDown(const juce::MouseEvent&);
    virtual void handleMouseDrag(const juce::MouseEvent&);
    virtual void handleMouseUp(const juce::MouseEvent&);

private:

    // juce::ComponentListener
    void componentMovedOrResized(juce::Component&, bool wasMoved, bool wasResized) override;

    // JavaScript methods and properties
    static JSValue js_getViewBounds(JSContext* ctx, JSValueConst self);
    static JSValue js_getScreenBounds(JSContext* ctx, JSValueConst self);

    // Proxy to intercept mouse events and forward them to this component element.
    std::unique_ptr<MouseEventsProxy> mouseEventsProxy{};

    // Shadow effect.
    juce::DropShadow shadow{};
    std::unique_ptr<juce::DropShadower> dropShadower{};

    // Drag and drop
    bool draggable{};
    bool acceptDrop{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentElement)
};

} // namespace vitro
