#pragma once

#include <vector>

#include "ControlledInputInt.h"
#include "rectangle.h"
#include "chain.h"
#include <unordered_map>

class ConfigurationSpace
{
public:
	ConfigurationSpace(Chain* chain, std::vector<Rectangle*> rectangles, int discrLevel = 360);

	bool RenderImGui(Chain* chain, std::vector<Rectangle*> rectangles);
	void RenderTexture() const;

	bool CheckCollision(glm::vec2 angles) const;
	std::vector<glm::vec2> FindShortestPath(glm::vec2 startConf, glm::vec2 endConf);
private:
	ControlledInputInt discrLevel;
	std::vector<std::vector<char>> table;
	GLuint texture;
	std::vector<glm::vec4> rectanglesCoords;
	std::vector<Rectangle*> rectangles;
	glm::vec2 lengths;

	void ClearTable();
	void CalculateTable(Chain* chain);
	GLuint CreateTexture(std::pair<std::vector<glm::vec2>, std::unordered_map<int, float>> pd = {}) const;
	glm::vec2 GetIndices(const glm::vec2 angles) const;
	glm::vec2 GetAngles(const glm::vec2 indices) const;

	bool IsRectangleDiscrepancy(const std::vector<Rectangle*>& rectangles) const;
	std::vector<glm::vec4> GetRectangles(const std::vector<Rectangle*>& rectangles) const;
};