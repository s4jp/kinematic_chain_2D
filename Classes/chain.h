#pragma once

#include "figure.h"

class Chain : public Figure
{
public:
	Chain(float L1, float L2, float angle1 = 0, float angle2 = 0);

	void Render(int colorLoc) override;
	void RenderCircles(int colorLoc);

	void UpdateLengths(float L1, float L2);
	void SetAngles(float a1, float a2);
	std::vector<glm::vec2> CalculateJoints(float a1, float a2) const;
	std::vector<glm::vec2> InverseKinematics(glm::vec2 target) const;
	void SetColor(glm::vec4 color);
	glm::vec2 GetLengths();
private:
	float L1, L2;
	float angle1, angle2;
	glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 1.f);

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> InitializeAndCalculate(float L1, float L2, float angle1, float angle2);

	void NormalizeAngleInRadians(float& angle) const;
};