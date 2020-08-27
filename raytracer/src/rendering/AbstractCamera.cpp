#include "AbstractCamera.hpp"

namespace Rt {

    AbstractCamera::AbstractCamera(QObject* parent) : QObject(parent) {}

    CornerRays AbstractCamera::get_corner_rays() {
        Rt::CornerRays corner_rays;

        const glm::vec3& position = get_position();
        const glm::mat4& view = get_view();
        const glm::mat4& perspective = get_perspective();

        glm::mat4 inv_view_persp = glm::inverse(perspective*view);
        glm::vec4 tmp;

        tmp = inv_view_persp * glm::vec4(-1, -1, 0, 1);
        corner_rays.r00 = glm::vec3(tmp)/tmp.w - position;

        tmp = inv_view_persp * glm::vec4( 1, -1, 0, 1);
        corner_rays.r10 = glm::vec3(tmp)/tmp.w - position;

        tmp = inv_view_persp * glm::vec4(-1,  1, 0, 1);
        corner_rays.r01 = glm::vec3(tmp)/tmp.w - position;

        tmp = inv_view_persp * glm::vec4( 1,  1, 0, 1);
        corner_rays.r11 = glm::vec3(tmp)/tmp.w - position;

        return corner_rays;
    }

}

