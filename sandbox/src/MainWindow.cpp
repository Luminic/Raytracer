#include "MainWindow.hpp"
#include <QPushButton>
#include <QDockWidget>
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

    QDockWidget* scene_hierarchy_container = new QDockWidget(this);
    scene_hierarchy = new Rt::SceneHierarchy(scene_hierarchy_container);
    scene_hierarchy_container->setWidget(scene_hierarchy);

    QDockWidget* properties_container = new QDockWidget(this);
    properties = new Rt::Properties(properties_container);
    properties_container->setWidget(properties);

    connect(scene_hierarchy, &Rt::SceneHierarchy::node_selected, properties, &Rt::Properties::node_selected);

    addDockWidget(Qt::RightDockWidgetArea, scene_hierarchy_container);
    addDockWidget(Qt::RightDockWidgetArea, properties_container);

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
    face_material->texture_paths[0] = "resources/textures/awesomeface.png";

    std::shared_ptr<Rt::Material> metal_material = std::make_shared<Rt::Material>("Jupiter");
    metal_material->albedo = glm::vec3(1.0f,0.2f,0.2f);
    metal_material->metalness = 1.0f;
    metal_material->roughness = 1.0f;
    metal_material->texture_paths[0] = "resources/textures/Metal004_4K-JPG/Metal004_4K_Color";
    metal_material->texture_paths[2] = "resources/textures/Metal004_4K-JPG/Metal004_4K_Roughness";
    metal_material->texture_paths[3] = "resources/textures/Metal004_4K-JPG/Metal004_4K_Metalness";
    metal_material->texture_paths[5] = "resources/textures/Metal004_4K-JPG/Metal004_4K_Normal";

    std::shared_ptr<Rt::Material> brick_material = std::make_shared<Rt::Material>("brick_material");
    brick_material->albedo = glm::vec3(1.0f,1.0f,1.0f);
    brick_material->metalness = 0.0f;
    brick_material->roughness = 1.0f;
    brick_material->texture_paths[0] = "resources/textures/Bricks/church_bricks_02_diff_png_4k.jpg";
    brick_material->texture_paths[2] = "resources/textures/Bricks/church_bricks_02_rough_4k";
    brick_material->texture_paths[4] = "resources/textures/Bricks/church_bricks_02_ao_4k";
    brick_material->texture_paths[5] = "resources/textures/Bricks/church_bricks_02_nor_4k";
    
    std::shared_ptr<Rt::Material> floor_material = std::make_shared<Rt::Material>("floor_material");
    floor_material->albedo = glm::vec3(1.0f,1.0f,1.0f);
    floor_material->metalness = 0.0f;
    floor_material->roughness = 1.0f;
    floor_material->texture_paths[0] = "resources/textures/Floor/floor_tiles_02_diff_1k.jpg";
    floor_material->texture_paths[2] = "resources/textures/Floor/floor_tiles_02_rough_1k.jpg";
    floor_material->texture_paths[4] = "resources/textures/Floor/floor_tiles_02_ao_1k.jpg";
    floor_material->texture_paths[5] = "resources/textures/Floor/floor_tiles_02_nor_1k.jpg";

    std::shared_ptr<Rt::Mesh> mesh = std::make_shared<Rt::Mesh>(
        std::vector<Rt::Vertex>{
            Rt::Vertex(glm::vec4(-1.0f,0.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec4(1.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,1.0f,0.0f,0.0f), glm::vec2(0.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 1.0f,0.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec4(1.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,1.0f,0.0f,0.0f), glm::vec2(1.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 0.0f,1.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec4(1.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,1.0f,0.0f,0.0f), glm::vec2(0.5f, 1.0f))
        },
        std::vector<Rt::Index>{
            0, 1, 2
        }
    );
    mesh->set_material(brick_material);
    
    static_meshes.push_back(mesh);
    scene = new Rt::Scene(static_meshes);
    scene_hierarchy->add_scene(scene);

    viewport.get_renderer()->set_scene(scene);


    std::shared_ptr<Rt::Mesh> mesh1 = std::make_shared<Rt::Mesh>(
        std::vector<Rt::Vertex>{
            Rt::Vertex(glm::vec4(-1.0f,-1.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec4(1.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,1.0f,0.0f,0.0f), glm::vec2(0.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 1.0f,-1.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec4(1.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,1.0f,0.0f,0.0f), glm::vec2(1.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 1.0f, 1.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec4(1.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,1.0f,0.0f,0.0f), glm::vec2(1.0f, 1.0f)),
            Rt::Vertex(glm::vec4(-1.0f, 1.0f,0.0f,1.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec4(1.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,1.0f,0.0f,0.0f), glm::vec2(0.0f, 1.0f))
        },
        std::vector<Rt::Index>{
            0, 1, 2,
            2, 3, 0
        }
    );
    mesh1->set_material(metal_material);

    std::shared_ptr<Rt::Node> node1 = std::make_shared<Rt::Node>(mesh1);
    node1->set_translation(glm::vec3(2.0f,0.0f,0.0f));
    node1->set_rotation(glm::vec3(3.14f, 1.507f, 0.0f));

    std::shared_ptr<Rt::Mesh> mesh2 = std::make_shared<Rt::Mesh>(
        std::vector<Rt::Vertex>{
            Rt::Vertex(glm::vec4(-2.5f,0.0f,-2.0f, 1.0f), glm::vec4(0.0f,-1.0f,0.0f,0.0f), glm::vec4(1.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec2(0.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 2.5f,0.0f,-2.0f, 1.0f), glm::vec4(0.0f,-1.0f,0.0f,0.0f), glm::vec4(1.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec2(1.0f, 0.0f)),
            Rt::Vertex(glm::vec4( 0.0f,0.0f, 2.33f,1.0f), glm::vec4(0.0f,-1.0f,0.0f,0.0f), glm::vec4(1.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,1.0f,0.0f), glm::vec2(0.5f, 1.0f)),
        },
        std::vector<Rt::Index>{
            0, 1, 2
        }
    );
    mesh2->set_material(floor_material);

    scene->add_node(node1);
    scene->add_mesh(mesh2);
}