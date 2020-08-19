#include "Material.hpp"

namespace Rt {
    Material::Material(const std::string& name, QObject* parent) :
        QObject(parent),
        name(name)
    {
        albedo = glm::vec3(1.0f,0.0f,1.0f);
        F0 = glm::vec3(0.04f);
        roughness = 0.5f;
        metalness = 0.0f;
        AO = 0.1f;
    }

    const std::string& Material::get_name() const {
        return name;
    }

    void Material::as_byte_array(unsigned char byte_array[material_size_in_opengl], TextureIndex texture_indices[nr_material_textures]) const {
        unsigned char const* tmp = reinterpret_cast<unsigned char const*>(&albedo);
        std::copy(tmp, tmp+12, byte_array);

        tmp = reinterpret_cast<unsigned char const*>(&F0);
        std::copy(tmp, tmp+12, byte_array+16);

        tmp = reinterpret_cast<unsigned char const*>(&roughness);
        std::copy(tmp, tmp+4, byte_array+32);

        tmp = reinterpret_cast<unsigned char const*>(&metalness);
        std::copy(tmp, tmp+4, byte_array+36);

        tmp = reinterpret_cast<unsigned char const*>(&AO);
        std::copy(tmp, tmp+4, byte_array+40);

        tmp = reinterpret_cast<unsigned char const*>(&texture_indices[0]);
        std::copy(tmp, tmp+4, byte_array+44);

        tmp = reinterpret_cast<unsigned char const*>(&texture_indices[1]);
        std::copy(tmp, tmp+4, byte_array+48);

        tmp = reinterpret_cast<unsigned char const*>(&texture_indices[2]);
        std::copy(tmp, tmp+4, byte_array+52);

        tmp = reinterpret_cast<unsigned char const*>(&texture_indices[3]);
        std::copy(tmp, tmp+4, byte_array+56);

        tmp = reinterpret_cast<unsigned char const*>(&texture_indices[4]);
        std::copy(tmp, tmp+4, byte_array+60);
    }
}