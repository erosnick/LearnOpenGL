#pragma once

#include <stdint.h>
#include <string>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class ShaderType : int32_t {
	VERTEX = GL_VERTEX_SHADER,
	FRAGMENT = GL_FRAGMENT_SHADER,
	GEOMETRY = GL_GEOMETRY_SHADER,
	TESS_CONTROL = GL_TESS_CONTROL_SHADER,
	TESS_EVALUATION = GL_TESS_EVALUATION_SHADER
};

class Shader
{
public:
	Shader();

	void create();

	bool compileShaderFromFile(const std::string& fileName, ShaderType type);
	bool compileShaderFromString(const char* source, ShaderType type);

	bool link();

	void use();

	int get() const;

	bool isLinked() const;

	void bindAttribLocation(uint32_t location, const std::string& name);
	void bindFragDataLocation(uint32_t location, const std::string& name);
	void setUniform(const std::string& name, float x, float y, float z);
	void setUniform(const std::string& name, const glm::vec3& v);
	void setUniform(const std::string& name, const glm::vec4& v);
	void setUniform(const std::string& name, const glm::mat3& v);
	void setUniform(const std::string& name, const glm::mat4& v);
	void setUniform(const std::string& name, float value);
	void setUniform(const std::string& name, int32_t value);
	void setUniform(const std::string& name, bool value);

	void printActiveAttributes();
	void printActiveUniforms();

private:

	int32_t getUniformLocation(const std::string& name);
	bool fileExists(const std::string& fileName);

private:

	int32_t program = -1;
	bool linked = false;
	std::string log = "";
};

