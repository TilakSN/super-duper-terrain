#version 130

varying vec2 shift_tex;

uniform sampler2D tex;

void main() {
    gl_FragColor = texture(tex, shift_tex);
}