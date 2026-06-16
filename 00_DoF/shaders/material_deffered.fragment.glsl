#version 430 core

layout(binding = 0) uniform sampler2D u_diffuseTexture;
layout(binding = 1) uniform sampler2D u_specularTexture;
layout(binding = 2) uniform sampler2D u_normalTexture;
layout(binding = 3) uniform sampler2D u_displacementTexture;
layout(binding = 4) uniform sampler2D u_ambientOccTexture;

in vec4 position;
in vec2 texCoords;
in vec3 normal;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec4 out_position;

uniform float u_near;
uniform float u_far;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Transform [0,1] depth to [-1,1] NDC
    return (2.0 * u_near * u_far) / (u_far + u_near - z * (u_far - u_near));
}

void main() {
	out_color = texture(u_diffuseTexture, texCoords);
	out_normal = normalize(normal);
	out_position = vec4(position.xyz / position.w, LinearizeDepth(gl_FragCoord.z));
}
