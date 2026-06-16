#version 430 core

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projMat;
uniform mat3 u_normalMat;
uniform vec3 u_viewPos;

layout(location = 0) in vec3 in_vert;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texCoord;

layout(location = 3) in vec3 in_position;
layout(location = 4) in float in_age;
layout(location = 5) in vec3 in_color;
layout(location = 6) in float in_maxAge;
layout(location = 7) in float in_size;
layout(location = 8) in float in_stretch;
layout(location = 9) in float in_shape;
layout(location = 10) in float in_rotation;

out vec3 f_normal;
out vec3 f_position;
out vec2 f_texCoord;
out vec3 f_color;
out float f_lifePercent;
out float f_shape;

void main(void)
{
	// Billboard - make quad face the camera
	vec3 camForward = normalize(u_viewPos - in_position);
	vec3 camUp = vec3(0.0, 1.0, 0.0);
	vec3 camRight = normalize(cross(camUp, camForward));
	camUp = cross(camForward, camRight);

	// Current particle size (varies with life)
	float lifePercent = in_age / in_maxAge;
	float sizeScale = sin(lifePercent * 3.14159) * in_size;

	// Scale and rotate the quad in billboard space.
	vec2 local = vec2(in_vert.x, in_vert.y * in_stretch) * sizeScale;
	float cs = cos(in_rotation);
	float sn = sin(in_rotation);
	vec2 rotated = vec2(local.x * cs - local.y * sn, local.x * sn + local.y * cs);

	// Billboard offset
	vec3 worldPos = in_position
		+ camRight * rotated.x
		+ camUp * rotated.y;

	gl_Position = u_projMat * u_viewMat * vec4(worldPos, 1.0);

	f_normal = camForward;
	f_position = worldPos;
	f_texCoord = in_texCoord;
	f_color = in_color;
	f_lifePercent = lifePercent;
	f_shape = in_shape;
}

