#include "axis.h"

Axis::Axis(glm::vec2 viewportSize) : Figure(Calculate(viewportSize)) {}

void Axis::Render(int colorLoc)
{
	vao.Bind();

	glUniform4f(colorLoc, 1.f, 0.f, 0.f, 1.f);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);

	glUniform4f(colorLoc, 0.f, 1.f, 0.f, 1.f);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(2 * sizeof(GLuint)));

	vao.Unbind();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Axis::Calculate(glm::vec2 viewportSize) const
{
	std::vector<GLfloat> vertices = {
		// x-axis
		0.f,					viewportSize.y / 2,
		viewportSize.x,			viewportSize.y / 2,
		// y-axis
		viewportSize.x / 2,		0.f,
		viewportSize.x / 2,		viewportSize.y
	};

	std::vector<GLuint> indices = {
		0, 1,
		2, 3
	};

	return { vertices, indices };
}
