#version 430 core

layout (local_size_x = 512) in;

struct Agent {
    float pos[3];
    float vel[3];
    uint cell;
};

shared Agent localAgents[1024];

layout(std430, binding = 0) buffer agentBuffer {
    Agent agents[];
};

layout(location = 1) uniform uint size;

layout(location = 4) uniform uint segmentSize;
layout(location = 5) uniform uint iteration;


void swap(uint elemID, uint oelemID) {
    Agent tmp = agents[elemID];
    agents[elemID] = agents[oelemID];
    agents[oelemID] = tmp;
}

void localSwap(uint elemID, uint oelemID) {
    Agent tmp = localAgents[elemID];
    localAgents[elemID] = localAgents[oelemID];
    localAgents[oelemID] = tmp;
}

// Implementation found at: https://github.com/vojtatom/flocking-cpp/blob/master/src/glcontext.cpp
void main(void) {
    

    uint id = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;
    uint locId = gl_LocalInvocationID.x;
    uint hstep = segmentSize >> 1;

    if (id < (size >> 1)) {

        bool asc = hstep > (id % segmentSize);
        hstep = hstep >> iteration;
        uint elemID, oelemID;

        if (hstep > gl_WorkGroupSize.x) {

            elemID = id + (id / hstep) * hstep;
            oelemID = elemID + hstep;
            bool comp = agents[elemID].cell < agents[oelemID].cell;
                 
            if (asc)
                comp = !comp;

            if (comp)
                swap(elemID, oelemID);

        } else  {

            uint copyId = id * 2;
            uint locCopyId = locId * 2;
            localAgents[locCopyId] = agents[copyId];
            localAgents[locCopyId + 1] = agents[copyId + 1];
            barrier();

            elemID = locId + (locId / hstep) * hstep;
            oelemID = elemID + hstep;

            while (hstep > 0) {     
                bool comp = localAgents[elemID].cell < localAgents[oelemID].cell;
                
                if (asc)
                    comp = !comp;

                if (comp)
                    localSwap(elemID, oelemID);
                
                hstep >>= 1;

                elemID = locId + (locId / hstep) * hstep;
                oelemID = elemID + hstep;
                barrier();
            }
            agents[copyId] = localAgents[locCopyId];
            agents[copyId + 1] = localAgents[locCopyId + 1];
        }
    }
}