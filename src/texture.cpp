#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>
#include <iostream>
#include <stdexcept>

Texture::Texture(std::string file_location, bool sRGB) : file_location(file_location) {
	// load and generate the texture
	int width, height, num_channels;
	//if (!std::filesystem::exists(file_location)) {
	//	std::cout << "Failed to load texture" << std::endl;
	//	throw std::exception("sd");
	//}
	//stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(file_location.c_str(), &width, &height, &num_channels, 0);
	
	if (!data) {
		std::cout << "Failed to load texture" << std::endl;
		throw std::exception("Failed to load texture");
	}

	GLenum format = GL_RGB;
	GLenum format2 = GL_RGB;
	if (num_channels == 1) {
		format = GL_RED;
		format2 = GL_RED;
	}
	else if (num_channels == 3) {
		format = GL_RGB;
		format2 = GL_RGB;
		if (sRGB) {
			format = GL_SRGB;
		}
	}
	else if (num_channels == 4) {
		format = GL_RGBA;
		format2 = GL_RGBA;
		if (sRGB) {
			format = GL_SRGB_ALPHA;
		}
	}


	// Setup GL tex
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format2, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	stbi_image_free(data);

	// Free bound texture
	glBindTexture(GL_TEXTURE_2D, 0);
}


Texture::~Texture() {
	glDeleteTextures(1, &texture_id);
}


void Texture::bind(GLenum tex_unit) {
	glActiveTexture(tex_unit);
	glBindTexture(GL_TEXTURE_2D, texture_id);
}

unsigned int Texture::getTextureId() {
	return texture_id;
}
