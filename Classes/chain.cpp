#include "chain.h"
#include <glm/gtc/type_ptr.hpp>

const float lineWidth = 10.0f;
const float defaultLineWidth = 1.0f;

Chain::Chain(glm::vec2 startPos, float L1, float L2, float angle1, float angle2) 
	: Figure(InitializeAndCalculate(startPos, L1, L2, angle1, angle2)) {}

void Chain::Render(int colorLoc)
{
	vao.Bind();

	glLineWidth(lineWidth);

	glUniform4fv(colorLoc, 1, glm::value_ptr(glm::vec4(1,1,1,1)));
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

glm::vec2 Chain::GetAngles() const
{
	return glm::vec2(angle1, angle2);
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Chain::InitializeAndCalculate(glm::vec2 startPos, float L1, float L2, float angle1, float angle2)
{
	this->startPos = startPos;
	this->L1 = L1;
	this->L2 = L2;
	this->angle1 = angle1;
	this->angle2 = angle2;

	return Calculate();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Chain::Calculate()
{
	joints.clear();
	joints.push_back(startPos);
	joints.push_back(joints[0] + glm::vec2(L1 * cos(glm::radians(angle1)), L1 * sin(glm::radians(angle1))));
	joints.push_back(joints[1] + glm::vec2(L2 * cos(glm::radians(angle1 + angle2)), L2 * sin(glm::radians(angle1 + angle2))));

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