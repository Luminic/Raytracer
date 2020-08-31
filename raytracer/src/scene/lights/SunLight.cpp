#include "SunLight.hpp"

namespace Rt {

    SunLight::SunLight() {
        init();
    }

    SunLight::SunLight(const glm::vec3& radiance, float ambient_multiplier) {
        init();
        set_radiance(radiance);
        set_ambient_multiplier(ambient_multiplier);
    }

    void SunLight::init() {
        light_type = AbstractLight::LightType::SUNLIGHT;
        set_visibility(AbstractLight::Visibility::INVISIBLE);
        setObjectName("SunLight");
    }

    void SunLight::as_byte_array(unsigned char byte_array[light_size_in_opengl], const glm::mat4& transformation) const {
        // Decompose the transformation matrix
        glm::vec3 position = glm::vec3(transformation[3]);
        glm::vec3 direction = glm::normalize(glm::mat3(transformation) * glm::vec3(0.0f,-1.0f,0.0f));

        const unsigned char* tmp = reinterpret_cast<unsigned char const*>(&position);
        std::copy(tmp, tmp+12, byte_array);

        tmp = reinterpret_cast<unsigned char const*>(&light_type);
        std::copy(tmp, tmp+4, byte_array+12);

        tmp = reinterpret_cast<unsigned char const*>(&direction);
        std::copy(tmp, tmp+12, byte_array+16);

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