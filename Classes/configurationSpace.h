#pragma once

#include <vector>

#include "ControlledInputInt.h"
#include "rectangle.h"
#include "chain.h"

class ConfigurationSpace
{
public:
	ConfigurationSpace(Chain* chain, std::vector<Rectangle*> rectangles, int discrLevel = 360);

	bool RenderImGui(Chain* chain, std::vector<Rectangle*> rectangles);
	void RenderTexture() const;

	void RoundToNearest(glm::vec2& angles) const;
	bool CheckCollision(glm::vec2 angles) const;
	std::vector<glm::vec2> FindShortestPath(glm::vec2 startIdx, glm::vec2 endIdx);
	void AddPointsToTexture(std::vector<glm::vec2> points);
private:
	ControlledInputInt discrLevel;
	std::vector<std::vector<char>> table;
	GLuint texture;
	std::vector<Rectangle*> rectangles;
	glm::vec2 lengths;

	void ClearTable();
	void CalculateTable(Chain* chain);
	GLuint CreateTexture(std::vector<glm::vec2> path = {}) const;
};