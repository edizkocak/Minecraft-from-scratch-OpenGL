#include "chunk.h"
#include "../model/player.h"
#include "network.h"

extern std::shared_ptr<Player> the_player;

Frustum Chunk::frustum;


bool Sphere::isOnOrForwardPlan(Plan& plan, glm::vec3 point)
{
    glm::vec3 n = glm::normalize(plan.normal);
    float d = glm::dot(-n, plan.pos);

    float signed_distance_to_plan = glm::dot(n, point) + d;

    return  signed_distance_to_plan > -radius;
}


bool Sphere::isOnFrustum(Frustum& camFrustum)
{
    return (isOnOrForwardPlan(camFrustum.leftFace, center) &&
            isOnOrForwardPlan(camFrustum.rightFace, center) &&
            isOnOrForwardPlan(camFrustum.farFace, center) &&
            isOnOrForwardPlan(camFrustum.nearFace, center) &&
            isOnOrForwardPlan(camFrustum.topFace, center) &&
            isOnOrForwardPlan(camFrustum.bottomFace, center));
}


Chunk::Chunk(glm::ivec3 pos)
    : opos(pos)
{
    blocks.resize(globals::chunkDim);
    for (int i=0; i < globals::chunkDim; ++i) {
        blocks[i].resize(globals::chunkDim);
        for (int j=0; j < globals::chunkDim; ++j) {
            blocks[i][j].resize(globals::chunkDim);
        }
    }

    sphere.center = glm::vec3(opos) + glm::vec3(0.5f * globals::chunkDim);
    sphere.radius = glm::distance(glm::vec3(opos), sphere.center);

}

void 
Chunk::set(glm::ivec3 pos, const Chunk::BlockPtr &block) {
    int x = pos[0] % globals::chunkDim;
    int y = pos[1] % globals::chunkDim;
    int z = pos[2] % globals::chunkDim;
    blocks[x][y][z] = block;
}

Chunk::BlockPtr 
Chunk::operator () (glm::ivec3 pos) {
    pos -= opos;
    return blocks[pos.x][pos.y][pos.z];
}


void 
Chunk::draw(Shader &shader) {
    if(!visible){ return;}
    if(!sphere.isOnFrustum(frustum)){
        return;
    }

    for(const auto &slice : blocks){
        for(const auto &row:slice){
            for(const auto &block:row){
                if(block && block->visible){
                    block->draw(shader);
                }
            }
        }
    }
}


Frustum Chunk::createFrustumFromCamera()
{
    auto cam = current_camera();

    if(cam->name == "default"){
        return frustum;
    }

    float nearDist = cam->near;
    float farDist = globals::CULLING_FAR;
    float fovY = glm::radians(cam->fov_degree);
    float aspect = cam->aspect_ratio();


    Frustum frustum;

    glm::vec3 p = glm::vec3(cam->pos[0], cam->pos[1], cam->pos[2]);

    float Hnear = 2.0 * tan(fovY * .5f) * nearDist;
    float Wnear = Hnear * aspect;


    float Hfar = 2.0 * tan(fovY * .5f) * farDist;
    float Wfar = Hfar * aspect;


    glm::vec3 d = glm::normalize(cam->dir);
    glm::vec3 up = glm::normalize(cam->up);
    glm::vec3 right = glm::normalize(glm::cross(up, d));

    up = glm::cross(d, right);

    glm::vec3 nc = p + d * nearDist;
    glm::vec3 fc = p + d * farDist;

    // glm::vec3 ftl = fc + (up * (Hfar/2)) - (right * (Wfar /2));
    glm::vec3 ftr = fc + (up * (Hfar/2)) + (right * (Wfar /2));
    glm::vec3 fbl = fc - (up * (Hfar /2)) - (right * (Wfar /2));
    glm::vec3 fbr = fc - (up * (Hfar /2)) + (right * (Wfar /2));

    glm::vec3 ntl = nc + (up * (Hnear /2)) - (right * (Wnear /2));
    glm::vec3 ntr = nc + (up * (Hnear /2)) + (right * (Wnear /2));
    glm::vec3 nbl = nc - (up * (Hnear /2)) - (right * (Wnear /2));
    glm::vec3 nbr = nc - (up * (Hnear /2)) + (right * (Wnear /2));

    auto cur_cam = current_camera();

    glm::vec3 a = (nc + right * (Wnear/2)) - p;
    a = glm::normalize(a);

    glm::vec3 right_normal = glm::cross(a, up);

    glm::vec3 v1 = ftr - ntr;
    glm::vec3 v2 = ntl - ntr;

    glm::vec3 upper_normal = glm::cross(glm::normalize(v1), glm::normalize(v2));

    v1 = fbl - nbl;
    v2 = ntl - nbl;

    glm::vec3 left_normal = glm::cross(glm::normalize(v2), glm::normalize(v1));

    v1 = nbl - nbr;
    v2 = fbr - nbr;

    glm::vec3 bottom_normal = glm::cross(glm::normalize(v1), glm::normalize(v2));


    frustum.nearFace   = { nc, d, 0.f};
    frustum.farFace    = { fc, -d, 0.f};
    frustum.rightFace  = { p, right_normal, 0.f };
    frustum.leftFace   = { p, left_normal, 0.f };
    frustum.topFace    = { p, upper_normal, 0.f };
    frustum.bottomFace = { p, bottom_normal, 0.f };

    return frustum;
}
