#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <custom/ModelTrans.h>

int main() {
	ModelTrans mattorotate;
	mattorotate.rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	
}