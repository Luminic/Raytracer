#include "OpenGLWidget.hpp"

#include <QDebug>
#include <QOpenGLDebugLogger>


uint32_t round_up_to_pow_2(uint32_t x) {
    /*
    In C++20 we can use:
        #include <bit>
        std::bit_ceil(x)
    */
    // Current implementation from https://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}


OpenGLWidget::OpenGLWidget(QWidget* parent) : QOpenGLWidget(parent) {
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    setFormat(format);
}

OpenGLWidget::~OpenGLWidget() {
    glDeleteVertexArrays(1, &frame_vao);
    glDeleteBuffers(1, &frame_vbo);
}

void OpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();

    #ifdef QT_DEBUG
        QOpenGLContext* ctx = QOpenGLContext::currentContext();
        QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
        if (!logger->initialize()) {
            qWarning("QOpenGLDebugLogger failed to initialize.");
        }
        if (!ctx->hasExtension(QByteArrayLiteral("GL_KHR_debug"))) {
            qWarning("KHR Debug extension unavailable.");
        }

        connect(logger, &QOpenGLDebugLogger::messageLogged, this,
            [](const QOpenGLDebugMessage& message){
                if (message.severity() == QOpenGLDebugMessage::HighSeverity) {
                    qCritical(message.message().toLatin1().constData());
                }
                else if (message.severity() != QOpenGLDebugMessage::NotificationSeverity) {
                    qWarning(message.message().toLatin1().constData());
                }
            }
        );
        logger->startLogging();
    #endif

    qDebug() << "GL Version:" << QString((const char*)glGetString(GL_VERSION));

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_DEBUG_OUTPUT);
    glDisable(GL_DEPTH_TEST); // OpenGL's default depth testing isn't useful when using compute shaders for raytracing
    glClearColor(0.0, 0.0, 0.0, 1.0);


    // Create the frame
    float vertices[] = {
        // Top left triangle
        -1.0f,  1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
        // Bottom left triangle
        -1.0f, -1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f
    };

    glGenVertexArrays(1, &frame_vao);
    glBindVertexArray(frame_vao);

    glGenBuffers(1, &frame_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, frame_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Setup the render shader
    ShaderStage comp_shaders[] = {
        ShaderStage{GL_COMPUTE_SHADER, "rendering/shaders/raytrace.glsl"}
    };

    render_shader.load_shaders(comp_shaders, 1);
    render_shader.validate();

    glGetProgramiv(render_shader.get_id(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);
    render_result.create(width(), height());

    // Setup the frame shader to draw the render to the screen
    ShaderStage shaders[] = {
        ShaderStage{GL_VERTEX_SHADER, "rendering/shaders/framebuffer_vs.glsl"},
        ShaderStage{GL_FRAGMENT_SHADER, "rendering/shaders/framebuffer_fs.glsl"}
    };

    frame_shader.load_shaders(shaders, 2);
    frame_shader.validate();
}

void OpenGLWidget::resizeGL(int w, int h) {
    camera.update_perspective_matrix(float(w)/h);
    render_result.resize(width(), height());
}

void OpenGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Render
    render_shader.use();
    render_shader.set_vec3("eye", glm::vec3(0.0f));
    render_shader.set_vec3("ray00", glm::vec3(0.0f));
    render_shader.set_vec3("ray10", glm::vec3(0.0f));
    render_shader.set_vec3("ray01", glm::vec3(0.0f));
    render_shader.set_vec3("ray11", glm::vec3(0.0f));

    glBindImageTexture(0, render_result.get_id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    unsigned int worksize_x = round_up_to_pow_2(width());
    unsigned int worksize_y = round_up_to_pow_2(height());
    glDispatchCompute(worksize_x/work_group_size[0], worksize_y/work_group_size[1], 1);

    // Clean up & make sure the shader has finished writing to the image
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Draw the render result to the screen
    frame_shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_result.get_id());
    frame_shader.set_int("render", 0);
    glBindVertexArray(frame_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

Camera* OpenGLWidget::get_camera() {
    return &camera;
}