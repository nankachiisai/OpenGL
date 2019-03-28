#version 450 core

in vec3 position;
uniform mat4 transformMatrix;

void main(void) {
	gl_Position = transformMatrix * vec4(position, 1.0);
}