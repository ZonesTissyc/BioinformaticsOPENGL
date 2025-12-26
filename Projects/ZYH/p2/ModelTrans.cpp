#include "ModelTrans.h"


void ModelTrans::translate(float x, float y, float z) {
    modelMatrix = modelMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
}


void ModelTrans::rotate(float angle, float x, float y, float z) {
    modelMatrix = modelMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(x,y,z));
}

void ModelTrans::scale(float x, float y, float z) {
     modelMatrix = modelMatrix * glm::scale(glm::mat4(1.0f),  glm::vec3(x,y,z));
}

void ModelTrans::rotateAroundPoint(float x, float y, float z,
    float angle, float X, float Y , float Z,
    float scalex = 1.0f, float scaley = 1.0f, float scalez = 1.0f)
{
    glm::mat4 transToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-x, -y, -z));
   
    glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(X, Y,Z));
 
    glm::mat4 transBack = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));

    glm::mat4 aroundPointMat = transToOrigin * rotateMat * transBack;

    modelMatrix = modelMatrix * aroundPointMat;
}