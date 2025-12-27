#include "Projection.h"
using namespace std;

glm::mat4 Projection::getProjection() {
    float aspect = (float)Width / (float)Height;
    glm::mat4 ProjectionMat=glm::perspective(glm::radians(FOV), aspect, near, far);
    return ProjectionMat;
}

int main() {
    if (!glfwInit()) {
        cout << "GLFW³õÊ¼»¯Ê§°Ü" <<" ";
        return -1;
    }
    Projection proj(60.0f, 0.1f, 100.0f, 800, 600);
    glm::mat4 useMat = proj.getProjectionMat();
    proj.setFOV(70.0f); 
    glm::mat4 newMat = proj.getProjectionMat(); 
    float* matData = (float*)glm::value_ptr(newMat); 
    for (int i = 0; i < 4; i++) { 
        for (int j = 0; j < 4; j++) {
            cout << matData[j * 4 + i] << "\t"; 
        }
        cout << "\n";
    }
    proj.setFOV(75.0f);
    glm::mat4 newProjectionMat = proj.getProjectionMat(); 
    glfwTerminate();
    return 0;
}
