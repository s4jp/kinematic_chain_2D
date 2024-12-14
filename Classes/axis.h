#pragma once
#include "figure.h"

class Axis : public Figure
{
public:
	Axis(glm::vec2 viewportSize);

	void Render(int colorLoc) override;
private:
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate(glm::vec2 viewportSize) const;
};