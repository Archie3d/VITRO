namespace vitro {

class ElementsFactory;
class Loader;

/** UI Context.

    UI Context aggregates global UI functionalityes like
    the stylesheet, elements factory, resources loader.
*/
class Context
{
public:
    Context();
    virtual ~Context();

    const Loader& getLoader() const;
    Loader& getLoader();

    const Stylesheet& getStylesheet() const;
    Stylesheet& getStylesheet();

    const ElementsFactory& getElementsFactory() const;
    ElementsFactory& getElementsFactory();

    void initialize();

    void reset();

private:

    struct Impl;
    std::unique_ptr<Impl> d;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Context)
};

} // namespace vitro
