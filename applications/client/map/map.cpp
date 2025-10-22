#include "map.h"
#include "../../common/networking/MathUtil.h"

using namespace std;

Map::Map(glm::ivec3 size)
    : extent{size * glm::ivec3(globals::chunkDim)}, size(size) {
    init();
}

void 
Map::init() {
    chunks.resize(size[0]);
    for (int i=0; i < size[0]; ++i) {
        chunks[i].resize(size[1]);
        for (int j=0; j < size[1]; ++j) {
            chunks[i][j].resize(size[2]);
            for(int k=0; k<size[2]; ++k){
                chunks[i][j][k] = std::make_shared<Chunk>(glm::ivec3(
                        i * globals::chunkDim, j * globals::chunkDim, k * globals::chunkDim
                        ));
            }
        }
    }
}

void
Map::draw(Shader &shader) {
    for(const auto &slice : chunks){
        for(const auto &row:slice){
            for(const auto &chunk:row){
                if(chunk){
                    chunk->draw(shader);
                }
            }
        }
    }
}

//glm::ivec3 Map::random_surface_pos() {
//    while(true){
//        const auto x = (int)(MathUtil::rand_0_1() * extent[0]);
//        const auto z = (int)(MathUtil::rand_0_1() * extent[2]);
//
//        const glm::ivec3 pos = surface_at({x,0, z});
//        if(pos != IVEC3_INVALID_POS && get(pos)->type != globals::ModelType::water){
//            return pos;
//        }
//    }
//}

glm::ivec3 Map::surface_at(glm::ivec3 pos) {
    return surface_at(pos, extent[1] - 1);
}

glm::ivec3 Map::surface_at(glm::ivec3 pos, int max_height) {
    // start at max height and check each block until not Air (nullptr)

    max_height = glm::min(max_height, extent[1] - 1);

    for(int y = max_height; y>=0; y--){
        const glm::ivec3 test_pos = {pos[0], y, pos[2]};
        BlockPtr block = get(test_pos);
        if(block){
            return test_pos;
        }
    }
    return IVEC3_INVALID_POS;
}

bool
Map::inval(glm::ivec3 pos) const {
    if(pos.x < 0 || pos.y < 0 || pos.z < 0) return true;
    if(pos.x >= extent.x || pos.y >= extent.y || pos.z >= extent.z) return true;
    return false;
}

Map::BlockPtr
Map::get(glm::ivec3 pos) {
    if(inval(pos)) return nullptr;

    const glm::ivec3 chunkIdx = pos/globals::chunkDim;
    return (*get_chunk(chunkIdx))(pos);
}

void Map::set(glm::ivec3 pos, const BlockPtr &block) {
    const glm::ivec3 chunkIdx = pos/globals::chunkDim;
    const auto &chunk = get_chunk(chunkIdx);
    chunk->set(pos, block);

    if(!block || block->type == globals::ModelType::air || block->type == globals::ModelType::torch){
        // a transparent block is always visible
        // update visibility of all neighbors
        for(const auto diff : std::vector<glm::ivec3>{{1,0,0},{0,1,0},{0,0,1},{-1,0,0},{0,-1,0},{0,0,-1}}){
            const glm::ivec3 otherPos = pos + diff;
            updateVisibility(otherPos);
        }
    }else{
        // non-transparent block
        // check if it is fully covered or visible
        updateVisibility(pos);
    }
}

void Map::updateInitialBlockVisibility() {
    for (int i = 0; i < extent.x; i++) {
        for (int j = 0; j < extent.y; j++) {
            for (int k = 0; k < extent.z; k++) {
                updateVisibility({i,j,k});
            }
        }
    }
}

void Map::updateVisibility(glm::ivec3 pos) {
    const BlockPtr& block = get(pos);
    if(!block || block->type == globals::ModelType::air || block->type == globals::ModelType::torch){
        // a transparent block is always visible
        return;
    }

    // check if block can be seen
    for(const auto diff : std::vector<glm::ivec3>{{1,0,0},{0,1,0},{0,0,1},{-1,0,0},{0,-1,0},{0,0,-1}}){
        const glm::ivec3 otherPos = pos + diff;
        const BlockPtr otherBlock = get(otherPos);

        if(!otherBlock || otherBlock->type == globals::ModelType::air || otherBlock->type == globals::ModelType::torch){
            // otherBlock is transparent
            // this block is not fully covered
            block->visible = true;
            return;
        }
    }

    block->visible = false;
}

Map::ChunkPtr 
Map::get_chunk(glm::ivec3 chunkIdx)
{
    return chunks[chunkIdx[0]][chunkIdx[1]][chunkIdx[2]];
}
