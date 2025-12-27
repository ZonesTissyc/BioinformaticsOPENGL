#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
class Projection
{
private:
    float FOV;
    float near, far;
    int Width, Height;
    glm::mat4 ProjectionMat;
public:
    Projection(float fov, float near_toset, float far_toset, float width, float height) {
        FOV = fov;
        near = near_toset;
        far = far_toset;
        Width = width;
        Height = height;
        ProjectionMat = getProjection();
    }
    void setFOV(float fov) {
        FOV = fov;
    }
    void setNear(float near_toset) {
        near = near_toset;
    }
    void setFar(float far_toset) {
        far = far_toset;
    }
    void setWidth(int Width_toset) {
        Width = Width_toset;
    }
    void setHeight(int Height_toset) {
        Height = Height_toset;
    }
    float getFOV() {
        return FOV;
    }
    float getNear() {
        return near;
    }
    float getFar() {
        return far;
    }
    int getWidth() {
        return Width;
    }
    int getHeight() {
        return Height;
    }
    glm::mat4 getProjection();
    glm::mat4 getProjectionMat() {
        return ProjectionMat;
    }
};