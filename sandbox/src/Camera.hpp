#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <QObject>

#include <glm/glm.hpp>

#include <rendering/AbstractCamera.hpp>

struct CameraDirectionVectors {
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
};

class Camera : public Rt::AbstractCamera {
    Q_OBJECT;

public:
    glm::vec3 position;
    glm::vec3 euler_angles;

    Camera(float aspect_ratio=1.0f, float fov=45.0f, QObject* parent=nullptr);
    virtual ~Camera() override;

    virtual const glm::vec3& get_position() const override;
    virtual const glm::mat4& get_view() const override;
    virtual const glm::mat4& get_perspective() const override;

    virtual void update_perspective(float new_aspect_ratio=0.0f) override;
    virtual void update_view() override;
    
    virtual CameraDirectionVectors get_camera_direction_vectors() const;

private:
    float aspect_ratio;
    float fov;

    glm::mat4 perspective;
    glm::mat4 view;
};

#endif