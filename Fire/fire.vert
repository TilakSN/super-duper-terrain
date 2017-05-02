#version 130

attribute vec3 VertexPosition;
attribute vec2 TexCoord;

varying vec2 shift_tex;

uniform mat4 transform;
uniform float shift;

void main() {
    // pos = transform * point
    gl_Position = transform * vec4(VertexPosition, 1.0);
    shift_tex = vec2(shift, 0.0) + TexCoord;
}
