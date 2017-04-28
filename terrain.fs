#version 130

flat in int tex;
varying vec2 texco;

uniform sampler2D water;
uniform sampler2D grass;
uniform sampler2D snow;
// uniform sampler2D mapSampler;

void main() {
    // if (tex <= 0)
        gl_FragColor = texture2D(water, texco);
    // else if (tex <= 1)
    //     gl_FragColor = texture2D(grass, texco);
    // else
    //     gl_FragColor = texture2D(snow, texco);
}