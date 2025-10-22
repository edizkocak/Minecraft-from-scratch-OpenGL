#pragma once
#include <cstdint>
#include <vector>
#include <memory>

#include "chunk.h"

class Map 
{
public:
    using ChunkPtr = std::shared_ptr<Chunk>;
    using BlockPtr = std::shared_ptr<Block>;
    using Grid = std::vector<std::vector<std::vector<ChunkPtr>>>;

    /**
     * @param size in Chunks
     */
    explicit Map(glm::ivec3 size = {3,1,3});

    Map(const Map &) = delete;
    Map & operator = (const Map &) = delete;

protected:
    void init();

public:
//    glm::ivec3 random_surface_pos();

    /**
     * @return Coordinates of highest Block at pos.xz
     * <br>
     * May be IVEC3_INVALID_POS if there is only Air!
     */
    glm::ivec3 surface_at(glm::ivec3 pos);

    glm::ivec3 surface_at(glm::ivec3 pos, int max_height);

    /**
     * @return True if pos is not a valid block position
     */
    [[nodiscard]] bool inval(glm::ivec3 pos) const;

    /**
     * Alias for Map::get(pos).
     */
    BlockPtr operator ()(glm::ivec3 pos) {
        return get(pos);
    }

    /**
     * Tile containing block at pos.
     * <br>
     * If pos is invalid, then nullptr is returned.
     */
    BlockPtr get(glm::ivec3 pos);

    void set(glm::ivec3 pos, const BlockPtr &block);

    /**
     * Get Chunk by it's index.
     */
    ChunkPtr get_chunk(glm::ivec3 chunkIdx);

    void draw(Shader &shader);

    /**
     * Call this once after all (initial) blocks have been set.
     */
    void updateInitialBlockVisibility();

protected:
    void updateVisibility(glm::ivec3 pos);

public:
    /**
     * Size of map in blocks.
     */
    const glm::ivec3 extent;

protected:
    Grid chunks;

    /**
     * Size of map in chunks.
     */
    glm::ivec3 size;
};

extern std::shared_ptr<Map> the_map; 
