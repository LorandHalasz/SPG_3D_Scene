#version 410 core

in vec3 textureCoordinates;

out vec4 color;

uniform samplerCube skybox;
uniform float fogDensity;

void main()
{
	float fogFactor = clamp(exp(-pow(25 * fogDensity, 2)), 0.0f, 1.0f);
	vec4 fogColor = vec4(0.4f, 0.2f, 0.1f, 1.0f);
    color = fogColor * (1.0f - fogFactor) + texture(skybox, textureCoordinates) * fogFactor;
}
