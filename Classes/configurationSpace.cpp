#include "configurationSpace.h"

ConfigurationSpace::ConfigurationSpace(int discrLevel) 
	: discrLevel("Discr. level", discrLevel, 1, 1)
{
	this->ClearTable();
	this->texture = CreateTexture();
}

bool ConfigurationSpace::RenderImGui(Chain* chain, std::vector<Rectangle*> rectangles)
{
	bool change = false;

	discrLevel.Render();
	if (ImGui::Button("Calculate")) {
		this->rectangles = rectangles;
		this->lengths = chain->GetLengths();
		CalculateTable(chain);
		texture = CreateTexture();
		change = true;
	}
	if (this->rectangles != rectangles || this->lengths != chain->GetLengths()) {
		ImGui::SameLine();
		ImGui::Text("Pending changes!");
	}

	return change;
}

void ConfigurationSpace::RenderTexture() const
{
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    ImGui::Begin("Colision texture", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Image((void*)(intptr_t)texture, ImVec2(table.size(), table.size()));
    ImGui::End();
}

void ConfigurationSpace::RoundToNearest(glm::vec2& angles) const
{
	int n = discrLevel.GetValue();
	float step = 360.0f / n;
	angles.x = round(angles.x / step) * step;
	angles.y = round(angles.y / step) * step;

	if (angles.x >= 360.0f) angles.x = 0.0f;
	if (angles.y >= 360.0f) angles.y = 0.0f;
}

bool ConfigurationSpace::CheckCollision(glm::vec2 angles) const
{
	int n = discrLevel.GetValue();
	float step = 360.0f / n;
	int i = (int)(angles.x / step);
	int j = (int)(angles.y / step);
	return table[i][j];
}

void ConfigurationSpace::ClearTable()
{
	int n = discrLevel.GetValue();
	table.clear();
	table.resize(n, std::vector<char>(n, 0));
}

void ConfigurationSpace::CalculateTable(Chain* chain)
{
	this->ClearTable();

	int n = discrLevel.GetValue();
	float step = 360.0f / n;

	std::vector<glm::vec2> joints;

	for (int i = 0; i < n; i++) {
		float angle1 = i * step;

		for (int j = 0; j < n; j++) {
			float angle2 = j * step;

			joints = chain->CalculateJoints(angle1, angle2);

			for (Rectangle* rect : rectangles) {
				if (rect->CheckCollision(joints)) {
					table[i][j] = true;
					break;
				}
			}
		}
	}
}

GLuint ConfigurationSpace::CreateTexture() const
{
    int n = table.size();
    if (n == 0 || table[0].size() == 0) return 0;

    std::vector<unsigned char> pixels(n * n * 4);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int index = (i * n + j) * 4;
            if (table[i][j] == 1) {
                pixels[index + 0] = 255; // R
                pixels[index + 1] = 0;   // G
                pixels[index + 2] = 0;   // B
                pixels[index + 3] = 255; // A
            }
            else {
                pixels[index + 0] = 0;   // R
                pixels[index + 1] = 255; // G
                pixels[index + 2] = 0;   // B
                pixels[index + 3] = 255; // A
            }
        }
    }

    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, n, n, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}
