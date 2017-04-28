#version 130

attribute vec3 VertexPosition;
attribute vec2 TexCoord;

uniform mat4 transform;
uniform float height;

flat out int tex;
varying vec2 texco;

void main() {
    gl_Position = transform * vec4(VertexPosition, 1.0);
    tex = 0;
    if (gl_Position.z > 0.1)
        tex++;
    if (gl_Position.z > 0.5)
        tex++;
    texco = TexCoord;
}