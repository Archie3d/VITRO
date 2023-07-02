namespace vitro {

/** UI Context.

    UI Context aggregates global UI functionalityes like
    the stylesheet, elements factory, resources loader.
*/
class Context
{
public:
    Context();
    virtual ~Context();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Context)
};

} // namespace vitro
