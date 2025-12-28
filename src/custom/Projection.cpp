#include <custom/Projection.h>
using namespace std;

glm::mat4 Projection::getProjection() {
    float aspect = (float)Width / (float)Height;
    glm::mat4 ProjectionMat=glm::perspective(glm::radians(FOV), aspect, near, far);
    return ProjectionMat;
}

