#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <custom/window.h>
#include <learnopengl/shader.h>
#include <random>
using namespace std;
using namespace glm;

vec2 bezier1(vector<vec2> &posList, float t) {
	int length = posList.size();
	if (length == 1) {
		return posList[0];	// 数组长度为1直接返回
	}
	vector<vec2> newList(length - 1);
	vec2 newvert;			// 新的数组来存储插值的点
	for (int i = 0; i < length - 1; i++) {
		newvert = (1 - t) * posList[i] + t * posList[i + 1];
		newList[i] = newvert;
	}
	return bezier1(newList, t);	// 进行下一层递归
}

vec2 bezier2(const vector<vec2>& posList, float t) {
	int n = posList.size();
	if (n == 0) return vec2(0.0f);
	vector<vec2> temp = posList;  // 拷贝一份
	// 迭代计算
	for (int k = 1; k < n; ++k) {      // 总共 n-1 轮
		for (int i = 0; i < n - k; ++i) {
			temp[i] = (1 - t) * temp[i] + t * temp[i + 1];
		}
	}
	return temp[0];
}

unsigned long long factorial(int n) {
	unsigned long long f = 1;
	for (int i = 2; i <= n; ++i) f *= i;
	return f;
}

// 计算组合数 C(n, k)
unsigned long long C(int n, int k) {
	return factorial(n) / (factorial(k) * factorial(n - k));
}

// 贝塞尔曲线公式版本
vec2 bezier3(const vector<vec2>& posList, float t) {
	int n = posList.size() - 1;
	vec2 point(0.0f);
	for (int i = 0; i <= n; ++i) {
		float coeff = C(n, i) * pow(1 - t, n - i) * pow(t, i);
		point += coeff * posList[i];
	}
	return point;
}


// 生成随机数

vector <vec2> randPos (int n){
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<float> dist(-1.0f, 1.0f); // 坐标范围 [-1, 1]

	vector<vec2> points;
	for (int i = 0; i < n; ++i) {
		float x = dist(gen);
		float y = dist(gen);
		points.push_back(vec2(x, y));
	}

	for (int i = 0; i < points.size(); ++i) {
		cout << "Point " << i << ": (" << points[i].x << ", " << points[i].y << ")\n";
	}
	return points;
}

int main() {
	
	vector<vec2> controlPos{
	{0.147706, -0.524932},
	{0.3143,  0.643419},
	{ -0.700356, -0.720207},
	{ -0.539508, 0.0859653}
	};
	

	int total = 200;
	
	vector<vec2> curvePoints;

	string choose = "公式";
	if (choose == "递归") {
		for (int i = 0; i <= total; ++i) {
			float t = i / (total *1.0f);
			curvePoints.push_back(bezier1(controlPos, t));
		}
	}
	else if (choose == "迭代")
	{
		for (int i = 0; i <= total; ++i) {
			float t = i / (total *1.0f);
			curvePoints.push_back(bezier2(controlPos, t));
		}
	}
	else
	{
		for (int i = 0; i <= total; ++i) {
			float t = i / (total *1.0f);
			curvePoints.push_back(bezier3(controlPos, t));
		}
	}
	vector<float> vertices;
	for (vec2 p : curvePoints) {
		vertices.push_back(p.x);
		vertices.push_back(p.y);
	}
	Window window(1270, 720, "bezier");
	glViewport(0, 0, 1270, 720);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		vertices.size() * sizeof(float),
		vertices.data(),
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// 着色器编译
	string rootURL = "../../../";
	string vsURL =rootURL + "shaders/bezier.vs";
	string fsURL =rootURL + "shaders/bezier.fs";
	Shader shader(vsURL.c_str(), fsURL.c_str());
	while (window.noClose()) {
		if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window.get(), true);
		
		glClear(GL_COLOR_BUFFER_BIT);
		shader.use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINE_STRIP, 0, vertices.size() / 2);
		window.swapBuffers();
	}
	
}