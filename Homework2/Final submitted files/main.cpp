#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "textfile.h"

#include "Vectors.h"
#include "Matrices.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define NUM_MODEL 5
#define PI 3.1415926 

#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

using namespace std;

// Default window size
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 800;

bool mouse_pressed = false;
int starting_press_x = -1;
int starting_press_y = -1;

enum TransMode
{
	GeoTranslation = 0,
	GeoRotation = 1,
	GeoScaling = 2,
	LightEdit = 3,
	ShininessEdit = 4,
};
TransMode cur_trans_mode = GeoTranslation;

struct Uniform
{
	GLint iLocMVP;
};
Uniform uniform;

vector<string> filenames; // .obj filename list

struct PhongMaterial
{
	Vector3 Ka;
	Vector3 Kd;
	Vector3 Ks;
	float shininess;
};

typedef struct
{
	GLuint vao;
	GLuint vbo;
	GLuint vboTex;
	GLuint ebo;
	GLuint p_color;
	int vertex_count;
	GLuint p_normal;
	PhongMaterial material;
	int indexCount;
	GLuint m_texture;
} Shape;

struct model
{
	Vector3 position = Vector3(0, 0, 0);
	Vector3 scale = Vector3(1, 1, 1);
	Vector3 rotation = Vector3(0, 0, 0);	// Euler form

	vector<Shape> shapes;
};
vector<model> models;

struct camera
{
	Vector3 position;
	Vector3 center;
	Vector3 up_vector;
};
camera main_camera;

struct project_setting
{
	GLfloat nearClip, farClip;
	GLfloat fovy;
	GLfloat aspect;
	GLfloat left, right, top, bottom;
};
project_setting proj;

Matrix4 view_matrix;
Matrix4 project_matrix;
Matrix4 model_matrix;

int cur_idx = 0; // represent which model should be rendered now

//-- Light -----------------------------------------------------------------
int lightType = 0;
float shininess = 64;
int vertex_perpixel = 0;

GLuint iLocV; // viewing matrix
GLuint iLocM; // model matrix
GLuint iLocLightType;
GLuint iLocKa;
GLuint iLocKd;
GLuint iLocKs;
GLuint iLocShininess;

struct iLocLightInfo {
	GLuint position;
	GLuint direction;
	GLuint spotDirection;
	GLuint ambient;
	GLuint diffuse;
	GLuint specular;
	GLuint spotExponent;
	GLuint spotCutoff;
	GLuint constantAttenuation;
	GLuint linearAttenuation;
	GLuint quadraticAttenuation;
}iLocLightInfo[3];

struct LightInfo {
	Vector3 position;
	Vector3 direction;
	Vector3 spotDirection;
	Vector3 ambient;
	Vector3 diffuse;
	Vector3 specular; 
	float spotExponent;
	float spotCutoff;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
}lightInfo[3];

