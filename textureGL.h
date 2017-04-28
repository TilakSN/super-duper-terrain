#pragma once
#include <bits/stdc++.h>
#include "includeGL.h"
#include "utils.h"
#include <glm/glm.hpp>
#include <FreeImage.h>
using namespace std;

class TextureGL {
    protected:
        GLuint handle;
        GLuint width;
        GLuint height;
        GLbyte *data;
        int color_width;
        GLuint format;
        GLuint input_format;

    public:
        TextureGL() {
            handle = GL_INVALID_VALUE;
            width = 0;
            height = 0;
            data = NULL;
            color_width = 0;
            format = GL_INVALID_VALUE;
            input_format = GL_INVALID_VALUE;
        }

        void load_from_file(string filename, GLenum file_format=0) {
            const char *name = filename.c_str();
            FREE_IMAGE_FORMAT image_format = FIF_UNKNOWN;
            FIBITMAP *image(0);

            image_format = FreeImage_GetFileType(name, 0);
            if (image_format == FIF_UNKNOWN)
                image_format = FreeImage_GetFIFFromFilename(name);

            if (image_format == FIF_UNKNOWN)
                error_message("Unknown file format");

            if (FreeImage_FIFSupportsReading(image_format))
                image = FreeImage_Load(image_format, name);

            if (image == 0)
                error_message("Unable to open file");

            width = FreeImage_GetWidth(image);
            height = FreeImage_GetHeight(image);
            color_width = FreeImage_GetLine(image) / width;
            int length = width * height * color_width;

            BYTE *file_data = FreeImage_GetBits(image);
            data = new GLbyte[length];
            if (length == 0 or data <= 0)
                error_message("Unable to load texture.");

            memcpy(data, file_data, length);
            format = color_width == 3 ? GL_RGB : GL_RGBA;
            input_format = file_format ? file_format : format;

            FreeImage_Unload(image);
        }

        ~TextureGL() {
            if (data)
                delete[] data;
            if (handle != GL_INVALID_VALUE)
                glDeleteTextures(1, &handle);
        }

        GLuint get_width() const {
            return width;
        }

        GLuint get_height() const {
            return height;
        }

        void load_texture(int h) {
            handle = h;
            if (color_width < 3 or color_width > 4)
                error_message("Invalid texture");

            glBindTexture(GL_TEXTURE_2D, handle);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, input_format, GL_UNSIGNED_BYTE, data);
        }

        void use_texture(GLuint uniform_location, GLuint texture_position) {
            glActiveTexture(GL_TEXTURE0 + texture_position);
            glBindTexture(GL_TEXTURE_2D, handle);
            glUniform1i(uniform_location, texture_position - 1);
        }

        GLfloat get_greyscale(GLfloat tx, GLfloat ty) {
            int x = (int) tx * width;
            int y = (int) ty * height;
            int i = (width * x + y) * color_width;
            return data[i] / 256.0f;
        }
};