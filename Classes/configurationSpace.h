#pragma once

#include <vector>

#include "ControlledInputInt.h"
#include "rectangle.h"
#include "chain.h"

class ConfigurationSpace
{
public:
	ConfigurationSpace(int discrLevel = 360);

	bool RenderImGui(Chain* chain, std::vector<Rectangle*> rectangles);
	void RenderTexture() const;

	void RoundToNearest(glm::vec2& angles) const;
	bool CheckCollision(glm::vec2 angles) const;
private:
	ControlledInputInt discrLevel;
	std::vector<std::vector<char>> table;
	GLuint texture;
	std::vector<Rectangle*> rectangles;
	glm::vec2 lengths;

	void ClearTable();
	void CalculateTable(Chain* chain);
	GLuint CreateTexture() const;
};