#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float aspect_ratio, float fov, QObject* parent) :
    Rt::AbstractCamera(parent),
    aspect_ratio(aspect_ratio),
    fov(fov)
{
    position = glm::vec3(0.0f);
    euler_angles = glm::vec3(0.0f);

    perspective = glm::infinitePerspective(fov, aspect_ratio, 0.1f);
}

Camera::~Camera() {}

const glm::vec3& Camera::get_position() const {
    return position;
}

const glm::mat4& Camera::get_view() const {
    return view;
}

const glm::mat4& Camera::get_perspective() const {
    return perspective;
}

void Camera::update_perspective(float new_aspect_ratio) {
    if (new_aspect_ratio != 0.0f) aspect_ratio = new_aspect_ratio;
    perspective = glm::infinitePerspective(fov, aspect_ratio, 0.1f);
}

void Camera::update_view() {
    CameraDirectionVectors cam_vecs = get_camera_direction_vectors();
    view = glm::lookAt(position, position + cam_vecs.front, cam_vecs.up);
}

CameraDirectionVectors Camera::get_camera_direction_vectors() const {
    float yaw = glm::radians(euler_angles[0]);
    float pitch = glm::radians(euler_angles[1]);
    // float roll = glm::radians(euler_angles[2]);

    CameraDirectionVectors cam_vecs;

    cam_vecs.front = glm::vec3(
        sin(yaw) * cos(pitch),
        sin(pitch),
        -cos(yaw) * cos(pitch)
    );

    cam_vecs.up = glm::vec3(0.0f, 1.0f, 0.0);

    cam_vecs.right = glm::normalize(glm::cross(cam_vecs.up, cam_vecs.front));

    return cam_vecs;
}
