#pragma once

// ===== ImGui =====
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// ===== GLFW / GLM =====
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// ===== STD =====
#include <string>
#include <cstdio>

class Iui
{
public:
    // ------------------------------
    // 构造
    // ------------------------------
    Iui(GLFWwindow* window, int width = 1280, int height = 720)
        : m_window(window), m_width(width), m_height(height)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    // ------------------------------
    // 析构
    // ------------------------------
    ~Iui()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    // ------------------------------
    // 每帧开始
    // ------------------------------
    inline void beginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    // ------------------------------
    // 每帧结束
    // ------------------------------
    inline void endFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // ------------------------------
    // 窗口大小更新（在 resize callback 里调）
    // ------------------------------
    inline void setWindowSize(int width, int height)
    {
        m_width = width;
        m_height = height;
    }

    // ==========================================================
    // HUD
    // ==========================================================

    // ------------------------------
    // 左上角 FPS
    // ------------------------------
    inline void showFPS(float fontScale = 1.4f)
    {
        ImGuiIO& io = ImGui::GetIO();
        float fps = io.Framerate;

        ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));

        ImGui::Begin("##FPS",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoBackground);

        ImGui::SetWindowFontScale(fontScale);
        ImGui::Text("FPS: %.1f", fps);

        ImGui::End();
    }

    // ------------------------------
    // 右上角坐标
    // ------------------------------
    inline void showPos(const glm::vec3& pos, float fontScale = 1.2f)
    {
        const float margin = 10.0f;

        ImGui::SetNextWindowPos(
            ImVec2((float)m_width - margin, margin),
            ImGuiCond_Always,
            ImVec2(1.0f, 0.0f)   // 右上角锚点
        );

        ImGui::Begin("##Position",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoBackground);

        ImGui::SetWindowFontScale(fontScale);

        ImGui::Text("Pos:");
        ImGui::Text(" x: %.2f", pos.x);
        ImGui::Text(" y: %.2f", pos.y);
        ImGui::Text(" z: %.2f", pos.z);

        ImGui::End();
    }

    // ------------------------------
    // 任意文本 HUD（你扩展用）
    // ------------------------------
    inline void showTextTopLeft(const std::string& text, float fontScale = 1.0f)
    {
        ImGui::SetNextWindowPos(ImVec2(10.0f, 60.0f));

        ImGui::Begin("##Text",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoBackground);

        ImGui::SetWindowFontScale(fontScale);
        ImGui::TextUnformatted(text.c_str());

        ImGui::End();
    }


    inline void drawCrosshair(
        float size = 10.0f*2,
        float thickness = 2.0f,
        ImU32 color = IM_COL32(255, 0, 0, 255))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        ImVec2 center(
            io.DisplaySize.x * 0.5f,
            io.DisplaySize.y * 0.5f
        );

        // 横线
        drawList->AddLine(
            ImVec2(center.x - size, center.y),
            ImVec2(center.x + size, center.y),
            color,
            thickness
        );

        // 竖线
        drawList->AddLine(
            ImVec2(center.x, center.y - size),
            ImVec2(center.x, center.y + size),
            color,
            thickness
        );
    }
private:
    GLFWwindow* m_window = nullptr;
    int m_width = 1280;
    int m_height = 720;
};
