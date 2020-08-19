#ifndef RT_RENDERER_HPP
#define RT_RENDERER_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include "RaytracerGlobals.hpp"

#include "AbstractCamera.hpp"
#include "Shader.hpp"
#include "OpenGLFunctions.hpp"
#include "materials/Texture.hpp"
#include "materials/Material.hpp"
#include "materials/MaterialManager.hpp"
#include "scene/Vertex.hpp"
#include "scene/Node.hpp"
#include "scene/Mesh.hpp"
#include "scene/Scene.hpp"

namespace Rt {

    class RAYTRACER_LIB_EXPORT Renderer : public QObject {
        Q_OBJECT;

    public:
        Renderer(QObject* parent=nullptr);
        ~Renderer();
        void initialize(OpenGLFunctions* gl);

        bool update();

        // Returns true for a successful render
        // and false for an unsuccessful render (render_result will be unchanged)
        bool render(Texture* render_result, unsigned int width, unsigned int height);

        void set_camera(AbstractCamera* new_camera);
        AbstractCamera* get_camera();

        void set_scene(Scene* new_scene);
        Scene* get_scene();

    private:
        OpenGLFunctions* gl;

        AbstractCamera* camera;

        Scene* scene;

        Shader render_shader;
        int work_group_size[3];

        // std::vector<Vertex> vertices;
        // std::vector<unsigned int> indices;
        // unsigned int vertex_ssbo;
        // unsigned int index_ssbo;

        // Note: not a "real" opengl vertex shader; rather, this is a compute
        // shader carrying out the function of a vertex shader
        Shader vertex_shader;
        int vertex_shader_work_group_size[3];
        // This MUST match the Y_SIZE in vertex_shader.glsl
        // See definition there for explanation
        static constexpr int Y_SIZE = 64;

        unsigned int vertex_ssbo;
        unsigned int vertex_ssbo_size;

        unsigned int static_vertex_ssbo;
        unsigned int static_vertex_ssbo_size;
        unsigned int static_index_ssbo;
        unsigned int static_index_ssbo_size;

        unsigned int dynamic_vertex_ssbo;
        unsigned int dynamic_vertex_ssbo_size;
        unsigned int dynamic_index_ssbo;
        unsigned int dynamic_index_ssbo_size;

        std::vector<unsigned char> dynamic_vertices;
        std::vector<Index> dynamic_indices;

        unsigned int mesh_ssbo;
        unsigned int mesh_ssbo_size;

        std::vector<unsigned char> meshes;
        void traverse_node_tree(Node* node, glm::mat4 transformation=glm::mat4(1.0f));

        unsigned int material_ssbo;
        unsigned int material_ssbo_size;

        unsigned int material_texture_array;
        TextureIndex nr_material_textures;
        void update_material_textures();

        unsigned int prev_width;
        unsigned int prev_height;
    };

}

#endif