void setUniformVariables(GLuint p)
{
	vertex_perpixel = glGetUniformLocation(p, "vertex_perpixel");

	uniform.iLocMVP = glGetUniformLocation(p, "mvp");
	iLocV = glGetUniformLocation(p, "view_matrix");
	iLocM = glGetUniformLocation(p, "model_matrix");
	iLocLightType = glGetUniformLocation(p, "light_type");
	iLocKa = glGetUniformLocation(p, "material.Ka");
	iLocKd = glGetUniformLocation(p, "material.Kd");
	iLocKs = glGetUniformLocation(p, "material.Ks");
	iLocShininess = glGetUniformLocation(p, "material.shininess");
	
	iLocLightInfo[0].position = glGetUniformLocation(p, "light[0].position");
	iLocLightInfo[0].ambient = glGetUniformLocation(p, "light[0].Ambient");
	iLocLightInfo[0].diffuse = glGetUniformLocation(p, "light[0].Diffuse");
	iLocLightInfo[0].specular = glGetUniformLocation(p, "light[0].Specular");
	iLocLightInfo[0].spotDirection = glGetUniformLocation(p, "light[0].spotDirection");
	iLocLightInfo[0].spotCutoff = glGetUniformLocation(p, "light[0].spotCutoff");
	iLocLightInfo[0].spotExponent = glGetUniformLocation(p, "light[0].spotExponent");
	iLocLightInfo[0].constantAttenuation = glGetUniformLocation(p, "light[0].constantAttenuation");
	iLocLightInfo[0].linearAttenuation = glGetUniformLocation(p, "light[0].linearAttenuation");
	iLocLightInfo[0].quadraticAttenuation = glGetUniformLocation(p, "light[0].quadraticAttenuation");

	iLocLightInfo[1].position = glGetUniformLocation(p, "light[1].position");
	iLocLightInfo[1].ambient = glGetUniformLocation(p, "light[1].Ambient");
	iLocLightInfo[1].diffuse = glGetUniformLocation(p, "light[1].Diffuse");
	iLocLightInfo[1].specular = glGetUniformLocation(p, "light[1].Specular");
	iLocLightInfo[1].spotDirection = glGetUniformLocation(p, "light[1].spotDirection");
	iLocLightInfo[1].spotCutoff = glGetUniformLocation(p, "light[1].spotCutoff");
	iLocLightInfo[1].spotExponent = glGetUniformLocation(p, "light[1].spotExponent");
	iLocLightInfo[1].constantAttenuation = glGetUniformLocation(p, "light[1].constantAttenuation");
	iLocLightInfo[1].linearAttenuation = glGetUniformLocation(p, "light[1].linearAttenuation");
	iLocLightInfo[1].quadraticAttenuation = glGetUniformLocation(p, "light[1].quadraticAttenuation");

	iLocLightInfo[2].position = glGetUniformLocation(p, "light[2].position");
	iLocLightInfo[2].ambient = glGetUniformLocation(p, "light[2].Ambient");
	iLocLightInfo[2].diffuse = glGetUniformLocation(p, "light[2].Diffuse");
	iLocLightInfo[2].specular = glGetUniformLocation(p, "light[2].Specular");
	iLocLightInfo[2].spotDirection = glGetUniformLocation(p, "light[2].spotDirection");
	iLocLightInfo[2].spotCutoff = glGetUniformLocation(p, "light[2].spotCutoff");
	iLocLightInfo[2].spotExponent = glGetUniformLocation(p, "light[2].spotExponent");
	iLocLightInfo[2].constantAttenuation = glGetUniformLocation(p, "light[2].constantAttenuation");
	iLocLightInfo[2].linearAttenuation = glGetUniformLocation(p, "light[2].linearAttenuation");
	iLocLightInfo[2].quadraticAttenuation = glGetUniformLocation(p, "light[2].quadraticAttenuation");
}

void updateLight()
{
	for (int i = 0; i < 3; i++) {
		glUniform3f(iLocLightInfo[i].ambient, lightInfo[i].ambient.x, lightInfo[i].ambient.y, lightInfo[i].ambient.z);
		glUniform3f(iLocLightInfo[i].diffuse, lightInfo[i].diffuse.x, lightInfo[i].diffuse.y, lightInfo[i].diffuse.z);
		glUniform3f(iLocLightInfo[i].specular, lightInfo[i].specular.x, lightInfo[i].specular.y, lightInfo[i].specular.z);
		glUniform3f(iLocLightInfo[i].position, lightInfo[i].position.x, lightInfo[i].position.y, lightInfo[i].position.z);

		if (i > 0) {
			glUniform1f(iLocLightInfo[i].constantAttenuation, lightInfo[i].constantAttenuation);
			glUniform1f(iLocLightInfo[i].linearAttenuation, lightInfo[i].linearAttenuation);
			glUniform1f(iLocLightInfo[i].quadraticAttenuation, lightInfo[i].quadraticAttenuation);
		}
	}
	glUniform3f(iLocLightInfo[2].spotDirection, lightInfo[2].spotDirection.x, lightInfo[2].spotDirection.y, lightInfo[2].spotDirection.z);
	glUniform1f(iLocLightInfo[2].spotExponent, lightInfo[2].spotExponent);
	glUniform1f(iLocLightInfo[2].spotCutoff, lightInfo[2].spotCutoff);
}

