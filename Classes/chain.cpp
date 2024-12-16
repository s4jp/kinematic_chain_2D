#include "chain.h"
#include <glm/gtc/type_ptr.hpp>

const float lineWidth = 10.0f;
const float defaultLineWidth = 1.0f;
const float sameAngleThreshold = 0.1f;

Chain::Chain(float L1, float L2, float angle1, float angle2) 
	: Figure(InitializeAndCalculate(L1, L2, angle1, angle2)) {}

void Chain::Render(int colorLoc)
{
	vao.Bind();

	glLineWidth(lineWidth);

	glUniform4fv(colorLoc, 1, glm::value_ptr(color));
	glDrawElements(GL_LINE_STRIP, indices_count, GL_UNSIGNED_INT, 0);

	glLineWidth(defaultLineWidth);

	vao.Unbind();
}

void Chain::UpdateLengths(float L1, float L2)
{
	this->L1 = L1;
	this->L2 = L2;

	RefreshBuffers(Calculate());
}

void Chain::SetAngles(float a1, float a2)
{
	angle1 = a1;
	angle2 = a2;
	RefreshBuffers(Calculate());
}

std::vector<glm::vec2> Chain::CalculateJoints(float a1, float a2) const
{
	std::vector<glm::vec2> newJoints;

	newJoints.push_back(glm::vec2(0.f));
	newJoints.push_back(newJoints[0] + glm::vec2(L1 * cos(glm::radians(a1)), L1 * sin(glm::radians(a1))));
	newJoints.push_back(newJoints[1] + glm::vec2(L2 * cos(glm::radians(a1 + a2)), L2 * sin(glm::radians(a1 + a2))));

	return newJoints;
}

std::vector<glm::vec2> Chain::InverseKinematics(glm::vec2 target) const
{
	float x = target.x;
	float y = target.y;
	float dist = sqrt(x * x + y * y);

	if (dist > L1 + L2)
		return {};

	auto calculateA1 = [&](float a2) {
		float a1 = atan2(y, x) - atan2(L2 * sin(a2), L1 + L2 * cos(a2));
		return a1;
	};

	std::vector<glm::vec2> angles;

	float cosA2 = (dist * dist - L1 * L1 - L2 * L2) / (2.f * L1 * L2);
	cosA2 = glm::clamp(cosA2, -1.f, 1.f);

	float a2 = acos(cosA2);
	float a2alt = 2.f * M_PI - a2;
	float a1 = calculateA1(a2);
	float a1alt = calculateA1(a2alt);

	NormalizeAngleInRadians(a1);
	NormalizeAngleInRadians(a1alt);
	NormalizeAngleInRadians(a2);
	NormalizeAngleInRadians(a2alt);

	angles.push_back({ glm::degrees(a1), glm::degrees(a2) });
	if (abs(a1 - a1alt) > sameAngleThreshold && 
		abs(a2 - a2alt) > sameAngleThreshold && 
		abs(a1 - a1alt) < 2 * M_PI - sameAngleThreshold && 
		abs(a2 - a2alt) < 2 * M_PI - sameAngleThreshold)
		angles.push_back({ glm::degrees(a1alt), glm::degrees(a2alt) });

	return angles;
}

void Chain::SetColor(glm::vec4 color)
{
	this->color = color;
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Chain::InitializeAndCalculate(float L1, float L2, float angle1, float angle2)
{
	this->L1 = L1;
	this->L2 = L2;
	this->angle1 = angle1;
	this->angle2 = angle2;

	return Calculate();
}

void Chain::NormalizeAngleInRadians(float& angle) const
{
	while (angle < 0.f)
		angle += 2.f * M_PI;
	while (angle >= 2.f * M_PI)
		angle -= 2.f * M_PI;
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Chain::Calculate() const
{
	auto joints = this->CalculateJoints(this->angle1, this->angle2);

	std::vector<GLfloat> vertices = {
		joints[0].x, joints[0].y,
		joints[1].x, joints[1].y,
		joints[2].x, joints[2].y,
	};
	std::vector<GLuint> indices = {
		0, 1, 2,
	};

	return std::make_tuple(vertices, indices);
}