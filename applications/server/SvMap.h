#ifndef GRAPA_SVMAP_H
#define GRAPA_SVMAP_H

#include <unordered_map>
#include <cppgl.h>
#include "../client/model/model.h"

class SvMap {
public:
    explicit SvMap(glm::ivec3 size);

    static SvMap newFlatMap();
    static SvMap newHeightmap();

    [[nodiscard]] globals::ModelType get(glm::ivec3 pos) const;

    void set(glm::ivec3 pos, const globals::ModelType &block);

    /**
     * @return True if pos is not a valid block position
     */
    [[nodiscard]] bool inval(glm::ivec3 pos) const;

    /**
     * @Precond: At least one non-water block at surface.
´    */
    [[nodiscard]] glm::ivec3 randSurfacePos() const;
    /**
     * @Precond: At least one non-water block inside xz-circle around center.xz with given radius.
     */
    [[nodiscard]] glm::ivec3 randSurfacePos(glm::ivec3 center, float radius) const;

    /**
     *
     * @Precond: pos.xz inside map extent
     * @return Coordinates of highest Block at pos.xz
     * <br>
     * May be IVEC3_INVALID_POS if there is only Air!
     */
    [[nodiscard]] glm::ivec3 surfaceAt(glm::ivec3 pos) const;
    /**
     * @Precond: pos.xz inside map extent
     */
    [[nodiscard]] glm::ivec3 surfaceAt(glm::ivec3 pos, int maxHeight) const;

    /**
     * True if all blocks in range block at bottomPos and block at bottomPos+(0,height,0) are air.
     */
    [[nodiscard]] bool posTraversable(glm::vec3 bottomPos, float height) const;
    /**
     * True if all `heightInBlocks` blocks starting at block `bottom` are air.
     */
    [[nodiscard]] bool blockTraversable(glm::ivec3 bottom, int heightInBlocks) const;

    /**
     * Size of map in chunks.
     */
    glm::ivec3 size;
    /**
     * Size of map in blocks.
     */
    const glm::ivec3 extent;

protected:
    std::vector<std::vector<std::vector<globals::ModelType>>> grid;
};

#endif //GRAPA_SVMAP_H
