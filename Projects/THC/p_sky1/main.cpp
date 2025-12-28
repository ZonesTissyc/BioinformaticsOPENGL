#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <custom/InputController.h>
#include <custom/Camera.h>
#include <custom/window.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>

int main() {
	Window window(1280, 720, "windows");
	Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
	InputController controller(camera, 3.0f, 1.0f);
	
	while (window.noClose()) {

	}
	
}