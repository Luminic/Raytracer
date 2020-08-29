#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QTimer>

#include <scene/Scene.hpp>
#include <rendering/OpenGLFunctions.hpp>
#include <settings/SceneHierarchy.hpp>
#include <settings/Properties.hpp>

#include "Viewport.hpp"
#include "Camera.hpp"
#include "CameraController.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT;

public:
    MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private:
    void main_loop();

    void initialization(Rt::OpenGLFunctions* gl);

    QTimer timer;

    Camera camera;
    Rt::Scene* scene;

    CameraController camera_controller;
    Viewport viewport;

    Rt::SceneHierarchy* scene_hierarchy;
    Rt::Properties* properties;
};

#endif
