#pragma once

#include "figure.h"
#include <string>

class Rectangle : public Figure
{
public:
	static int counter;

	std::string name;
	int movementMode = 0;

	Rectangle(glm::vec2 startPos);
	void RenderSelect(int colorLoc, bool selected);
	
	void UpdateEnd(glm::vec2 endPos);
	glm::vec4 GetRectangle() const;
	glm::vec4 GetColor(bool selected = false) const;
	void Print() const;
	char CheckCollision(const std::vector<glm::vec2> joints) const;
	void RenderImgui();
	void EndCreation();
	bool IsInCreation() const { return inCreation; }
private:
	glm::vec2 startPos;
	glm::vec2 endPos;
	bool inCreation;

	void Render(int colorLoc) override;

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> InitializeAndCalculate(glm::vec2 startPos);
};