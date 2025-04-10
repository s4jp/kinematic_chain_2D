#include "configurationSpace.h"
#include <queue>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float flashDuration = 0.5f;

const std::vector<glm::vec2> cardinalDirections = {
    {0, 1}, {1, 0}, {0, -1}, {-1, 0}
};

ConfigurationSpace::ConfigurationSpace(Chain* chain, std::vector<Rectangle*> rectangles, int discrLevel)
	: discrLevel("Discr. level", discrLevel, 1, 1)
{
	this->rectangles = rectangles;
	this->rectanglesCoords = GetRectangles(rectangles);
	this->lengths = chain->GetLengths();
	this->ClearTable();
	this->texture = CreateTexture();
}

bool ConfigurationSpace::RenderImGui(Chain* chain, std::vector<Rectangle*> rectangles)
{
	bool change = false;

	discrLevel.Render();
	if (ImGui::Button("Calculate")) {
		this->rectangles = rectangles;
		this->rectanglesCoords = GetRectangles(rectangles);
		this->lengths = chain->GetLengths();
		CalculateTable(chain);
		texture = CreateTexture();
		change = true;
		timeElapsed = 0.0f;
	}

	if (IsRectangleDiscrepancy(rectangles) || this->lengths != chain->GetLengths() || this->table.size() != discrLevel.GetValue()) {
		ImGui::SameLine();
		timeElapsed += ImGui::GetIO().DeltaTime;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, (std::sin(timeElapsed * (2.f * M_PI / flashDuration)) + 1.f) / 2.f));
		ImGui::Text("Pending changes!");
		ImGui::PopStyleColor();
	}

	return change;
}