//--------------------------------------------------------------------------

static GLvoid Normalize(GLfloat v[3])
{
	GLfloat l;

	l = (GLfloat)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= l;
	v[1] /= l;
	v[2] /= l;
}

static GLvoid Cross(GLfloat u[3], GLfloat v[3], GLfloat n[3])
{

	n[0] = u[1] * v[2] - u[2] * v[1];
	n[1] = u[2] * v[0] - u[0] * v[2];
	n[2] = u[0] * v[1] - u[1] * v[0];
}

// V [TODO] given a translation vector then output a Matrix4 (Translation Matrix)
Matrix4 translate(Vector3 vec)
{
	Matrix4 mat;

	mat = Matrix4(
		1, 0, 0, vec.x,
		0, 1, 0, vec.y,
		0, 0, 1, vec.z,
		0, 0, 0, 1
	);

	return mat;
}

// V [TODO] given a scaling vector then output a Matrix4 (Scaling Matrix)
Matrix4 scaling(Vector3 vec)
{
	Matrix4 mat;

	mat = Matrix4(
		vec.x, 0, 0, 0,
		0, vec.y, 0, 0,
		0, 0, vec.z, 0,
		0, 0, 0, 1
	);

	return mat;
}

// V [TODO] given a float value then ouput a rotation matrix alone axis-X (rotate alone axis-X)
Matrix4 rotateX(GLfloat val)
{
	Matrix4 mat;

	mat = Matrix4(
		1, 0, 0, 0,
		0, cos(val), -sin(val), 0,
		0, sin(val), cos(val), 0,
		0, 0, 0, 1
	);

	return mat;
}

// V [TODO] given a float value then ouput a rotation matrix alone axis-Y (rotate alone axis-Y)
Matrix4 rotateY(GLfloat val)
{
	Matrix4 mat;

	mat = Matrix4(
		cos(val), 0, sin(val), 0,
		0, 1, 0, 0,
		-sin(val), 0, cos(val), 0,
		0, 0, 0, 1
	);

	return mat;
}

// V [TODO] given a float value then ouput a rotation matrix alone axis-Z (rotate alone axis-Z)
Matrix4 rotateZ(GLfloat val)
{
	Matrix4 mat;

	mat = Matrix4(
		cos(val), -sin(val), 0, 0,
		sin(val), cos(val), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);

	return mat;
}

Matrix4 rotate(Vector3 vec)
{
	return rotateX(vec.x)*rotateY(vec.y)*rotateZ(vec.z);
}

// V [TODO] compute viewing matrix accroding to the setting of main_camera
void setViewingMatrix()
{
	// view_matrix[...] = ...
	Vector3 P1P2 = main_camera.center - main_camera.position;
	Vector3 P1P3 = main_camera.up_vector;
	Vector3 Rz = -((P1P2).normalize());
	Vector3 Rx = P1P2.cross(P1P3).normalize();
	Vector3 Ry = Rz.cross(Rx).normalize();

	Matrix4 R = Matrix4(
		Rx[0], Rx[1], Rx[2], 0,
		Ry[0], Ry[1], Ry[2], 0,
		Rz[0], Rz[1], Rz[2], 0,
		0, 0, 0, 1
	);

	Matrix4 T = Matrix4(
		1, 0, 0, -main_camera.position.x,
		0, 1, 0, -main_camera.position.y,
		0, 0, 1, -main_camera.position.z,
		0, 0, 0, 1
	);

	view_matrix = R * T;
}

