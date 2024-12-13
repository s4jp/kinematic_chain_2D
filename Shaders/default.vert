#version 460 core

layout (location = 0) in vec2 aPos;

uniform vec2 screenSize;
uniform bool isNormalized;

void main()
{
   vec2 ndcPos = aPos;

   if (!isNormalized) 
		ndcPos = ndcPos / screenSize * 2.f - 1.f;

   gl_Position = vec4(ndcPos, 0.f, 1.f);
}