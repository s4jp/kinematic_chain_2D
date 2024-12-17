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
#include "ControlledInputFloat.h"
#include "ControlledInputInt.h"

#include "axis.h"
#include "rectangle.h"
#include "chain.h"
#include "configurationSpace.h"

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
void changeToRelativeCoords(double& xpos, double& ypos);
void calculateAndVerifyConfigs(bool start, bool end, bool keepSelection = false);
void clearPath();

bool isCreatingRectangle();
int checkForRectangle(glm::vec2 pos);

int viewportSizeLoc, colorLoc;

static const glm::vec2 viewportSize(1200, 800);
static const int guiWidth = 300;
static int mode = 0;
static ControlledInputFloat L1("L1", 150.0f, 1.f, 1.f);
static ControlledInputFloat L2("L2", 100.0f, 1.f, 1.f);
bool drawCircles = false;

Axis* axis;
std::vector<Rectangle*> rectangles;
Chain* chain;
Chain* endChain;
ConfigurationSpace* confSpace;

static glm::vec2 targets[2] = { glm::vec2(0.f), glm::vec2(0.f) };
static std::vector<glm::vec2> startConfigs;
static std::vector<glm::vec2> endConfigs;
static int selectedConfigs[2] = { -1, -1 };
enum Mode { OFF, START, END };
static Mode targetMode = Mode::START;

std::vector<glm::vec2> path;
static float pathIndex = -1;
static ControlledInputInt speed("Speed [%]", 25, 1, 1);

