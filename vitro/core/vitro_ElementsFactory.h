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

    void stashElement(const Element::Ptr& element);
    void removeStashedElement(const Element::Ptr& element);

    void clearStashedElements();
    void clearUnreferencedStashedElements();

private:

    Context& context;

    using CreateFunc = std::function<Element::Ptr()>;
    std::map<juce::Identifier, CreateFunc> creators{};

    std::vector<Element::Ptr> stashedElements{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementsFactory)
};

} // namespace vitro
