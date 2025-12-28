#include <custom/ModelTrans.h>


void ModelTrans::translate(const glm::vec3& translation) {
    modelMatrix = glm::translate(modelMatrix, translation);
}


void ModelTrans::rotate(float angle, const glm::vec3& axis) {
    float rad = glm::radians (angle);
    modelMatrix = glm::rotate(modelMatrix,rad ,axis);
}

void ModelTrans::scale(const glm::vec3& scaleFactors) {
     modelMatrix =  glm::scale(modelMatrix, scaleFactors);
}

void ModelTrans::rotateAroundPoint(const glm::vec3& point, float angle, const glm::vec3& axis, const glm::vec3& scaleFactors)
{
    glm::mat4 transToOrigin = glm::translate(modelMatrix, -point);
    float rad = glm::radians(angle);
    glm::mat4 rotateMat = glm::rotate(modelMatrix, rad, axis);
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scaleFactors);
  
    glm::mat4 transBack = glm::translate(modelMatrix, point);

    glm::mat4 aroundPointMat = transToOrigin * rotateMat * scaleMat * transBack;

    modelMatrix = modelMatrix * aroundPointMat;
}