static int selectedRectangle = -1;

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
	endChain = new Chain(L1.GetValue(), L2.GetValue());
	endChain->SetColor(glm::vec4(1.f, 0.5f, 0.5f, 1.f));
	confSpace = new ConfigurationSpace(chain, rectangles);

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
        for (int i = 0; i < rectangles.size(); i++)
			rectangles[i]->RenderSelect(colorLoc, selectedRectangle == i);
		if (selectedConfigs[0] >= 0)
		    chain->Render(colorLoc);
        if (selectedConfigs[1] >= 0)
			endChain->Render(colorLoc);
		if (drawCircles)
		    chain->RenderCircles(colorLoc);

        // imgui rendering
        ImGui::Begin("Menu", 0,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		ImGui::SeparatorText("Mode");
		ImGui::RadioButton("Positions setup", &mode, 0); ImGui::SameLine();
		ImGui::RadioButton("Obstacles setup", &mode, 1);

        if (mode == 0) {
			ImGui::SeparatorText("Targets");

            if (ImGui::Button("Set start")) {
                targetMode = Mode::START;
                clearPath();
            }
            if (targetMode == Mode::START) {
                ImGui::SameLine();
                ImGui::Text("Selection active!");
            }
            if (selectedConfigs[0] >= 0 && targetMode != Mode::START) {
                ImGui::SameLine();
                ImGui::Text("Position: %.0f, %.0f", targets[0].x, targets[0].y);

                ImGui::Spacing();
                glm::vec2 angles = chain->GetAngles();
                ImGui::Text(u8"Config.: q1 = %.2f°, q2 = %.2f°", angles.x, angles.y);
            }
            if (startConfigs.size() > 1 && targetMode != Mode::START) {
                if (ImGui::SliderInt("Start conf.", &selectedConfigs[0], 0, startConfigs.size() - 1)) {
                    clearPath();
                    chain->SetAngles(startConfigs[selectedConfigs[0]].x, startConfigs[selectedConfigs[0]].y);
                }
            }

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            if (ImGui::Button("Set end")){
                targetMode = Mode::END;
                clearPath();
            }	
            if (targetMode == Mode::END) {
                ImGui::SameLine();
                ImGui::Text("Selection active!");
            }
            else {
                if (selectedConfigs[1] >= 0) {
                    ImGui::SameLine();
                    ImGui::Text("Position: %.0f, %.0f", targets[1].x, targets[1].y);

                    ImGui::Spacing();
					glm::vec2 angles = endChain->GetAngles();
                    ImGui::Text(u8"Config.: q1 = %.2f°, q2 = %.2f°", angles.x, angles.y);
                }
                if (endConfigs.size() > 1) {
                    if (ImGui::SliderInt("End conf.", &selectedConfigs[1], 0, endConfigs.size() - 1)) {
                        clearPath();
                        endChain->SetAngles(endConfigs[selectedConfigs[1]].x, endConfigs[selectedConfigs[1]].y);
                    }
                }
            }
        }
        else if (mode == 1) {
            ImGui::Spacing();

			if (rectangles.size() > 0) {
                if (ImGui::BeginListBox("Obstacles")) {
                    for (int i = 0; i < rectangles.size(); i++) {
                        if (ImGui::Selectable(rectangles[i]->name.c_str(), selectedRectangle == i)) {
                            selectedRectangle = selectedRectangle == i ? -1 : i;
                        }
                    }
                    ImGui::EndListBox();
                }

				if (selectedRectangle >= 0) {
					rectangles[selectedRectangle]->RenderImgui();
				}

				ImGui::Separator();

                if (ImGui::Button("Clear all obstacles")) {
                    for (auto& r : rectangles)
                        r->Delete();
                    rectangles.clear();
                    selectedRectangle = -1;
                }
			}
        }

		ImGui::SeparatorText("Parameters");
		L1.Render();
		L2.Render();
        if (ImGui::Button("Set lengths")){
			chain->UpdateLengths(L1.GetValue(), L2.GetValue());
			endChain->UpdateLengths(L1.GetValue(), L2.GetValue());
        }

		ImGui::SeparatorText("Configuration space");
        if (confSpace->RenderImGui(chain, rectangles))
			calculateAndVerifyConfigs(true, true, true);

        if (selectedConfigs[0] >= 0 && selectedConfigs[1] >= 0) {
			ImGui::SeparatorText("Pathfinding");

            if (ImGui::Button("Find and run")) {
				clearPath();
                path = confSpace->FindShortestPath(startConfigs[selectedConfigs[0]], endConfigs[selectedConfigs[1]]);
				pathIndex = path.size() > 0 ? 0 : -1;
            }
            if (path.size() > 0) {
                ImGui::SameLine();
                ImGui::Text("Path found! Size: %d", path.size());

				if (pathIndex >= 0 && pathIndex <= path.size() + (float)speed.GetValue() / 100) {
                    ImGui::Text("Animation running...");
                    int idx = (int)pathIndex % path.size();
					chain->SetAngles(path[idx].x, path[idx].y);
					pathIndex += (float)speed.GetValue() / 100.f;
				}
            }
        }

		ImGui::SeparatorText("Other");
		ImGui::Checkbox("Show guide circles", &drawCircles);
		speed.Render();

        ImGui::End();

		confSpace->RenderTexture();
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
	endChain->Delete();

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

	if (xpos < 0 || xpos > viewportSize.x || ypos < 0 || ypos > viewportSize.y)
		return;

	changeToRelativeCoords(xpos, ypos);
    //std::cout << xpos << " " << ypos << std::endl;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (mode == 1) {
            if (isCreatingRectangle()) {
                rectangles[rectangles.size() - 1]->UpdateEnd(glm::vec2(xpos, ypos));
				rectangles[rectangles.size() - 1]->EndCreation();
            }
            else {
                rectangles.push_back(new Rectangle(glm::vec2(xpos, ypos)));
            }
        }
        else if (mode == 0) {
            if (targetMode == Mode::OFF) return;

            if (targetMode == Mode::START) {
                targets[0] = glm::vec2(xpos, ypos);
				calculateAndVerifyConfigs(true, false);
            }
            else if (targetMode == Mode::END) {
                targets[1] = glm::vec2(xpos, ypos);
                calculateAndVerifyConfigs(false, true);
            }

            targetMode = Mode::OFF;
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
        else if (mode == 0) {
			targetMode = Mode::OFF;
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

void calculateAndVerifyConfigs(bool start, bool end, bool keepSelection)
{
    clearPath();

    if (start) {
        glm::vec2 selectedConfig = (selectedConfigs[0] >= 0 && keepSelection) ? startConfigs[selectedConfigs[0]] : glm::vec2(-1.f);
        selectedConfigs[0] = -1;

		if (targets[0] != glm::vec2(0.f))
            startConfigs = chain->InverseKinematics(targets[0]);

        for (int i = 0; i < startConfigs.size(); i++) {
            if (confSpace->CheckCollision(startConfigs[i])) {
                startConfigs.erase(startConfigs.begin() + i);
				i--;
            }
			else if (selectedConfig == startConfigs[i]) {
				selectedConfigs[0] = i;
			}
        }
        selectedConfigs[0] = startConfigs.size() == 0 ? -1 : (selectedConfigs[0] >= 0 ? selectedConfigs[0] : 0);
        if (selectedConfigs[0] >= 0)
            chain->SetAngles(startConfigs[selectedConfigs[0]].x, startConfigs[selectedConfigs[0]].y);
    }
	if (end) {
        glm::vec2 selectedConfig = (selectedConfigs[1] >= 0 && keepSelection) ? endConfigs[selectedConfigs[1]] : glm::vec2(-1.f);
        selectedConfigs[1] = -1;

		if (targets[1] != glm::vec2(0.f))
            endConfigs = chain->InverseKinematics(targets[1]);

        for (int i = 0; i < endConfigs.size(); i++) {
            if (confSpace->CheckCollision(endConfigs[i])) {
                endConfigs.erase(endConfigs.begin() + i);
                i--;
            }
            else if (selectedConfig == endConfigs[i]) {
                selectedConfigs[1] = i;
            }
        }
		selectedConfigs[1] = endConfigs.size() == 0 ? -1 : (selectedConfigs[1] >= 0 ? selectedConfigs[1] : 0);
        if (selectedConfigs[1] >= 0)
            endChain->SetAngles(endConfigs[selectedConfigs[1]].x, endConfigs[selectedConfigs[1]].y);
	}
}

void clearPath()
{
	path = {};
	pathIndex = -1;
}

bool isCreatingRectangle()
{
	return rectangles.size() != 0 && rectangles[rectangles.size() - 1]->IsInCreation();
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
