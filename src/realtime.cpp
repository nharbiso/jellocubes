#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "utils/shaderloader.h"

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    glErrorCheck(glDeleteProgram(this->phongShader));
    this->scene.getPrimitives().clear(); // remove previous primitives and free any underlying memory
    this->scene.getLights().clear();

    glErrorCheck(glDeleteProgram(this->pixelFilterShader));
    glErrorCheck(glDeleteProgram(this->kernelFilterShader));
    glErrorCheck(glDeleteProgram(this->sobelCombineShader));
    glErrorCheck(glDeleteVertexArrays(1, &this->fullscreenVAO));
    glErrorCheck(glDeleteBuffers(1, &this->fullscreenVBO));
    glErrorCheck(glDeleteTextures(this->numFBOs, this->outputTextures));
    glErrorCheck(glDeleteRenderbuffers(this->numFBOs, this->outputRenderbuffers));
    glErrorCheck(glDeleteFramebuffers(this->numFBOs, this->outputFBOs));

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    this->screenWidth = round(size().width() * m_devicePixelRatio);
    this->screenHeight = round(size().height() * m_devicePixelRatio);
    glViewport(0, 0, this->screenWidth, this->screenHeight);

    // Students: anything requiring OpenGL calls when the program starts should be done here

    // Initialize all shaders, set any textures they use to the used texture slot
    this->phongShader = ShaderLoader::createShaderProgram("resources/shaders/phong.vert", "resources/shaders/phong.frag");
    glErrorCheck();
    glErrorCheck(glUseProgram(this->phongShader));
    glErrorCheck(glUniform1i(glGetUniformLocation(this->phongShader, "texture"), 0));
    glErrorCheck(glUseProgram(0));

    this->shadowMapShader = ShaderLoader::createShaderProgram("resources/shaders/shadowmap.vert", "resources/shaders/shadowmap.frag");
    glErrorCheck();
    glErrorCheck(glUseProgram(0));

    this->pixelFilterShader = ShaderLoader::createShaderProgram("resources/shaders/postprocess.vert", "resources/shaders/pixelfilter.frag");
    glErrorCheck();
    glErrorCheck(glUseProgram(this->pixelFilterShader));
    glErrorCheck(glUniform1i(glGetUniformLocation(this->pixelFilterShader, "outputImage"), 0));
    glErrorCheck(glUseProgram(0));

    this->kernelFilterShader = ShaderLoader::createShaderProgram("resources/shaders/postprocess.vert", "resources/shaders/kernelfilter.frag");
    glErrorCheck();
    glErrorCheck(glUseProgram(this->kernelFilterShader));
    glErrorCheck(glUniform1i(glGetUniformLocation(this->kernelFilterShader, "outputImage"), 0));
    glErrorCheck(glUseProgram(0));

    this->sobelCombineShader = ShaderLoader::createShaderProgram("resources/shaders/postprocess.vert", "resources/shaders/sobelcombine.frag");
    glErrorCheck();
    glErrorCheck(glUseProgram(this->sobelCombineShader));
    glErrorCheck(glUniform1i(glGetUniformLocation(this->sobelCombineShader, "outputXImage"), 0));
    glErrorCheck(glUniform1i(glGetUniformLocation(this->sobelCombineShader, "outputYImage"), 1));
    glErrorCheck(glUseProgram(0));

    std::vector<GLfloat> fullscreen_quad_data = {
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f
    };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glErrorCheck(glGenBuffers(1, &this->fullscreenVBO));
    glErrorCheck(glBindBuffer(GL_ARRAY_BUFFER, this->fullscreenVBO));
    glErrorCheck(glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW));
    glErrorCheck(glGenVertexArrays(1, &this->fullscreenVAO));
    glErrorCheck(glBindVertexArray(this->fullscreenVAO));

    // Add attributes to VAO
    glErrorCheck(glEnableVertexAttribArray(0));
    glErrorCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr));
    glErrorCheck(glEnableVertexAttribArray(1));
    glErrorCheck(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat))));

    // Unbind the fullscreen quad's VBO and VAO
    glErrorCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
    glErrorCheck(glBindVertexArray(0));

    this->makeFullscreenFBOs();
}

