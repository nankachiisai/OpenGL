#define _CRT_SECURE_NO_WARNINGS

#include <GL/glew.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static void display(void);
static void idle(void);
static int getShaderSource(char *fileName, GLenum shaderType, GLuint *compiledProgram);
static int useShaders(GLuint VertShader, GLuint FragShader, GLuint *program);
static int freeShaders(GLuint VertShader, GLuint FragShader, GLuint program);
static int transferData(float *data, int num, GLuint *VBO);
static int bindAttributeVariable(GLuint program, GLuint VBO, char *name);
static int bindUniformVariable4x4(GLuint program, float *data, char *name);

const double PI = 3.14159;

static float vertices[] = {
	-1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f
};

static float colors[] = {
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
};

static float rotationMatrix[] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

static GLuint program;

int main(int argc, char *argv[]) {
	GLenum err;
	GLuint vShader, fShader;
	GLuint VBO[2];

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

	// アイドル関数の設定
	glutIdleFunc(idle);

	// シェーダを読み込む
	getShaderSource("main.vert", GL_VERTEX_SHADER, &vShader);
	getShaderSource("main.frag", GL_FRAGMENT_SHADER, &fShader);
	useShaders(vShader, fShader, &program);

	// データをGPUに転送する
	transferData(vertices, sizeof(vertices), &VBO[0]);
	transferData(colors, sizeof(colors), &VBO[1]);

	// VBOとバーテックスシェーダのin変数とを関連付ける
	bindAttributeVariable(program, VBO[0], "position");
	bindAttributeVariable(program, VBO[1], "vColor");

	// メインループ
	glutMainLoop();

	// シェーダオブジェクト、シェーダプログラムを破棄する
	freeShaders(vShader, fShader, program);
	program = 0;

	// VBOを削除する
	glDeleteBuffers(2, VBO);

	return 0;
}

static void display(void) {
	static double degree = 0.0;
	double rad;
	GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	// ウィンドウを白で塗りつぶす
	glClearBufferfv(GL_COLOR, 0, white);

	// 回転行列を生成する
	rad = degree * PI / 180.0;
	rotationMatrix[0] = cos(rad);
	rotationMatrix[1] = -sin(rad);
	rotationMatrix[4] = sin(rad);
	rotationMatrix[5] = cos(rad);

	// 回転行列をuniform変数に関連付ける
	bindUniformVariable4x4(program, rotationMatrix, "rotationMatrix");

	// 描画
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glFlush();

	if (fabs(degree - 360.0) < 0.01) {
		degree = 0.0;
	}
	else {
		degree += 0.01;
	}

	return;
}

void idle(void) {
	glutPostRedisplay();
}

static int getShaderSource(char *fileName, GLenum shaderType, GLuint *compiledProgram) {
	char *source;
	int capacity = 1024;
	int usage = 0;
	FILE *fp;
	GLuint shader;
	GLint compileStatus;
	int returnValue;

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

	// ファイルをクローズ
	returnValue = fclose(fp);
	if (returnValue == EOF) {
		return -1;
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
	source = NULL;

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

static int transferData(float *data, int num, GLuint *VBO) {
	// VBOを作成する
	glGenBuffers(1, VBO);

	// VBOにデータをセットする
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num, data, GL_STATIC_DRAW);

	return 0;
}

static int bindAttributeVariable(GLuint program, GLuint VBO, char *name) {
	GLint attr;

	// VBOをバインドする
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// VBOとin変数を関連付ける
	attr = glGetAttribLocation(program, name);
	glEnableVertexAttribArray(attr);
	glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, 0, 0);

	return 0;
}

static int bindUniformVariable4x4(GLuint program, float *data, char *name) {
	GLint uniform;

	// uniform変数の場所をgetする
	uniform = glGetUniformLocation(program, name);

	// 4x4の行列をuniform変数に関連付ける
	glUniformMatrix4fv(uniform, 1, GL_FALSE, data);

	return 0;
}