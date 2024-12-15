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

#include "axis.h"
#include "rectangle.h"
#include "ControlledInputFloat.h"
#include "chain.h"

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
void changeToRelativeCoords(double& xpos, double& ypos);

bool isCreatingRectangle();
int checkForRectangle(glm::vec2 pos);

int viewportSizeLoc, colorLoc;

const glm::vec2 viewportSize(1200, 800); 
const int guiWidth = 300;
static int mode = 0;
ControlledInputFloat L1("L1", 150.0f, 1.f, 1.f);
ControlledInputFloat L2("L2", 100.0f, 1.f, 1.f);

Axis* axis;
std::vector<Rectangle*> rectangles;
Chain* chain;

int main() { 
    #pragma region gl_boilerplate
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(viewportSize.x + guiWidth, viewportSize.y, "Kinematic chain 2D", NULL, NULL);
    if (window == NULL) {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, viewportSize.x, viewportSize.y);
    glEnable(GL_DEPTH_TEST);

    GLFWimage icon;
    icon.pixels = stbi_load("icon.png", &icon.width, &icon.height, 0, 4);
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(icon.pixels);
    #pragma endregion

	// callbacks
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouseMoveCallback);

    // shaders and uniforms
    Shader shaderProgram("Shaders\\default.vert", "Shaders\\default.frag");
    viewportSizeLoc = glGetUniformLocation(shaderProgram.ID, "viewportSize");
    colorLoc = glGetUniformLocation(shaderProgram.ID, "color");

	// objects
	axis = new Axis(viewportSize);
	chain = new Chain(L1.GetValue(), L2.GetValue());

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
        ImGui::SetNextWindowSize(ImVec2(guiWidth, viewportSize.y));
        ImGui::SetNextWindowPos(ImVec2(viewportSize.x, 0));
        #pragma endregion
        
        shaderProgram.Activate();
        glUniform2fv(viewportSizeLoc, 1, glm::value_ptr(viewportSize));

        // render
		axis->Render(colorLoc);
		for (auto& r : rectangles)
			r->Render(colorLoc);
		chain->Render(colorLoc);

        // imgui rendering
        ImGui::Begin("Menu", 0,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		ImGui::SeparatorText("Mode");
		ImGui::RadioButton("Positions setup", &mode, 0); ImGui::SameLine();
		ImGui::RadioButton("Obstacles setup", &mode, 1);

		ImGui::SeparatorText("Parameters");
		L1.Render();
		L2.Render();
		if (ImGui::Button("Set lengths")) 
            chain->UpdateLengths(L1.GetValue(), L2.GetValue());

		ImGui::SeparatorText("Conf. space coords");
		glm::vec2 angles = chain->GetAngles();
		ImGui::Text(u8"a1 = %.2f°, a2 = %.2f°", angles.x, angles.y);

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

	// assets cleanup
    shaderProgram.Delete();
	axis->Delete();
	for (auto& r : rectangles)
		r->Delete();
	chain->Delete();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
    #pragma endregion
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
	changeToRelativeCoords(xpos, ypos);
    //std::cout << xpos << " " << ypos << std::endl;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (mode == 1) {
            if (isCreatingRectangle()) {
                rectangles[rectangles.size() - 1]->UpdateEnd(glm::vec2(xpos, ypos));
                rectangles[rectangles.size() - 1]->inCreation = false;
            }
            else {
                rectangles.push_back(new Rectangle(glm::vec2(xpos, ypos)));
            }
        }
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (mode == 1) {
            if (isCreatingRectangle()) return;

            int i = checkForRectangle(glm::vec2(xpos, ypos));
            if (i != -1) {
                rectangles[i]->Delete();
                rectangles.erase(rectangles.begin() + i);
            }
        }
	}
}

void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos){
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
        return;

	changeToRelativeCoords(xpos, ypos);

    if (mode == 1) {
        if (isCreatingRectangle()) {
            rectangles[rectangles.size() - 1]->UpdateEnd(glm::vec2(xpos, ypos));
        }
    }
}

void changeToRelativeCoords(double& xpos, double& ypos)
{
	xpos -= viewportSize.x / 2.f;
	ypos -= viewportSize.y / 2.f;
	ypos *= -1.f;
}

bool isCreatingRectangle()
{
	return rectangles.size() != 0 && rectangles[rectangles.size() - 1]->inCreation;
}

int checkForRectangle(glm::vec2 pos)
{
	for (int i = 0; i < rectangles.size(); i++) {
		glm::vec4 r = rectangles[i]->GetRectangle();
		if (pos.x >= r.x && pos.x <= r.z && pos.y >= r.y && pos.y <= r.w)
			return i;
	}
	return -1;
}
