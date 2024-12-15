#pragma once

#include <vector>

#include "ControlledInputInt.h"
#include "rectangle.h"
#include "chain.h"

class ConfigurationSpace
{
public:
	ConfigurationSpace(int discrLevel = 360);
	void RenderImGui(Chain* chain, std::vector<Rectangle*> rectangles);
	void RenderTexture() const;

private:
	ControlledInputInt discrLevel;
	std::vector<std::vector<char>> table;
	GLuint texture;

	void ClearTable();
	void CalculateTable(Chain* chain, std::vector<Rectangle*> rectangles);
	GLuint CreateTexture() const;
};