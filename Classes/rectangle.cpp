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

    auto isInside = [&rect](const glm::vec2& point) -> bool {
        return point.x >= rect.x && point.x <= rect.z &&
            point.y >= rect.y && point.y <= rect.w;
     };

    auto lineIntersect = [](const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& q1, const glm::vec2& q2) -> bool {
        auto orientation = [](const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) -> float {
            return (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);
            };

        auto onSegment = [](const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) -> bool {
            return c.x >= std::min(a.x, b.x) && c.x <= std::max(a.x, b.x) &&
                c.y >= std::min(a.y, b.y) && c.y <= std::max(a.y, b.y);
            };

        float o1 = orientation(p1, p2, q1);
        float o2 = orientation(p1, p2, q2);
        float o3 = orientation(q1, q2, p1);
        float o4 = orientation(q1, q2, p2);

        if (o1 * o2 < 0 && o3 * o4 < 0)
            return true;

        // on-line cases
        if (o1 == 0 && onSegment(p1, p2, q1)) return true;
        if (o2 == 0 && onSegment(p1, p2, q2)) return true;
        if (o3 == 0 && onSegment(q1, q2, p1)) return true;
        if (o4 == 0 && onSegment(q1, q2, p2)) return true;

        return false;
     };

    glm::vec2 rectEdges[] = {
        {rect.x, rect.y}, {rect.z, rect.y}, {rect.z, rect.w}, {rect.x, rect.w}
    };

    for (auto& joint : joints) {
        if (isInside(joint)) {
            return true;
        }
    }

    for (size_t i = 0; i < joints.size() - 1; ++i) {
        glm::vec2 joint1 = joints[i];
        glm::vec2 joint2 = joints[i + 1];

        for (int j = 0; j < 4; ++j) {
            glm::vec2 edgeStart = rectEdges[j];
            glm::vec2 edgeEnd = rectEdges[(j + 1) % 4];

            if (lineIntersect(joint1, joint2, edgeStart, edgeEnd)) {
                return true;
            }
        }
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