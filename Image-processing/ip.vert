#version 130
attribute vec3 VertexPosition;
attribute vec2 TexCoord;

varying vec2 texuv;

uniform float x_distort;
uniform float y_distort;
uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(VertexPosition, 1.0);
    texuv =  vec2(TexCoord.x + sin(TexCoord.y * 10) * x_distort, TexCoord.y + sin(TexCoord.x * 10) * y_distort);
}