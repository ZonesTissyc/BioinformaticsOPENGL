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
	Window window(1280, 720, "animation");
	std::string rootURL = R"(../../../)";
	std::string vsURL = rootURL + "shaders/anim_model.vs";
	std::string fsURL = rootURL + "shaders/anim_model.fs";
	Shader shader1(vsURL.c_str(), fsURL.c_str());
	stbi_set_flip_vertically_on_load(true);
	// 使用 custom::Camera（不包含 learnopengl 的 camera.h，避免重定义）
	Camera camera(glm::vec3(0.0f, 1.0f, 5.0f));
	InputController controller(camera, 2.6f, 0.2f);

	// 加载模型（骨骼）与动画（从同一 glb 文件读取）
	Model model(rootURL + "resources/model/sci-fi_solder/sci-fi_solder.glb");
	Animation animation(rootURL + "resources/model/sci - fi_solder/sci-fi_solder.glb", &model);
	Animator animator(&animation);

	// 着色器预设（投影可在窗口大小变化时更新）
	shader1.use();
	shader1.setMat4("projection", glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f));

	// 计时
	float lastFrame = static_cast<float>(glfwGetTime());

	while (window.noClose()) {
		// deltaTime
		float currentFrame = static_cast<float>(glfwGetTime());
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// 处理输入（InputController 使用 custom::Camera）
		controller.processKeyboardInput(window.get(), deltaTime);
		controller.processMouseInput(window.get());

		// 更新动画状态
		animator.UpdateAnimation(deltaTime);

		// 渲染
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader1.use();

		// 上传相机矩阵（custom::Camera 提供 getView()）
		shader1.setMat4("view", camera.getView());
		shader1.setMat4("projection", glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f));

		// 模型变换
		glm::mat4 modelMat = glm::mat4(1.0f);
		shader1.setMat4("model", modelMat);

		// 上传骨骼矩阵到 shader（shader 中应声明 e.g. "uniform mat4 finalBonesMatrices[100];"）
		auto finalMatrices = animator.GetFinalBoneMatrices();
		for (size_t i = 0; i < finalMatrices.size(); ++i) {
			std::string name = "finalBonesMatrices[" + std::to_string(i) + "]";
			shader1.setMat4(name, finalMatrices[i]);
		}

		// 绘制模型
		model.Draw(shader1);

		// 交换缓冲与事件轮询
		window.swapBuffers();
	}

	return 0;
}