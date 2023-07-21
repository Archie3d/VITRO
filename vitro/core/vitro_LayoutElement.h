namespace vitro {

/** Element that participate in the UI layout

    This class defines an element that will be positioned
    by the layout engine.
*/
class LayoutElement : public StyledElement
{
public:

    using Ptr = std::shared_ptr<LayoutElement>;

    static JSClassID jsClassID;

    LayoutElement(const juce::Identifier& tag, Context& ctx);

    ~LayoutElement();

    // vitro::Element
    bool isLayoutElement() const override { return true; }

    JSClassID getJSClassID() const override { return LayoutElement::jsClassID; }

    /** Returns the layout element bounds.

        This method does not perform any computation but only returns
        the currently calculated layout bounds.
    */
    juce::Rectangle<float> getLayoutElementBounds() const;

    /** Returns the closest parent layout element.

        This method walks the elements tree up until it finds a layout element.
        If there is none, nullptr will be returned.

        @note This method will fine the closest parent layout element even if it
              is not yet connected throught the layout nodes tree. It iis used precisely
              to establish this connection.
    */
    LayoutElement::Ptr getParentLayoutElement();

    /** Call the layout update on the elements tree.

        This will cause the a layout to be rebuilt recursively
        by following the layout nodes. This method only updates the layout nodes
        properties, but does not perform the actual placement. For this the layout
        engine needs to know the target area, which is done via @ref recalculateLayout

        @see recalculateLayout

        @return true if layout has changed after update, false otherwise.
    */
    bool updateLayout();

    /** Recalculate the layout elements bounds.

        This method perform the layout computation starting from this element.
        All component elements' bounds will be updated recursively.
        This is a method to be called when resizing the top UI container.
    */
    void recalculateLayout(float width, float height);

    // @internal
    static void registerJSPrototype(JSContext* ctx, JSValue prototype);

protected:

    // vitro::Element
    void numberOfChildrenChanged() override;
    void reconcileElement() override;

private:

    // JavaScript methods and properties
    static JSValue js_getLocalBounds(JSContext* ctx, JSValueConst self);

    // Internal layout structure (referencing Yoga layout node).
    struct Layout;
    std::unique_ptr<Layout> layout;

    // Flag indicating the children composition has been changed.
    // This is used when reporting the layout change, since adding or
    // removing children most likely causes the layout changes.
    bool childrenChanged{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayoutElement)
};

} // namespace vitro
