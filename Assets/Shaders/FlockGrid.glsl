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

layout(std430, binding = 1) buffer indexBuffer
{
    uint indices[];
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

layout(location = 11) uniform uint cellsPerAxis;
layout(location = 12) uniform float cellDimensionReciprocal;

vec3 align = vec3(0);
vec3 separate = vec3(0);
vec3 cohese = vec3(0);
int neighbours;

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

vec3 closestPointOnLine(vec3 point) {
    vec3 rayLine = avLineEnd - avLineStart;
	float t = dot(point - avLineStart, rayLine) / dot(rayLine, rayLine);
	return avLineStart + rayLine * clamp(t, 0.0, 1.0);
}

vec3 interactWithRay(vec3 bPos) {
    if (mouseState == 0)
        return vec3(0);

    vec3 avoidancePoint = closestPointOnLine(bPos);
    vec3 offset = bPos - avoidancePoint;

	float sqrDist = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z;

	if (sqrDist < radii.w) {
		float strength = 1.0 - (sqrDist / radii.w);
		return offset * strength * (mouseState == 1 ? 1.0 : -1.0);
	}
    return vec3(0);
}

uint gridIndex(float curPos[3]) {
    return uint((curPos[0] + bound) * cellDimensionReciprocal) 
    + uint((curPos[1] + bound) * cellDimensionReciprocal) * cellsPerAxis 
    + uint((curPos[2] + bound) * cellDimensionReciprocal) * cellsPerAxis * cellsPerAxis;
}

uint gridIndex(float x, float y, float z) {
    return uint((x + bound) * cellDimensionReciprocal) 
    + uint((y + bound) * cellDimensionReciprocal) * cellsPerAxis 
    + uint((z + bound) * cellDimensionReciprocal) * cellsPerAxis * cellsPerAxis;
}

void flock(Agent a, uint oID) {
    vec3 pos = vec3(a.pos[0], a.pos[1], a.pos[2]);
    vec3 vel = vec3(a.vel[0], a.vel[1], a.vel[2]);

    vec3 oPos = vec3(agents[oID].pos[0], agents[oID].pos[1], agents[oID].pos[2]);
    vec3 oVel = vec3(agents[oID].vel[0], agents[oID].vel[1], agents[oID].vel[2]);

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

void traverseCell(Agent a, uint x, uint y, uint z)
{
    uint id = x + y * cellsPerAxis + z * cellsPerAxis * cellsPerAxis;
    uint start = indices[id];
    uint end = indices[id + 1];

    if (start == end)
        return;

    for (uint i = start; i < end; ++i)
        flock(a, i);
}

void main(void) {
    uint bID = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;

    Agent agent = agents[bID];

    vec3 pos = vec3(agent.pos[0], agent.pos[1], agent.pos[2]);
    vec3 vel = vec3(agent.vel[0], agent.vel[1], agent.vel[2]);

    align = vel;
    separate = vel;
    cohese = pos;
    neighbours = 1;
    vec3 acceleration = vec3(0);

    uint gridXY = cellsPerAxis * cellsPerAxis;
    uint z = agent.cell / gridXY;
    uint znext = (z + 1 + cellsPerAxis) % cellsPerAxis;
    uint zprev = (z - 1 + cellsPerAxis) % cellsPerAxis;
    uint tmpy = agent.cell - (z * gridXY);
    uint y = tmpy / cellsPerAxis;
    uint ynext = (y + 1 + cellsPerAxis) % cellsPerAxis;
    uint yprev = (y - 1 + cellsPerAxis) % cellsPerAxis;
    uint x = tmpy - y * cellsPerAxis;
    uint xnext = (x + 1 + cellsPerAxis) % cellsPerAxis;
    uint xprev = (x - 1 + cellsPerAxis) % cellsPerAxis;
        
    traverseCell(agent, xprev, yprev, zprev);
    traverseCell(agent, x, yprev, zprev);
    traverseCell(agent, xnext, yprev, zprev);

    traverseCell(agent, xprev, y, zprev);
    traverseCell(agent, x, y, zprev);
    traverseCell(agent, xnext, y, zprev);

    traverseCell(agent, xprev, ynext, zprev);
    traverseCell(agent, x, ynext, zprev);
    traverseCell(agent, xnext, ynext, zprev);

    traverseCell(agent, xprev, yprev, z);
    traverseCell(agent, x, yprev, z);
    traverseCell(agent, xnext, yprev, z);

    traverseCell(agent, xprev, y, z);
    traverseCell(agent, x, y, z);
    traverseCell(agent, xnext, y, z);

    traverseCell(agent, xprev, ynext, z);
    traverseCell(agent, x, ynext, z);
    traverseCell(agent, xnext, ynext, z);

    traverseCell(agent, xprev, yprev, znext);
    traverseCell(agent, x, yprev, znext);
    traverseCell(agent, xnext, yprev, znext);

    traverseCell(agent, xprev, y, znext);
    traverseCell(agent, x, y, znext);
    traverseCell(agent, xnext, y, znext);

    traverseCell(agent, xprev, ynext, znext);
    traverseCell(agent, x, ynext, znext);
    traverseCell(agent, xnext, ynext, znext);

    cohese /= neighbours;
    cohese -= pos;

    acceleration += steer(align, vel) * weights.x;
    acceleration += steer(separate, vel) * weights.y;
    acceleration += steer(cohese, vel) * weights.z;
    acceleration += interactWithRay(pos) * weights.w;

    agents[bID].pos[0] += vel.x * dt;
    agents[bID].pos[1] += vel.y * dt;
    agents[bID].pos[2] += vel.z * dt;

    vel += acceleration;
    vel = clamp(vel, vec3(-maxVel), vec3(maxVel));

    agents[bID].vel[0] = vel.x;
    agents[bID].vel[1] = vel.y;
    agents[bID].vel[2] = vel.z;

    wrap(agents[bID].pos);

    agents[bID].cell = gridIndex(agents[bID].pos);
}