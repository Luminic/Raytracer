#ifndef RT_ABSTRACT_CAMERA_HPP
#define RT_ABSTRACT_CAMERA_HPP

#include <QObject>

#include <glm/glm.hpp>

#include "RaytracerGlobals.hpp"

namespace Rt {

    struct CornerRays {
        glm::vec3 r00;
        glm::vec3 r10;
        glm::vec3 r01;
        glm::vec3 r11;
    };

    class RAYTRACER_LIB_EXPORT AbstractCamera : public QObject {
        Q_OBJECT;

    public:
        AbstractCamera(QObject* parent=nullptr);
        virtual ~AbstractCamera() {};

        virtual const glm::vec3& get_position() const = 0;
        virtual const glm::mat4& get_view() const = 0;
        virtual const glm::mat4& get_perspective() const = 0;

        virtual void update_perspective(float new_aspect_ratio=0.0f) = 0;
        virtual void update_view() = 0;
        
        virtual CornerRays get_corner_rays();
    };

}

#endif