#include <bits/stdc++.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <FreeImage.h>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

#define BUFFER_OFFSET(x) (void *)(x)
using namespace std;

const float fov = M_PI / 5;
const float z_near = 0.1f;
const float z_far = 10.0f;

const string texture_names = "flame.png";
const string vertex_shader_name = "fire.vert";
const string fragment_shader_name = "fire.frag";

bool compute_transform = true;

float rotation_speed = 0.0;
float angle = 0.0f;
float radius = 3.0f;
float shift_tex = 0.0f;

int planes = 4;

int animation_speed = 10;
int animation_count = 0;

GLuint program_handle;
GLuint vertex_shader_handle;
GLuint fragment_shader_handle;
GLuint vertex_buffer_handle;
GLuint texture_buffer_handle;
GLuint index_buffer_handle;
GLuint flame_handle;

GLuint sl_transform;
GLuint sl_position;
GLuint sl_texture_coord;
GLuint sl_flame;
GLuint sl_shift;

glm::mat4 camera(1.0f), projection(1.0f), view(1.0f);

int num_vertices;

void assert_error(const string &message) {
    cerr << message << endl;
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
        assert_error("Failed to open shader.");

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
        assert_error("Failed to compile shader.");
    }
    return handle;
}

void load_texture(const string &filename, GLuint handle, GLenum file_format=0) {
    const char *name = filename.c_str();
    FREE_IMAGE_FORMAT image_format = FIF_UNKNOWN;
    FIBITMAP *image(0);

    image_format = FreeImage_GetFileType(name, 0);
    if (image_format == FIF_UNKNOWN)
        image_format = FreeImage_GetFIFFromFilename(name);

    if (image_format == FIF_UNKNOWN)
        assert_error("Unknown texture format.");

    if (FreeImage_FIFSupportsReading(image_format))
        image = FreeImage_Load(image_format, name);

    if (image == 0)
        assert_error("Unable to open file.");

    GLuint width, height, channels, length;
    width = FreeImage_GetWidth(image);
    height = FreeImage_GetHeight(image);
    channels = FreeImage_GetLine(image) / width;
    if (channels < 3 or channels > 4)
        assert_error("Invalid texture.");
    length = width * height * channels;

    BYTE *file_data = FreeImage_GetBits(image);
    GLbyte *data = new GLbyte[length];
    if (length == 0 or data <= 0)
        assert_error("Unable to load texture.");

    memcpy(data, file_data, length);
    GLuint format = (channels == 3) ? GL_RGB : GL_RGBA;
    GLuint input_format = (file_format) ? file_format : format;

    FreeImage_Unload(image);

    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, input_format, GL_UNSIGNED_BYTE, data);
    delete[] data;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void generate_shaders() {
    vertex_shader_handle = load_shader(vertex_shader_name, GL_VERTEX_SHADER);
    fragment_shader_handle = load_shader(fragment_shader_name, GL_FRAGMENT_SHADER);
}

void generate_program() {
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
            cerr << "Program info:\n" << log << endl;
            delete[] log;
        }
        assert_error("Failed to link program.");
    }

    glDetachShader(program_handle, vertex_shader_handle);
    glDetachShader(program_handle, fragment_shader_handle);

    glDeleteShader(vertex_shader_handle);
    glDeleteShader(fragment_shader_handle);
}

void generate_textures() {
    glGenTextures(1, &flame_handle);
    load_texture(texture_names, flame_handle, GL_BGRA);
}

void generate_vertices() {
    num_vertices = 6;
    GLfloat vertices[num_vertices * 3];
    GLfloat texture_coord[num_vertices * 2];
    int v_i = 0, c_i = 0;
    #define PUSH(a, b, c, d) vertices[v_i++] = 0.0f; vertices[v_i++] = a; vertices[v_i++] = b; texture_coord[c_i++] = c; texture_coord[c_i++] = d
    PUSH(-1.0f, -1.0f, 0.0f, 0.0f);
    PUSH(1.0f, -1.0f, 0.03125f, 0.0f);
    PUSH(1.0f, 1.0f, 0.03125f, 1.0f);

    PUSH(-1.0f, -1.0f, 0.0f, 0.0f);
    PUSH(1.0f, 1.0f, 0.03125f, 1.0f);
    PUSH(-1.0f, 1.0f, 0.0f, 1.0f);

    glGenBuffers(1, &vertex_buffer_handle);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * v_i, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &texture_buffer_handle);
    glBindBuffer(GL_ARRAY_BUFFER, texture_buffer_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * c_i, texture_coord, GL_STATIC_DRAW);
}

