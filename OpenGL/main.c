#include <GL/glew.h>
#include <GL/glut.h>

static void display(void);

int main(int argc, char *argv[]) {
	GLenum err;

	// GLUTの初期化
	glutInit(&argc, argv);

	// ウィンドウの作成
	glutCreateWindow(argv[0]);

	// glewの初期化
	err = glewInit();
	if (err != GLEW_OK) {
		return -1;
	}

	// 描画関数の設定
	glutDisplayFunc(display);

	// メインループ
	glutMainLoop();

	return 0;
}

static void display(void) {
	GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };

	// ウィンドウを赤で塗りつぶす
	glClearBufferfv(GL_COLOR, 0, red);
	glFlush();

	return;
}