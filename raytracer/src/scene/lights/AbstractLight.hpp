#ifndef RT_ABSTRACT_LIGHT_HPP
#define RT_ABSTRACT_LIGHT_HPP

#include "RaytracerGlobals.hpp"
#include "scene/Node.hpp"

namespace Rt {

    constexpr int light_size_in_opengl = 48;

    class RAYTRACER_LIB_EXPORT AbstractLight : public Node {
        Q_OBJECT;

    public:
        AbstractLight();
        virtual ~AbstractLight() {}

        enum Visibility : int32_t {
            INVISIBLE = 0,
            SPHERE = 1
        };
        Q_ENUM(Visibility);

        enum LightType : int32_t {
            SUNLIGHT = 0,
            POINTLIGHT = 1,
            DIRLIGHT = 2, // Currently unimplemented
            UNKNOWN = -1
        };
        Q_ENUM(LightType);
        LightType get_light_type() const;

        // Transformation should already be multiplied by the Light's Node
        // transformation matrix
        virtual void as_byte_array(unsigned char byte_array[light_size_in_opengl], const glm::mat4& parent_transformation) const = 0;

        virtual void set_radiance(const glm::vec3& new_radiance);
        virtual const glm::vec3& get_radiance() const;

        virtual void set_ambient_multiplier(float new_ambient_multiplier);
        virtual float get_ambient_multiplier() const;

        virtual void set_visibility(Visibility new_visibility);
        virtual Visibility get_visibility() const;

    signals:
        void radiance_changed(const glm::vec3&);
        void ambient_multiplier_changed(float);
        void visibility_changed(Visibility);

    protected:
        LightType light_type;
    
    private:
        glm::vec3 radiance;
        float ambient_multiplier;
        Visibility visibility;
    };

}

#endif