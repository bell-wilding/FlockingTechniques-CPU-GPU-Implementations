#version 430 core

layout (local_size_x = 1024) in;

struct Agent {
    float pos[3];
    float vel[3];
    uint cell;
};

layout(std430, binding = 0) buffer agentBuffer
{
    Agent agents[];
};

layout(location = 1) uniform uint size;
layout(location = 2) uniform float dt;
layout(location = 3) uniform float bound;

// x = alignment, y = separation, z = cohesion, w = avoidance
layout(location = 4) uniform vec4 radii;
layout(location = 5) uniform vec4 weights;

layout(location = 6) uniform float maxVel;
layout(location = 7) uniform float maxAngle;

// 0 = no click, 1 = left click, 2 = right click
layout(location = 8) uniform int mouseState;

layout(location = 9) uniform vec3 avLineStart;
layout(location = 10) uniform vec3 avLineEnd;

void wrap(inout float curPos[3]) {
    if (curPos[0] < -bound)
    {
        curPos[0] = bound;
    }
    else if (curPos[0] > bound)
    {
        curPos[0] = -bound;
    }

    if (curPos[1] < -bound)
    {
        curPos[1] = bound;
    }
    else if (curPos[1] > bound)
    {
        curPos[1] = -bound;
    }

    if (curPos[2] < -bound)
    {
        curPos[2] = bound;
    }
    else if (curPos[2] > bound)
    {
        curPos[2] = -bound;
    }
}

vec3 steer(vec3 desired, vec3 velocity) {
    vec3 steer = normalize(desired) * maxVel - velocity;
    steer = clamp(steer, vec3(-maxAngle), vec3(maxAngle));
    return steer;
}

vec3 ClosestPointOnLine(vec3 point) {
    vec3 rayLine = avLineEnd - avLineStart;
	float t = dot(point - avLineStart, rayLine) / dot(rayLine, rayLine);
	return avLineStart + rayLine * clamp(t, 0.0, 1.0);
}

vec3 InteractWithRay(vec3 bPos) {
    if (mouseState == 0)
        return vec3(0);

    vec3 avoidancePoint = ClosestPointOnLine(bPos);
    vec3 offset = bPos - avoidancePoint;

	float sqrDist = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z;

	if (sqrDist < radii.w) {
		float strength = 1.0 - (sqrDist / radii.w);
		return offset * strength * (mouseState == 1 ? 1.0 : -1.0);
	}
    return vec3(0);
}

void main(void) 
{
    uint id = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;

    vec3 pos = vec3(agents[id].pos[0], agents[id].pos[1], agents[id].pos[2]);
    vec3 vel = vec3(agents[id].vel[0], agents[id].vel[1], agents[id].vel[2]);

    vec3 align = vel;
    vec3 separate = vel;
    vec3 cohese = pos;
    int neighbours = 1;
    vec3 acceleration = vec3(0);

    for (int i = 0; i < size; i++) {
        if (i != id) {
            vec3 oPos = vec3(agents[i].pos[0], agents[i].pos[1], agents[i].pos[2]);
            vec3 oVel = vec3(agents[i].vel[0], agents[i].vel[1], agents[i].vel[2]);

            vec3 offset = oPos - pos;
            float sqrDist = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z;

            if (sqrDist < radii.x) {
                align += oVel;
            }
            if (sqrDist < radii.y) {
                float strength = 1.0 - (sqrDist / radii.y);
                separate += -offset * strength;
            }
            if (sqrDist < radii.z) {
                neighbours++;
                cohese += oPos;
            }
        }
    }

    cohese /= neighbours;
    cohese -= pos;

    acceleration += steer(align, vel) * weights.x;
    acceleration += steer(separate, vel) * weights.y;
    acceleration += steer(cohese, vel) * weights.z;
    acceleration += InteractWithRay(pos) * weights.w;

    agents[id].pos[0] += vel.x * dt;
    agents[id].pos[1] += vel.y * dt;
    agents[id].pos[2] += vel.z * dt;

    vel += acceleration;
    vel = clamp(vel, vec3(-maxVel), vec3(maxVel));

    agents[id].vel[0] = vel.x;
    agents[id].vel[1] = vel.y;
    agents[id].vel[2] = vel.z;

    wrap(agents[id].pos);
}