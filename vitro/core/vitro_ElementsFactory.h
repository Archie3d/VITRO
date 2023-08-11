namespace vitro {

/** UI elements factory.

    This class is a factory for UI elements. Normally an instance
    of the factory lives in the Context object, from where the factory
    is used to construct new elements.
*/
class ElementsFactory
{
public:

    ElementsFactory() = delete;

    /** Construct a factory.

        A factory must be associated with a context. This context
        then gets injected into the elements constructed by this factory.
    */
    ElementsFactory(Context& ctx);

    /** Register an element class with this factory.

        Element's class must expose a public 'tag' member which
        corresponds to it's tag.
    */
    template <class T>
    void registerElement()
    {
        creators[T::tag] = [this](){ return std::make_shared<T>(context); };
        context.registerJSClass<T>(T::tag.toString().toRawUTF8());
    }

    /** Reset the factory.

        This will remove all creators.

        @note The factory should be reset when resetting the context,
              otherwise the mapping may stay invalid from the previous
              context.
    */
    void reset();

    /** Register default UI elements.

        This method registers default UI elements with this factory.
    */
    void registerDefaultElements();

    /** Create element for given tag.

        This method create a new element which has been previously
        registered with this factory. If the element's tag is unknown to
        the factory a generic Component element will be created under the
        given tag.

        @returns Pointer to created element.
    */
    Element::Ptr createElement(const juce::Identifier& tag);

    /** Add element to the stash.

        The factory maintains a collection of shared pointers for the
        elements that might be in transition between script and native context.
        Until finalized (attached to the elements tree or garbage collected by
        the scripting engine) the epement's pointer must be kept alive in order
        to prevent memory access problems. Elements that live on stash only
        will eventually be collected and deleted.

        @see clearUnreferencedStashedElements
    */
    void stashElement(const Element::Ptr& element);

    /** Remove element from the stash.

        This method is normally called when the element gets attached to its parent.
        At this point the parent becomes the owner of the element, which can now
        be removed from the stash.
    */
    void removeStashedElement(const Element::Ptr& element);

    /** Remove all stashed elements.

        This will clear the stash collection. All elements that are not
        referenced anywhere else will be deleted.
    */
    void clearStashedElements();

    /** Remove all elements that are referenced by the stash only.

        This method perform "garbage collection" of the stashed elements.
        All the elements that live in the stash only and are not referenced
        from the script context will be deleted.
    */
    void clearUnreferencedStashedElements();

private:

    Context& context;

    using CreateFunc = std::function<Element::Ptr()>;
    std::map<juce::Identifier, CreateFunc> creators{};

    std::vector<Element::Ptr> stashedElements{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementsFactory)
};

} // namespace vitro
