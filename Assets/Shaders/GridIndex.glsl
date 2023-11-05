#version 430 core

layout (local_size_x = 1024) in;

struct Agent {
    float pos[3];
    float vel[3];
    uint  cell;
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
layout(location = 2) uniform uint cellsPerAxis;

// Implementation found at: https://github.com/vojtatom/flocking-cpp/blob/master/src/glcontext.cpp
void main(void) 
{
    uint id = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;
   
    uint cellID = agents[id].cell;
    uint prevAgentCellID;
        
    if (id == 0)
    {
        prevAgentCellID = 0;
        indices[0] = 0;   
    } else 
        prevAgentCellID = agents[id - 1].cell;

        
    if (cellID > prevAgentCellID)
    {
        for(uint i = prevAgentCellID + 1; i <= cellID; ++i)
            indices[i] = id;           
    }

    if (id == size - 1)
    {
        uint lastCell = cellsPerAxis * cellsPerAxis * cellsPerAxis;
        for(uint i = cellID + 1; i < lastCell + 1; ++i)
            indices[i] = id + 1;  
    }
}