#ifndef RT_SCENE_HPP
#define RT_SCENE_HPP

#include <QObject>

#include <vector>

#include "RaytracerGlobals.hpp"
#include "scene/Vertex.hpp"
#include "scene/Mesh.hpp"
#include "scene/Node.hpp"
#include "materials/Material.hpp"
#include "materials/MaterialManager.hpp"

namespace Rt {

    // Warning: While Scene is a Node, using Scene as a child node
    // is *strongly* discouraged
    class RAYTRACER_LIB_EXPORT Scene : public Node {
        Q_OBJECT;

    public:
        Scene();
        Scene(const std::vector<std::shared_ptr<Mesh>>& static_meshes);
        virtual ~Scene() override;

        MaterialManager& get_material_manager();

        const std::vector<unsigned char>& get_static_vertices() const;
        const std::vector<Index>& get_static_indices() const;
        const std::vector<unsigned char>& get_static_meshes() const;

    private:
        MaterialManager material_manager;

        // Should match OpenGL memory layout
        std::vector<unsigned char> static_vertices;
        std::vector<Index> static_indices;
        std::vector<unsigned char> static_meshes;

    };

}

#endif