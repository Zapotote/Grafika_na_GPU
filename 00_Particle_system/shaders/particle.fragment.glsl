#version 430 core

in vec3 f_normal;
in vec3 f_position;
in vec2 f_texCoord;
in vec3 f_color;
in float f_lifePercent;
in float f_shape;

out vec4 out_fragColor;

void main() {
	float alpha;
	float centerGlow;

	if (f_shape > 6.5) {
		vec2 uv = f_texCoord;
		vec2 centered = uv - vec2(0.5);
		centered.y *= 0.68;
		float drop = length(centered * vec2(1.25, 0.78));
		float taper = smoothstep(0.0, 0.55, uv.y);
		float halfWidth = mix(0.09, 0.28, taper) * (1.0 - smoothstep(0.72, 1.0, uv.y) * 0.45);
		float side = 1.0 - smoothstep(halfWidth * 0.65, halfWidth, abs(uv.x - 0.5));
		float vertical = smoothstep(0.0, 0.08, uv.y) * (1.0 - smoothstep(0.9, 1.0, uv.y));
		float softDrop = 1.0 - smoothstep(0.22, 0.5, drop);
		alpha = max(side * vertical, softDrop * 0.65);
		centerGlow = 1.0 - smoothstep(0.0, 0.26, length(f_texCoord - vec2(0.44, 0.62)));
	} else if (f_shape > 5.5) {
		vec2 uv = f_texCoord;
		vec2 centered = uv - vec2(0.5);
		float dist = length(centered);
		float star = max(1.0 - smoothstep(0.0, 0.46, dist), 0.0);
		float crossA = 1.0 - smoothstep(0.012, 0.08, abs(centered.x));
		float crossB = 1.0 - smoothstep(0.012, 0.08, abs(centered.y));
		float rays = max(crossA * (1.0 - smoothstep(0.0, 0.48, abs(centered.y))), crossB * (1.0 - smoothstep(0.0, 0.48, abs(centered.x))));
		alpha = max(star * star, rays * 0.7);
		centerGlow = 1.0 - smoothstep(0.0, 0.28, dist);
	} else if (f_shape > 4.5 && f_shape < 5.5) {
		vec2 uv = f_texCoord;
		float x = abs(uv.x - 0.5);
		float bolt = 1.0 - smoothstep(0.028, 0.095, x);
		float body = smoothstep(0.0, 0.018, uv.y) * (1.0 - smoothstep(0.982, 1.0, uv.y));
		alpha = bolt * body;
		centerGlow = 1.0 - smoothstep(0.0, 0.052, x);
	} else if (f_shape > 3.5 && f_shape < 4.5) {
		vec2 uv = f_texCoord;
		vec2 centered = uv - vec2(0.5);
		float dist = length(centered);
		float core = 1.0 - smoothstep(0.0, 0.36, dist);
		float verticalRay = (1.0 - smoothstep(0.02, 0.12, abs(centered.x))) * (1.0 - smoothstep(0.0, 0.5, abs(centered.y)));
		float horizontalRay = (1.0 - smoothstep(0.02, 0.12, abs(centered.y))) * (1.0 - smoothstep(0.0, 0.5, abs(centered.x)));
		alpha = max(core * core, max(verticalRay, horizontalRay) * 0.45);
		centerGlow = 1.0 - smoothstep(0.0, 0.28, dist);
	} else if (f_shape > 2.5 && f_shape < 3.5) {
		vec2 uv = f_texCoord;
		vec2 centered = uv - vec2(0.5);
		float dist = length(centered * vec2(0.55, 0.95));
		float veil = 1.0 - smoothstep(0.0, 0.9, dist);
		alpha = veil * veil * 0.14;
		centerGlow = 0.0;
	} else if (f_shape > 1.5 && f_shape < 2.5) {
		vec2 uv = f_texCoord;
		vec2 centered = uv - vec2(0.5);
		float dist = length(centered * vec2(0.82, 1.18));
		float softEdge = 1.0 - smoothstep(0.08, 0.64, dist);
		float diffuseCore = 1.0 - smoothstep(0.0, 0.46, dist);
		alpha = softEdge * (0.24 + diffuseCore * 0.26);
		centerGlow = 0.02;
	} else if (f_shape > 0.5) {
		vec2 uv = f_texCoord;
		float y = uv.y;
		float x = abs(uv.x - 0.5);
		float waist = smoothstep(0.04, 0.42, y) * (1.0 - smoothstep(0.72, 1.0, y));
		float halfWidth = mix(0.08, 0.46, waist);
		float lick = 0.035 * sin(y * 18.0 + f_lifePercent * 9.0);
		float flameMask = 1.0 - smoothstep(halfWidth * 0.58, halfWidth, x + lick);
		float verticalMask = smoothstep(0.0, 0.11, y) * (1.0 - smoothstep(0.86, 1.0, y));
		alpha = flameMask * verticalMask;
		centerGlow = 1.0 - smoothstep(0.0, halfWidth * 0.52, x);
	} else {
		vec2 center = vec2(0.5);
		float distFromCenter = length(f_texCoord - center);
		alpha = 1.0 - smoothstep(0.18, 0.5, distFromCenter);
		centerGlow = 1.0 - smoothstep(0.0, 0.32, distFromCenter);
	}

	float lifeFade = sin(f_lifePercent * 3.14159) * 0.8 + 0.2;
	alpha *= lifeFade;
	vec3 finalColor = f_color * (0.8 + centerGlow * 0.7);

	if (f_shape > 0.5 && f_shape < 1.5) {
		vec2 uv = f_texCoord;
		float y = uv.y;
		float x = abs(uv.x - 0.5);
		float core = 1.0 - smoothstep(0.0, 0.16, x);
		float baseHeat = 1.0 - smoothstep(0.15, 0.92, y);
		vec3 hotCore = mix(vec3(1.0, 0.55, 0.02), vec3(1.0, 0.96, 0.32), baseHeat);
		finalColor = mix(f_color, hotCore, core * 0.35) * (0.95 + centerGlow * 0.55);
	} else if (f_shape > 4.5 && f_shape < 5.5) {
		finalColor = mix(f_color, vec3(0.95, 0.98, 1.0), centerGlow * 0.85) * (1.05 + centerGlow * 0.65);
	}

	if (alpha < 0.01) discard;

	out_fragColor = vec4(finalColor, alpha);
}

