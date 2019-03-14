#version 450 core

in vec3 position;
in vec3 vColor;
out vec3 fColor;
uniform mat4 rotationMatrix;

void main(void) {
	gl_Position = rotationMatrix * vec4(position, 1.0);
	fColor = vColor;
}