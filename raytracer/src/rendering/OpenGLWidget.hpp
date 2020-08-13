#ifndef OPENGL_WIDGET_HPP
#define OPENGL_WIDGET_HPP

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>

#include "RaytracerGlobals.hpp"

#include <vector>

#include "Renderer.hpp"
#include "Shader.hpp"
#include "materials/Texture.hpp"

namespace Rt {

    class RAYTRACER_LIB_EXPORT OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core {
        Q_OBJECT;

    public:
        OpenGLWidget(QWidget* parent=nullptr);
        ~OpenGLWidget();

        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;

        void main_loop();

        Renderer* get_renderer();

    private:
        unsigned int frame_vbo;
        unsigned int frame_vao;
        Shader frame_shader;

        Renderer renderer;
        Texture* render_result;
    };

}

#endif