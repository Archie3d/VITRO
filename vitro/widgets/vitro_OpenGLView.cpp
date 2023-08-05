namespace vitro {

JSClassID OpenGLView::jsClassID = 0;

const Identifier OpenGLView::tag("OpenGLView");
const Identifier OpenGLView::tagTexture("texture");
const Identifier OpenGLView::tagShader("shader");
const Identifier OpenGLView::tagUniform("uniform");

//==============================================================================

const static String vertexShader = R"(
#version 330 core
layout (location = 0) in vec3 pos;

uniform vec2 iResolution;
uniform vec2 iMouse;

out vec2 pixelPos;
out vec2 fragCoord;

void main()
{
    pixelPos = 0.5f * vec2(pos.x + 1.0f, pos.y + 1.0f) * iResolution.xy;
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}
)";

//==============================================================================

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

void OpenGLView::RenderPass::addUniform(std::unique_ptr<Uniform>&& uniform)
{
    jassert(uniform != nullptr);
    uniforms.push_back(std::move(uniform));
}

void OpenGLView::RenderPass::render()
{
    using namespace juce::gl;

    if (targetFrameBuffer != nullptr)
        targetFrameBuffer->makeCurrentRenderingTarget();

    program.use();

    applyDefaultUniforms();
    applyUniforms();

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    if (targetFrameBuffer != nullptr)
        targetFrameBuffer->releaseAsRenderingTarget();
}

void OpenGLView::RenderPass::applyDefaultUniforms()
{
    auto& state{ openGLView.getState() };

    state.lock();
    const auto screenBounds{ state.screenBounds };
    const auto mouse{ state.mouse };
    const auto frame{ state.frame };
    const auto timeDelta{ state.timeDelta };
    state.unlock();

    float w{ screenBounds.getWidth() };
    float h{ screenBounds.getHeight() };

    program.setUniform(
        "iViewResolution",
        w > 0 ? w : 1.0f,
        h > 0 ? h : 1.0f
    );

    if (targetFrameBuffer != nullptr) {
        w = targetFrameBuffer->getWidth();
        h = targetFrameBuffer->getHeight();
    }

    program.setUniform(
        "iResolution",
        w > 0 ? w : 1.0f,
        h > 0 ? h : 1.0f
    );

    program.setUniform("iFrame", frame);
    program.setUniform("iTimeDelta", timeDelta);
    program.setUniform("iTime", frame * timeDelta);

    program.setUniform(
        "iMouse",
        mouse.getX(), screenBounds.getHeight() - mouse.getY()
    );
}

void OpenGLView::RenderPass::applyUniforms()
{
    for (auto&& uniform : uniforms)
        uniform->apply(program);
}

//==============================================================================

OpenGLView::Uniform::Uniform(OpenGLView::RenderPass& rp)
    : renderPass{ rp }
{
}

OpenGLView::Uniform::~Uniform()
{
    if (element != nullptr) {
        element->setUpdateHook({});
        element.reset();
    }
}

bool OpenGLView::Uniform::isVector() const
{
    return type == Type::Vec2 || type == Type::Vec3 || type == Type::Vec4 || type == Type::FloatVec;
}

void OpenGLView::Uniform::setValue(const juce::var& val)
{
    if (val.isString()) {
        setValueFromString(val.toString());
        return;
    }

    switch (type) {
    case Type::Invalid:
        break;
    case Type::Int:
        value.intValue = (int)val;
        break;
    case Type::Float:
        value.floatValue = (float)val;
        break;
    case Type::Vec2:
        if (auto* arr{ val.getArray() }) {
            for (int i = 0; i < jmin(2, arr->size()); ++i)
                value.vecValue[i] = (float)arr->getReference(i);
        }
        break;
    case Type::Vec3:
        if (auto* arr{ val.getArray() }) {
            for (int i = 0; i < jmin(3, arr->size()); ++i)
                value.vecValue[i] = (float)arr->getReference(i);
        }
        break;
    case Type::Vec4:
        if (auto* arr{ val.getArray() }) {
            for (int i = 0; i < jmin(4, arr->size()); ++i)
                value.vecValue[i] = (float)arr->getReference(i);
        }
        break;
    case Type::FloatVec:
        if (auto* arr{ val.getArray() }) {
            floatVec.resize((size_t)arr->size());
            for (int i = 0; i < arr->size(); ++i)
                floatVec[(size_t)i] = (float)arr->getReference(i);
        }
        break;
    case Type::Texture:
        setTexture(val.toString());
        jassertfalse;
        break;
    default:
        jassertfalse;
        break;
    }
}

