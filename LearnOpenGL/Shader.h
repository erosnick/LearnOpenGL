#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace std;

enum EShaderType
{
	VertexShader,
	FragmentShader
};

class Shader
{
public:

	// Read and create shader.
	Shader();

	void load(const string& vertexPath, const string& fragmentPath);

	// Use program.
	void use();

	// uniform utility function.
	void setBool(const string& name, bool value) const;
	void setInt(const string& name, int value) const;
	void setFloat(const string& name, float value) const;
	void setVec3(const string& name, float v0, float v1, float v2);
	void setVec3(const string& name, const glm::vec3& v);
	void setVec4(const string& name, float v0, float v1, float v2, float v3);
	void setMat4(const string& name, glm::mat4 transform);
private:

	string loadShader(const string& path);

	void compileShader(unsigned int shader, EShaderType shaderType, const string& shaderSource);

	bool loadGLLoader();

	// Program ID
	unsigned int shaderProgram;
};