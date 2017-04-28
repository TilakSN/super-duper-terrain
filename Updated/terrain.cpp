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

const float fov = 70.0f;
const float z_near = 0.1f;
const float z_far = 10.0f;

const int mesh_size = 256;

const string texture_names[] = {"water.jpg", "grass.jpg", "snow.jpg", "heightmap.jpg"};
const string vertex_shader_name = "super_duper_terrain.vert";
const string fragment_shader_name = "super_duper_terrain.frag";

bool animate_water = true;
bool compute_transform = true;

float height_map = 0.0;
float rotation_speed = 0.0;
float tilt_speed = 0.0;
float angle = M_PI / 4.0;
float phi = M_PI / 5.0;
float radius = 2.0f;

GLuint program_handle;
GLuint vertex_shader_handle;
GLuint fragment_shader_handle;
GLuint vertex_buffer_handle;
GLuint texture_buffer_handle;
GLuint water_handle;
GLuint grass_handle;
GLuint snow_handle;
GLuint height_map_handle;

GLuint sl_transform;
GLuint sl_position;
GLuint sl_texture_coord;
GLuint sl_height;
GLuint sl_water;
GLuint sl_grass;
GLuint sl_snow;
GLuint sl_height_map;

glm::mat4 camera(1.0f), projection(1.0f), view(1.0f);

int num_vertices;

void controls() {
    cout << "Controls:" << endl;
    cout << "q: increase height" << endl;
    cout << "a: decrease height" << endl;
    cout << "w: increase clockwise rotation speed" << endl;
    cout << "s: increase anti-clockwise rotation speed" << endl;
    cout << "e: tilt up" << endl;
    cout << "d: tilt down" << endl;
    cout << "r: zoom out" << endl;
    cout << "f: zoom in" << endl;
}

void _error(const string &message) {
    cerr << message << endl;
    exit(EXIT_FAILURE);
}

template <typename T>
void clamp(T &value, T minimum, T maximum) {
    if (minimum > maximum)
        return;
    if (value < minimum)
        value = minimum;
    else if (value > maximum)
        value = maximum;
}

GLuint load_shader(const string &filename, GLenum type) {
    GLuint handle = glCreateShader(type);
    ifstream input_stream(filename);
    if (!input_stream.is_open())
        _error("Failed to open shader.");

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
        _error("Failed to compile shader.");
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
        _error("Unknown texture format.");

    if (FreeImage_FIFSupportsReading(image_format))
        image = FreeImage_Load(image_format, name);

    if (image == 0)
        _error("Unable to open file.");

    GLuint width, height, channels, length;
    width = FreeImage_GetWidth(image);
    height = FreeImage_GetHeight(image);
    channels = FreeImage_GetLine(image) / width;
    if (channels < 3 or channels > 4)
        _error("Invalid texture.");
    length = width * height * channels;

    BYTE *file_data = FreeImage_GetBits(image);
    GLbyte *data = new GLbyte[length];
    if (length == 0 or data <= 0)
        _error("Unable to load texture.");

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
        _error("Failed to link program.");
    }

    glDetachShader(program_handle, vertex_shader_handle);
    glDetachShader(program_handle, fragment_shader_handle);

    glDeleteShader(vertex_shader_handle);
    glDeleteShader(fragment_shader_handle);
}

void generate_textures() {
    glGenTextures(1, &water_handle);
    load_texture(texture_names[0], water_handle, GL_BGR);
    glGenTextures(1, &grass_handle);
    load_texture(texture_names[1], grass_handle, GL_BGR);
    glGenTextures(1, &snow_handle);
    load_texture(texture_names[2], snow_handle, GL_BGR);
    glGenTextures(1, &height_map_handle);
    load_texture(texture_names[3], height_map_handle);
}

