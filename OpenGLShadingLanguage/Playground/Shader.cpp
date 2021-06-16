#include "Shader.h"

#include <iostream>
#include <fstream>

void Shader::create()
{
	program = glCreateProgram();
}

bool Shader::compileShaderFromFile(const std::string& fileName, ShaderType type)
{
	std::ifstream file(fileName, std::ios_base::ate);

	if (!file.is_open()) {
		std::cout << "Open shader file " + fileName << " failed." << std::endl;
		return false;
	}

	auto length = file.tellg();

	file.seekg(0);

	std::string shaderSource;
	shaderSource.resize(length);

	file.read(&shaderSource[0], length);

	file.close();

	return compileShaderFromString(shaderSource.c_str(), type);
}

bool Shader::compileShaderFromString(const char* source, ShaderType type)
{
	auto shader = glCreateShader(static_cast<int32_t>(type));

	glShaderSource(shader, 1, &source, nullptr);

	glCompileShader(shader);

	int32_t result;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (result == 0) {
		std::cout << "Vertex Shader compilation failed!\n";
		auto logLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength > 0) {
			char* log = new char[logLength];
			int32_t written;
			glGetShaderInfoLog(shader, logLength, &written, log);
			std::cout << log << std::endl;
			delete[] log;
		}

		return false;
	}

	glAttachShader(program, shader);

	return true;
}

bool Shader::link()
{
	glLinkProgram(program);

	auto status = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (status == 0) {
		std::cout << "Failed to link shader program!\n";
		auto logLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength > 0) {
			char* log = new char[logLength];
			auto written = 0;
			glGetProgramInfoLog(program, logLength, &written, log);
			std::cout << "Program log " << log << std::endl;
			delete[] log;
		}

		return false;
	}

	linked = true;

	return true;
}

void Shader::use()
{
	glUseProgram(program);
}

int Shader::get() const
{
	return program;
}

bool Shader::isLinked() const
{
	return linked;
}

void Shader::bindAttribLocation(uint32_t location, const std::string& name)
{
	glBindAttribLocation(program, location, name.c_str());
}

void Shader::bindFragDataLocation(uint32_t location, const std::string& name)
{
	glBindFragDataLocation(program, location, name.c_str());
}

void Shader::setUniform(const std::string& name, float x, float y, float z)
{	
	glUniform3f(getUniformLocation(name), x, y, z);
}

void Shader::setUniform(const std::string& name, const glm::vec3& v)
{
	glUniform3fv(getUniformLocation(name), 1, &v[0]);
}

void Shader::setUniform(const std::string& name, const glm::vec4& v)
{
	glUniform4fv(getUniformLocation(name), 1, &v[0]);
}

void Shader::setUniform(const std::string& name, const glm::mat3& v)
{
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &v[0][0]);
}

void Shader::setUniform(const std::string& name, const glm::mat4& v)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &v[0][0]);
}

void Shader::setUniform(const std::string& name, float value)
{
	glUniform1f(getUniformLocation(name), value);
}

void Shader::setUniform(const std::string& name, int32_t value)
{
	glUniform1i(getUniformLocation(name), value);
}

void Shader::setUniform(const std::string& name, bool value)
{
	glUniform1i(getUniformLocation(name), value);
}

void Shader::printActiveAttributes() {
	int32_t maxLength = 0;
	int32_t numAttributes = 0;

	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttributes);
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);

	char* name = new char[maxLength];

	int32_t written = 0;
	int32_t size = 0;
	uint32_t location = 0;
	uint32_t type = 0;

	std::cout << "Index | Name\n";

	for (auto i = 0; i < numAttributes; i++) {
		glGetActiveAttrib(program, i, maxLength, &written, &size, &type, name);
		location = glGetAttribLocation(program, name);
		std::cout << "    " << location << " | " << name << std::endl;
	}

	delete[] name;
}

void Shader::printActiveUniforms() {
	int32_t numUniforms = 0;
	int32_t maxLength = 0;

	glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);

	char* uniformName = new char[maxLength];

	int32_t size = 0;
	int32_t location = 0;
	int32_t written = 0;
	uint32_t type = 0;

	std::cout << "*****" << name << "*****" << std::endl;
	std::cout << "Location | Name\n";
	std::cout << "---------------------------------\n";


	for (auto i = 0; i < numUniforms; i++) {
		glGetActiveUniform(program, i, maxLength, &written, &size, &type, uniformName);
		location = glGetUniformLocation(program, uniformName);
		std::cout << "       " << location << " | " << uniformName << std::endl;
	}

	delete[] uniformName;
}

int32_t Shader::getUniformLocation(const std::string& name)
{
	int32_t location = glGetUniformLocation(program, name.c_str());

    if (location < 0) {
        //std::cout << "Uniform variable " + name + " not found.\n";
    }

	return location;
}

bool Shader::fileExists(const std::string& fileName)
{
	std::ifstream file(fileName);

	return file.is_open();
}
