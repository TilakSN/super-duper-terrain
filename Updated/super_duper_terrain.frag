#version 130

varying float ht;
varying vec2 tex;

uniform sampler2D waterTex;
uniform sampler2D grassTex;
uniform sampler2D snowTex;

void main() {
    if (ht <= 0.0)
        gl_FragColor = texture(waterTex, tex);
    else if (ht < 0.005)
        gl_FragColor = mix(texture(waterTex, tex), texture(grassTex, tex), ht * 200);
    else if (ht < 0.30)
        gl_FragColor = texture(grassTex, tex);
    else if (ht < 0.40)
        gl_FragColor = mix(texture(grassTex, tex), texture(snowTex, tex), ht * 10 - 3.0);
    else
        gl_FragColor = texture(snowTex, tex);
}