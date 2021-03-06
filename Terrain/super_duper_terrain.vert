#version 130

attribute vec3 VertexPosition;
attribute vec2 TexCoord;

varying float ht;
varying vec2 tex;

uniform float height;
uniform mat4 transform;
uniform sampler2D heightMap;

void main() {
    vec3 col = texture(heightMap, TexCoord).rgb;
    ht = height * (col.r * 0.29 + col.g * 0.59 + col.b * 0.12);
    gl_Position = transform * vec4(VertexPosition.xy, ht * 0.2, 1.0);
    tex = TexCoord;
}