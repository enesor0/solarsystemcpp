#pragma once

class Starfield
{
public:
	explicit Starfield(unsigned int starCount = 1500);
	~Starfield();

	Starfield(const Starfield&) = delete;
	Starfield& operator=(const Starfield&) = delete;

	void draw() const;

private:
	unsigned int vao_ = 0;
	unsigned int vbo_ = 0;
	int starCount_ = 0;
};