// V [TODO] compute persepective projection matrix
void setPerspective()
{
	// GLfloat f = ...
	// project_matrix [...] = ...
	GLfloat f = tan(proj.fovy * PI * 0.5 / 180);
	project_matrix = Matrix4(
		1.0 / (f * proj.aspect), 0, 0, 0,
		0, 1.0 / f, 0, 0,
		0, 0, (proj.farClip + proj.nearClip) / (proj.nearClip - proj.farClip), 2.0 * proj.farClip * proj.nearClip / (proj.nearClip - proj.farClip),
		0, 0, -1, 0
	);
}

void setGLMatrix(GLfloat* glm, Matrix4& m) {
	glm[0] = m[0];  glm[4] = m[1];   glm[8] = m[2];    glm[12] = m[3];
	glm[1] = m[4];  glm[5] = m[5];   glm[9] = m[6];    glm[13] = m[7];
	glm[2] = m[8];  glm[6] = m[9];   glm[10] = m[10];   glm[14] = m[11];
	glm[3] = m[12];  glm[7] = m[13];  glm[11] = m[14];   glm[15] = m[15];
}

// Vertex buffers
GLuint VAO, VBO;

// Call back function for window reshape
void ChangeSize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	// V [TODO] change your aspect ratio
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;
	proj.aspect = (float)width / (float)height;
	setPerspective();
}

