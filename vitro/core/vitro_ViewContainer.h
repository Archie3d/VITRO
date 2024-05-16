namespace vitro {

/** View container.

    This class aggregates the UI context and the View component.
    It handles the UI loading and reloading.

    @note A context cannot be shared between different view containers.

    @see Context
    @see View
*/
class ViewContainer : public juce::Component
{
public:

    class Listener
    {
    public:
        virtual void onContextCreated(vitro::Context* ctx) {}
        virtual void onViewLoaded(vitro::View*) {}
        virtual ~Listener() = default;
    };

    ViewContainer();
    ~ViewContainer();

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    /** Assign a local directory for the loader to look for the resources. */
    void setLocalDirectory(const juce::File& dir);

    /** Reset and (re)load the UI.

        This will reset the current view and the context, and create
        new ones loading the UI definition from the specified resources.
    */
    void loadFromResource(const juce::String& xmlLocation,
                          const juce::String& cssLocation = "",
                          const juce::String& scriptLocation = "");

    /** Return the internal context pointer.

        Context is used to access the resources loader, stylesheet, JavaScript engine,
        and elements factory.

        @note When reloading the UI the old context gets deleted,
              so the pointer returned previously by this method will be invalid.
    */
    vitro::Context* getContext() { return context.get(); }

    /** Return the internal view.

        The view provides access to all the internal elements.

        @note A caller must not keep the view pointer, since it may be deleted by
              this container on reload, which will result in creation of a new view.
    */
    vitro::View* getView() { return view.get(); }

    // juce::Component
    void resized() override;

private:
    std::unique_ptr<vitro::Context> context{};
    std::shared_ptr<vitro::View> view{};

    juce::File localDir{};

    juce::ListenerList<Listener> listeners{};
};

} // namespace vitro
