#pragma once

#define GLM_FORCE_RADIANS
#include "includeGL.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

class CameraGL {
    protected:
        bool require_computation;
        glm::vec3 eye, look_at, up;
        double aspect, fov, z_near, z_far;
        glm::mat4 view, projection, camera;

        void compute_projection() {
            projection = glm::perspective(fov, aspect, z_near, z_far);
            require_computation = true;
        }

        void compute_view() {
            view = glm::lookAt(eye, look_at, up);
            require_computation = true;
        }

        void compute_matrix() {
            camera = projection * view;
            require_computation = false;
        }

    public:
        CameraGL(): eye(1.0f, 1.0f, 1.0f), look_at(0.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f) {
            compute_view();
            project(60.0f, 1.0f, 0.1f, 50.0f);
        }

        void look(const glm::vec3 &v_eye, const glm::vec3 &v_look_at, const glm::vec3 &v_up) {
            eye = v_eye;
            look_at = v_look_at;
            up = v_up;
            compute_view();
        }

        void project(float v_fov, float v_aspect, float v_near, float v_far) {
            fov = v_fov;
            aspect = v_aspect;
            z_near = v_near;
            z_far = v_far;
            compute_projection();
        }

        void set_position(const glm::vec3 &v_eye) {
            eye = v_eye;
            compute_view();
        }

        void set_look(const glm::vec3 &v_look_at) {
            look_at = v_look_at;
            compute_view();
        }

        glm::mat4 get_matrix() {
            if (require_computation)
                compute_matrix();
            return camera;
        }
};