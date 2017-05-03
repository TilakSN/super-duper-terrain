#include <bits/stdc++.h>
#include <GL/glew.h>
#include <GL/glut.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <FreeImage.h>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

using namespace std;

GLuint program_handle, texture_handle, vb_handle, tb_handle;
float x_distort = 0, y_distort = 0, r_intensity = 1, g_intensity = 1, b_intensity = 1, blur = 0;
int vertex_count;
glm::mat4 camera;
GLuint sl_cam, sl_x_distort, sl_y_distort, sl_r_intensity, sl_g_intensity, sl_b_intensity, sl_blur, sl_vertex, sl_coord, sl_tex;

void error(const string &msg) {
    cerr << msg << endl;
    exit(EXIT_FAILURE);
}

template <typename T>
bool clamp(T &value, T minimum, T maximum) {
    if (minimum > maximum)
        return false;
    if (value < minimum)
        value = minimum;
    else if (value > maximum)
        value = maximum;
    else
        return false;
    return true;
}

GLuint load_shader(const string &filename, GLenum type) {
    GLuint handle = glCreateShader(type);
    ifstream input_stream(filename);
    if (!input_stream.is_open())
        error("Failed to open shader.");

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
        error("Failed to compile shader.");
    }
    return handle;
}

GLuint load_texture(const string &filename, GLenum file_format=0) {
    const char *name = filename.c_str();
    FREE_IMAGE_FORMAT image_format = FIF_UNKNOWN;
    FIBITMAP *image(0);

    image_format = FreeImage_GetFileType(name, 0);
    if (image_format == FIF_UNKNOWN)
        image_format = FreeImage_GetFIFFromFilename(name);

    if (image_format == FIF_UNKNOWN)
        error("Unknown texture format.");

    if (FreeImage_FIFSupportsReading(image_format))
        image = FreeImage_Load(image_format, name);

    if (image == 0)
        error("Unable to open file.");

    GLuint width, height, channels, length;
    width = FreeImage_GetWidth(image);
    height = FreeImage_GetHeight(image);
    channels = FreeImage_GetLine(image) / width;
    if (channels < 3 or channels > 4)
        error("Invalid texture.");
    length = width * height * channels;

    BYTE *file_data = FreeImage_GetBits(image);
    GLbyte *data = new GLbyte[length];
    if (length == 0 or data <= 0)
        error("Unable to load texture.");

    memcpy(data, file_data, length);
    GLuint format = (channels == 3) ? GL_RGB : GL_RGBA;
    GLuint input_format = (file_format) ? file_format : format;

    FreeImage_Unload(image);
    
    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, input_format, GL_UNSIGNED_BYTE, data);
    delete[] data;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return handle;
}

