#ifndef RT_MESH_HPP
#define RT_MESH_HPP

#include <QObject>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "RaytracerGlobals.hpp"
#include "scene/Vertex.hpp"
#include "materials/Material.hpp"

namespace Rt {

    constexpr int mesh_size_in_opengl = 80;

    typedef uint32_t MeshIndex;

    class RAYTRACER_LIB_EXPORT Mesh : public QObject {
        Q_OBJECT;

    public:
        Mesh(std::shared_ptr<Material> material=nullptr);
        Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, std::shared_ptr<Material> material=nullptr);
        virtual ~Mesh();

        virtual void set_material(std::shared_ptr<Material> new_material);
        virtual std::shared_ptr<Material> get_material();

        virtual const std::vector<Vertex>& get_vertices() const;
        virtual void insert_vertices(const std::vector<Vertex>& new_vertices, size_t location);
        virtual void erase_vertices(size_t first, size_t last);

        virtual const std::vector<Index>& get_indices() const;
        virtual void insert_indices(const std::vector<Index>& new_indices, size_t location);
        virtual void erase_indices(size_t first, size_t last);

        virtual void as_byte_array(unsigned char byte_array[mesh_size_in_opengl], const glm::mat4& transformation, Index vertex_offset, Index index_offset, MaterialIndex material_index) const;
    
    private:
        std::shared_ptr<Material> material;

        std::vector<Vertex> vertices;
        std::vector<Index> indices;
    };

}

#endif