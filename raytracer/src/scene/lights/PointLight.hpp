#ifndef RT_POINTLIGHT_HPP
#define RT_POINTLIGHT_HPP

#include "RaytracerGlobals.hpp"
#include "scene/lights/AbstractLight.hpp"

namespace Rt {

    class RAYTRACER_LIB_EXPORT PointLight : public AbstractLight {
        Q_OBJECT;

    public:
        PointLight();
        PointLight(const glm::vec3& pos, const glm::vec3& radiance, float ambient_multiplier=1.0f);
        PointLight(const glm::vec3& pos);
        virtual ~PointLight() {};

        virtual void as_byte_array(unsigned char byte_array[light_size_in_opengl], const glm::mat4& transformation) const override;

    private:
        void init();
    };

}

#endif