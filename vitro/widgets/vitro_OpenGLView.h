namespace vitro {

class OpenGLView : public vitro::ComponentElement,
                   public juce::Component,
                   private juce::OpenGLRenderer,
                   private juce::Timer
{
public:

    const static juce::Identifier tag;  // <OpenGLView>

    static JSClassID jsClassID;

    OpenGLView(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::OpenGLView::jsClassID; }

    juce::Component* getComponent() override { return this; }

private:

    class RenderPass final
    {
    public:
        RenderPass(juce::OpenGLContext& oglContext);
    private:
        juce::OpenGLContext& openGLContext;
        juce::OpenGLShaderProgram shader;
        std::unique_ptr<juce::OpenGLFrameBuffer> targetFrameBuffer{};
    };


    // juce::OpenGLRenderer
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    // juce::Timer
    void timerCallback() override;

    juce::OpenGLContext openGLContext{};

    unsigned int quadBuffers[2];

    juce::OpenGLShaderProgram program;
};

} // namespace vitro