void Realtime::makeFullscreenFBOs() {
    for(int i = 0; i < this->numFBOs; i++) {
        // Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
        glErrorCheck(glGenTextures(1, &this->outputTextures[i]));
        glErrorCheck(glActiveTexture(GL_TEXTURE0));
        glErrorCheck(glBindTexture(GL_TEXTURE_2D, this->outputTextures[i]));

        glErrorCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->screenWidth, this->screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        glErrorCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        glErrorCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        glErrorCheck(glBindTexture(GL_TEXTURE_2D, 0));

        // Generate and bind a renderbuffer of the right size, set its format, then unbind
        glErrorCheck(glGenRenderbuffers(1, &this->outputRenderbuffers[i]));
        glErrorCheck(glBindRenderbuffer(GL_RENDERBUFFER, this->outputRenderbuffers[i]));
        glErrorCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->screenWidth, this->screenHeight));
        glErrorCheck(glBindRenderbuffer(GL_RENDERBUFFER, 0));

        // Generate and bind FBO
        glErrorCheck(glGenFramebuffers(1, &this->outputFBOs[i]));
        glErrorCheck(glBindFramebuffer(GL_FRAMEBUFFER, this->outputFBOs[i]));

        // Add texture as a color attachment, and renderbuffer as a depth+stencil attachment, to FBO
        glErrorCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->outputTextures[i], 0));
        glErrorCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->outputRenderbuffers[i]));

        // Unbind the FBO
        glErrorCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here

    // Find number of single-pass post-processing effects and their arguments
    std::vector<std::tuple<GLuint, int, std::function<void()>>> postProcessArgs;
    if(settings.perPixelFilter1 || settings.perPixelFilter2 || settings.perPixelFilter3) {
        std::function<void()> bindUniforms = std::bind(&Realtime::bindPixelFilters, this);
        postProcessArgs.push_back({this->pixelFilterShader, 1, bindUniforms});
    }
    if(settings.kernelBasedFilter1) {
        std::function<void()> bindUniforms = std::bind(&Realtime::bindKernelFilter, this, this->sharpenKernel, 3);
        postProcessArgs.push_back({this->kernelFilterShader, 1, bindUniforms});
    }
    if(settings.kernelBasedFilter2) {
        std::function<void()> bindUniforms = std::bind(&Realtime::bindKernelFilter, this, this->blurKernel, 5);
        postProcessArgs.push_back({this->kernelFilterShader, 1, bindUniforms});
    }
    int numEffects = postProcessArgs.size() + settings.kernelBasedFilter3;
    int inputInd = 0;
    int outputInd = (postProcessArgs.size() % 2 == 0) ? 0 : 1;

    // Render main image
    // Bind output FBO (input for post-processing)
    GLuint outputFBO = (numEffects == 0) ? this->defaultFBO : this->outputFBOs[outputInd];
    glErrorCheck(glBindFramebuffer(GL_FRAMEBUFFER, outputFBO));
    glErrorCheck(glViewport(0, 0, this->screenWidth, this->screenHeight));

    // Clear screen color and depth before painting
    glErrorCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Activate shader program
    glErrorCheck(glUseProgram(this->phongShader));

    this->scene.bindSceneUniforms(this->phongShader);
    glErrorCheck(glUniform1i(glGetUniformLocation(this->phongShader, "textureMapEnabled"), settings.extraCredit3));
    glErrorCheck(glUniform1i(glGetUniformLocation(this->phongShader, "shadowMapEnabled"), settings.extraCredit4));

    for(const std::unique_ptr<Primitive>& primitive : this->scene.getPrimitives())
        primitive->draw(this->phongShader, false);

    // Deactivate shader program
    glErrorCheck(glUseProgram(0));


    // Apply single-pass post-processing effects
    int numApplied = 0;
    for(auto& [shader, numInputTextures, bindUniforms] : postProcessArgs) {
        inputInd = outputInd;
        outputInd = (inputInd + 1) % 2;
        outputFBO = (numApplied == numEffects - 1) ? this->defaultFBO : this->outputFBOs[outputInd];
        this->postProcessFilter(shader, &this->outputTextures[inputInd], numInputTextures, outputFBO, bindUniforms);
        numApplied++;
    }

    // Apply multi-pass post-processing effects
    if(settings.kernelBasedFilter3) {
        // Due to configuration of input/output FBOs, last render outputted to index 0
        std::function<void()> bindXUniforms = std::bind(&Realtime::bindKernelFilter, this, this->sobelXKernel, 3);
        this->postProcessFilter(this->kernelFilterShader, &this->outputTextures[0], 1, this->outputFBOs[1], bindXUniforms);
        std::function<void()> bindYUniforms = std::bind(&Realtime::bindKernelFilter, this, this->sobelYKernel, 3);
        this->postProcessFilter(this->kernelFilterShader, &this->outputTextures[0], 1, this->outputFBOs[2], bindYUniforms);

        this->postProcessFilter(this->sobelCombineShader, &this->outputTextures[1], 2, this->defaultFBO, [](){});
    }
}

