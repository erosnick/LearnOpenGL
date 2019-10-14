#include "Shader.h"

Shader::Shader()
{

}

void Shader::load(const std::string& vertexPath, const std::string& fragmentPath)
{
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	string vertexShaderSource = loadShader(vertexPath);

	compileShader(vertexShader, VertexShader, vertexShaderSource);

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	string fragmentShaderSource = loadShader(fragmentPath);

	compileShader(fragmentShader, FragmentShader, fragmentShaderSource);

	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	int success;
	char infoLog[512];

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

bool Shader::loadGLLoader()
{
	return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

void Shader::use()
{
	glUseProgram(shaderProgram);
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
	int locaiton = glGetUniformLocation(shaderProgram, name.c_str());
	glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::setVec3(const string& name, float v0, float v1, float v2)
{
	glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), v0, v1, v2);
}

void Shader::setVec3(const string& name, const glm::vec3& v)
{
	glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), v.x, v.y, v.z);
}

void Shader::setVec4(const std::string& name, float v0, float v1, float v2, float v3)
{
	glUniform4f(glGetUniformLocation(shaderProgram, name.c_str()), v0, v1, v2, v3);
}

void Shader::setMat4(const string& name, glm::mat4 transform)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(transform));
}

string Shader::loadShader(const string& path)
{
	ifstream shaderFile;

	shaderFile.open(path);

	stringstream buffer;

	buffer << shaderFile.rdbuf();

	string shaderSource(buffer.str());

	return shaderSource;
}

void Shader::compileShader(unsigned int shader, EShaderType shaderType, const std::string& shaderSource)
{
	GLchar const* shaderSources[] = { shaderSource.c_str() };

	glShaderSource(shader, 1, shaderSources, nullptr);
	glCompileShader(shader);

	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);

		if (shaderType == VertexShader)
		{
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		else if (shaderType == FragmentShader)
		{
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		
	}
}
