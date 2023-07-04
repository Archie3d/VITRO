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

    /** Construct an element.

        @param tag Tag to be assigned to this element.
        @param ctx Global context reference.
    */
    Element(const juce::Identifier& tag, Context& ctx);

    virtual ~Element();

    /** Tells whether this element is a styled element. */
    virtual bool isStyledElement() const { return false; }

    /** Tells whether this element is a layout element. */
    virtual bool isLayoutElement() const { return false; }

    /** Tells whether this element is a UI component. */
    virtual bool isComponentElement() const { return false; }

    /** Returns this element's tag. */
    juce::Identifier getTag() const;

    /** Retruns element's id.

        Element's id is read from the 'id' attribute. This means
        that id can potentially change, unline element's tag.
    */
    juce::String getId() const;

    /** Returns this element's parent.

        @return Pointer to parent element or nullptr if there is no parent.
    */
    Element* getParentElement() const;

    /** Return the top-most element on the try.

        This method will traverse the elements tree upwards
        and return the top-most element that has no parent.
    */
    Element* getTopLevelElement();

    /** Find the first element with given id.

        This method returns the first element (among this one and its children)
        which has a given id property value.

        @return Element with given id, or nullptr if not found.
    */
    Element* getElementById(const juce::String& id) const;

    /** Add a child element.

        @note This element takes full ownership of its children elements.
        @param element Pointer to child element to be added.
    */
    void addChildElement(Element* element);

    /** Remove child element.

        @param element Pointer to element to be removed.
        @param deleteObject Flag telling that removed element's object should be deleted.
    */
    void removeChildElement(Element* element, bool deleteObject);

    /** Remove all child elements.

        @note All the removed elements will be deleted.
    */
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

        This method calls @ref updateElementIfNeeded() on all the children of this element.
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

    void forEachChild(const std::function<void(Element*)>& func, bool recursive = true);

    /** Element's tag and the attributes are stored here. */
    juce::ValueTree valueTree;

    /** Global context reference. */
    Context& context;

    /** Pointer to the parent element. Can be nullptr for top-most element. */
    Element* parent{};

    /** Array of this element's direct children. */
    juce::OwnedArray<Element> children{};

private:

    // juce::ValueTree::Listener
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;

    /// This flag indicates that the element must be updated.
    /// @see updateElementIfNeeded
    bool updatePending{};

    // Here the list of changed attributes is stored. This list gets cleared
    // once the element is updated.
    std::set<juce::Identifier> changedAttributes{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Element)
};

} // namespace vitro
