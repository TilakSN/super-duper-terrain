#pragma once
#include <bits/stdc++.h>
#include "includeGL.h"
#include "utils.h"
#include "shaderGL.h"
#include <glm/glm.hpp>
using namespace std;

class ProgramGL {
    protected:
        GLuint handle;
        ShaderGL vertex_shader, fragment_shader;
    public:
        ProgramGL(const string &vertex_shader_filename, const string &fragment_shader_filename):
        vertex_shader(vertex_shader_filename, GL_VERTEX_SHADER),
        fragment_shader(fragment_shader_filename, GL_FRAGMENT_SHADER) {
            handle = GL_INVALID_VALUE;
        }
        
        void compile() {
            handle = glCreateProgram();
            if (!handle)
                error_message("Failed to create program.");

            vertex_shader.compile();
            fragment_shader.compile();

            glAttachShader(handle, vertex_shader.get_handle());
            glAttachShader(handle, fragment_shader.get_handle());
            glLinkProgram(handle);

            GLint result;
            glGetProgramiv(handle, GL_LINK_STATUS, &result);
            if (!result) {
                GLint log_length;
                glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
                if (log_length > 0) {
                    char *log = new char[log_length];
                    glGetProgramInfoLog(handle, log_length, &result, log);
                    cerr << "Program info:\n" << log << endl;
                    delete[] log;
                }
                error_message("Failed to link program.");
            }

            glUseProgram(handle);
        }

        GLuint get_handle() const {
            return handle;
        }
        
        GLuint get_attrib_location(const string &name) const {
            return glGetAttribLocation(handle, name.c_str());
        }

        GLint get_uniform_location(const string &name) const {
            return glGetUniformLocation(handle, name.c_str());
        }

        void set_uniform(const string &name, int value) const {
            glUniform1i(get_uniform_location(name), value);
        }

        void set_uniform(const string &name, float value) const {
            glUniform1f(get_uniform_location(name), value);
        }

        void set_uniform(const string &name, const glm::mat4 &value) const {
            glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, &value[0][0]);
        }

        void set_uniform(const string &name, const glm::vec4 &value) const {
            glUniform4fv(get_uniform_location(name), 1, &value[0]);
        }

        void set_uniform(const string &name, const glm::vec3 &value) const {
            glUniform3fv(get_uniform_location(name), 1, &value[0]);
        }

        void set_uniform(const string &name, const glm::vec2 &value) const {
            glUniform2fv(get_uniform_location(name), 1, &value[0]);
        }

        ~ProgramGL() {
            glDetachShader(handle, vertex_shader.get_handle());
            glDetachShader(handle, fragment_shader.get_handle());
            delete &vertex_shader;
            delete &fragment_shader;
            glDeleteProgram(handle);
        }
};