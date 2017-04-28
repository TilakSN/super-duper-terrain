#pragma once

#include "includeGL.h"
#include <vector>
using namespace std;

class VertexBuffer {
    protected:
        GLuint handle;
        GLfloat *data;
        int n;
        int degree;

    public:
        VertexBuffer(GLfloat *elements, int num, int d) {
            handle = 0;
            data = elements;
            n = num;
            degree = d;
        }

        void generate(GLuint h) {
            handle = h;
            glBindBuffer(GL_ARRAY_BUFFER, handle);
            glBufferData(GL_ARRAY_BUFFER, degree * n * sizeof(GLfloat), data, GL_STATIC_DRAW);
        }

        void enable(GLuint pid) {
            glEnableVertexAttribArray(pid);
            glBindBuffer(GL_ARRAY_BUFFER, handle);
            glVertexAttribPointer(pid, degree, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
        }

        void disable(GLuint pid) {
            glDisableVertexAttribArray(pid);
        }

        GLuint get_handle() const {
            return handle;
        }
};

class VertexArray {
    protected:
        GLuint handle;
        int n;
        vector<VertexBuffer> buffers;

    public:
        VertexArray(int num) {
            n = num; // number of vertices
            handle = 0;
        }

        GLuint get_handle() const {
            return handle;
        }

        void add_buffer(float *data, int degree) {
            VertexBuffer b(data, n, degree);
            buffers.push_back(b);
        }

        void generate() {
            if (handle)
                exit(EXIT_FAILURE);

            glGenVertexArrays(1, &handle);
            glBindVertexArray(handle);

            GLint n = buffers.size();
            GLuint *vbo = new GLuint[n];
            glGenBuffers(n, vbo);

            for (int i = -1; ++i < n;)
                buffers[i].generate(vbo[i]);
        }
        
        void draw(GLuint *pid) {
            enable(pid);
            glDrawArrays(GL_TRIANGLES, 0, n);
            disable(pid);
        }

        void enable(GLuint *pid) {
            int len = buffers.size();
            for (int i = -1; ++i < len;)
                buffers[i].enable(pid[i]);
        }

        void disable(GLuint *pid) {
            int len = buffers.size();
            for (int i = -1; ++i < len;)
                buffers[i].disable(pid[i]);
        }
};