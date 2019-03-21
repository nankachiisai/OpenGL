#version 450 core

in vec3 position;
uniform mat4 rotationMatrix;
uniform mat4 expantionMatrix;

void main(void) {
	gl_Position = expantionMatrix * rotationMatrix * vec4(position, 1.0);
}