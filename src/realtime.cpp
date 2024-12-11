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
    this->scene.free();

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1);
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
    glErrorCheck(glEnable(GL_BLEND));
    glErrorCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    glErrorCheck(glBlendEquation(GL_FUNC_ADD));

    // Initialize all shaders, set any textures they use to the used texture slot
    this->phongShader = ShaderLoader::createShaderProgram("resources/shaders/phong.vert", "resources/shaders/phong.frag");
    glErrorCheck();
    glErrorCheck(glUseProgram(this->phongShader));
    glErrorCheck(glUniform1i(glGetUniformLocation(this->phongShader, "texture"), 0));
    glErrorCheck(glUseProgram(0));

    scene.initScene();
}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    // Render main image
    glErrorCheck(glViewport(0, 0, this->screenWidth, this->screenHeight));

    // Clear screen color and depth before painting
    glErrorCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Activate shader program
    glErrorCheck(glUseProgram(this->phongShader));

    this->scene.bindSceneUniforms(this->phongShader);
    glErrorCheck(glUniform1i(glGetUniformLocation(this->phongShader, "textureMapEnabled"), settings.textureMappingEnabled));

    for(const std::unique_ptr<Primitive>& primitive : this->scene.getPrimitives())
        primitive->draw(this->phongShader);

    // Deactivate shader program
    glErrorCheck(glUseProgram(0));
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    this->screenWidth = round(size().width() * m_devicePixelRatio);
    this->screenHeight = round(size().height() * m_devicePixelRatio);
    glErrorCheck(glViewport(0, 0, this->screenWidth, this->screenHeight));

    // Students: anything requiring OpenGL calls when the window resizes should be done here
    this->scene.getCamera().updateAspectRatio((float) w / h);
}

void Realtime::resetScene() {
    this->makeCurrent();
    this->scene.resetScene();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::addObstacle() {
    this->makeCurrent();
    this->scene.addObstacle();
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
    this->scene.updateScene();

    update(); // asks for a PaintGL() call to occur
}
