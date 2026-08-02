#include "graphics/shader.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

shader::shader(const std::string& vertexPath, const std::string& fragmentPath)
{
	const unsigned int vertexShader = compile
	(GL_VERTEX_SHADER,
		readTextFile(vertexPath));

	unsigned int fragmentShader = 0;

	try
	{
		fragmentShader = compile(
			GL_FRAGMENT_SHADER,
			readTextFile(fragmentPath)
		);

		programID_ = glCreateProgram();

		glAttachShader(programID_, vertexShader);
		glAttachShader(programID_, fragmentShader);
		glLinkProgram(programID_);
		int success = 0;
		glGetProgramiv(programID_, GL_LINK_STATUS, &success);

		if (success == GL_FALSE)
		{
			char log[512];
			glGetProgramInfoLog(programID_, sizeof(log), nullptr, log);

			glDeleteProgram(programID_);
			programID_ = 0;

			throw std::runtime_error("Shader link hatasi: " + std::string(log));
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
	catch (...) {

		glDeleteShader(vertexShader);

		if (fragmentShader != 0)
		{
			glDeleteShader(fragmentShader);
		}
		throw;

	}
}

	shader::~shader()	
{
    if (programID_ != 0)
    {
        glDeleteProgram(programID_);
    }
}

void shader::use() const
{
	glUseProgram(programID_);
}

std::string shader::readTextFile(const std::string& path)
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

unsigned int shader::compile(
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