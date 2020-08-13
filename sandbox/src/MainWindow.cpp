#include "MainWindow.hpp"
#include <QPushButton>
#include <QApplication>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    camera_controller(&camera),
    viewport(&camera, &camera_controller)
{
    camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
    
    setWindowTitle("Raytracer");
    resize(800, 600);

    setCentralWidget(&viewport);

    show();

    connect(&timer, &QTimer::timeout, this, &MainWindow::main_loop);
    timer.start(16);
}

MainWindow::~MainWindow() {}

void MainWindow::main_loop() {
    viewport.main_loop();
}