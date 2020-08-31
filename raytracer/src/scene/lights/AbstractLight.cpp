#include "AbstractLight.hpp"

namespace Rt {

    AbstractLight::AbstractLight() {
        node_type = Node::NodeType::LIGHT;
        light_type = LightType::UNKNOWN;
        setObjectName("Light");

        radiance = glm::vec3(1.0f);
        ambient_multiplier = 1.0f;
        visibility = AbstractLight::Visibility::SPHERE;
    }

    AbstractLight::LightType AbstractLight::get_light_type() const {
        return light_type;
    }

    void AbstractLight::set_radiance(const glm::vec3& new_radiance) {
        radiance = new_radiance;
        emit radiance_changed(radiance);
    }

    const glm::vec3& AbstractLight::get_radiance() const {
        return radiance;
    }

    void AbstractLight::set_ambient_multiplier(float new_ambient_multiplier) {
        ambient_multiplier = new_ambient_multiplier;
        emit ambient_multiplier_changed(ambient_multiplier);
    }

    float AbstractLight::get_ambient_multiplier() const {
        return ambient_multiplier;
    }

    void AbstractLight::set_visibility(AbstractLight::Visibility new_visibility) {
        visibility = new_visibility;
        emit visibility_changed(visibility);
    }


    AbstractLight::Visibility AbstractLight::get_visibility() const {
        return visibility;
    }

}