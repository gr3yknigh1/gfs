/*
 * FILE      demos\badcraft\camera.cpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 */
#include "camera.hpp"

#include <gfs/macros.h>

Camera
CameraMake(void)
{
    Camera camera = INIT_EMPTY_STRUCT(Camera);

    camera.position = {0, 0, 3.0f};
    camera.front = {0, 0, -1.0f};
    camera.up = {0, 1.0f, 0};

    camera.yaw = -90.0f;
    camera.pitch = 0.0f;

    camera.speed = 10.0f;
    camera.sensitivity = 0.5f;
    camera.fov = 45.0f;

    camera.near = 0.1f;
    camera.far = 100.0f;

    return camera;
}

void
CameraRotate(Camera *camera, f32 xOffset, f32 yOffset)
{
    xOffset *= camera->sensitivity;
    yOffset *= camera->sensitivity;

    camera->yaw += xOffset * 1;
    camera->pitch += yOffset * -1;

    camera->pitch = glm::clamp(camera->pitch, -89.0f, 89.0f);

    f32 yawRad = glm::radians(camera->yaw);
    f32 pitchRad = glm::radians(camera->pitch);

    glm::vec3 direction = LITERAL(glm::vec3){
        glm::cos(yawRad) * glm::cos(pitchRad),
        glm::sin(pitchRad),
        glm::sin(yawRad) * glm::cos(pitchRad),
    };
    camera->front = glm::normalize(direction);
}

glm::mat4
CameraGetViewMatix(Camera *camera)
{
    return glm::lookAt(
        camera->position, camera->position + camera->front, camera->up);
}

glm::mat4
CameraGetProjectionMatix(Camera *camera, i32 viewportWidth, i32 viewportHeight)
{
    return glm::perspective(
        glm::radians(camera->fov),
        static_cast<f32>(viewportWidth) / static_cast<f32>(viewportHeight),
        camera->near, camera->far);
}