void Realtime::postProcessFilter(GLuint shader, GLuint* inputTextures, int numInputTextures,
                                 GLuint outputFBO, std::function<void()> bindUniforms) {
    // Bind to output FBO, render output with post processing
    glErrorCheck(glBindFramebuffer(GL_FRAMEBUFFER, outputFBO));
    glErrorCheck(glViewport(0, 0, this->screenWidth, this->screenHeight));
    glErrorCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Bind shader, set any post-processing uniforms
    glErrorCheck(glUseProgram(shader));
    bindUniforms();

    // Bind fullscreen VAO and input texture(s)
    glErrorCheck(glBindVertexArray(this->fullscreenVAO));
    for(int i = 0; i < numInputTextures; i++) {
        glErrorCheck(glActiveTexture(this->textureMacros[i]));
        glErrorCheck(glBindTexture(GL_TEXTURE_2D, inputTextures[i]));
    }

    // Draw fullscreen vertices
    glErrorCheck(glDrawArrays(GL_TRIANGLES, 0, 6));

    // Unbind texture(s), VAO, and shader
    for(int i = 0; i < numInputTextures; i++) {
        glErrorCheck(glActiveTexture(this->textureMacros[i]));
        glErrorCheck(glBindTexture(GL_TEXTURE_2D, 0));
    }
    glErrorCheck(glBindVertexArray(0));
    glErrorCheck(glUseProgram(0));
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    this->screenWidth = round(size().width() * m_devicePixelRatio);
    this->screenHeight = round(size().height() * m_devicePixelRatio);
    glErrorCheck(glViewport(0, 0, this->screenWidth, this->screenHeight));

    glErrorCheck(glDeleteTextures(this->numFBOs, this->outputTextures));
    glErrorCheck(glDeleteRenderbuffers(this->numFBOs, this->outputRenderbuffers));
    glErrorCheck(glDeleteFramebuffers(this->numFBOs, this->outputFBOs));

    // Regenerate FBO
    this->makeFullscreenFBOs();

    // Students: anything requiring OpenGL calls when the window resizes should be done here
    this->scene.getCamera().updateAspectRatio((float) w / h);
}

void Realtime::sceneChanged() {
    this->makeCurrent();
    TessellationParams tslParams = {
        .param1 = settings.shapeParameter1,
        .param2 = settings.shapeParameter2,
        .limitByNum = settings.extraCredit1,
        .numPrimitives = scene.getPrimitives().size(),
        .limitByDist = settings.extraCredit2,
        .cameraPos = scene.getCamera().getPosition(),
        .nearPlane = settings.nearPlane,
        .farPlane = settings.farPlane
    };
    this->scene.updateScene(settings.sceneFilePath, tslParams, this->shadowMapShader);
    // this->defaultFBO = this->numFBOs + this->scene.getLights().size() + 1;
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    this->makeCurrent();
    this->scene.getCamera().updatePlanes(settings.nearPlane, settings.farPlane);

    TessellationParams tslParams = {
        .param1 = settings.shapeParameter1,
        .param2 = settings.shapeParameter2,
        .limitByNum = settings.extraCredit1,
        .numPrimitives = scene.getPrimitives().size(),
        .limitByDist = settings.extraCredit2,
        .cameraPos = scene.getCamera().getPosition(),
        .nearPlane = settings.nearPlane,
        .farPlane = settings.farPlane
    };
    for(const std::unique_ptr<Primitive>& primitive : this->scene.getPrimitives())
        primitive->updateParams(tslParams);

    update(); // asks for a PaintGL() call to occur
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        Camera& camera = this->scene.getCamera();
        camera.rotateCamera(glm::vec4(0, 1, 0, 0), M_PI * deltaX / 1000);
        camera.rotateCamera(camera.getRight(), M_PI * deltaY / 1000);

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    Camera& camera = this->scene.getCamera();
    const float speed = 5; // units/s
    glm::vec4 oldPos = camera.getPosition();
    if(m_keyMap[Qt::Key_W]) {
        oldPos += camera.getLook() * speed * deltaTime;
    }
    if(m_keyMap[Qt::Key_S]) {
        oldPos += -camera.getLook() * speed * deltaTime;
    }
    if(m_keyMap[Qt::Key_A]) {
        oldPos += -camera.getRight() * speed * deltaTime;
    }
    if(m_keyMap[Qt::Key_D]) {
        oldPos += camera.getRight() * speed * deltaTime;
    }
    if(m_keyMap[Qt::Key_Space]) {
        oldPos += glm::vec4(0, 1, 0, 0) * speed * deltaTime;
    }
    if(m_keyMap[Qt::Key_Control]) {
        oldPos += glm::vec4(0, -1, 0, 0) * speed * deltaTime;
    }
    camera.updatePos(oldPos);

    this->makeCurrent();
    TessellationParams tslParams = {
        .param1 = settings.shapeParameter1,
        .param2 = settings.shapeParameter2,
        .limitByNum = settings.extraCredit1,
        .numPrimitives = scene.getPrimitives().size(),
        .limitByDist = settings.extraCredit2,
        .cameraPos = scene.getCamera().getPosition(),
        .nearPlane = settings.nearPlane,
        .farPlane = settings.farPlane
    };
    for(const std::unique_ptr<Primitive>& primitive : this->scene.getPrimitives())
        primitive->updateParams(tslParams);

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
