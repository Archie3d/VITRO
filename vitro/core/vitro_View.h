namespace vitro {

/** The top-most view container.

    The view must be the top-most element of the UI hierarchy.
*/

class View : public ComponentElement,
             public juce::Component,
             private juce::AsyncUpdater
{
public:

    const static juce::Identifier tag;  // <View>

    static JSClassID jsClassID;

    View(Context& ctx);
    ~View();

    JSClassID getJSClassID() const override { return vitro::View::jsClassID; }

    juce::Component* getComponent() override { return this; }

    void initialize() override;

    void populateFromXml(const juce::XmlElement& xmlElement);

    void populateFromXmlString(const juce::String& xmlString);

    void populateFromXmlResource(const juce::String& location);

    /** Inject the view into JS global scope

        The view will be accessible as global `view` object.
    */
    void exposeToJS();

    static void registerJSPrototype(JSContext* jsCtx, JSValue prototype);

    // juce::Component
    void resized() override;

protected:

    // vitro::Element
    void update() override;

private:

    // juce::AsyncUpdater
    void handleAsyncUpdate() override;

    void updateEverything();

    void recalculateLayoutToCurrentBounds();
};

} // namespace vitro
