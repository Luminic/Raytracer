#include "Scene.hpp"
#include <glm/glm.hpp>

namespace Rt {

    Scene::Scene() {}

    Scene::Scene(const std::vector<std::shared_ptr<Mesh>>& static_meshes) {
        for (auto mesh : static_meshes) {
            Index vertex_offset = static_vertices.size()/vertex_size_in_opengl;
            Index index_offset = static_indices.size();
            // Add static indices
            const std::vector<Index>& mesh_indices = mesh->get_indices();
            static_indices.insert(std::end(static_indices), std::begin(mesh_indices), std::end(mesh_indices));

            // Add static vertices
            const std::vector<Vertex>& mesh_vertices = mesh->get_vertices();
            for (auto vert : mesh_vertices) {
                unsigned char vertex_bytes[vertex_size_in_opengl];
                vert.as_byte_array(vertex_bytes);
                static_vertices.insert(std::end(static_vertices), vertex_bytes, vertex_bytes+vertex_size_in_opengl);
            }

            // Add static meshes
            unsigned char mesh_bytes[mesh_size_in_opengl];
            MaterialIndex material_index = material_manager.get_material_index(mesh->get_material().get());
            mesh->as_byte_array(mesh_bytes, glm::mat4(1.0f), vertex_offset, index_offset, material_index);
            this->static_meshes.insert(std::end(this->static_meshes), mesh_bytes, mesh_bytes+mesh_size_in_opengl);
        }
    }

    Scene::~Scene() {}

    MaterialManager& Scene::get_material_manager() {
        return material_manager;
    }

    const std::vector<unsigned char>& Scene::get_static_vertices() const {
        return static_vertices;
    }

    const std::vector<Index>& Scene::get_static_indices() const {
        return static_indices;
    }

    const std::vector<unsigned char>& Scene::get_static_meshes() const {
        return static_meshes;
    }

}