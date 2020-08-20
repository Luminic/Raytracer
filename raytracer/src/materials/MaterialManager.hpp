#ifndef RT_MATERIAL_MANAGER
#define RT_MATERIAL_MANAGER

#include <QObject>
#include <vector>
#include <unordered_map>

#include "RaytracerGlobals.hpp"
#include "Material.hpp"
#include "Texture.hpp"

namespace Rt {

    class RAYTRACER_LIB_EXPORT MaterialManager : public QObject {
        Q_OBJECT;

    public:
        MaterialManager(unsigned int texture_width=512, unsigned int texture_height=512, QObject* parent=nullptr);
        virtual ~MaterialManager();

        unsigned int get_texture_width() const;
        unsigned int get_texture_height() const;

        // Adds the material to the material array if not already present
        // Material indices will not change once set
        MaterialIndex get_material_index(const Material* material);

        // Add texture to the texture array if not already in
        // Texture indices will not change once set
        TextureIndex get_texture_index(const std::string& texture_path);

        // Number of bytes in each texture_width*texture_height sized image
        unsigned int bytes_per_image() const;

        const std::vector<unsigned char>& get_materials() const;
        const std::vector<unsigned char>& get_material_textures() const;

    private:
        const unsigned int texture_width;
        const unsigned int texture_height;

        // Should match OpenGL memory layout for materials
        std::vector<unsigned char> materials;
        std::unordered_map<std::string, MaterialIndex> material_name_to_index;

        // Stores the texture data if the texture array needs resizing
        // and the texture information needs to be re-added
        // The every texture should be sized texture_width*texture_height*4*8
        std::vector<unsigned char> material_textures;
        std::unordered_map<std::string, TextureIndex> texture_path_to_index;

    };

}

#endif