#include "Mesh.hpp"
#include <QDebug>

namespace Rt {

    Mesh::Mesh(std::shared_ptr<Material> material) : material(material) {
        setObjectName("Mesh");
    }

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, std::shared_ptr<Material> material) :
        material(material),
        vertices(vertices),
        indices(indices)
    {
        setObjectName("Mesh");
    }

    Mesh::~Mesh() {}

    void Mesh::set_material(std::shared_ptr<Material> new_material) {
        material = new_material;
    }

    std::shared_ptr<Material> Mesh::get_material() {
        return material;
    }

    const std::vector<Vertex>& Mesh::get_vertices() const {
        return vertices;
    }

    void Mesh::insert_vertices(const std::vector<Vertex>& new_vertices, size_t location) {
        vertices.insert(std::begin(vertices)+location, std::begin(new_vertices), std::end(new_vertices));
    }

    void Mesh::erase_vertices(size_t first, size_t last) {
        vertices.erase(std::begin(vertices)+first, std::begin(vertices)+last);
    }


    const std::vector<Index>& Mesh::get_indices() const {
        return indices;
    }

    void Mesh::insert_indices(const std::vector<Index>& new_indices, size_t location) {
        indices.insert(std::begin(indices)+location, std::begin(new_indices), std::end(new_indices));
    }

    void Mesh::erase_indices(size_t first, size_t last) {
        indices.erase(std::begin(indices)+first, std::begin(indices)+last);
    }


    void Mesh::as_byte_array(unsigned char byte_array[mesh_size_in_opengl], const glm::mat4& transformation, Index vertex_offset, Index index_offset, MaterialIndex material_index) const {
        unsigned char const* tmp = reinterpret_cast<unsigned char const*>(&transformation);
        std::copy(tmp, tmp+64, byte_array);

        tmp = reinterpret_cast<unsigned char const*>(&vertex_offset);
        std::copy(tmp, tmp+4, byte_array+64);

        tmp = reinterpret_cast<unsigned char const*>(&index_offset);
        std::copy(tmp, tmp+4, byte_array+68);

        Index nr_indices = indices.size();
        tmp = reinterpret_cast<unsigned char const*>(&nr_indices);
        std::copy(tmp, tmp+4, byte_array+72);

        tmp = reinterpret_cast<unsigned char const*>(&material_index);
        std::copy(tmp, tmp+4, byte_array+76);
    }

}