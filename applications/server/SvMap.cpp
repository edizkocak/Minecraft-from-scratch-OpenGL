#include "SvMap.h"
#include "image.h"
#include "MathUtil.h"

SvMap::SvMap(glm::ivec3 size)
        : size(size), extent{size * glm::ivec3(globals::chunkDim)} {

    grid = std::vector<std::vector<std::vector<globals::ModelType>>>(
            extent.x,
            std::vector<std::vector<globals::ModelType>>(
                    extent.y,
                    std::vector<globals::ModelType>(extent.z, globals::ModelType::air))
    );
}

SvMap SvMap::newFlatMap() {
    SvMap map({3, 1, 3});

    for (int i = 0; i < map.extent.x; i++) {
        int j = 0;
        {
            for (int k = 0; k < map.extent.z; k++) {
                map.set({i, j, k}, globals::ModelType::grass);
            }
        }

    }

    return map;
}

SvMap SvMap::newHeightmap() {
    const unsigned short WATER_HEIGHT = 0;
    std::unique_ptr<Image> img = std::make_unique<PngImage>(globals::renderData() / "mapgen" / "iceland.png");

    // 2089 x 1340
    // TODO
     SvMap map(glm::ivec3(40,3,26));
     float yScale = 0.4;

//     SvMap map(glm::ivec3(10,2,5));
//     SvMap map(glm::ivec3(1,1,1));
//     float yScale = 0.2;

    float xFactor = float(img->getWidth()) / float(map.extent.x);
    float zFactor = float(img->getHeight()) / float(map.extent.z);
    float yFactor = float(map.extent.y) / 255.f;

    // keep original image x-z aspect ratio
    {
        xFactor = glm::max(xFactor, zFactor);
        zFactor = xFactor;
    }

    // lower height scale, otherwise mountains are too extreme
    yFactor *= yScale;

    for (int x = 0; x < map.extent.x; x++) {
        for (int z = 0; z < map.extent.z; z++) {
            const unsigned short height = img->getPixelClamped(
                    int(float(x) * xFactor),
                    int(float(z) * zFactor)
            );

            if (height <= WATER_HEIGHT) {
                const glm::ivec3 pos = glm::ivec3(x, 0, z);
                map.set(pos, globals::ModelType::water);
                continue;
            }

            int y_max = (int)(float(height) * yFactor);
            for(int y = 0; y < y_max; y++){
                const glm::ivec3 pos = glm::ivec3(x, y, z);
                map.set(pos, globals::ModelType::dirt);
            }

            const glm::ivec3 groundPos = glm::ivec3(x, 0, z);
            map.set(groundPos, globals::ModelType::dirt);

            const glm::ivec3 pos = glm::ivec3(x, y_max, z);
            map.set(pos, globals::ModelType::grass);
        }
    }

    return map;
}

globals::ModelType SvMap::get(glm::ivec3 pos) const {
    return grid[pos.x][pos.y][pos.z];
}

void SvMap::set(glm::ivec3 pos, const globals::ModelType &block) {
    grid[pos.x][pos.y][pos.z] = block;
}

bool SvMap::inval(glm::ivec3 pos) const {
    if(pos.x < 0 || pos.y < 0 || pos.z < 0) return true;
    if(pos.x >= extent.x || pos.y >= extent.y || pos.z >= extent.z) return true;
    return false;
}

glm::ivec3 SvMap::randSurfacePos() const {
    while(true){
        const auto x = (int)(MathUtil::rand_0_1() * extent[0]);
        const auto z = (int)(MathUtil::rand_0_1() * extent[2]);

        const glm::ivec3 xzPos = {x,0, z};
        if(inval(xzPos)) continue;
        const glm::ivec3 pos = surfaceAt(xzPos);
        if(pos != IVEC3_INVALID_POS && get(pos) != globals::ModelType::water){
            return pos;
        }
    }
}

glm::ivec3 SvMap::randSurfacePos(glm::ivec3 center, float radius) const {
    while(true){
        const auto d_x = (int)(MathUtil::rand() * radius);
        const auto d_z = (int)(MathUtil::rand() * radius);

        if(glm::length(glm::vec2{d_x, d_z}) > radius) continue;

        const glm::ivec3 xzPos = {center.x + d_x,0, center.z + d_z};
        if(inval(xzPos)) continue;
        const glm::ivec3 pos = surfaceAt(xzPos);
        if(pos != IVEC3_INVALID_POS && get(pos) != globals::ModelType::water){
            return pos;
        }
    }
}

glm::ivec3 SvMap::surfaceAt(glm::ivec3 pos) const {
    return surfaceAt(pos, extent.y-1);
}
glm::ivec3 SvMap::surfaceAt(glm::ivec3 pos, int maxHeight) const {
    // start at max height and check each block until not air

    maxHeight = glm::min(maxHeight, extent[1] - 1);

    for(int y = maxHeight; y>=0; y--){
        const glm::ivec3 test_pos = {pos[0], y, pos[2]};
        if(get(test_pos) != globals::ModelType::air){
            return test_pos;
        }
    }
    return IVEC3_INVALID_POS;
}

bool SvMap::posTraversable(glm::vec3 pos, float height) const {
    const glm::ivec3 bottomBlock = glm::floor(pos);
    const int heightInBlocks = (int) glm::ceil(pos.y + height) - bottomBlock.y;

    return blockTraversable(bottomBlock, heightInBlocks);
}

bool SvMap::blockTraversable(glm::ivec3 bottom, int heightInBlocks) const {
    if(inval(bottom) || inval({bottom.x, bottom.y + heightInBlocks-1, bottom.z})) return false;

    for(int i = 0; i<heightInBlocks; i++){
        const auto type = get({bottom.x, bottom.y + i, bottom.z});
        if(type != globals::ModelType::air) {
            return false;
        }
    }
    return true;
}
