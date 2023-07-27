namespace vitro {

class OpenGLView : public vitro::ComponentElement,
                   public juce::Component,
                   private juce::OpenGLRenderer,
                   private juce::Timer
{
public:

    const static juce::Identifier tag;          // <OpenGLView>
    const static juce::Identifier tagTexture;   // <texture>
    const static juce::Identifier tagShader;    // <shader>

    static JSClassID jsClassID;

    OpenGLView(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::OpenGLView::jsClassID; }

    juce::Component* getComponent() override { return this; }

    juce::OpenGLContext& getOpenGLContext() { return openGLContext; }

    unsigned int findTextureID(const juce::String& name) const;

protected:

    // vitro::Element
    void update() override;

private:

    class RenderPass final
    {
    public:
        RenderPass(OpenGLView& oglView, const juce::String& passName);

        juce::String getName() const { return name; }
        bool isValid() const { return valid; }
        void setShader(const juce::String& code);
        void setTargetFrameBuffer(int width, int height);
        bool hasTargetFrameBuffer() const;
        unsigned int getTextureID() const;
        void render();

    private:
        OpenGLView& openGLView;
        juce::String name{};
        bool valid{};
        juce::OpenGLShaderProgram program;
        std::unique_ptr<juce::OpenGLFrameBuffer> targetFrameBuffer{};
    };

    void populateTextureElement(const Element::Ptr& elem);
    void populateShaderElement(const Element::Ptr& elem);
    void populateItems();

    // juce::OpenGLRenderer
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    // juce::Timer
    void timerCallback() override;

    // juce::ValueTree::Listener
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;


    juce::OpenGLContext openGLContext{};

    unsigned int quadBuffers[2];

    std::vector<std::unique_ptr<RenderPass>> renderPasses{};
    std::map<juce::String, std::unique_ptr<juce::OpenGLTexture>> textures{};

    std::mutex updateMutex{};
    std::atomic<bool> itemsUpdatePending{};
};

} // namespace vitro