void OpenGLView::Uniform::setValueFromString(const juce::String& str)
{
    StringArray sa{};

    if (isVector())
        sa = StringArray::fromTokens(str, ",", "\"'");

    switch (type) {
    case Type::Invalid:
        break;
    case Type::Int:
        value.intValue = str.getIntValue();
        break;
    case Type::Float:
        value.floatValue = str.getFloatValue();
        break;
    case Type::Vec2:
        for (int i = 0; i < jmin(2, sa.size()); ++i)
            value.vecValue[i] = sa[i].getFloatValue();
        break;
    case Type::Vec3:
        for (int i = 0; i < jmin(3, sa.size()); ++i)
            value.vecValue[i] = sa[i].getFloatValue();
        break;
    case Type::Vec4:
        for (int i = 0; i < jmin(4, sa.size()); ++i)
            value.vecValue[i] = sa[i].getFloatValue();
        break;
    case Type::FloatVec:
        floatVec.resize((size_t)sa.size());
        for (int i = 0; i < sa.size(); ++i)
            floatVec[(size_t)i] = sa[i].getFloatValue();
        break;
    case Type::Texture:
        setTexture(str);
        break;
    default:
        jassertfalse;
        break;
    }
}

void OpenGLView::Uniform::setTexture(const juce::String& textureName)
{
    jassert(type == Type::Texture);
    value.textureIDValue = renderPass.getOpenGLView().findTextureID(textureName);
}

void OpenGLView::Uniform::triggerValueUpdate(const juce::var& val)
{
    std::scoped_lock lock(mutex);
    valueToUpdate = val;
    valueUpdatePending = true;
}

OpenGLView::Uniform::Type OpenGLView::Uniform::getTypeFromString(const String& str)
{
    const String s{ str.trim().toLowerCase() };

    if (s == "int") return Type::Int;
    if (s == "float") return Type::Float;
    if (s == "vec2") return Type::Vec2;
    if (s == "vec3") return Type::Vec3;
    if (s == "vec4") return Type::Vec4;
    if (s == "float_vec") return Type::FloatVec;
    if (s == "texture") return Type::Texture;

    return Type::Invalid;
}

void OpenGLView::Uniform::updateValueIfNeeded()
{
    std::scoped_lock lock(mutex);
    if (valueUpdatePending) {
        setValue(valueToUpdate);
        valueUpdatePending = false;
    }
}

void OpenGLView::Uniform::apply(juce::OpenGLShaderProgram& program)
{
    updateValueIfNeeded();

    const char* cName{ name.toRawUTF8() };

    switch (type) {
    case Type::Invalid:
        break;
    case Type::Int:
        program.setUniform(cName, value.intValue);
        break;
    case Type::Float:
        program.setUniform(cName, value.floatValue);
        break;
    case Type::Vec2:
        program.setUniform(cName, value.vecValue[0], value.vecValue[1]);
        break;
    case Type::Vec3:
        program.setUniform(cName, value.vecValue[0], value.vecValue[1], value.vecValue[2]);
        break;
    case Type::Vec4:
        program.setUniform(cName, value.vecValue[0], value.vecValue[1], value.vecValue[2], value.vecValue[3]);
        break;
    case Type::FloatVec:
        program.setUniform(cName, floatVec.data(), (GLsizei)floatVec.size());
        break;
    case Type::Texture:
        program.setUniform(cName, value.intValue);
        break;
    default:
        jassertfalse;
        break;
    }
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
    openGLContext.setComponentPaintingEnabled(true); // Paint child components as well
    openGLContext.setContinuousRepainting(false);
    openGLContext.attachTo(*this);
}

void OpenGLView::populateTextureElement(const Element::Ptr& elem)
{
    jassert(elem);
    jassert(elem->getTag() == tagTexture);

    const String name{ elem->getAttribute(attr::name).toString() };

    if (name.isEmpty()) {
        DBG("Texture name attribute cannot be empty");
        return;
    }

    const String src{ elem->getAttribute(attr::src).toString() };

    if (src.isEmpty()) {
        DBG("Texture src attribute cannot be empty");
        return;
    }

    auto image{ context.getLoader().loadImage(src) };

    if (!image.isValid()) {
        DBG("Unable to load texture '" << name << "' image from " << src);
        return;
    }

    if (!OpenGLTexture::isValidSize(image.getWidth(), image.getHeight())) {
        DBG("Texture '" << name << "' image size (" << image.getWidth() << ", " << image.getHeight() << ") is invalid");
        return;
    }

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

    const int width{ (int)elem->getAttribute(attr::width) };
    const int height{ (int)elem->getAttribute(attr::height) };

    if (width > 0 && height > 0)
        pass->setTargetFrameBuffer(width, height);

    auto& renderPassRef{ *pass };

    if (pass->isValid())
        renderPasses.push_back(std::move(pass));

    elem->forEachChild([this, &renderPassRef](const Element::Ptr& subElem) {
        const auto tag{ subElem->getTag() };

        if (tag == tagUniform) {
            populateShaderUniform(renderPassRef, subElem);
        }
        // Ignore other elements
    }, false);
}

