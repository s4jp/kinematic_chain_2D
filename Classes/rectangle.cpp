#include "rectangle.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Rectangle::Rectangle(glm::vec2 startPos) : Figure(InitializeAndCalculate(startPos))
{
	this->inCreation = true;
}

void Rectangle::Render(int colorLoc)
{
	vao.Bind();

	glUniform4fv(colorLoc, 1, glm::value_ptr(GetColor()));
	glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, 0);

	vao.Unbind();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Rectangle::InitializeAndCalculate(glm::vec2 startPos)
{
	this->startPos = startPos;
	this->endPos = startPos;

	return Calculate();
}

void Rectangle::UpdateEnd(glm::vec2 endPos)
{
	this->endPos = endPos;
	RefreshBuffers(Calculate());
}

glm::vec4 Rectangle::GetRectangle() const
{
	glm::vec2 start = glm::vec2(glm::min(startPos.x, endPos.x), glm::min(startPos.y, endPos.y));
	glm::vec2 end = glm::vec2(glm::max(startPos.x, endPos.x), glm::max(startPos.y, endPos.y));

	return glm::vec4(start, end);
}

glm::vec4 Rectangle::GetColor() const
{
	if (inCreation)
		// very light gray
		return glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	else
		return glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
}

void Rectangle::Print() const
{
	std::cout << "(" << startPos.x << " x " << startPos.y << ")" << " ";
	std::cout << "(" << endPos.x << " x " << endPos.y << ")" << std::endl;
}

char Rectangle::CheckCollision(const std::vector<glm::vec2> joints) const
{
	glm::vec4 rect = GetRectangle();
	for (int i = 0; i < joints.size(); i++) {
		if (joints[i].x >= rect.x && joints[i].x <= rect.z &&
			joints[i].y >= rect.y && joints[i].y <= rect.w)
			return true;
	}
	return false;
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Rectangle::Calculate() const
{
	std::vector<GLfloat> vertices = {
		startPos.x, startPos.y,
		endPos.x, startPos.y,
		endPos.x, endPos.y,
		startPos.x, endPos.y
	};
	std::vector<GLuint> indices = {
		0, 1, 2,
		2, 3, 0
	};
	return std::make_tuple(vertices, indices);
}