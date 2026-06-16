#version 430 core

layout(binding = 0) uniform sampler2D u_scene;
layout(binding = 1) uniform sampler2D u_position;

uniform vec2 u_texelSize;
uniform float u_focusDistance;
uniform float u_focusRange;
uniform float u_maxBlurRadius;
uniform int u_debugView;

in vec2 texCoords;

out vec4 fragColor;

const vec2 sampleOffsets[16] = vec2[](
	vec2(-0.326, -0.406),
	vec2(-0.840, -0.074),
	vec2(-0.696,  0.457),
	vec2(-0.203,  0.621),
	vec2( 0.962, -0.195),
	vec2( 0.473, -0.480),
	vec2( 0.519,  0.767),
	vec2( 0.185, -0.893),
	vec2( 0.507,  0.064),
	vec2( 0.896,  0.412),
	vec2(-0.322, -0.933),
	vec2(-0.792, -0.598),
	vec2(-0.425,  0.096),
	vec2(-0.056,  0.932),
	vec2( 0.126,  0.310),
	vec2(-0.940,  0.343)
);

float circleOfConfusion(float linearDepth) {
	if (linearDepth <= 0.0) {
		return 0.0;
	}

	float focusError = max(abs(linearDepth - u_focusDistance) - u_focusRange, 0.0);
	return clamp(focusError / u_focusRange, 0.0, 1.0);
}

vec3 blurDebugColor(float blurAmount) {
	vec3 sharp = vec3(0.05, 0.12, 0.55);
	vec3 medium = vec3(1.0, 0.72, 0.08);
	vec3 blurred = vec3(1.0, 0.05, 0.02);
	if (blurAmount < 0.5) {
		return mix(sharp, medium, blurAmount * 2.0);
	}
	return mix(medium, blurred, (blurAmount - 0.5) * 2.0);
}

void main() {
	float linearDepth = texture(u_position, texCoords).w;
	float blurAmount = circleOfConfusion(linearDepth);
	float blurRadius = blurAmount * u_maxBlurRadius;

	if (u_debugView == 1) {
		fragColor = vec4(blurDebugColor(blurAmount), 1.0);
		return;
	}

	vec3 color = texture(u_scene, texCoords).rgb;
	float totalWeight = 1.0;

	for (int i = 0; i < 16; ++i) {
		vec2 sampleCoords = texCoords + sampleOffsets[i] * u_texelSize * blurRadius;
		float sampleDepth = texture(u_position, sampleCoords).w;
		float sampleBlur = circleOfConfusion(sampleDepth);
		float weight = mix(0.35, 1.0, sampleBlur);

		color += texture(u_scene, sampleCoords).rgb * weight;
		totalWeight += weight;
	}

	vec3 blurredColor = color / totalWeight;
	vec3 sharpColor = texture(u_scene, texCoords).rgb;
	fragColor = vec4(mix(sharpColor, blurredColor, blurAmount), 1.0);
}