void OpenGLView::populateShaderUniform(RenderPass& renderPass, const Element::Ptr& elem)
{
    jassert(elem);
    jassert(elem->getTag() == tagUniform);

    const String name{ elem->getAttribute(attr::name).toString() };

    if (name.isEmpty()) {
        DBG("uniform name attribute cannot be empty.");
        return;
    }

    const String type{ elem->getAttribute(attr::type).toString() };

    if (type.isEmpty()) {
        DBG("uniform type attribute cannot be empty.");
        return;
    }

    const Uniform::Type uniformType{ Uniform::getTypeFromString(type) };

    if (uniformType == Uniform::Type::Invalid) {
        DBG("uniform type '" << type << "' is invalid.");
        return;
    }

    auto uniform{ std::make_unique<Uniform>(renderPass) };
    uniform->setName(name);
    uniform->setType(uniformType);

    if (const auto& val{ elem->getAttribute(attr::value) }; !val.isVoid())
        uniform->setValue(val);

    uniform->setElement(elem);

    elem->setUpdateHook([u = uniform.get()](const Element::Ptr& el) {
        if (const auto&& [changed, val]{ el->getAttributeChanged(attr::value) }; changed) {
            u->triggerValueUpdate(val);
        }
    });

    renderPass.addUniform(std::move(uniform));
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

void OpenGLView::updateState()
{
    state.lock();
    ++state.frame;
    state.mouse = getMouseXYRelative().toFloat();
    state.screenBounds = getScreenBounds().toFloat();
    state.unlock();
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

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    openGLContext.extensions.glGenBuffers(2, quadBuffers);
    openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadBuffers[0]);
    openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, quadBuffers[1]);
    openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    openGLContext.extensions.glEnableVertexAttribArray(0);
    openGLContext.extensions.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

unsigned int OpenGLView::findTextureID(const String& name) const
{
    if (const auto it{ textures.find(name) }; it != textures.end())
        return it->second->getTextureID();

    for (auto&& pass : renderPasses) {
        if (pass->hasTargetFrameBuffer() && pass->getName() == name)
            return pass->getTextureID();
    }

    DBG("Unable to find texture " << name);
    return 0;
}

void OpenGLView::resized()
{
    updateState();
}

void OpenGLView::update()
{
    ComponentElement::update();

    if (const auto&& [changed, val]{ getAttributeChanged("fps") }; changed) {
        int fps{ (int)val };

        state.lock();

        if (fps > 0) {
            state.timeDelta = 1.0f / (float)fps;
            startTimerHz(fps);
        } else {
            stopTimer();
            state.timeDelta = 0.0f;
            state.frame = 0;
        }

        state.unlock();
    }
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

    glEnable(GL_DEBUG_OUTPUT);

    glBindVertexArray(VAO);

    // Binding textures
    for (auto&& [name, texture] : textures) {
        glActiveTexture(GL_TEXTURE0 + texture->getTextureID());
        texture->bind();
    }

    // Bind frame buffers
    for (auto&& pass : renderPasses) {
        if (pass->hasTargetFrameBuffer()) {
            const auto textureID{ pass->getTextureID() };
            glActiveTexture(GL_TEXTURE0 + textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
        }
    }

    for (auto&& pass : renderPasses) {
        if (pass->isValid())
            pass->render();
    }

    // Unbinf VAO and textures
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Disable debug output in JUCE part, otherwise there is a flood
    // of debug messages on buggers reactivation.
    glDisable(GL_DEBUG_OUTPUT);
}

void OpenGLView::openGLContextClosing()
{
    using namespace juce::gl;
    
    openGLContext.extensions.glBindVertexArray(0);
    openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
    openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    openGLContext.extensions.glDeleteBuffers(2, quadBuffers);

    renderPasses.clear();
    textures.clear();
}

void OpenGLView::timerCallback()
{
    updateState();
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
