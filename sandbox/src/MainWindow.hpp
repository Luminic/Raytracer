#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QTimer>

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

    QTimer timer;

    Camera camera;
    CameraController camera_controller;
    Viewport viewport;
};

#endif
