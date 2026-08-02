#pragma once

#include <glm/glm.hpp>

#include <string>

class Shader
{
public:
	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	void use() const;
	void setMat4(const std::string& uniformName, const glm::mat4& value) const;

	void setVec3(
		const std::string& uniformName, const glm::vec3& value
	) const;
	void setFloat(const std::string& uniformName, float value) const;
	void setInt(const std::string& uniformName, int value) const;

private:
	unsigned int programId_ = 0;

	static std::string readTextFile(const std::string& filePath);
	static unsigned int compile(unsigned int shaderType, const std::string& source);
};
