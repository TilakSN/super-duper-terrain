#pragma once
#include <bits/stdc++.h>
#include "includeGL.h"
#include "utils.h"
using namespace std;

class ShaderGL {
    protected:
        GLuint handle;
        GLint type;
        string filename;
    public:
        ShaderGL() {
            handle = GL_INVALID_VALUE;
            filename = "";
            type = GL_INVALID_VALUE;
        }

        ShaderGL(const string &name, GLint t) {
            filename = name;
            type = t;
            handle = GL_INVALID_VALUE;
        }

        void compile() {
            if (type == GL_INVALID_VALUE)
                error_message("Shader type is not known.");

            if (handle != GL_INVALID_VALUE)
                error_message("Shader is already compiled");

            handle = glCreateShader(type);
            if (!handle)
                error_message("Failed to create shader.");

            ifstream input_stream(filename);
            if (!input_stream.is_open())
                error_message("Failed to open shader.");

            stringstream code_stream;
            code_stream << input_stream.rdbuf();
            input_stream.close();

            const string code_string = code_stream.str();
            char const * code_pointer = code_string.c_str();
            glShaderSource(handle, 1, &code_pointer, NULL);
            glCompileShader(handle);

            GLint result;
            glGetShaderiv(handle, GL_COMPILE_STATUS, &result);
            if (!result) {
                GLint log_length;
                glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
                if (log_length > 0) {
                    char *log = new char[log_length];
                    glGetShaderInfoLog(handle, log_length, &result, log);
                    cerr << "Shader info:\n" << log << endl;
                    delete[] log;
                }
                error_message("Failed to compile shader.");
            }
        }

        GLuint get_handle() const {
            return handle;
        }

        ~ShaderGL() {
            glDeleteShader(handle);
        }
};