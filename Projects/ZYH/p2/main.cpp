#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "ModelTrans.h"


void processInput(GLFWwindow* window);

#pragma region 正方体顶点数据
float vertices[] = {
    // ---- 位置 ----       ---- 纹理坐标 ----
    // 后脸 (Z = -0.5)
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    // 前脸 (Z = 0.5)
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    // 左侧面 (X = -0.5)
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    // 右侧面 (X = 0.5)
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     // 底面 (Y = -0.5)
     -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

     // 顶面 (Y = 0.5)
     -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
     -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};
#pragma endregion 

#pragma region 顶点着色器源码

std::string vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos; 

uniform mat4 model;      // 模型矩阵
uniform mat4 view;       // 观察矩阵 (Viewing Transformation)
uniform mat4 projection; // 投影矩阵

void main()
{
    // MVP 变换：将顶点从局部空间转换到裁剪空间
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

#pragma endregion 

#pragma region 片段着色器源码
std::string fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0); // 橘色
}
)";
#pragma endregion

int main() {

#pragma region 窗口初始化
    if (!glfwInit()) {
        std::cout << "glfwInit is fail!" << std::endl;
    }
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Model_test", nullptr, nullptr);
    glfwSetWindowPos(window, 1000, 150); // 调整窗口的位置
#pragma endregion 返回window指针

#pragma region opengl初始化
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // opengl大版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // opengl小版本
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST); // 初始化时调用一次


#pragma endregion 上下文关联window，获取函数指针

#pragma region 着色器编译
    // 编译顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vSrc = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vSrc, NULL);
    glCompileShader(vertexShader);

    // 编译片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fSrc = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fSrc, NULL);
    glCompileShader(fragmentShader);

    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 删除着色器对象（已经链接好了，不需要了）
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
#pragma endregion 

#pragma region VBO & VAO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 1. 绑定 VAO
    glBindVertexArray(VAO);  //绑定开始

    // 2. 把顶点数组复制到缓冲中
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 3. 设置顶点属性指针（告诉 OpenGL 如何解析这些数据）
    // 参数：属性位置(0), 组成元素个数(3), 类型, 是否标准化, 步长, 偏移量
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);       // 绑定结束
#pragma endregion 

#pragma region 构造MVP矩阵
    // 1. 模型矩阵：让正方体随时间旋转
    //glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
    // 2. 观察矩阵：相机向后移动 3 个单位
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
    // 3. 投影矩阵：45度视野
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    ModelTrans modeltrans;

    // 找到 Uniform 的位置并赋值
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
#pragma endregion 
  

#pragma region 帧循环
    while (!glfwWindowShouldClose(window))
    {
#pragma region 处理输入
        processInput(window);
#pragma endregion 

#pragma region VAO段渲染
        glUseProgram(shaderProgram); 

        //glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
        modeltrans.reset();
       // modeltrans.translate(glm::vec3(0.5f, 1.0f, 0.0f));
      // modeltrans.scale(glm::vec3(0.5f, 0.5f, 0.5f));
       //modeltrans.rotate(45.0f, glm::vec3(0.5f, 1.0f, 1.0f));
        //modeltrans.rotateAroundPoint(glm::vec3(1.0f, 0.5f, 0.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f),glm::vec3(1.0f,1.0f,1.0f));
       
       float angle = (float)glfwGetTime() * 180.0f; // 角度随时间变化
      modeltrans.rotateAroundPoint(glm::vec3(1.0f, 0.0f, 0.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f),glm::vec3( 1.0f, 1.0f, 1.0f));
      glm::mat4 model = modeltrans.getModelMatrix();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色缓、深度缓冲
        glBindVertexArray(VAO);     // 指定绑定的VAO
        glDrawArrays(GL_TRIANGLES, 0, 36); // 绘制 36 个顶点

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
#pragma endregion
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
#pragma endregion
    return 0;
}


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}