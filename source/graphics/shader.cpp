#include "graphics/shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
	const unsigned int vertexShader = compile(
		GL_VERTEX_SHADER,
		readTextFile(vertexPath)
	);

	unsigned int fragmentShader = 0;

	try
	{
		fragmentShader = compile(
			GL_FRAGMENT_SHADER,
			readTextFile(fragmentPath)
		);

		programId_ = glCreateProgram();

		glAttachShader(programId_, vertexShader);
		glAttachShader(programId_, fragmentShader);
		glLinkProgram(programId_);
		int success = 0;
		glGetProgramiv(programId_, GL_LINK_STATUS, &success);

		if (success == GL_FALSE)
		{
			char log[512];
			glGetProgramInfoLog(programId_, sizeof(log), nullptr, log);

			glDeleteProgram(programId_);
			programId_ = 0;

			throw std::runtime_error("Shader link hatasi: " + std::string(log));
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
	catch (...)
	{

		glDeleteShader(vertexShader);

		if (fragmentShader != 0)
		{
			glDeleteShader(fragmentShader);
		}
		throw;
	}
}

Shader::~Shader()
{
	if (programId_ != 0)
	{
		glDeleteProgram(programId_);
	}
}

void Shader::use() const
{
	glUseProgram(programId_);
}

void Shader::setMat4(
	const std::string& uniformName,
	const glm::mat4& matrix) const
{
	const int location = glGetUniformLocation(
		programId_,
		uniformName.c_str()
	);

	if (location != -1)
	{
		glUniformMatrix4fv(
			location,
			1,
			GL_FALSE,
			glm::value_ptr(matrix)
		);
	}
}

void Shader::setVec3(
	const std::string& uniformName,
	const glm::vec3& value) const
{
	const int location = glGetUniformLocation(
		programId_,
		uniformName.c_str()
	);

	if (location != -1)
	{
		glUniform3fv(
			location,
			1,
			glm::value_ptr(value)
		);
	}
}

std::string Shader::readTextFile(const std::string& path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		throw std::runtime_error("Shader dosyasi acilamadi: " + path);
	}

	std::stringstream content;
	content << file.rdbuf();

	return content.str();
}

unsigned int Shader::compile(
	unsigned int shaderType,
	const std::string& source)
{
	const unsigned int shaderId = glCreateShader(shaderType);
	const char* sourceCode = source.c_str();

	glShaderSource(shaderId, 1, &sourceCode, nullptr);
	glCompileShader(shaderId);

	int success = 0;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		char log[512];
		glGetShaderInfoLog(shaderId, sizeof(log), nullptr, log);

		glDeleteShader(shaderId);

		throw std::runtime_error("Shader derleme hatasi: " + std::string(log));
	}

	return shaderId;
}
