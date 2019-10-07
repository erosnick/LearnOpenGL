#pragma once
#include <glad/glad.h>
#include <string>

class Texture
{
public:

	Texture() {}

	void load(const std::string& path, unsigned int texture = GL_TEXTURE0);

	void activate(unsigned int texture);

	void setupTextureFilters();

	void use();

private:

	unsigned int textureID;
};