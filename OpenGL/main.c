#define _CRT_SECURE_NO_WARNINGS

#include <GL/glew.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "bunny.h"

static void display(void);
static void idle(void);
static int getShaderSource(char *fileName, GLenum shaderType, GLuint *compiledProgram);
static int useShaders(GLuint VertShader, GLuint FragShader, GLuint *program);
static int freeShaders(GLuint VertShader, GLuint FragShader, GLuint program);
static int transferData(float *data, int num, GLenum bufferType, GLuint *buffer);
static int bindAttributeVariable(GLuint program, GLuint VBO, char *name);
static int bindUniformVariable4x4(GLuint program, float *data, char *name);
static int loadBunny(char *filename, bunny *b);
static int freeBunny(bunny *b);

const double PI = 3.14159;

static float vertices[] = {
	-1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f
};

static float rotationMatrix[] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

static float expantionMatrix[] = {
	5.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 5.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 5.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

static bunny b;

static GLuint program;
GLuint VBO; // Vertex Buffer Object
GLuint IBO; // Index Buffer Object

int main(int argc, char *argv[]) {
	GLenum err;
	GLuint vShader, fShader;
	int returnValue;

	// スタンフォードバニーの読み込み
	returnValue = loadBunny("bun_zipper.ply", &b);
	if (returnValue == -1) {
		return -1;
	}

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
	transferData(b.vertices, b.vertexNum, GL_ARRAY_BUFFER, &VBO);
	transferData(b.vertexIndices, b.indexNum, GL_ELEMENT_ARRAY_BUFFER, &IBO);

	// VBOとバーテックスシェーダのin変数とを関連付ける
	bindAttributeVariable(program, VBO, "position");

	// メインループ
	glutMainLoop();

	// シェーダオブジェクト、シェーダプログラムを破棄する
	freeShaders(vShader, fShader, program);
	program = 0;

	// バッファを削除する
	glDeleteBuffers(1, VBO);
	glDeleteBuffers(1, IBO);

	// スタンフォードバニーを削除する
	freeBunny(&b);

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
	rotationMatrix[2] = sin(rad);
	rotationMatrix[8] = -sin(rad);
	rotationMatrix[10] = cos(rad);

	// 回転行列、拡大行列をuniform変数に関連付ける
	bindUniformVariable4x4(program, rotationMatrix, "rotationMatrix");
	bindUniformVariable4x4(program, expantionMatrix, "expantionMatrix");

	// 描画
	glDrawElements(GL_TRIANGLES, b.indexNum, GL_UNSIGNED_INT, 0);

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
	fp = NULL;

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

static int transferData(float *data, int num, GLenum bufferType, GLuint *buffer) {
	// バッファを作成する
	glGenBuffers(1, buffer);

	// バッファにデータをセットする
	glBindBuffer(bufferType, *buffer);
	glBufferData(bufferType, sizeof(float) * num, data, GL_STATIC_DRAW);

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

static int loadBunny(char *filename, bunny *b) {
	FILE *fp;
	int returnValue;
	char *buf;
	int strLength;
	int cmpResult;
	float x, y, z, confidence, intensity;
	int tmp, ix, iy, iz;
	float *vertices;
	int *indices;

	//ファイルオープン
	fp = fopen(filename, "r");
	if (fp == NULL) {
		return -1;
	}

	// 読み取り用バッファを1KB確保
	buf = (char *) malloc(sizeof(char) * 1024);
	if (buf == NULL) {
		return -1;
	}

	// end_headerまで読み飛ばす
	while (1) {
		// 行を読み込む
		returnValue = fgets(buf, sizeof(char) * 1024, fp);
		if (returnValue == NULL) {
			return -1;
		}

		// 文字列比較
		strLength = strlen(buf);
		cmpResult = strncmp(buf, "end_header\n", strLength);
		if (cmpResult == 0) {
			// ヘッダを読み終えた
			break;
		}
		
	}

	// 読み取り用バッファをfreeする
	free(buf);

	// 頂点配列を確保する。35947 * 3 * 4 = 431,364バイト必要。
	// 頂点配列は、使用後、freeすること。
	const int vertNum = 35947 * 3; // 要素数
	vertices = malloc(sizeof(float) * vertNum);
	if (vertices == NULL) {
		return -1;
	}

	// ひたすら読み込んでいく
	for (int i = 0; i < 35947; i++) {
		fscanf(fp, "%f %f %f %f %f", &x, &y, &z, &confidence, &intensity);
		vertices[3 * i + 0] = x;
		vertices[3 * i + 1] = y;
		vertices[3 * i + 2] = z;
	}

	// 頂点インデックス配列を確保する。69451 * 3 * 4 = 833,412バイト必要。
	// 頂点インデックス配列は、使用後、freeすること。
	const int idxNum = 69451 * 3;	// 要素数
	indices = malloc(sizeof(unsigned int) * idxNum);
	if (indices == NULL) {
		return -1;
	}

	// ひたすら読み込む
	for (int i = 0; i < 69451; i++) {
		fscanf(fp, "%d %d %d %d", &tmp, &ix, &iy, &iz);
		indices[3 * i + 0] = ix;
		indices[3 * i + 1] = iy;
		indices[3 * i + 2] = iz;
	}

	// ファイルクローズ
	returnValue = fclose(fp);
	if (returnValue == EOF) {
		return -1;
	}
	fp = NULL;

	// 頂点配列を返す
	b->vertices = vertices;
	b->vertexNum = vertNum;

	// 頂点インデックス配列を返す
	b->vertexIndices = indices;
	b->indexNum = idxNum;

	return 0;
}

static int freeBunny(bunny *b) {
	free(b->vertices);
	b->vertices = NULL;
	b->vertexNum = 0;

	free(b->vertexIndices);
	b->vertexIndices = NULL;
	b->indexNum = 0;

	return 0;
}