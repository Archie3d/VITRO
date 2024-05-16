namespace vitro {

/** Resource loader.

    This class provides mechanism to load binary and other resources.
    The loader can supply content from the embedded binary resources
    as well as from a local folder or remote.
*/
class Loader
{
public:

    /** Listener on resources loading operations.

        Loader will notify registered listeners when resource (text or image)
        gets loaded either from a local file system (if local directory is set)
        or the binary data.
    */
    class Listener
    {
    public:
        virtual ~Listener() = default;

        /** When lodaded from a file, the file argument points to the file used. */
        virtual void onResourceLoaded(const juce::String& location, const juce::File& file) = 0;
    };

    Loader();

    /** Specify a local directory to be used to load resources.

        If specified, the local folder will be used first to
        locate resources to be loaded. Id don't exist the loader
        will attempt to locate the resource in the binary data (if enabled).
    */
    void setLocalDirectory(const juce::File& dir);

    /** Load text from local resources. */
    juce::String loadText(const juce::String& location);

    /** Load XML from local resources. */
    std::unique_ptr<juce::XmlElement> loadXML(const juce::String& location);

    /** Load an image from local resources. */
    juce::Image loadImage(const juce::String& location);

    /** Register a listener with this loader. */
    void addListener(Listener* listener);

    /** Unregister a listener from this loader. */
    void removeListener(Listener* listener);

private:

    bool hasLocalPath{ false };
    juce::File localPath{};

    juce::ListenerList<Listener> listeners{};

    void notifyOnResourceLoaded(const juce::String& location, const juce::File& file = {});

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Loader)
};

} // namespace vitro
