#ifndef RT_MATERIAL_HPP
#define RT_MATERIAL_HPP

#include <QObject>
#include <glm/glm.hpp>
#include <string>

#include "RaytracerGlobals.hpp"

namespace Rt {

    constexpr int material_size_in_opengl = 80;

    typedef int32_t TextureIndex;
    typedef uint32_t MaterialIndex;

    class RAYTRACER_LIB_EXPORT Material : public QObject {
        Q_OBJECT;

    public:
        static constexpr unsigned int nr_material_textures = 6;

        // Warning: Names are used to uniquely id materials; make sure to use unique names!
        Material(const std::string& name, QObject* parent=nullptr);

        const std::string& get_name() const;

        // The order of texture indices should match that of texture_paths
        void as_byte_array(unsigned char byte_array[material_size_in_opengl], TextureIndex texture_indices[nr_material_textures]) const;

        glm::vec3 albedo;
        glm::vec3 F0;
        float roughness;
        float metalness;
        float AO;

        // In order, the texture paths should be albedo, F0, roughness, metalness, AO, normal
        std::string texture_paths[nr_material_textures];
    
    private:
        std::string name;
    };

}

#endif