namespace vitro {

class Context;
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
class Element : public std::enable_shared_from_this<Element>,
                private juce::ValueTree::Listener
{
public:

    using Ptr = std::shared_ptr<Element>;
    using WeakPtr = std::weak_ptr<Element>;

    /** Weak reference to the element stored in JS object.

        This structure is used to store a native weak reference to this
        element inside the JavaScript object that represents this element.
    */
    struct JSObjectRef final
    {
        Element::WeakPtr element{};
        JSObjectRef(const Element::Ptr& el);
        ~JSObjectRef();
    };

    const static juce::Identifier tag;  // <Element>

    static JSClassID jsClassID;

    Element() = delete;

    Element(Context& ctx);

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

    /** Tells whether this element consumes the inner XML tags.

        If this method returnes true the inner XML elements parsing will
        be delegated to the element, instead of being added to the elements tree.

        @see forwardXmlElement
    */
    virtual bool hasInnerXml() const { return false; }

    /** Returns this element's tag. */
    juce::Identifier getTag() const;

    /** Returns JS prototype class ID. */
    virtual JSClassID getJSClassID() const { return Element::jsClassID; }

    /** Retruns element's id.

        Element's id is read from the 'id' attribute. This means
        that id can potentially change, unline element's tag.
    */
    juce::String getId() const;

    /** Assign ID of this element. */
    void setId(const juce::String& id);

    /** Returns this element's parent.

        @return Pointer to parent element or nullptr if there is no parent.
    */
    Element::Ptr getParentElement() const;

    /** Return the top-most element on the try.

        This method will traverse the elements tree upwards
        and return the top-most element that has no parent.
    */
    Element::Ptr getTopLevelElement();

    /** Find the first element with given id.

        This method returns the first element (among this one and its children)
        which has a given id property value.

        @return Element with given id, or nullptr if not found.
    */
    Element::Ptr getElementById(const juce::String& id);

    /** Add a child element.

        @note This element takes full ownership of its children elements.
        @param element Pointer to child element to be added.
    */
    void addChildElement(const Element::Ptr& element);

    /** Remove child element.

        @param element Pointer to element to be removed.
    */
    void removeChildElement(const Element::Ptr& element);

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

    /** Populate this element from XML. */
    virtual void forwardXmlElement(const juce::XmlElement&) {};

    // @internal
    // This method gets called upon class registration with the factory.
    // It must create the element's JS prototype object with all the
    // properties and methods exposed.
    static void registerJSPrototype(JSContext* jsCtx, JSValue prototype);

    /** Store this element in factory's stash. */
    void stash();

    /** Remove this element from factory's stash. */
    void unstash();

    /** Returns a duplicate of the internal JS value. */
    JSValue duplicateJSValue();

    /** Returns the JS object reference counter. */
    int getJSValueRefCount() const;

    /** Evaluate onload attribute script.*/
    void evaluateOnLoadScript();

protected:

    /** Perform element initialization.

        This method is called by the factory immediately after being constructed.
        The default implementation initializes the internal JSValue of the element.

        @note Overriding methods must make sure to call initialize of the parent class.
     */
    virtual void initialize();

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

    /** Children elements iterator. */
    void forEachChild(const std::function<void(const Element::Ptr&)>& func, bool recursive = true);

    /** Evaluate script or function stored as element's attribute. */
    void evaluateAttributeScript(const juce::Identifier& attr, const juce::var& data = {});

    // The following are helper functions to register
    // element's JavaScript prototype object.
    using JSGetter = JSValue(*)(JSContext*, JSValueConst);
    using JSSetter = JSValue(*)(JSContext*, JSValueConst, JSValueConst);

    static void registerConstructor(JSContext* jsCtx, JSValue proto, juce::StringRef name, JSCFunction func, int numArgs);
    static void registerJSMethod(JSContext* jsCtx, JSValue proto, juce::StringRef name, JSCFunction func);
    static void registerJSProperty(JSContext* jsCtx, JSValue proto, juce::StringRef name, JSGetter getter, JSSetter setter = nullptr);

    /** Element's tag and the attributes are stored here. */
    juce::ValueTree valueTree;

    /** Global context reference. */
    Context& context;

    /** Pointer to the parent element. Can be nullptr for top-most element. */
    Element::WeakPtr parent{};

    /** Array of this element's direct children. */
    std::vector<Element::Ptr> children{};

    /** JavaScript object associated with this element. */
    JSValue jsValue{ JS_UNINITIALIZED };

    /** This flag tells element is in destruction phase. */
    bool inDestructor{ false };

private:

    friend class ElementsFactory;
    friend struct JSObjectRef;

    // Initialize the internal JS object
    void initJSValue();

    // Schedule the elements tree update
    void triggerUpdate();

    // juce::ValueTree::Listener
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;

    // JavaScript methods and properties
    static JSValue js_getTagName(JSContext* jsCtx, JSValueConst self);
    static JSValue js_getId(JSContext* jsCtx, JSValueConst self);
    static JSValue js_setId(JSContext* jsCtx, JSValueConst self, JSValueConst val);
    static JSValue js_getStyle(JSContext* jsCtx, JSValueConst self);
    static JSValue js_setStyle(JSContext* jsCtx, JSValueConst self, JSValueConst val);
    static JSValue js_getAttribute(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg);
    static JSValue js_setAttribute(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg);
    static JSValue js_hasAttribute(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg);
    static JSValue js_getAttributes(JSContext* jsCtx, JSValueConst self);
    static JSValue js_getParentElement(JSContext* ctx, JSValueConst self);
    static JSValue js_getChildren(JSContext* ctx, JSValueConst self);
    static JSValue js_getElementById(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg);
    static JSValue js_appendChild(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg);
    static JSValue js_removeChild(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg);

    /// This flag indicates that the element must be updated.
    /// @see updateElementIfNeeded
    bool updatePending{};

    // Here the list of changed attributes is stored. This list gets cleared
    // once the element is updated.
    std::set<juce::Identifier> changedAttributes{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Element)
};

} // namespace vitro
