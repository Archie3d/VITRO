namespace vitro {

JSClassID OpenGLView::jsClassID = 0;

const Identifier OpenGLView::tag("OpenGLView");

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


OpenGLView::RenderPass::RenderPass(OpenGLContext& oglContext)
    : openGLContext{ oglContext },
      shader(openGLContext)
{
}

//==============================================================================

OpenGLView::OpenGLView(Context& ctx)
    : ComponentElement(OpenGLView::tag, ctx),
      program(openGLContext)
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

    auto ok = program.addVertexShader(vertexShader);
    if (!ok) {
        DBG("Vertex shader error: " << program.getLastError());
    }

    ok = program.addFragmentShader(fragmentShader);
    if (!ok) {
        DBG("Fragment shader error: " << program.getLastError());
    }

    ok = program.link();
    if (!ok) {
        DBG("Failed to link shader: " << program.getLastError());
    }
}

void OpenGLView::renderOpenGL()
{
    using namespace juce::gl;

    openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadBuffers[0]);
    openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, quadBuffers[1]);

    program.use();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void OpenGLView::openGLContextClosing()
{
    using namespace juce::gl;

    // @todo Here we should clean all the uniforms

    openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
    openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    openGLContext.extensions.glDeleteBuffers(2, quadBuffers);
}

void OpenGLView::timerCallback()
{
    repaint();
}

} // namespace vitro
