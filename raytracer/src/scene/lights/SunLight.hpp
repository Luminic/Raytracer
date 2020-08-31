#ifndef RT_SUNLIGHT_HPP
#define RT_SUNLIGHT_HPP

#include <glm/glm.hpp>

#include "RaytracerGlobals.hpp"
#include "scene/lights/AbstractLight.hpp"

namespace Rt {

    class RAYTRACER_LIB_EXPORT SunLight : public AbstractLight {
        Q_OBJECT;

    public:
        SunLight();
        SunLight(const glm::vec3& radiance, float ambient_multiplier=1.0f);
        virtual ~SunLight() {}

        virtual void as_byte_array(unsigned char byte_array[light_size_in_opengl], const glm::mat4& transformation) const override;

    private:
        void init();

    };

}

#endif