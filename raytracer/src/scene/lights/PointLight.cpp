#include "PointLight.hpp"

namespace Rt {

    PointLight::PointLight() {
        init();
    }

    PointLight::PointLight(const glm::vec3& pos, const glm::vec3& radiance, float ambient_multiplier) {
        set_translation(pos);
        set_radiance(radiance);
        set_ambient_multiplier(ambient_multiplier);
        init();
    }

    PointLight::PointLight(const glm::vec3& pos) {
        set_translation(pos);
        init();
    }

    void PointLight::init() {
        light_type = AbstractLight::LightType::POINTLIGHT;
        set_visibility(AbstractLight::Visibility::SPHERE);
        setObjectName("PointLight");
    }

    void PointLight::as_byte_array(unsigned char byte_array[light_size_in_opengl], const glm::mat4& transformation) const {
        // Decompose the transformation matrix
        glm::vec3 position = glm::vec3(transformation[3]);

        const unsigned char* tmp = reinterpret_cast<unsigned char const*>(&position);
        std::copy(tmp, tmp+12, byte_array);

        tmp = reinterpret_cast<unsigned char const*>(&light_type);
        std::copy(tmp, tmp+4, byte_array+12);

        // PointLights have no concept of direction

        AbstractLight::Visibility visibility = get_visibility();
        tmp = reinterpret_cast<unsigned char const*>(&visibility);
        std::copy(tmp, tmp+4, byte_array+28);

        tmp = reinterpret_cast<unsigned char const*>(&get_radiance());
        std::copy(tmp, tmp+12, byte_array+32);

        float ambient_multiplier = get_ambient_multiplier();
        tmp = reinterpret_cast<unsigned char const*>(&ambient_multiplier);
        std::copy(tmp, tmp+4, byte_array+44);
    }

}