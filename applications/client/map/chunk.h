#pragma once

#include <algorithm>
#include "cppgl/src/cppgl.h"
#include "block.h"


struct Plan
{
    glm::vec3 pos;

    // unit vector
    glm::vec3 normal;

    // distance from origin to the nearest point in the plan
    float distance;
};


struct Frustum
{
    Plan topFace;
    Plan bottomFace;

    Plan rightFace;
    Plan leftFace;

    Plan farFace;
    Plan nearFace;
};

struct Sphere
{
    glm::vec3 center;
    float radius;
    float distance;

    bool isOnOrForwardPlan(Plan& plan, glm::vec3 point);
    bool isOnFrustum(Frustum& camFrustum);
};


class Chunk 
{
public:
    using BlockPtr = std::shared_ptr<Block>;
    using Grid = std::vector<std::vector<std::vector<BlockPtr>>>;

    explicit Chunk(glm::ivec3 pos);
    
    //
    // add a specific Block to the chunk
    // x,y,z in world space (not normalized to origin)
    //
    void set(glm::ivec3 pos, const BlockPtr &block);
    
    BlockPtr operator () (glm::ivec3 pos);

    void draw(Shader &shader);

    bool visible = true;

    Sphere sphere;
    static Frustum frustum;
    static Frustum createFrustumFromCamera();

private:
    // lower left position
    // a chunk contains all blocks in range [opos.x, opos.x+dimension-1]x[opos.y, opos.y+dimension-1]x[opos.z, opos.z+dimension-1]
    glm::ivec3 opos;

    Grid blocks;
};
