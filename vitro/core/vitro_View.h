namespace vitro {

/** The top-most view container.

    The view must be the top-most element of the UI hierarchy.
*/

class View : public ComponentElementWithBackground,
             public juce::Component,
             public juce::DragAndDropContainer,
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

    /** Inject the view into JS global scope

        The view will be accessible as global `view` object.
    */
    void exposeToJS();

    static void registerJSPrototype(JSContext* jsCtx, JSValue prototype);

    // juce::Component
    void resized() override;
    void paint(juce::Graphics& g) override;

protected:

    // vitro::Element
    void update() override;

private:

    // juce::AsyncUpdater
    void handleAsyncUpdate() override;

    void updateEverything();

    void recalculateLayoutToCurrentBounds();

    // JavaScript methods and properties
    static JSValue js_createElement(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg);
    static JSValue js_isDragAndDropActive(JSContext* ctx, JSValueConst self, int argc, JSValueConst* arg);

    juce::Colour backgroundColour{};
};

} // namespace vitro
