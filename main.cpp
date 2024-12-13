#define _USE_MATH_DEFINES
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

void window_size_callback(GLFWwindow *window, int width, int height);

int screenSizeLoc, isNormalizedLoc, colorLoc;

glm::vec2 screenSize(1500, 800);
const int guiWidth = 300;

int main() { 
    #pragma region gl_boilerplate
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(screenSize.x, screenSize.y, "Kinematic chain 2D", NULL, NULL);
    if (window == NULL) {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, screenSize.x - guiWidth, screenSize.y);
    glEnable(GL_DEPTH_TEST);

    GLFWimage icon;
    icon.pixels = stbi_load("icon.png", &icon.width, &icon.height, 0, 4);
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(icon.pixels);
    #pragma endregion

    // shaders and uniforms
    Shader shaderProgram("Shaders\\default.vert", "Shaders\\default.frag");
	screenSizeLoc = glGetUniformLocation(shaderProgram.ID, "screenSize");
	isNormalizedLoc = glGetUniformLocation(shaderProgram.ID, "isNormalized");
    colorLoc = glGetUniformLocation(shaderProgram.ID, "color");

    // callbacks
    glfwSetWindowSizeCallback(window, window_size_callback);

    #pragma region imgui_boilerplate
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    #pragma endregion

    while (!glfwWindowShouldClose(window)) 
    {
        #pragma region init
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(guiWidth, screenSize.y));
        ImGui::SetNextWindowPos(ImVec2(screenSize.x - guiWidth, 0));
        #pragma endregion
        
        shaderProgram.Activate();
        glUniform2fv(screenSizeLoc, 1, glm::value_ptr(screenSize));

        // render


        // imgui rendering
        if (ImGui::Begin("Menu", 0,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {}

        ImGui::End();
        #pragma region rest
        ImGui::Render();
        //std::cout << ImGui::GetIO().Framerate << std::endl;
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
        #pragma endregion
    }
    #pragma region exit
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    shaderProgram.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
    #pragma endregion
}

// callbacks
void window_size_callback(GLFWwindow *window, int width, int height) {
	screenSize = { width, height };
    glViewport(0, 0, screenSize.x - guiWidth, screenSize.y);
}