// Render function for display rendering
void RenderScene(void) {	
	// clear canvas
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	updateLight();

	Matrix4 T, R, S;
	// V [TODO] update translation, rotation and scaling
	T = translate(models[cur_idx].position);
	R = rotate(models[cur_idx].rotation);
	S = scaling(models[cur_idx].scale);

	Matrix4 MVP;
	GLfloat mvp[16];

	// V [TODO] multiply all the matrix
	MVP = project_matrix * view_matrix * T * R * S;
	// row-major ---> column-major
	setGLMatrix(mvp, MVP);

	// use uniform to send mvp to vertex shader
	model_matrix = T * R * S;
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, view_matrix.getTranspose());
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, model_matrix.getTranspose());
	glUniformMatrix4fv(uniform.iLocMVP, 1, GL_FALSE, mvp);
	for (int i = 0; i < models[cur_idx].shapes.size(); i++)
	{
		Matrix4 N = T * S;

		glBindVertexArray(models[cur_idx].shapes[i].vao);
		
		// vertex or per-pixel lighting
		glUniform1i(vertex_perpixel, 0);
		glViewport(0, 0, float(WINDOW_WIDTH) / 2, WINDOW_HEIGHT);
		glDrawArrays(GL_TRIANGLES, 0, models[cur_idx].shapes[i].vertex_count);

		glUniform1i(vertex_perpixel, 1);
		glViewport(float(WINDOW_WIDTH) / 2, 0, float(WINDOW_WIDTH) / 2, WINDOW_HEIGHT);
		glDrawArrays(GL_TRIANGLES, 0, models[cur_idx].shapes[i].vertex_count);

		Vector3 Ka = models[cur_idx].shapes[i].material.Ka;
		Vector3 Kd = models[cur_idx].shapes[i].material.Kd;
		Vector3 Ks = models[cur_idx].shapes[i].material.Ks;

		glUniform3f(iLocKa, Ka.x, Ka.y, Ka.z);
		glUniform3f(iLocKd, Kd.x, Kd.y, Kd.z);
		glUniform3f(iLocKs, Ks.x, Ks.y, Ks.z);
		glUniform1i(iLocLightType, lightType);
		glUniform1f(iLocShininess, shininess);
	}
}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// V [TODO] Call back function for keyboard
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_Z) {
			if (cur_idx > 0)
				cur_idx = cur_idx - 1;
			else
				cur_idx = NUM_MODEL - 1;
			cout << "switch to previous model\n";
		}
		if (key == GLFW_KEY_X) {
			cur_idx = (cur_idx + 1) % NUM_MODEL;
			cout << "switch to next model\n";
		}
		if (key == GLFW_KEY_T) {
			cur_trans_mode = GeoTranslation;
			cout << "switch to translation mode\n";
		}
		if (key == GLFW_KEY_S) {
			cur_trans_mode = GeoScaling;
			cout << "switch to scale mode\n";
		}
		if (key == GLFW_KEY_R) {
			cur_trans_mode = GeoRotation;
			cout << "switch to rotation mode\n";
		}
		if (key == GLFW_KEY_L) {
			lightType = (lightType + 1) % 3;
			cout << "switch between directional/point/spot light\n";
			cout << "Current light type = " << lightType << endl;
		}
		if (key == GLFW_KEY_K) {
			cur_trans_mode = LightEdit;
			cout << "switch to light editing mode\n";
		}
		if (key == GLFW_KEY_J) {
			cur_trans_mode = ShininessEdit;
			cout << "switch to shininess editing mode\n";
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// V [TODO] scroll up positive, otherwise it would be negtive
	float coef = 0.1;

	if (yoffset > 0) {
		if (cur_trans_mode == GeoTranslation)
			models[cur_idx].position.z += coef;
		if (cur_trans_mode == GeoScaling)
			models[cur_idx].scale.z += coef;
		if (cur_trans_mode == GeoRotation)
			models[cur_idx].rotation.z += (PI / 180.0) * 4;
		if (cur_trans_mode == LightEdit) {
			if (lightType == 2)
				lightInfo[lightType].spotCutoff += coef * 0.1;
			else
				lightInfo[lightType].diffuse += Vector3(coef, coef, coef);
		}
		if (cur_trans_mode == ShininessEdit)
			shininess += coef * 50;
	}
	else {
		if (cur_trans_mode == GeoTranslation)
			models[cur_idx].position.z -= coef;
		if (cur_trans_mode == GeoScaling)
			models[cur_idx].scale.z -= coef;
		if (cur_trans_mode == GeoRotation)
			models[cur_idx].rotation.z -= (PI / 180.0) * 4;
		if (cur_trans_mode == LightEdit) {
			if (lightType == 2)
				lightInfo[lightType].spotCutoff -= coef * 0.1;
			else
				lightInfo[lightType].diffuse -= Vector3(coef, coef, coef);
		}
		if (cur_trans_mode == ShininessEdit)
			shininess -= coef * 50;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// V [TODO] mouse press callback function
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		mouse_pressed = true;
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		mouse_pressed = false;
		
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	// V [TODO] cursor position callback function
	int delt_x, delt_y;
	float coef = 0.01;

	if (mouse_pressed) {
		delt_x = xpos - starting_press_x;
		delt_y = ypos - starting_press_y;

		if (cur_trans_mode == GeoTranslation) {
			models[cur_idx].position.x += delt_x * coef;
			models[cur_idx].position.y -= delt_y * coef;
		}
		if (cur_trans_mode == GeoScaling) {
			models[cur_idx].scale.x += delt_x * coef;
			models[cur_idx].scale.y -= delt_y * coef;
		}
		if (cur_trans_mode == GeoRotation) {
			models[cur_idx].rotation.x -= PI / 180.0 * delt_y * (coef * 100);
			models[cur_idx].rotation.y -= PI / 180.0 * delt_x * (coef * 100);
		}
		if (cur_trans_mode == LightEdit) {
			lightInfo[lightType].position.x += delt_x * coef;
			lightInfo[lightType].position.y -= delt_y * coef;
		}
	}
	starting_press_x = xpos;
	starting_press_y = ypos;
}

void setShaders()
{
	GLuint v, f, p;
	char *vs = NULL;
	char *fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("shader.vs");
	fs = textFileRead("shader.fs");

	glShaderSource(v, 1, (const GLchar**)&vs, NULL);
	glShaderSource(f, 1, (const GLchar**)&fs, NULL);

	free(vs);
	free(fs);

	GLint success;
	char infoLog[1000];
	// compile vertex shader
	glCompileShader(v);
	// check for shader compile errors
	glGetShaderiv(v, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(v, 1000, NULL, infoLog);
		std::cout << "ERROR: VERTEX SHADER COMPILATION FAILED\n" << infoLog << std::endl;
	}

	// compile fragment shader
	glCompileShader(f);
	// check for shader compile errors
	glGetShaderiv(f, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(f, 1000, NULL, infoLog);
		std::cout << "ERROR: FRAGMENT SHADER COMPILATION FAILED\n" << infoLog << std::endl;
	}

	// create program object
	p = glCreateProgram();

	// attach shaders to program object
	glAttachShader(p,f);
	glAttachShader(p,v);

	// link program
	glLinkProgram(p);
	// check for linking errors
	glGetProgramiv(p, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(p, 1000, NULL, infoLog);
		std::cout << "ERROR: SHADER PROGRAM LINKING FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(v);
	glDeleteShader(f);

	uniform.iLocMVP = glGetUniformLocation(p, "mvp");
	setUniformVariables(p); // bind variables

	if (success)
		glUseProgram(p);
    else
    {
        system("pause");
        exit(123);
    }
}

void normalization(tinyobj::attrib_t* attrib, vector<GLfloat>& vertices, vector<GLfloat>& colors, vector<GLfloat>& normals, tinyobj::shape_t* shape)
{
	vector<float> xVector, yVector, zVector;
	float minX = 10000, maxX = -10000, minY = 10000, maxY = -10000, minZ = 10000, maxZ = -10000;

	// find out min and max value of X, Y and Z axis
	for (int i = 0; i < attrib->vertices.size(); i++)
	{
		//maxs = max(maxs, attrib->vertices.at(i));
		if (i % 3 == 0)
		{

			xVector.push_back(attrib->vertices.at(i));

			if (attrib->vertices.at(i) < minX)
			{
				minX = attrib->vertices.at(i);
			}

			if (attrib->vertices.at(i) > maxX)
			{
				maxX = attrib->vertices.at(i);
			}
		}
		else if (i % 3 == 1)
		{
			yVector.push_back(attrib->vertices.at(i));

			if (attrib->vertices.at(i) < minY)
			{
				minY = attrib->vertices.at(i);
			}

			if (attrib->vertices.at(i) > maxY)
			{
				maxY = attrib->vertices.at(i);
			}
		}
		else if (i % 3 == 2)
		{
			zVector.push_back(attrib->vertices.at(i));

			if (attrib->vertices.at(i) < minZ)
			{
				minZ = attrib->vertices.at(i);
			}

			if (attrib->vertices.at(i) > maxZ)
			{
				maxZ = attrib->vertices.at(i);
			}
		}
	}

	float offsetX = (maxX + minX) / 2;
	float offsetY = (maxY + minY) / 2;
	float offsetZ = (maxZ + minZ) / 2;

	for (int i = 0; i < attrib->vertices.size(); i++)
	{
		if (offsetX != 0 && i % 3 == 0)
		{
			attrib->vertices.at(i) = attrib->vertices.at(i) - offsetX;
		}
		else if (offsetY != 0 && i % 3 == 1)
		{
			attrib->vertices.at(i) = attrib->vertices.at(i) - offsetY;
		}
		else if (offsetZ != 0 && i % 3 == 2)
		{
			attrib->vertices.at(i) = attrib->vertices.at(i) - offsetZ;
		}
	}

	float greatestAxis = maxX - minX;
	float distanceOfYAxis = maxY - minY;
	float distanceOfZAxis = maxZ - minZ;

	if (distanceOfYAxis > greatestAxis)
	{
		greatestAxis = distanceOfYAxis;
	}

	if (distanceOfZAxis > greatestAxis)
	{
		greatestAxis = distanceOfZAxis;
	}

	float scale = greatestAxis / 2;

	for (int i = 0; i < attrib->vertices.size(); i++)
	{
		//std::cout << i << " = " << (double)(attrib.vertices.at(i) / greatestAxis) << std::endl;
		attrib->vertices.at(i) = attrib->vertices.at(i) / scale;
	}
	size_t index_offset = 0;
	for (size_t f = 0; f < shape->mesh.num_face_vertices.size(); f++) {
		int fv = shape->mesh.num_face_vertices[f];

		// Loop over vertices in the face.
		for (size_t v = 0; v < fv; v++) {
			// access to vertex
			tinyobj::index_t idx = shape->mesh.indices[index_offset + v];
			vertices.push_back(attrib->vertices[3 * idx.vertex_index + 0]);
			vertices.push_back(attrib->vertices[3 * idx.vertex_index + 1]);
			vertices.push_back(attrib->vertices[3 * idx.vertex_index + 2]);
			// Optional: vertex colors
			colors.push_back(attrib->colors[3 * idx.vertex_index + 0]);
			colors.push_back(attrib->colors[3 * idx.vertex_index + 1]);
			colors.push_back(attrib->colors[3 * idx.vertex_index + 2]);
			// Optional: vertex normals
			if (idx.normal_index >= 0) {
				normals.push_back(attrib->normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib->normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib->normals[3 * idx.normal_index + 2]);
			}
		}
		index_offset += fv;
	}
}

string GetBaseDir(const string& filepath) {
	if (filepath.find_last_of("/\\") != std::string::npos)
		return filepath.substr(0, filepath.find_last_of("/\\"));
	return "";
}

void LoadModels(string model_path)
{
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	tinyobj::attrib_t attrib;
	vector<GLfloat> vertices;
	vector<GLfloat> colors;
	vector<GLfloat> normals;

	string err;
	string warn;

	string base_dir = GetBaseDir(model_path); // handle .mtl with relative path

#ifdef _WIN32
	base_dir += "\\";
#else
	base_dir += "/";
#endif

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str(), base_dir.c_str());

	if (!warn.empty()) {
		cout << warn << std::endl;
	}

	if (!err.empty()) {
		cerr << err << std::endl;
	}

	if (!ret) {
		exit(1);
	}

	printf("Load Models Success ! Shapes size %d Material size %d\n", int(shapes.size()), int(materials.size()));
	model tmp_model;

	vector<PhongMaterial> allMaterial;
	for (int i = 0; i < materials.size(); i++)
	{
		PhongMaterial material;
		material.Ka = Vector3(materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
		material.Kd = Vector3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
		material.Ks = Vector3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);

		// set shininess
		material.shininess = 64;

		allMaterial.push_back(material);
	}

	for (int i = 0; i < shapes.size(); i++)
	{

		vertices.clear();
		colors.clear();
		normals.clear();
		normalization(&attrib, vertices, colors, normals, &shapes[i]);
		// printf("Vertices size: %d", vertices.size() / 3);

		Shape tmp_shape;
		glGenVertexArrays(1, &tmp_shape.vao);
		glBindVertexArray(tmp_shape.vao);

		glGenBuffers(1, &tmp_shape.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_FLOAT), &vertices.at(0), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		tmp_shape.vertex_count = vertices.size() / 3;

		glGenBuffers(1, &tmp_shape.p_color);
		glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_color);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GL_FLOAT), &colors.at(0), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &tmp_shape.p_normal);
		glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_normal);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GL_FLOAT), &normals.at(0), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// not support per face material, use material of first face
		if (allMaterial.size() > 0)
			tmp_shape.material = allMaterial[shapes[i].mesh.material_ids[0]];
		tmp_model.shapes.push_back(tmp_shape);
	}
	shapes.clear();
	materials.clear();
	models.push_back(tmp_model);
}

