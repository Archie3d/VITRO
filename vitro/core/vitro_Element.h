namespace vitro {

class LayoutElement;

/** UI element base class

    This is a base class for all the elements that compose the UI.
    An element is identified by its tag, and it has a set of properties
    (attributes) stored as a ValueTree.

    Element is a part of the UI tree, but it does not necessarily correspond
    to a layout or a visual component. For this the Element class is further
    specialised into LayoutElement and ComponentElement.

    @see StyledElement
    @see LayoutElement
    @see ComponentElement
*/
class Element : private juce::ValueTree::Listener
{
public:

    Element() = delete;
    Element(const juce::Identifier& tag, Context& ctx);
    virtual ~Element();

    virtual bool isStyledElement() const { return false; }
    virtual bool isLayoutElement() const { return false; }
    virtual bool isComponentElement() const { return false; }

    /** Returns this element's tag. */
    juce::Identifier getTag() const;

    /** Retruns element's id. */
    juce::String getId() const;

    Element* getParentElement() const;
    Element* getTopLevelElement();
    void addChildElement(Element* element);
    void removeChildElement(Element* element, bool deleteObject);
    void removeAllChildElements();

    /** Assign element's attribute.

        This will assign an attribute stored in this element.
        Some attributes have special meaning (like src) and may
        provoke the element's hierarchy changes. Setting an attribute with
        the notification flag will trigger the elements update, which
        is important since attribute values may change which style selectors
        match this element.
    */
    void setAttribute(const juce::Identifier& name, const juce::var& value, bool notify = true);

    /** Return element's attribute value.

        This method returns the attribute value stored in this element.
        If the attribute does not exist a void var reference will be returned.
    */
    const juce::var& getAttribute(const juce::Identifier& name) const;

    /** Tell whether an attribute exists.

        @returns true if the attributre exists, false otherwise.
    */
    bool hasAttribute(const juce::Identifier& name) const;

    /** Perform an update on this element.

        This method calls the @ref update() on this element if it's
        been flagged for update. And then calls @ref updateChildren().
    */
    void updateElementIfNeeded();

    /** Force update the element.

        This will refresh the element even if it has not been flagged
        as pending for update.
     */
    void forceUpdate();

    /** Update all the children.

        This method calls @ref updateElementIdNeeded() on all the children of this element.
     */
    void updateChildren();

protected:

    /** Update this element.

        Elements will override this method to perform an update of the inner state.
        Normally an element will refresh it's view based on the attributes and
        style properties here.
     */
    virtual void update() {}

    /** Notify this element when adding or removing children

        This method is used to inform this element when number of children changes.
        This may be important for layout and style recalculation.
    */
    virtual void numberOfChildrenChanged() {}

    /** Assign the source attribute.

        The default implementation loads the source as XML and
        populates the current element with children of the loaded XML.
        All the current children of the element get deleted first.

        Specialised implementation may interpret the src attribute differently.
    */
    virtual void setSrcAttribute(const juce::String& src);

    /** Returns element's attribute and whether it's been changed since the last update.

        Upon the change the attribute's identifier gets stored.
        Since the updates are batched (multiple attributes change will result
        in a single update) it is useful to track the attributes that changed
        in order to avoid unnecessary updates on the underlying UI components.
    */
    std::pair<bool, const juce::var&> getAttributeChanged(const juce::Identifier& attr) const;

    /** Notify all children of this element that they are about to be deleted. */
    void notifyChildrenAboutToBeRemoved();

    /** Inform this element that is it about to be deleted.

        This method is called just before the element gets removed
        from the UI layout and eventually deleted.
    */
    virtual void elementIsAboutToBeRemoved() {}

    /** Reconcile elements internal tree.

        This method is used to rebuild the internal elements tree.
        For example the layout elements will need to be connected to corresponding
        parent layout nodes (which may situate about this element).

        @see reconcileElement()
    */
    void reconcileElementTree();

    /** Reconcile this element on the internal tree.

        By overriding this method a derived element can reposition itself
        on the internal tree (layout tree or components tree).
    */
    virtual void reconcileElement() {}

    juce::ValueTree valueTree;

    Context& context;

    Element* parent{};

    juce::OwnedArray<Element> children{};

private:

    // juce::ValueTree::Listener
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;

    bool updatePending{};
    std::set<juce::Identifier> changedAttributes{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Element)
};

} // namespace vitro