glm::vec3 get_camera_position() {
    GLfloat x = radius * cos(angle);
    GLfloat y = radius * sin(angle);
    return glm::vec3(x, y, 1.0f);
}

void place_camera() {
    view = glm::lookAt(get_camera_position(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    camera = projection * view;
    compute_transform = false;
}

void calibrate_camera() {
    // fov, aspect ratio, z near, z far
    projection = glm::perspective(fov, glutGet(GLUT_WINDOW_WIDTH) * 1.0f / glutGet(GLUT_WINDOW_HEIGHT), z_near, z_far);
    camera = projection * view;
}

void setup_camera() {
    view = glm::lookAt(get_camera_position(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    calibrate_camera();
}

void get_locations() {
    sl_transform = glGetUniformLocation(program_handle, "transform");
    sl_position = glGetAttribLocation(program_handle, "VertexPosition");
    sl_texture_coord = glGetAttribLocation(program_handle, "TexCoord");
    sl_flame = glGetUniformLocation(program_handle, "tex");
    sl_shift = glGetUniformLocation(program_handle, "shift");
}

void free_memory() {
    glDeleteBuffers(1, &vertex_buffer_handle);
    glDeleteBuffers(1, &texture_buffer_handle);
    glDeleteProgram(program_handle);
    glDeleteTextures(1, &flame_handle);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {
        free_memory();
        glutLeaveMainLoop();
        return;
    }
    switch (key) {
        case 'q':
            planes += 2;
        case 'a':
            planes--;
            clamp(planes, 1, 32);
            break;
        case 'w':
            rotation_speed += 0.002f;
        case 's':
            rotation_speed -= 0.001f;
            clamp(rotation_speed, -1.0f, 1.0f);
            break;
        case 'e':
            animation_speed += 2;
        case 'd':
            animation_speed--;
            clamp(animation_speed, 0, 1000);
            break;
        case 'r':
            radius += 0.04f;
        case 'f':
            radius -= 0.02f;
            clamp(radius, 1.0f, 8.0f);
            compute_transform = true;
            break;
    }
    glutPostRedisplay();
}

void reshape_callback(int w, int h) {
    glViewport(0, 0, w, h);
    calibrate_camera();
}

void render() {
    if (rotation_speed != 0.0f) {
        angle += rotation_speed;
        compute_transform = true;
    }
    
    animation_count++;
    if (animation_speed <= animation_count) {
        animation_count = 0;
        shift_tex += 0.03125f;
        if (shift_tex > 0.99f)
            shift_tex -= 1.0f;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program_handle);
    
    if (compute_transform) {
        place_camera();
    }
    glm::mat4 tr = camera;
    glm::mat4 rot = glm::rotate((float) M_PI / planes, glm::vec3(0.0f, 0.0f, 1.0f));
    float st = shift_tex;
    
    for (int i = -1; ++i < planes;) {
        glUniformMatrix4fv(sl_transform, 1, GL_FALSE, &tr[0][0]);
        glUniform1f(sl_shift, st);
        
        st += 0.03125f;
        if (st > 0.99f)
            st = 0.0f;
        tr *= rot;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, flame_handle);
        glUniform1i(sl_flame, 0);

        glEnableVertexAttribArray(sl_position);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_handle);
        glVertexAttribPointer(sl_position, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glEnableVertexAttribArray(sl_texture_coord);
        glBindBuffer(GL_ARRAY_BUFFER, texture_buffer_handle);
        glVertexAttribPointer(sl_texture_coord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(sl_position);
        glDisableVertexAttribArray(sl_texture_coord);
    }

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    srand(time(NULL));
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Fire");
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        assert_error("Failed to initialize GLEW.");

    glutDisplayFunc(render);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape_callback);

    glClearColor(0, 0, 0, 0);
    // glDepthFunc(GL_LESS);
    // glEnable(GL_DEPTH_TEST);
    // glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    generate_shaders();
    generate_program();
    generate_textures();
    generate_vertices();
    setup_camera();
    get_locations();

    glutMainLoop();
    return 0;
}