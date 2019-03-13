#define _CRT_SECURE_NO_WARNINGS

#include <GL/glew.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

static void display(void);
static int getShaderSource(char *fileName, GLenum shaderType, GLuint *compiledProgram);
static int useShaders(GLuint VertShader, GLuint FragShader, GLuint *program);
static int freeShaders(GLuint VertShader, GLuint FragShader, GLuint program);

static float vertices[] = {
	-1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f
};

int main(int argc, char *argv[]) {
	GLenum err;
	GLuint vShader, fShader;
	GLuint program;
	GLuint VBO;
	GLint attr;

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

	// シェーダを読み込む
	getShaderSource("main.vert", GL_VERTEX_SHADER, &vShader);
	getShaderSource("main.frag", GL_FRAGMENT_SHADER, &fShader);
	useShaders(vShader, fShader, &program);

	// VBOを作成する
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// VBOにデータをセットする
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// VBOとattribute変数を関連付ける
	attr = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attr);
	glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// メインループ
	glutMainLoop();

	// シェーダオブジェクト、シェーダプログラムを破棄する
	freeShaders(vShader, fShader, program);

	// VBOとattribute変数の関連付けを外す
	glDisableVertexAttribArray(attr);

	// VBOを削除する
	glDeleteBuffers(1, &VBO);

	return 0;
}

static void display(void) {
	GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };

	// ウィンドウを赤で塗りつぶす
	glClearBufferfv(GL_COLOR, 0, red);

	// 描画
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glFlush();

	return;
}

static int getShaderSource(char *fileName, GLenum shaderType, GLuint *compiledProgram) {
	char *source;
	int capacity = 1024;
	int usage = 0;
	FILE *fp;
	GLuint shader;
	GLint compileStatus;

	// とりあえず1KB確保
	source = malloc(sizeof(char) * capacity);
	if (source == NULL) {
		return -1;
	}

	// ファイルオープン
	fp = fopen(fileName, "r");
	if (fp == NULL) {
		return -1;
	}

	// 終端までファイルを読み取る
	while (1) {
		int num;

		// バッファが満杯であれば、realloc
		if (capacity == usage) {
			char *tmpp;

			tmpp = realloc(source, capacity * 2);
			if (tmpp == NULL) {
				return -1;
			}
			source = tmpp;
			capacity *= 2;
		}

		// 文字列読み込み
		num = fread(&source[usage], sizeof(char), capacity - usage, fp);
		usage += num;

		// 読み取れるものがなくなったら終了
		if (num == 0) {
			// 念のため、Null terminateしておく
			source[usage] = '\0';
			usage++;

			break;
		}
	}

	// シェーダオブジェクトを作成する
	shader = glCreateShader(shaderType);

	// コンパイル
	glShaderSource(shader, 1, &source, &usage);
	glCompileShader(shader);

	// コンパイルステータスをgetする
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		return -1;
	}

	// freeする
	free(source);

	// コンパイル済みシェーダオブジェクトを返す
	*compiledProgram = shader;

	return 0;
}

static int useShaders(GLuint VertShader, GLuint FragShader, GLuint *program) {
	GLuint shaderp;
	GLint linkStatus;

	// シェーダプログラムを作成
	shaderp = glCreateProgram();

	// バーテックスシェーダとフラグメントシェーダをアタッチする
	glAttachShader(shaderp, VertShader);
	glAttachShader(shaderp, FragShader);

	// シェーダをリンクする
	glLinkProgram(shaderp);

	// リンクステータスをgetする
	glGetProgramiv(shaderp, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE) {
		return -1;
	}

	// シェーダプログラムを適用する
	glUseProgram(shaderp);

	// シェーダプログラムを返す
	*program = shaderp;

	return 0;
}

static int freeShaders(GLuint VertShader, GLuint FragShader, GLuint program) {
	// シェーダオブジェクトを削除する
	glDeleteShader(VertShader);
	glDeleteShader(FragShader);

	// シェーダプログラムを削除する
	glDeleteProgram(program);

	return 0;
}