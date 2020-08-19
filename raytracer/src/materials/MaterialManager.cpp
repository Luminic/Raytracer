#include "MaterialManager.hpp"
#include <QImage>
#include <QDebug>

namespace Rt {

    MaterialManager::MaterialManager(unsigned int texture_width, unsigned int texture_height, QObject* parent) :
        QObject(parent),
        texture_width(texture_width),
        texture_height(texture_height)
    {
        Material default_material("Rt::default_material");
        materials.resize(material_size_in_opengl);
        TextureIndex default_texture_indices[Material::nr_material_textures];
        for (size_t i=0; i<Material::nr_material_textures; i++) default_texture_indices[i] = -1;
        default_material.as_byte_array(materials.data(), default_texture_indices);
    }

    MaterialManager::~MaterialManager() {}

    unsigned int MaterialManager::get_texture_width() const {
        return texture_width;
    }

    unsigned int MaterialManager::get_texture_height() const {
        return texture_height;
    }

    MaterialIndex MaterialManager::get_material_index(const Material* material) {
        if (!material) return 0;
        auto material_it = material_name_to_index.find(material->get_name());
        if (material_it == material_name_to_index.end()) {
            // Load Material
            TextureIndex texture_indices[Material::nr_material_textures];
            for (unsigned int i=0; i<Material::nr_material_textures; i++)
                texture_indices[i] = get_texture_index(material->texture_paths[i]);
            
            // Add material to material array
            unsigned char material_byte_array[material_size_in_opengl];
            material->as_byte_array(material_byte_array, texture_indices);

            materials.insert(std::end(materials), std::begin(material_byte_array), std::end(material_byte_array));

            // Add index to dictionary
            size_t current_nr_materials = materials.size()/material_size_in_opengl;
            MaterialIndex mat_index = (MaterialIndex) current_nr_materials-1;
            material_name_to_index[material->get_name()] = mat_index;
            return mat_index;
        }
        // Already loaded
        return material_it->second;
    }

    TextureIndex MaterialManager::get_texture_index(const std::string& texture_path) {
        if (texture_path == "") 
            return (TextureIndex) -1;

        auto texture_it = texture_path_to_index.find(texture_path);
        if (texture_it == texture_path_to_index.end()) {
            // Load & Resize Texture
            QImage tex = QImage(texture_path.c_str()).scaled(texture_width, texture_height).mirrored();
            tex = tex.convertToFormat(QImage::Format_RGBA8888);
            const unsigned char* tex_bits = tex.constBits();
            material_textures.insert(std::end(material_textures), tex_bits, tex_bits+bytes_per_image());

            // Add index
            size_t current_nr_textures = material_textures.size()/bytes_per_image();
            TextureIndex tex_index = current_nr_textures-1;
            texture_path_to_index[texture_path] = tex_index;
            return tex_index;
        }
        // Already loaded
        return texture_it->second;
    }

    unsigned int MaterialManager::bytes_per_image() const {
        // 4 channels, 1 byte per channel
        return texture_width*texture_height*4;
    }

    const std::vector<unsigned char>& MaterialManager::get_materials() const {
        return materials;
    }
    const std::vector<unsigned char>& MaterialManager::get_material_textures() const {
        return material_textures;
    }

}
