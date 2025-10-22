#pragma once
#include "animated_model.h"
#include "item.h"
#include "screen_model.h"

enum class PlayerView {
    FIRST_PERSON,
    THIRD_PERSON,
    FRONT_VIEW
};

class Player : public AnimatedModel
{
public:
    using Base = AnimatedModel;

    explicit Player(int id, glm::vec3 pos, int gameTimeOffset);

    void draw_player_gui();

    /**
     * moves the player based on user input
     */
    void handleInput(double dt_ms);

    [[nodiscard]] std::map<globals::ModelType, int> nonEmptyInventoryItems() const;
    void updateInventory(globals::ModelType block, int change);
protected:
    /**
     * Decrease amount of block in inventory.
     * <br>
     * Change selectedItem if new amount == 0.
     */
    void rmFromInventory(globals::ModelType block, int change);
    /**
     * Increase amount of block in inventory.
     * <br>
     * Change selectedItem if amount of selectedItem == 0.
     */
    void addToInventory(globals::ModelType block, int change);

public:
    Camera player_cam;
    PlayerView playerView = PlayerView::FIRST_PERSON;

    std::shared_ptr<Item> player_sword;
    std::shared_ptr<Item> player_axe;

    std::shared_ptr<ScreenModel> screen_item;

    Timer mining_timer;
    bool mining = false;
    glm::ivec3 mined_block_pos = glm::vec3(-1);
    float mining_progress = 0.f;  // in range [0,1]

    bool player_hit = false;
    float redness_factor = 0.f;

    bool freezed = false;


    /**
     * Map of items and their count.
     */
    std::map<globals::ModelType, int> inventory;
    /**
     * Currently selected item.
     */
    globals::ModelType selectedItem = globals::ModelType::water;
    /**
     * Index of currently selected item.
     */
    int selectedItemIdx = 0;
protected:
    inline static std::vector<globals::ModelType> INVENTORY_LIST{
            globals::ModelType::water,
            globals::ModelType::wood,
            globals::ModelType::dirt,
            globals::ModelType::grass,
            globals::ModelType::leaf,
            globals::ModelType::stone,
            globals::ModelType::torch,
            //
            globals::ModelType::sword,
            globals::ModelType::axe
    };

public:
    Timer particle_timer;
    Timer gameTimer;
    Timer hit_redness_timer;

protected:
    void update() override;
    void doUpdatePlayer();


    void update3rdPerson();
    void update1stPerson();
    void updateFrontView();

    /**
     * Changes selected item.
     */
    void changeSelectedItem(int change);
    [[nodiscard]] int nextItemIdx(int idx, int times) const;
    [[nodiscard]] int prevItemIdx(int idx, int times) const;
};
