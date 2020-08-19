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

    struct CameraDirectionVectors {
        glm::vec3 front;
        glm::vec3 right;
        glm::vec3 up;
    };


    class RAYTRACER_LIB_EXPORT AbstractCamera : public QObject {
        Q_OBJECT;

    public:
        AbstractCamera(QObject* parent=nullptr) : QObject(parent) {}
        virtual ~AbstractCamera() {};

        virtual glm::vec3 get_position() const = 0;

        virtual void update_perspective(float new_aspect_ratio=0.0f) = 0;
        virtual void update_view() = 0;
        
        virtual CameraDirectionVectors get_camera_direction_vectors() const = 0;
        virtual CornerRays get_corner_rays() const = 0;
    };

}

#endif