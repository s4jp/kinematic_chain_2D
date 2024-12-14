#pragma once

#include "figure.h"

class Chain : public Figure
{
public:
	Chain(glm::vec2 startPos, float L1, float L2, float angle1 = 60, float angle2 = 45);

	void Render(int colorLoc) override;

	void UpdateLengths(float L1, float L2);
	glm::vec2 GetAngles() const;
private:
	glm::vec2 startPos;
	float L1, L2;
	float angle1, angle2;
	std::vector<glm::vec2> joints;

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate();
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> InitializeAndCalculate(glm::vec2 startPos, float L1, float L2, float angle1, float angle2);
};