namespace vitro {

JSClassID OpenGLView::jsClassID = 0;

const Identifier OpenGLView::tag("OpenGLView");
const Identifier OpenGLView::tagTexture("texture");
const Identifier OpenGLView::tagShader("shader");

//==============================================================================

const static String vertexShader = R"(
#version 330 core
layout (location = 0) in vec3 pos;

void main()
{
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}
)";

const static String fragmentShader = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} 
)";


OpenGLView::RenderPass::RenderPass(OpenGLView& oglView, const String& passName)
    : openGLView{ oglView },
      name{ passName },
      program(openGLView.getOpenGLContext())
{
}

void OpenGLView::RenderPass::setShader(const juce::String& code)
{
    auto ok{ program.addVertexShader(vertexShader) };

    if (!ok) {
        DBG("Render pass vertex shader error: " << program.getLastError());
        valid = false;
        return;
    }

    ok = program.addFragmentShader(code);

    if (!ok) {
        DBG("Render pass fragment shader error: " << program.getLastError());
        valid = false;
        return;
    }

    ok = program.link();

    if (!ok) {
        DBG("Render pass shader link error: " << program.getLastError());
        valid = false;
        return;
    }

    valid = true;
}

void OpenGLView::RenderPass::setTargetFrameBuffer(int width, int height)
{
    targetFrameBuffer = std::make_unique<OpenGLFrameBuffer>();

    auto ok{ targetFrameBuffer->initialise(openGLView.getOpenGLContext(), width, height) };

    if (!ok) {
        DBG("Render pass framebuffer initialisation failed");
        targetFrameBuffer.reset();
        valid = false;
    }

    valid = true;
}

bool OpenGLView::RenderPass::hasTargetFrameBuffer() const
{
    return targetFrameBuffer != nullptr;
}

unsigned int OpenGLView::RenderPass::getTextureID() const
{
    return targetFrameBuffer != nullptr ? targetFrameBuffer->getTextureID() : 0;
}

void OpenGLView::RenderPass::render()
{
    using namespace juce::gl;

    if (targetFrameBuffer != nullptr)
        targetFrameBuffer->makeCurrentRenderingTarget();

    program.use();

    // @todo Assign program uniforms here

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    if (targetFrameBuffer != nullptr)
        targetFrameBuffer->releaseAsRenderingTarget();
}

//==============================================================================

OpenGLView::OpenGLView(Context& ctx)
    : ComponentElement(OpenGLView::tag, ctx)
{
    openGLContext.setOpenGLVersionRequired(
#ifdef __APPLE__
        OpenGLContext::openGL4_1
#else
        OpenGLContext::openGL4_3
#endif
    );

    openGLContext.setRenderer(this);
    openGLContext.setComponentPaintingEnabled(false);
    openGLContext.attachTo(*this);
}

void OpenGLView::populateTextureElement(const Element::Ptr& elem)
{
    jassert(elem);
    jassert(elem->getTag() == tagTexture);

    const String name{ elem->getAttribute(attr::name).toString() };

    if (name.isEmpty())
        return;

    const String src{ elem->getAttribute(attr::src).toString() };

    if (src.isEmpty())
        return;

    auto image{ context.getLoader().loadImage(src) };

    if (!image.isValid())
        return;

    if (image.isValid()) {
        auto texture{ std::make_unique<juce::OpenGLTexture>() };
        texture->loadImage(image);
        textures.insert(std::make_pair(name, std::move(texture)));
    }
}

void OpenGLView::populateShaderElement(const Element::Ptr& elem)
{
    jassert(elem);
    jassert(elem->getTag() == tagShader);

    const String src{ elem->getAttribute(attr::src).toString() };

    if (src.isEmpty())
        return;

    const String shaderCode{ context.getLoader().loadText(src) };

    if (shaderCode.isEmpty())
        return;

    const String name{ elem->getAttribute(attr::name).toString() };

    auto pass{ std::make_unique<RenderPass>(*this, name) };
    pass->setShader(shaderCode);

    if (pass->isValid())
        renderPasses.push_back(std::move(pass));
}


void OpenGLView::populateItems()
{
    textures.clear();
    renderPasses.clear();

    forEachChild([this](const Element::Ptr& elem) {
        const auto tag{ elem->getTag() };

        if (tag == tagTexture) {
            populateTextureElement(elem);
        } else if (tag == tagShader) {
            populateShaderElement(elem);
        }
        // Ignore other elements
    }, false);
}

void OpenGLView::newOpenGLContextCreated()
{
    using namespace juce::gl;

    // 1|----|2
    //  |   /|
    //  |  / |
    //  | /  |
    // 0|/___|3
    const static float vertices[] = {
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f 
    };

    const static GLushort indices[6] = { 0, 1, 2, 0, 2, 3 };

    openGLContext.extensions.glGenBuffers(2, quadBuffers);
    openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadBuffers[0]);
    openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, quadBuffers[1]);
    openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    openGLContext.extensions.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    openGLContext.extensions.glEnableVertexAttribArray(0); 

    // @todo Stubber render pass
    auto pass = std::make_unique<RenderPass>(*this, "final");
    pass->setShader(fragmentShader);
    renderPasses.push_back(std::move(pass));
}

unsigned int OpenGLView::findTextureID(const String& name) const
{
    if (const auto it{ textures.find(name) }; it != textures.end())
        return it->second->getTextureID();

    for (auto&& pass : renderPasses) {
        if (pass->hasTargetFrameBuffer() && pass->getName() == name)
            return pass->getTextureID();
    }

    return 0;
}

void OpenGLView::update()
{
    ComponentElement::update();
}


void OpenGLView::renderOpenGL()
{
    using namespace juce::gl;

    {
        std::scoped_lock lock{ updateMutex };
    
        if (itemsUpdatePending) {
            populateItems();
            itemsUpdatePending = false;
        }
    }

    openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadBuffers[0]);
    openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, quadBuffers[1]);

    for (auto&& pass : renderPasses) {
        if (pass->isValid())
            pass->render();
    }
}

void OpenGLView::openGLContextClosing()
{
    using namespace juce::gl;

    // @todo Here we should clean all the uniforms

    openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
    openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    openGLContext.extensions.glDeleteBuffers(2, quadBuffers);

    renderPasses.clear();
    textures.clear();
}

void OpenGLView::timerCallback()
{
    openGLContext.triggerRepaint();
}

void OpenGLView::valueTreeChildAdded(juce::ValueTree& tree, juce::ValueTree& child)
{
    ComponentElement::valueTreeChildAdded(tree, child);
    
    std::scoped_lock lock{ updateMutex };
    itemsUpdatePending = true;
}

void OpenGLView::valueTreeChildRemoved(juce::ValueTree& tree, juce::ValueTree& child, int idx)
{
    ComponentElement::valueTreeChildRemoved(tree, child, idx);
    
    std::scoped_lock lock{ updateMutex };
    itemsUpdatePending = true;
}

} // namespace vitro
