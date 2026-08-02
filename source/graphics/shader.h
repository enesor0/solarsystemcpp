#pragma once

#include <iostream>

class shader
{
public:
	shader(const std::string& vertexPath, const std::string& fragmentPath);
	~shader();
    
	shader(const shader&) = delete;
	shader& operator=(const shader&) = delete;

	void use() const;


private:
	unsigned int programID_ = 0;

	static std::string readTextFile(const std::string& filePath);
	static unsigned int compile(unsigned int ShaderType, const std::string& source);

};
