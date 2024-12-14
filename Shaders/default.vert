#version 460 core

layout (location = 0) in vec2 aPos;

uniform vec2 viewportSize;
uniform bool inverseY;

void main()
{
	vec2 ndcPos = aPos / viewportSize * 2.f - 1.f;
	if (inverseY) ndcPos.y *= -1.f;

	gl_Position = vec4(ndcPos, 0.f, 1.f);
}