void ConfigurationSpace::RenderTexture() const
{
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    ImGui::Begin("Colision texture", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Symbols legend: triangle - start, square - end");
		ImGui::Spacing();
        ImGui::Image((void*)(intptr_t)texture, ImVec2(table.size(), table.size()));
    ImGui::End();
}

glm::vec2 ConfigurationSpace::GetIndices(const glm::vec2 angles) const
{
	auto roundToNearest = [](glm::vec2 q, int n) {
		float step = 360.0f / n;
		q.x = round(q.x / step) * step;
		q.y = round(q.y / step) * step;

		while (q.x >= 360.0f) q.x -= 360.0f;
		while (q.y >= 360.0f) q.y -= 360.0f;

		return q;
	};

	int n = discrLevel.GetValue();
	glm::vec2 rounded = roundToNearest(angles, n);

	float step = 360.0f / n;
	int i = (int)(rounded.x / step);
	int j = (int)(rounded.y / step);
	return { i, j };
}

bool ConfigurationSpace::CheckCollision(glm::vec2 angles) const
{
	glm::vec2 idx = GetIndices(angles);
	return table[idx.x][idx.y];
}

std::vector<glm::vec2> ConfigurationSpace::FindShortestPath(glm::vec2 startConf , glm::vec2 endConf)
{
	auto modulo = [](int x, int n) {
		return (x % n + n) % n;
	};

	glm::vec2 startIdx = GetIndices(startConf);
	glm::vec2 endIdx = GetIndices(endConf);

	int n = table.size();

    std::queue<glm::vec2> q;
    std::unordered_map<int, glm::vec2> backtrack;
	std::unordered_map<int, float> distance;

    q.push(startIdx);
	backtrack[startIdx.x * n + startIdx.y] = { -1, -1 }; // guard value
	distance[startIdx.x * n + startIdx.y] = 0;

    while (!q.empty()) {
        glm::vec2 current = q.front();
        q.pop();

        if (current == endIdx) {
            std::vector<glm::vec2> path;
            for (glm::vec2 at = endIdx; at.x != -1; at = backtrack[at.x * n + at.y]) {
                path.push_back(at);
            }
            std::reverse(path.begin(), path.end());

			this->texture = CreateTexture({ path, distance });
			for (auto& p : path) {
				p = GetAngles(p);
			}
            return path;
        }

        for (const auto& dir : cardinalDirections) {
            glm::vec2 neighbor = current + dir;
			int x = modulo(neighbor.x, n), y = modulo(neighbor.y, n);
			int neighborKey = x * n + y;

            if (table[x][y] == 0 && backtrack.find(x * n + y) == backtrack.end()) {
				q.push({ x,y });
				backtrack[neighborKey] = current;
				distance[neighborKey] = distance[current.x * n + current.y] + 1;
            }
        }
    }

	this->texture = CreateTexture({ {startIdx, endIdx }, distance });
	return {};
}

glm::vec2 ConfigurationSpace::GetAngles(const glm::vec2 indices) const
{
	int n = discrLevel.GetValue();
	float step = 360.0f / n;
	return { indices.x * step, indices.y * step };
}

bool ConfigurationSpace::IsRectangleDiscrepancy(const std::vector<Rectangle*>& rectangles) const
{
	std::vector<glm::vec4> newRectanglesCoords = GetRectangles(rectangles);

	if (this->rectanglesCoords.size() != newRectanglesCoords.size()) return true;

	for (int i = 0; i < rectangles.size(); i++) {
		if (this->rectanglesCoords[i] != newRectanglesCoords[i]) return true;
	}

	return false;
}

std::vector<glm::vec4> ConfigurationSpace::GetRectangles(const std::vector<Rectangle*>& rectangles) const
{
	std::vector<glm::vec4> result;

	for (int i = 0; i < rectangles.size(); i++) {
		result.push_back(rectangles[i]->GetRectangle());
	}

	return result;
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

GLuint ConfigurationSpace::CreateTexture(std::pair<std::vector<glm::vec2>, std::unordered_map<int, float>> pd) const
{
    int n = table.size();
    if (n == 0 || table[0].size() == 0) return 0;

    std::vector<unsigned char> pixels(n * n * 4);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int index = (i * n + j) * 4;
            if (table[i][j] == 0) {
                pixels[index + 0] = 0;		// R
                pixels[index + 1] = 0;		// G
                pixels[index + 2] = 0;		// B
                pixels[index + 3] = 255;    // A
            }
            else {
                pixels[index + 0] = 255;    // R
                pixels[index + 1] = 255;    // G
                pixels[index + 2] = 255;    // B
                pixels[index + 3] = 255;    // A
            }
        }
    }

	std::unordered_map<int, float> distance = pd.second;

	float maxDistance = 0;
	for (const auto& d : distance) {
		maxDistance = std::max(maxDistance, d.second);
	}
	for (auto& d : distance) {
		d.second = 1 - d.second / maxDistance;
	}

	for (const auto& d : distance) {
		int index = (d.first) * 4;
		pixels[index + 0] = 0;					// R
		pixels[index + 1] = 255 * d.second;     // G
		pixels[index + 2] = 0;					// B
		pixels[index + 3] = 255;				// A
	}

	std::vector<glm::vec2> path = pd.first;

	for (const auto& p : path) {
        int index = (p.x * n + p.y) * 4;
		pixels[index + 0] = 255;				// R
		pixels[index + 1] = 0;					// G
		pixels[index + 2] = 0;					// B
		pixels[index + 3] = 255;				// A
	}

	auto paintBlueSquare = [&](glm::vec2 center, int length = 15) {
		int halfLen = length / 2.f;
		for (int i = -halfLen; i <= halfLen; ++i) {
			for (int j = -halfLen; j <= halfLen; ++j) {
				int x = center.x + i;
				int y = center.y + j;

				if (x >= 0 && x < n && y >= 0 && y < n) {
					int index = (x * n + y) * 4;
					pixels[index + 0] = 0;     // R
					pixels[index + 1] = 0;     // G
					pixels[index + 2] = 255;   // B
					pixels[index + 3] = 255;   // A
				}
			}
		}
	};
	auto paintBlueTriangle = [&](glm::vec2 center, int length = 15) {
		float height = (sqrt(3.f) / 2.f) * length;
		glm::vec2 p1 = center + glm::vec2(0.f, -height / 2.f);
		glm::vec2 p2 = center + glm::vec2(-length / 2.f, height / 2.f);
		glm::vec2 p3 = center + glm::vec2(length / 2.f, height / 2.f);

		auto isPointInsideTriangle = [&](int x, int y) {
			auto sign = [](glm::vec2 p1, glm::vec2 p2, glm::vec2 p3) {
				return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
				};

			glm::vec2 pt(x, y);
			bool b1, b2, b3;

			b1 = sign(pt, p1, p2) < 0.f;
			b2 = sign(pt, p2, p3) < 0.f;
			b3 = sign(pt, p3, p1) < 0.f;

			return (b1 == b2) && (b2 == b3);
			};

		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				if (isPointInsideTriangle(i, j)) {
					int index = (i * n + j) * 4;
					pixels[index + 0] = 0;     // R
					pixels[index + 1] = 0;     // G
					pixels[index + 2] = 255;   // B
					pixels[index + 3] = 255;   // A
				}
			}
		}
	};

	if (path.size() >= 2){
		paintBlueTriangle(path[0]);
		paintBlueSquare(path[path.size() - 1]);
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