void generate_vertices() {
    num_vertices = mesh_size * mesh_size * 6;
    GLfloat vertices[num_vertices * 3];
    GLfloat texture_coord[num_vertices * 2];
    int v_i = 0, c_i = 0;
    float p_x = -1, p_y, t_x = 0, t_y, p_i = 2.0 / mesh_size, t_i = 1.0 / mesh_size;    
    for (int i = -1; ++i < mesh_size;) {
        p_y = -1;
        t_y = 0;
        for (int j = -1; ++j < mesh_size;) {
            #define PUSH(a, b, c, d) vertices[v_i++] = a; vertices[v_i++] = b; vertices[v_i++] = 0.0f; texture_coord[c_i++] = c; texture_coord[c_i++] = d
            PUSH(p_x, p_y, t_x, t_y);
            PUSH(p_x + p_i, p_y, t_x + t_i, t_y);
            PUSH(p_x + p_i, p_y + p_i, t_x + t_i, t_y + t_i);

            PUSH(p_x, p_y, t_x, t_y);
            PUSH(p_x + p_i, p_y + p_i, t_x + t_i, t_y + t_i);
            PUSH(p_x, p_y + p_i, t_x, t_y + t_i);

            p_y += p_i;
            t_y += t_i;
        }
        p_x += p_i;
        t_x += t_i;
    }

    glGenBuffers(1, &vertex_buffer_handle);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * v_i, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &texture_buffer_handle);
    glBindBuffer(GL_ARRAY_BUFFER, texture_buffer_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * c_i, texture_coord, GL_STATIC_DRAW);
}

glm::vec3 get_camera_position() {
    GLfloat z = radius * sin(phi);
    GLfloat xy = radius * cos(phi);
    GLfloat x = xy * cos(angle);
    GLfloat y = xy * sin(angle);
    return glm::vec3(x, y, z);
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
    sl_height = glGetUniformLocation(program_handle, "height");
    sl_water = glGetUniformLocation(program_handle, "waterTex");
    sl_grass = glGetUniformLocation(program_handle, "grassTex");
    sl_snow = glGetUniformLocation(program_handle, "snowTex");
    sl_height_map = glGetUniformLocation(program_handle, "heightMap");
}

void free_memory() {
    glDeleteBuffers(1, &vertex_buffer_handle);
    glDeleteBuffers(1, &texture_buffer_handle);
    glDeleteProgram(program_handle);
    glDeleteTextures(1, &water_handle);
    glDeleteTextures(1, &grass_handle);
    glDeleteTextures(1, &snow_handle);
    glDeleteTextures(1, &height_map_handle);
}

void keyboard_callback(unsigned char key, int x, int y) {
    if (key == 27) {
        free_memory();
        glutLeaveMainLoop();
        return;
    }
    switch (key) {
        case 'q':
            height_map += 0.02f;
        case 'a':
            height_map -= 0.01f;
            clamp(height_map, 0.0f, 1.0f);
            break;
        case 'w':
            rotation_speed += 0.002f;
        case 's':
            rotation_speed -= 0.001f;
            clamp(rotation_speed, -1.0f, 1.0f);
            break;
        case 'e':
            tilt_speed += 0.002f;
        case 'd':
            tilt_speed -= 0.001f;
            clamp(tilt_speed, -0.5f, 0.5f);
            break;
        case 'r':
            radius += 0.04f;
        case 'f':
            radius -= 0.02f;
            clamp(radius, 1.0f, 8.0f);
            compute_transform = true;
            break;
        case 'z':
            animate_water = not animate_water;
            break;
    }
    glutPostRedisplay();
}

void reshape_callback(int w, int h) {
    glViewport(0, 0, w, h);
    calibrate_camera();
}

void render_scene() {
    if (rotation_speed != 0.0f) {
        angle += rotation_speed;
        compute_transform = true;
    }
    if (tilt_speed != 0.0f) {
        phi += tilt_speed;
        compute_transform = true;
    }

    if (compute_transform) {
        place_camera();
        glutPostRedisplay();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program_handle);
    glUniformMatrix4fv(sl_transform, 1, GL_FALSE, &camera[0][0]);
    glUniform1f(sl_height, height_map);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, water_handle);
    glUniform1i(sl_water, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, grass_handle);
    glUniform1i(sl_grass, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, snow_handle);
    glUniform1i(sl_snow, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, height_map_handle);
    glUniform1i(sl_height_map, 3);

    glEnableVertexAttribArray(sl_position);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_handle);
    glVertexAttribPointer(sl_position, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glEnableVertexAttribArray(sl_texture_coord);
    glBindBuffer(GL_ARRAY_BUFFER, texture_buffer_handle);
    glVertexAttribPointer(sl_texture_coord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glDisableVertexAttribArray(sl_position);
    glDisableVertexAttribArray(sl_texture_coord);

    glutSwapBuffers();
    // glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Terrain");
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        _error("Failed to initialize GLEW.");

    glutDisplayFunc(render_scene);
    glutKeyboardFunc(keyboard_callback);
    glutReshapeFunc(reshape_callback);

    glClearColor(0, 0, 0, 0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    generate_shaders();
    generate_program();
    generate_textures();
    generate_vertices();
    setup_camera();
    get_locations();

    controls();
    
    glutMainLoop();
    return 0;
}