namespace vitro {

/** OpenGL component painted via a fragment shader.

    This widgets defines an OpenGL 2D rendering surface which is
    drawn by a fragment shader. The renderer may have addition shaders
    that output to a frame buffer that can be used then as a texture imput
    to other shaders.

    This element expects the following child elements:
        <texture>   defines a static texture loaded from the resource
        <shader>    fragment shader render pass

    A <shader> element in its turm may contains <uniform> elements for the
    uniform variables to be used by the shader.

    Attributes
        fps
*/
class OpenGLView : public vitro::ComponentElement,
                   public juce::Component,
                   private juce::OpenGLRenderer,
                   private juce::Timer
{
public:

    /** Component state.
      
        This is used to capture component's state on the main thread
        to pass this information to shaders on the rendering thread.
    */
    struct State final
    {
        mutable std::mutex mutex{};

        int frame{};
        float timeDelta{};
        juce::Point<float> mouse{};
        juce::Rectangle<float> screenBounds{};

        void lock() { mutex.lock(); }
        void unlock() { mutex.unlock(); }
    };

    const static juce::Identifier tag;          // <OpenGLView>
    const static juce::Identifier tagTexture;   // <texture>
    const static juce::Identifier tagShader;    // <shader>
    const static juce::Identifier tagUniform;   // <uniform>

    static JSClassID jsClassID;

    OpenGLView(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::OpenGLView::jsClassID; }

    juce::Component* getComponent() override { return this; }

    juce::OpenGLContext& getOpenGLContext() { return openGLContext; }

    unsigned int findTextureID(const juce::String& name) const;

    State& getState() { return state; }

    // juce::Component
    void resized() override;

protected:

    // vitro::Element
    void update() override;

private:

    class Uniform;

    /** Single render pass performed by a single fragment shader.
    
        A render pass applies fragment shader to the target frame buffer.
        An internal framebuffer can be defined, in this case the rendering result
        is cached and can be accessed as a texture from another render pass.
     */
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
        void addUniform(std::unique_ptr<Uniform>&& uniform);
        void render();

        const OpenGLView& getOpenGLView() const { return openGLView; }
        juce::OpenGLShaderProgram& getShaderProgram() { return program; }

    private:

        void applyDefaultUniforms();
        void applyUniforms();

        OpenGLView& openGLView;
        juce::String name{};
        bool valid{};
        juce::OpenGLShaderProgram program;
        std::unique_ptr<juce::OpenGLFrameBuffer> targetFrameBuffer{};
        std::vector<std::unique_ptr<Uniform>> uniforms{};
    };

    /** Shader uniform value.
    
        This is a helper class that contains uinform variable value.
     */
    class Uniform final
    {
    public:
        enum class Type
        {
            Invalid, Int, Float, Vec2, Vec3, Vec4, FloatVec, Texture
        };

        Uniform(RenderPass& rp);
        ~Uniform();

        void setType(Type t) { type = t; }
        Type getType() const { return type; }

        void setName(const juce::String& n) { name = n; }
        juce::String getName() const { return name; }

        /** Assign element associated with this uniform.
            
            This uniform object captures element's shared pointer in order
            to prevent it's accidental deletion.
        */
        void setElement(const Element::Ptr& ptr) { element = ptr; }

        /** Tells whether this uniform is a vector (i.e. not a scalar value). */
        bool isVector() const;

        void setValue(const juce::var& val);
        void setValueFromString(const juce::String& str);
        void setTexture(const juce::String& textureName);

        /** Trigger uniform value update.
        
            This will assing a uniform value asynchronously.
            This method will be called on corresponding element's value change
            on the main thread. The value itself will be assigned on the rendering
            thread on the next render pass.
        */
        void triggerValueUpdate(const juce::var& val);

        /** Apply uniform value to the shader program.

            @note This must be called on the render thread.
        */
        void apply(juce::OpenGLShaderProgram& program);

        /** Return uniform type from a type name. */
        static Type getTypeFromString(const juce::String& str);

    private:

        void updateValueIfNeeded();

        union Value
        {
            int intValue;
            unsigned int textureIDValue;
            float floatValue;
            float vecValue[4];
        };

        RenderPass& renderPass;
        Type type{ Type::Invalid };
        juce::String name{};
        Value value{};
        std::vector<float> floatVec{};

        Element::Ptr element{};

        mutable std::mutex mutex{};
        std::atomic<bool> valueUpdatePending{};
        juce::var valueToUpdate{};
    };

    void populateTextureElement(const Element::Ptr& elem);
    void populateShaderElement(const Element::Ptr& elem);
    void populateShaderUniform(RenderPass& renderPass, const Element::Ptr& elem);
    void populateItems();

    /** Capture the state. */
    void updateState();

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

    State state{};
};

} // namespace vitro
