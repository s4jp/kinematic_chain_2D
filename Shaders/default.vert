#version 460 core

layout (location = 0) in vec2 aPos;

uniform vec2 viewportSize;

void main()
{
	vec2 ndcPos = aPos / viewportSize * 2.f;

	gl_Position = vec4(ndcPos, 0.f, 1.f);
}