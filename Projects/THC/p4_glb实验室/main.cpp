#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <learnopengl/shader.h>
#include <learnopengl/mesh.h>
#include <learnopengl/model.h>
#include <custom/InputController.h>
#include <custom/Camera.h>
#include <custom/window.h>

int main() {
	std::string rootURL = R"(../../../shaders/)";
	Window window(1280, 720, "glb_test");
	std::string vsURL = rootURL + "1.model_loading.vs";
	std::string fsURL = rootURL + "1.model_loading.fs";
	Shader shader1(vsURL.c_str(), fsURL.c_str());
	Camera camera(glm::vec3(1.0f, 1.0f, 2.0f));
	InputController controller(camera, 0.5, 0.2);
	Model model("sci-fi_lab.glb", true);

	shader1.use();
	shader1.setMat4("view", camera.getView());
	shader1.setMat4("projection", glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f));
	shader1.setMat4("model", glm::mat4(1.0f));
	while (window.noClose()) {
		controller.processKeyboardInput(window.get(), 0.01); // 更新相机
		controller.processMouseInput(window.get());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader1.use();
		shader1.setMat4("view", camera.getView());
		shader1.setMat4("projection", glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f));
		shader1.setMat4("model", glm::mat4(1.0f));

		model.Draw(shader1);

		window.swapBuffers();
		
	}
}