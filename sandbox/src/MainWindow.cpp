#include "MainWindow.hpp"
#include <QPushButton>
#include <QApplication>
#include <QKeyEvent>
#include <QDebug>
#include <glm/glm.hpp>
#include <memory>

#include <scene/Mesh.hpp>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    camera_controller(&camera),
    viewport(&camera, &camera_controller)
{
    connect(&viewport, &Viewport::opengl_initialized, this, &MainWindow::initialization);

    camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
    
    setWindowTitle("Raytracer");
    resize(800, 600);

    setCentralWidget(&viewport);

    show();

    connect(&timer, &QTimer::timeout, this, &MainWindow::main_loop);
    timer.start(16);

    scene = nullptr;
}

MainWindow::~MainWindow() {
    delete scene;
}

void MainWindow::main_loop() {
    viewport.main_loop();
}

void MainWindow::initialization(Rt::OpenGLFunctions* gl) {
    std::vector<std::shared_ptr<Rt::Mesh>> static_meshes;

    std::shared_ptr<Rt::Material> face_material = std::make_shared<Rt::Material>("face_material");
    face_material->albedo = glm::vec3(1.0f);
    face_material->texture_paths[0] = "../resources/textures/awesomeface.png";

    std::shared_ptr<Rt::Material> jupiter_material = std::make_shared<Rt::Material>("Jupiter");
    jupiter_material->albedo = glm::vec3(1.0f,1.0f,1.0f);
    jupiter_material->metalness = 1.0f;
    jupiter_material->roughness = 1.0f;
    jupiter_material->texture_paths[0] = "../resources/textures/Metal004_4K-JPG/Metal004_4K_Color";
    jupiter_material->texture_paths[2] = "../resources/textures/Metal004_4K-JPG/Metal004_4K_Roughness";
    jupiter_material->texture_paths[3] = "../resources/textures/Metal004_4K-JPG/Metal004_4K_Metalness";

    std::shared_ptr<Rt::Mesh> mesh = std::make_shared<Rt::Mesh>(
        std::vector<Rt::Vertex>{
            Rt::Vertex(glm::vec4(-1.0f,0.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec2(0.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 1.0f,0.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec2(1.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 0.0f,1.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec2(0.5f, 1.0f))
        },
        std::vector<Rt::Index>{
            0, 1, 2
        }
    );
    mesh->set_material(face_material);
    
    static_meshes.push_back(mesh);
    scene = new Rt::Scene(static_meshes);

    viewport.get_renderer()->set_scene(scene);


    std::shared_ptr<Rt::Mesh> mesh1 = std::make_shared<Rt::Mesh>(
        std::vector<Rt::Vertex>{
            Rt::Vertex(glm::vec4( 2.0f,0.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec2(0.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 4.0f,0.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec2(1.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 4.0f,2.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec2(1.0f, 1.0f)),
            Rt::Vertex(glm::vec4( 2.0f,2.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec2(0.0f, 1.0f))
        },
        std::vector<Rt::Index>{
            0, 1, 2,
            2, 3, 0
        }
    );
    mesh1->set_material(jupiter_material);

    std::shared_ptr<Rt::Mesh> mesh2 = std::make_shared<Rt::Mesh>(
        std::vector<Rt::Vertex>{
            Rt::Vertex(glm::vec4(-2.5f,0.0f,-2.0f, 1.0f), glm::vec4(0.0f,1.0f,0.0f,0.0f), glm::vec2(0.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 2.5f,0.0f,-2.0f, 1.0f), glm::vec4(0.0f,1.0f,0.0f,0.0f), glm::vec2(1.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 0.0f,0.0f, 2.33f,1.0f), glm::vec4(0.0f,1.0f,0.0f,0.0f), glm::vec2(0.5f, 1.0f)),
        },
        std::vector<Rt::Index>{
            0, 1, 2
        }
    );
    mesh2->set_material(jupiter_material);

    scene->add_mesh(mesh1);
    scene->add_mesh(mesh2);
}