void initParameter()
{
	// V [TODO] Setup some parameters if you need
	proj.left = -1;
	proj.right = 1;
	proj.top = 1;
	proj.bottom = -1;
	proj.nearClip = 0.001;
	proj.farClip = 100.0;
	proj.fovy = 80;
	proj.aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

	main_camera.position = Vector3(0.0f, 0.0f, 2.0f);
	main_camera.center = Vector3(0.0f, 0.0f, 0.0f);
	main_camera.up_vector = Vector3(0.0f, 1.0f, 0.0f);

	setViewingMatrix();
	setPerspective();	//set default projection matrix as perspective matrix

	// set default arguments of light
	for (int i = 0; i < 3; i++) {
		lightInfo[i].diffuse = Vector3(1.0f, 1.0f, 1.0f);
		lightInfo[i].ambient = Vector3(0.15f, 0.15f, 0.15f);
		lightInfo[i].specular = Vector3(1.0f, 1.0f, 1.0f);
	}

	lightInfo[0].position = Vector3(1.0f, 1.0f, 1.0f);
	lightInfo[0].direction = Vector3(0.0f, 0.0f, 0.0f);

	lightInfo[1].position = Vector3(0.0f, 2.0f, 1.0f);
	lightInfo[1].constantAttenuation = 0.01f;
	lightInfo[1].linearAttenuation = 0.8f;
	lightInfo[1].quadraticAttenuation = 0.1f;

	lightInfo[2].position = Vector3(0.0f, 0.0f, 2.0f);
	lightInfo[2].spotDirection = Vector3(0.0f, 0.0f, -1.0f);
	lightInfo[2].spotExponent = 50;
	lightInfo[2].spotCutoff = 30 * (PI / 180.0);
	lightInfo[2].constantAttenuation = 0.05f;
	lightInfo[2].linearAttenuation = 0.3f;
	lightInfo[2].quadraticAttenuation = 0.6f;
}

