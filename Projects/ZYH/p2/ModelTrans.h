#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class ModelTrans{
private:
    glm::mat4 modelMatrix; 

public:
   
    ModelTrans() : modelMatrix(glm::mat4(1.0f))
    { }

    void reset()
    {
        modelMatrix = glm::mat4(1.0f);
    }

    void translate(float x, float y, float z);

    void rotate(float angle, float x, float y, float z);

    void scale(float x, float y, float z);

    void rotateAroundPoint(float x, float y, float z,
        float angle, float rotX, float rotY, float rotZ,
        float scalex = 1.0f, float scaley = 1.0f, float scalez= 1.0f);

    glm::mat4 getModelMatrix() const
    {
        return modelMatrix;
    }
};

