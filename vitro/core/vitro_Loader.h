namespace vitro {

/** Resource loader.

    This class provides mechanism to load binary and other resources.
    The loader can supply content from the embedded binary resources
    as well as from a local folder or remote.
*/
class Loader
{
public:

    Loader();

    /** Specify a local directory to be used to load resources.

        If specified, the local folder will be used first to
        locate resources to be loaded. Id don't exist the loader
        will attempt to locate the resource in the binary data (if enabled).
    */
    void setLocalDirectory(const juce::File& dir);

    /** Load text from local resources. */
    juce::String loadText(const juce::String& location) const;

    /** Load XML from local resources. */
    std::unique_ptr<juce::XmlElement> loadXML(const juce::String& location) const;

    /** Load an image from local resources. */
    juce::Image loadImage(const juce::String& location) const;

private:

    bool hasLocalPath{ false };
    juce::File localPath{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Loader)
};

} // namespace vitro