void initialize() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    GLuint vertex_shader_handle = load_shader("ip.vert", GL_VERTEX_SHADER);
    GLuint fragment_shader_handle = load_shader("ip.frag", GL_FRAGMENT_SHADER);
    
    program_handle = glCreateProgram();
    
    glAttachShader(program_handle, vertex_shader_handle);
    glAttachShader(program_handle, fragment_shader_handle);
    
    glLinkProgram(program_handle);
    
    GLint result;
    glGetProgramiv(program_handle, GL_LINK_STATUS, &result);
    if (!result) {
        GLint log_length;
        glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 0) {
            char *log = new char[log_length];
            glGetProgramInfoLog(program_handle, log_length, &result, log);
            cerr << "Program info log:" << endl << log << endl;
            delete[] log;
        }
        error("Failed to link program.");
    }
    
    texture_handle = load_texture("image.jpg", GL_BGR);
    
    const int mesh_size = 64;
    vertex_count = mesh_size * mesh_size * 6;
    float *vertices = new float[vertex_count * 3];
    float *texture_coordinates = new float[vertex_count * 2];
    int vertex = 0, tex_coord = 0;
    for (int i = 0; i < mesh_size; i++) {
        for (int j = 0; j < mesh_size; j++) {
            // triangle 1
            // vertex 1
            vertices[vertex++] = -1 + i * 2.0f / mesh_size;
            vertices[vertex++] = -1 + j * 2.0f / mesh_size;
            vertices[vertex++] = 0.0f;
            texture_coordinates[tex_coord++] = i * 1.0f / mesh_size;
            texture_coordinates[tex_coord++] = j * 1.0f / mesh_size;
            // vertex 2
            vertices[vertex++] = -1 + (i + 1) * 2.0f / mesh_size;
            vertices[vertex++] = -1 + j * 2.0f / mesh_size;
            vertices[vertex++] = 0.0f;
            texture_coordinates[tex_coord++] = (i + 1) * 1.0f / mesh_size;
            texture_coordinates[tex_coord++] = j * 1.0f / mesh_size;
            // vertex 3
            vertices[vertex++] = -1 + (i + 1) * 2.0f / mesh_size;
            vertices[vertex++] = -1 + (j + 1) * 2.0f / mesh_size;
            vertices[vertex++] = 0.0f;
            texture_coordinates[tex_coord++] = (i + 1) * 1.0f / mesh_size;
            texture_coordinates[tex_coord++] = (j + 1) * 1.0f / mesh_size;
            // triangle 2
            // vertex 1
            vertices[vertex++] = -1 + i * 2.0f / mesh_size;
            vertices[vertex++] = -1 + j * 2.0f / mesh_size;
            vertices[vertex++] = 0.0f;
            texture_coordinates[tex_coord++] = i * 1.0f / mesh_size;
            texture_coordinates[tex_coord++] = j * 1.0f / mesh_size;
            // vertex 2
            vertices[vertex++] = -1 + (i + 1) * 2.0f / mesh_size;
            vertices[vertex++] = -1 + (j + 1) * 2.0f / mesh_size;
            vertices[vertex++] = 0.0f;
            texture_coordinates[tex_coord++] = (i + 1) * 1.0f / mesh_size;
            texture_coordinates[tex_coord++] = (j + 1) * 1.0f / mesh_size;
            // vertex 3
            vertices[vertex++] = -1 + i * 2.0f / mesh_size;
            vertices[vertex++] = -1 + (j + 1) * 2.0f / mesh_size;
            vertices[vertex++] = 0.0f;
            texture_coordinates[tex_coord++] = i * 1.0f / mesh_size;
            texture_coordinates[tex_coord++] = (j + 1) * 1.0f / mesh_size;
        }
    }
    
    glGenBuffers(1, &vb_handle);
    glBindBuffer(GL_ARRAY_BUFFER, vb_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex, vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &tb_handle);
    glBindBuffer(GL_ARRAY_BUFFER, tb_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tex_coord, texture_coordinates, GL_STATIC_DRAW);
    
    delete[] vertices;
    delete[] texture_coordinates;
    
    camera = glm::perspective((float) M_PI / 5, 1.0f, 0.1f, 10.0f) * glm::lookAt(glm::vec3(2.0f, 2.0f, 4.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    sl_vertex = glGetAttribLocation(program_handle, "VertexPosition");
    sl_coord = glGetAttribLocation(program_handle, "TexCoord");
    sl_cam = glGetUniformLocation(program_handle, "transform");
    sl_x_distort = glGetUniformLocation(program_handle, "x_distort");
    sl_y_distort = glGetUniformLocation(program_handle, "y_distort");
    sl_r_intensity = glGetUniformLocation(program_handle, "r_intensity");
    sl_g_intensity = glGetUniformLocation(program_handle, "g_intensity");
    sl_b_intensity = glGetUniformLocation(program_handle, "b_intensity");
    sl_blur = glGetUniformLocation(program_handle, "blur_intensity");
    sl_tex = glGetUniformLocation(program_handle, "tex");
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    camera = glm::perspective((float) M_PI / 5, w * 1.0f / h, 0.1f, 10.0f) * glm::lookAt(glm::vec3(2.0f, 2.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {
        glDeleteBuffers(1, &vb_handle);
        glDeleteBuffers(1, &tb_handle);
        glDeleteProgram(program_handle);
        glDeleteTextures(1, &texture_handle);
        exit(EXIT_SUCCESS);
    }
    switch (key) {
        case 'q':
            x_distort += 0.01f;
        case 'a':
            x_distort -= 0.005f;
            break;
        case 'w':
            y_distort += 0.01f;
        case 's':
            y_distort -= 0.005f;
            break;
        case 'e':
            r_intensity += 0.02f;
        case 'd':
            r_intensity -= 0.01f;
            clamp(r_intensity, 0.0f, 1.0f);
            break;
        case 'r':
            g_intensity += 0.02f;
        case 'f':
            g_intensity -= 0.01f;
            clamp(g_intensity, 0.0f, 1.0f);
            break;
        case 't':
            b_intensity += 0.02f;
        case 'g':
            b_intensity -= 0.01f;
            clamp(b_intensity, 0.0f, 1.0f);
            break;
        case 'y':
            blur += 0.002f;
        case 'h':
            blur -= 0.001f;
            clamp(blur, 0.0f, 0.01f);
            break;
    }
    glutPostRedisplay();
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program_handle);
    glUniformMatrix4fv(sl_cam, 1, GL_FALSE, &camera[0][0]);
    glUniform1f(sl_x_distort, x_distort);
    glUniform1f(sl_y_distort, y_distort);
    glUniform1f(sl_r_intensity, r_intensity);
    glUniform1f(sl_g_intensity, g_intensity);
    glUniform1f(sl_b_intensity, b_intensity);
    glUniform1f(sl_blur, blur);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glUniform1i(sl_tex, 0);
    
    glEnableVertexAttribArray(sl_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, vb_handle);
    glVertexAttribPointer(sl_vertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    
    glEnableVertexAttribArray(sl_coord);
    glBindBuffer(GL_ARRAY_BUFFER, tb_handle);
    glVertexAttribPointer(sl_coord, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    
    glDisableVertexAttribArray(sl_vertex);
    glDisableVertexAttribArray(sl_coord);
    
    glutSwapBuffers();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Image Processing");
    
    if (glewInit() != GLEW_OK)
        error("Failed to initialize GLEW.");
    
    glutDisplayFunc(render);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    
    initialize();
    
    glutMainLoop();
    
    return 0;
}