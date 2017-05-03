#version 130
varying vec2 texuv;

uniform float r_intensity;
uniform float g_intensity;
uniform float b_intensity;

uniform float blur_intensity;

uniform sampler2D tex;

void main() {
    vec3 color = vec3(0.0, 0.0, 0.0);
    int count = 0;
    for (float x = -blur_intensity; x <= blur_intensity; x += 0.005) {
        for (float y = -blur_intensity; y <= blur_intensity; y += 0.005) {
            color += texture(tex, texuv + vec2(x, y)).rgb;
            count++;
        }
    }
    color /= count;
    gl_FragColor = vec4(r_intensity * color.r, g_intensity * color.g, b_intensity * color.b, 1.0);
}