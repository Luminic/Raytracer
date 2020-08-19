#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <QObject>

#include <glm/glm.hpp>

#include <rendering/AbstractCamera.hpp>

class Camera : public Rt::AbstractCamera {
    Q_OBJECT;

public:
    glm::vec3 position;
    glm::vec3 euler_angles;

    Camera(float aspect_ratio=1.0f, float fov=45.0f, QObject* parent=nullptr);
    virtual ~Camera() override;

    virtual glm::vec3 get_position() const override;

    virtual void update_perspective(float new_aspect_ratio=0.0f) override;
    virtual void update_view() override;
    
    virtual Rt::CameraDirectionVectors get_camera_direction_vectors() const override;
    virtual Rt::CornerRays get_corner_rays() const override;

private:
    float aspect_ratio;
    float fov;

    glm::mat4 perspective;
    glm::mat4 view;
};

#endif