#include <GL/glew.h>
#include <GL/glut.h>

static void display(void);

int main(int argc, char *argv[]) {
	GLenum err;

	// GLUT�̏�����
	glutInit(&argc, argv);

	// �E�B���h�E�̍쐬
	glutCreateWindow(argv[0]);

	// glew�̏�����
	err = glewInit();
	if (err != GLEW_OK) {
		return -1;
	}

	// �`��֐��̐ݒ�
	glutDisplayFunc(display);

	// ���C�����[�v
	glutMainLoop();

	return 0;
}

static void display(void) {
	GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };

	// �E�B���h�E��Ԃœh��Ԃ�
	glClearBufferfv(GL_COLOR, 0, red);
	glFlush();

	return;
}