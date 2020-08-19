#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

#include <QWidget>

#include <rendering/OpenGLWidget.hpp>
#include <rendering/OpenGLFunctions.hpp>
#include <rendering/Renderer.hpp>
#include "CameraController.hpp"

class Viewport : public QWidget {
    Q_OBJECT;

public:

    Viewport(Camera* camera, CameraController* cam_controller, QWidget* parent=nullptr);
    ~Viewport();

    void main_loop();

    Rt::Renderer* get_renderer();

signals:
    void opengl_initialized(Rt::OpenGLFunctions* gl);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void init();

    void capture_mouse();
    void release_mouse();

    Rt::OpenGLWidget gl_widget;
    Rt::Renderer* renderer;

    bool mouse_captured;
    Camera* camera;
    CameraController* cam_controller;
};

#endif