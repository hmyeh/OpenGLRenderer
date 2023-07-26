#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/gl.h> // include glad to get all the required OpenGL headers

#include <string>
#include <iostream>


class Texture {
private:
	std::string file_location;
	unsigned int texture_id;

public:
	Texture(std::string file_location, bool sRGB = false);
	~Texture();

	void bind(GLenum tex_unit);
	unsigned int getTextureId();

};

#endif
