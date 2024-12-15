#pragma once

#include "figure.h"

class Rectangle : public Figure
{
public:
	bool inCreation;

	Rectangle(glm::vec2 startPos);

	void Render(int colorLoc) override;
	
	void UpdateEnd(glm::vec2 endPos);
	glm::vec4 GetRectangle() const;
	glm::vec4 GetColor() const;
	void Print() const;
	char CheckCollision(const std::vector<glm::vec2> joints) const;
private:
	glm::vec2 startPos;
	glm::vec2 endPos;

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> InitializeAndCalculate(glm::vec2 startPos);
};