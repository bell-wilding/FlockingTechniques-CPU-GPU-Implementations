#version 430 core

struct Agent {
	float pos[3];
    float vel[3];
    uint cell;
};

uniform mat4 scaleMat		= mat4(1.0f);
uniform mat4 viewMatrix 	= mat4(1.0f);
uniform mat4 projMatrix 	= mat4(1.0f);

uniform bool visualiseGrid = false;
uniform float bound = 50;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;

layout(std430, binding = 0) buffer agentBuffer {
	Agent agents[];
};

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
} OUT;

/**
 * http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
 */
mat4 getRotationMat(vec3 vector)
{
	vec3 unit = vec3(0, 0, 1);
	vec3 f = normalize(vector);
	vec3 cross = cross(f, unit);
	vec3 a = normalize(cross);
	float s = length(cross);
	float c = dot(f, unit);
	float oc = 1.0 - c;

	return mat4(oc * a.x * a.x + c,        oc * a.x * a.y - a.z * s,  oc * a.z * a.x + a.y * s,  0.0,
                oc * a.x * a.y + a.z * s,  oc * a.y * a.y + c,        oc * a.y * a.z - a.x * s,  0.0,
                oc * a.z * a.x - a.y * s,  oc * a.y * a.z + a.x * s,  oc * a.z * a.z + c,        0.0,
                0.0,                       0.0,                       0.0,                       1.0);

}

mat4 getTranslationMat(vec3 position) 
{
	return mat4(1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				position.x, position.y, position.z, 1);
}

vec4 getColourFromCell(uint cell) {
	uint modCell = cell % 27;
	switch (modCell) {
	case 0:
		return vec4(1, 1, 1, 1);
	case 1:
		return vec4(1, 1, 0, 1);
	case 2:
		return vec4(1, 0, 1, 1);
	case 3:
		return vec4(1, 0, 0, 1);
	case 4:
		return vec4(0, 1, 1, 1);
	case 5:
		return vec4(0, 1, 0, 1);
	case 6:
		return vec4(0, 0, 1, 1);
	case 7:
		return vec4(0, 0, 0, 1);
	case 8:
		return vec4(0.5, 0.5, 0.5, 1);
	case 9:
		return vec4(0.5, 0.5, 0, 1);
	case 10:
		return vec4(0.5, 0, 0.5, 1);
	case 11:
		return vec4(0.5, 0, 0, 1);
	case 12:
		return vec4(0, 0.5, 0.5, 1);
	case 13:
		return vec4(0, 0.5, 0, 1);
	case 14:
		return vec4(0, 0, 0.5, 1);
	case 15:
		return vec4(0.25, 0.25, 0.25, 1);
	case 16:
		return vec4(0.25, 0.25, 0, 1);
	case 17:
		return vec4(0.25, 0, 0.25, 1);
	case 18:
		return vec4(0.25, 0, 0, 1);
	case 19:
		return vec4(0, 0.25, 0.25, 1);
	case 20:
		return vec4(0, 0.25, 0, 1);
	case 21:
		return vec4(0, 0, 0.25, 1);
	case 22:
		return vec4(0.75, 0.75, 0.75, 1);
	case 23:
		return vec4(0.75, 0.75, 0, 1);
	case 24:
		return vec4(0.75, 0, 0.75, 1);
	case 25:
		return vec4(0.75, 0, 0, 1);
	case 26:
		return vec4(0, 0.75, 0.75, 1);
	}

}

void main(void)
{
	Agent agent = agents[gl_InstanceID];

	vec3 pos = vec3(agent.pos[0], agent.pos[1], agent.pos[2]);
	vec3 vel = vec3(agent.vel[0], agent.vel[1], agent.vel[2]);

	mat4 modelMatrix  = getTranslationMat(pos) * getRotationMat(vel) * scaleMat;
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	mat4 mvp 		  = (projMatrix * viewMatrix * modelMatrix);

	OUT.worldPos	  = (modelMatrix * vec4(position,1)).xyz;
	OUT.normal		  = normalize(normalMatrix * normalize(normal));

	if (visualiseGrid) {
		OUT.colour    = getColourFromCell(agent.cell);
	} else {
		OUT.colour = vec4((OUT.worldPos.x + bound) / (bound * 2.0), (OUT.worldPos.y + bound) / (bound * 2.0), (OUT.worldPos.z + bound) / (bound * 2.0), 1.0);
	}

	gl_Position		= mvp * vec4(position, 1.0);
}