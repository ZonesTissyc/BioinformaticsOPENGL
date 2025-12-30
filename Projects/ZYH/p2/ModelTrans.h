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

    void translate(const glm::vec3& translation);

    void rotate(float angle, const glm::vec3& axis);

    void scale(const glm::vec3& scaleFactors);

    void rotateAroundPoint(const glm::vec3& point, float angle, const glm::vec3& axis, const glm::vec3& scaleFactors);

    glm::mat4 getModelMatrix() const
    {
        return modelMatrix;
    }
    
    void setModelMatrix(const glm::mat4& matrix)
    {
        modelMatrix = matrix;
    }
};

