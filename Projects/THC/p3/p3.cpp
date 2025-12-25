#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

void processInput(GLFWwindow* window);//函数声明

#pragma region 正方体顶点数据
float vertices[] = {
    // ---- 位置 ----       ---- 纹理坐标 ----
    // 后脸 (Z = -0.5)
    -0.5f, -0.5f, -0.5f,  
     0.5f, -0.5f, -0.5f,  
     0.5f,  0.5f, -0.5f,  
     0.5f,  0.5f, -0.5f,  
    -0.5f,  0.5f, -0.5f,  
    -0.5f, -0.5f, -0.5f,

    // 前脸 (Z = 0.5)
    -0.5f, -0.5f,  0.5f,  
     0.5f, -0.5f,  0.5f,  
     0.5f,  0.5f,  0.5f,  
     0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f,  
    -0.5f, -0.5f,  0.5f,  

    // 左侧面 (X = -0.5)
    -0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f, -0.5f,  
    -0.5f, -0.5f, -0.5f,  
    -0.5f, -0.5f, -0.5f, 
    -0.5f, -0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f,  

    // 右侧面 (X = 0.5)
     0.5f,  0.5f,  0.5f,  
     0.5f,  0.5f, -0.5f,  
     0.5f, -0.5f, -0.5f,  
     0.5f, -0.5f, -0.5f,  
     0.5f, -0.5f,  0.5f,  
     0.5f,  0.5f,  0.5f,  

     // 底面 (Y = -0.5)
     -0.5f, -0.5f, -0.5f,  
      0.5f, -0.5f, -0.5f,  
      0.5f, -0.5f,  0.5f,  
      0.5f, -0.5f,  0.5f,  
     -0.5f, -0.5f,  0.5f,  
     -0.5f, -0.5f, -0.5f,  

     // 顶面 (Y = 0.5)
     -0.5f,  0.5f, -0.5f,  
      0.5f,  0.5f, -0.5f,  
      0.5f,  0.5f,  0.5f,  
      0.5f,  0.5f,  0.5f,  
     -0.5f,  0.5f,  0.5f,  
     -0.5f,  0.5f, -0.5f,  
};
#pragma endregion 

int main()
{
#pragma region 创建窗口对象，返回window指针
    if (!glfwInit()) {
        std::cout << "glfwInit is fail!" << std::endl;
    }
    GLFWwindow* window = glfwCreateWindow(1280, 720, "zzzyh", nullptr, nullptr);
    glfwSetWindowPos(window, 1000, 150); // 调整窗口的位置
#pragma endregion 

#pragma region 窗口初始化，配置OpenGL环境
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // opengl大版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // opengl小版本
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST); // 初始化时调用一次

#pragma endregion


//编译着色器
    std::string RootURL = R"(../../../)";
    std::string files_url[4];
    files_url[0] = RootURL + R"(shaders/1.colors/1.colors.vs)";
    files_url[1] = RootURL + R"(shaders/1.colors/1.colors.fs)";
    files_url[2] = RootURL + R"(shaders/1.colors/1.light_cube.vs)";
    files_url[3] = RootURL + R"(shaders/1.colors/1.light_cube.fs)";
    // Shader lightingShader("1.colors.vs", "1.colors.fs");//箱子
    // Shader lightCubeShader("1.light_cube.vs", "1.light_cube.fs");//光源
    Shader lightingShader(files_url[0].c_str(), files_url[1].c_str());//箱子
    Shader lightCubeShader(files_url[2].c_str(), files_url[3].c_str());//光源

#pragma region VBO & VAO
    unsigned int VBO, VAO;   //箱子
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 1. 绑定 VAO
    glBindVertexArray(VAO);  //绑定开始

    // 2. 把顶点数组复制到缓冲VBO中
    glBindBuffer(GL_ARRAY_BUFFER, VBO);//绑定VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 3. 设置顶点属性指针（告诉 OpenGL 如何解析VBO中的数据）
    // 参数：属性位置(0)：对应顶点着色器中layout(location=0)的aPos属性, 组成元素个数(3), 类型, 是否标准化, 步长, 偏移量
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);//启动这个顶点属性（被GPU读取）

    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);//解绑VBO
    glBindVertexArray(0);       // 绑定结束

//光VAO
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // 只需要绑定VBO不用再次设置VBO的数据，因为箱子的VBO数据中已经包含了正确的立方体顶点数据
     glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 设置灯立方体的顶点属性（对我们的灯来说仅仅只有位置数据）
     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);
     glEnableVertexAttribArray(0);

     glBindVertexArray(0);
#pragma endregion




#pragma region 主循环
     Camera camera;

    //窗口未被标记为关闭时持续运行
    while (!glfwWindowShouldClose(window))
    {

        // 处理输入
        processInput(window);

        // 渲染逻辑（清空颜色/深度缓冲、绘制图形等）
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



     #pragma region 物体的渲染
        //激活箱子着色器
        lightingShader.use();

        //设置物体、光源颜色
        lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        lightingShader.setVec3("lightPos", lightPos);

   
        //设置MVP矩阵
       
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix(); // 用camera类
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
        

        lightingShader.setMat4("model", model);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);// 把矩阵数据传递给着色器的 uniform 变量

        glBindVertexArray(VAO);       // 绑定箱子的VAO
        glDrawArrays(GL_TRIANGLES, 0, 36);  // 绘制
     #pragma endregion

#pragma region 渲染光源

        lightCubeShader.use();

        // 设置光源的MVP矩阵（让光源在世界空间的位置和光照参数一致）

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.2f, 1.0f, 2.0f)); // 和lightPos一致
        model = glm::scale(model, glm::vec3(0.2f)); // 缩小光源立方体
        lightCubeShader.setMat4("model", model);
      

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

#pragma endregion

        // 3. 交换前后缓冲区（双缓冲机制，避免画面闪烁）
        glfwSwapBuffers(window);
        // 4. 处理GLFW事件（如键盘/鼠标输入）
        glfwPollEvents();
    }

    // 主循环结束后，销毁窗口、终止GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
#pragma endregion

   
	return 0;
}


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}