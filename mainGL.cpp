#include <bits/stdc++.h>
#include "includeGL.h"
#include "utils.h"
#include <glm/glm.hpp>
#include "cameraGL.h"
#include "vertexGL.h"
#include "programGL.h"
#include "meshGL.h"
#include "textureGL.h"
using namespace std;

const string texture_names[4] = {"water.jpg", "grass.jpg", "snow.jpg"};
const string height_map_file = "heightmap.jpg";
const string vertex_shader_name = "terrain.vs";
const string fragment_shader_name = "terrain.fs";
const float z_near = 0.1f;
const float z_far = 10.0f;
const float fov = 75.0f;

ProgramGL *program;
TextureGL *textures;
VertexArray *vertex_array;
CameraGL camera;

GLuint *vbo_ids;

TextureGL height_texture;
float height_map = 1.0f;
// float run_time = 0.0f;

void display_callback() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    height_map += 0.01;
    if (height_map > 1)
        height_map -= 1;
    program->set_uniform("height", height_map);
    program->set_uniform("transform", camera.get_matrix());
    vertex_array->draw(vbo_ids);
    glutSwapBuffers();
    glutPostRedisplay();
}

void keypress_callback(unsigned char key_pressed, int mouse_x, int mouse_y) {
    if (key_pressed == 27) {
        glutLeaveMainLoop();
    }
    glutPostRedisplay();
}

void reshape_callback(int width, int height) {
    glViewport(0, 0, width, height);
    camera.project(fov, ((float) width) / height, z_near, z_far);
    glutPostRedisplay();
}

void create_program() {
    program = new ProgramGL(vertex_shader_name, fragment_shader_name);
    program->compile();
}

void generate_vertices() {
    height_texture.load_from_file(height_map_file);
    Mesh2D mesh(64, height_texture);
    vector<float> pos = mesh.get_position();
    vector<float> tex = mesh.get_texture_coordinates();
    vertex_array = new VertexArray(mesh.get_num_vertices());
    vertex_array->add_buffer(&pos[0], 3);
    vertex_array->add_buffer(&tex[0], 2);
    vertex_array->generate();
    vbo_ids = new GLuint[2];
    vbo_ids[0] = program->get_attrib_location("VertexPosition");
    vbo_ids[1] = program->get_attrib_location("TexCoord");
}

void generate_textures() {
    GLuint texture_handles[3];
    glGenTextures(3, texture_handles);
    textures = new TextureGL[3];
    string names[3] = {"water", "grass", "snow"};
    for (int i = -1; ++i < 3;) {
        textures[i].load_from_file(texture_names[i]);
        textures[i].load_texture(texture_handles[i]);
        textures[i].use_texture(program->get_uniform_location(names[i]), i);
    }
}

void setup_camera() {
    camera.project(fov, 1.0f, z_near, z_far);
    camera.look(glm::vec3(3.0f, 3.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    program->set_uniform("transform", camera.get_matrix());
}

void initialize_functions() {
    glutDisplayFunc(display_callback);
    glutKeyboardFunc(keypress_callback);
    glutReshapeFunc(reshape_callback);

    glClearColor(0, 0, 0, 0);
    // glEnable(GL_MULTISAMPLE);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    create_program();
    generate_vertices();
    generate_textures();
    setup_camera();
}

int main(int argc, char **argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);// | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL");
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        error_message("Failed to initialize glew");
    initialize_functions();
    glutMainLoop();
    cout << "Done" << endl;
    return 0;
}