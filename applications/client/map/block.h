#pragma once

#include <string>
#include "../model/model.h"
#include "../point_light.h"


class Block : public Model {
public:
    using Base = Model;

    Block(globals::ModelType type, glm::ivec3 pos);

    static std::shared_ptr<Block> newBlock(globals::ModelType type, glm::ivec3 pos);
};

class DirtBlock : public Block {
public:
    using Base = Block;

    explicit DirtBlock(glm::ivec3 pos);
};

class GrassBlock : public Block {
public:
    using Base = Block;

    explicit GrassBlock(glm::ivec3 pos);
};

class LeafBlock : public Block {
public:
    using Base = Block;

    explicit LeafBlock(glm::ivec3 pos);
};

class StoneBlock : public Block {
public:
    using Base = Block;

    explicit StoneBlock(glm::ivec3 pos);
};

class WaterBlock : public Block {
public:
    using Base = Block;

    explicit WaterBlock(glm::ivec3 pos);
};

class WoodBlock : public Block {
public:
    using Base = Block;

    explicit WoodBlock(glm::ivec3 pos);
};

class TorchBlock : public Block {
public:
    using Base = Block;

    ~TorchBlock();

    explicit TorchBlock(glm::ivec3 pos);

    std::shared_ptr<PointLight> point_light;
};