void setupRC()
{
	// setup shaders
	setShaders();
	initParameter();

	// OpenGL States and Values
	glClearColor(0.2, 0.2, 0.2, 1.0);
	vector<string> model_list{ "../NormalModels/bunny5KN.obj", "../NormalModels/dragon10KN.obj", "../NormalModels/lucy25KN.obj", "../NormalModels/teapot4KN.obj", "../NormalModels/dolphinN.obj"};
	// V [TODO] Load five model at here
	for (int i = 0; i < NUM_MODEL; i++)
		LoadModels(model_list[i]);
}

void glPrintContextInfo(bool printExtension)
{
	cout << "GL_VENDOR = " << (const char*)glGetString(GL_VENDOR) << endl;
	cout << "GL_RENDERER = " << (const char*)glGetString(GL_RENDERER) << endl;
	cout << "GL_VERSION = " << (const char*)glGetString(GL_VERSION) << endl;
	cout << "GL_SHADING_LANGUAGE_VERSION = " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	if (printExtension)
	{
		GLint numExt;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
		cout << "GL_EXTENSIONS =" << endl;
		for (GLint i = 0; i < numExt; i++)
		{
			cout << "\t" << (const char*)glGetStringi(GL_EXTENSIONS, i) << endl;
		}
	}
}


int main(int argc, char **argv)
{
    // initial glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // fix compilation on OS X
#endif

    
    // create window
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "110065512 HW2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    
    // load OpenGL function pointer
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
	// register glfw callback functions
    glfwSetKeyCallback(window, KeyCallback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);

    glfwSetFramebufferSizeCallback(window, ChangeSize);
	glEnable(GL_DEPTH_TEST);
	// Setup render context
	setupRC();

	// main loop
    while (!glfwWindowShouldClose(window))
    {
        // render
        RenderScene();
        
        // swap buffer from back to front
        glfwSwapBuffers(window);
        
        // Poll input event
        glfwPollEvents();
    }
	
	// just for compatibiliy purposes
	return 0;
}
