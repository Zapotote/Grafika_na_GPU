#version 430 core

uniform sampler2D u_textureSampler1;
uniform sampler2D u_textureSampler2;

out vec4 out_fragColor;

in vec3 f_normal;
in vec2 f_texCoord;

void main() {
    vec4 tex1 = texture(u_textureSampler1, f_texCoord);
    vec4 tex2 = texture(u_textureSampler2, f_texCoord);

    float brightness = (tex1.r + tex1.g + tex1.b) / 3.0;

    if (brightness < 0.1)
        discard;

    out_fragColor = 0.5 * (tex1 + tex2);
}