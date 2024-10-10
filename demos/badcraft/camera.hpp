#pragma once
/*
 * FILE      demos\badcraft\camera.hpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 */

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <gfs/types.h>

struct Camera {
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;

    f32 yaw;
    f32 pitch;

    f32 speed;
    f32 sensitivity;
    f32 fov;
};

Camera CameraMake(void);

void CameraRotate(Camera *camera, f32 xOffset, f32 yOffset);
glm::mat4 CameraGetViewMatix(Camera *camera);
glm::mat4 CameraGetProjectionMatix(Camera *camera, i32 viewportWidth, i32 viewportHeight);
