#include "Texture.h"
#include "stb_image.h"
#include <GLFW/glfw3.h>
#include <iostream>

void Texture::load(const std::string& path, unsigned int texture)
{
	glGenTextures(1, &textureID);
	activate(texture);
	glBindTexture(GL_TEXTURE_2D, textureID);

	setupTextureFilters();

	int width;
	int height;
	int numChannels;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &numChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);
}

void Texture::activate(unsigned int texture)
{
	glActiveTexture(texture);
}

void Texture::setupTextureFilters()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::use()
{
	glBindTexture(GL_TEXTURE_2D, textureID);
}

