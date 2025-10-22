#include "player.h"
#include "enemy.h"
#include "movable_model.h"
#include "../input/InputManager.h"
#include "../gui/Gui.h"

#include "cppgl/src/cppgl.h"
#include <glm/gtx/rotate_vector.hpp>
#include "globals.h"
#include "../map/map.h"
#include "../rendering/buffer.h"
#include "../particles.h"
#include "../networking/cl_msg.h"
#include "../../common/networking/MathUtil.h"
#include "../networking/ambassador.h"
#include "screen_model.h"

namespace n =  networking;

extern std::shared_ptr<InputBindings> the_bindings;
extern std::shared_ptr<Map> the_map;

extern std::shared_ptr<Arm> the_arm;
extern std::shared_ptr<Sword> the_sword;
extern std::shared_ptr<Axe> the_axe;

extern shared_ptr<Ambassador> the_ambassador;
extern std::shared_ptr<Particles> particles, particles_small;
extern int PARTICLE_EMITTER_TIMESLICE;

namespace {
    bool enemy_hit = false;
    float HIT_REDNESS_TIME = 1500.f;   // time in which being hit is visible with redness
}

Player::Player(int id, glm::vec3 pos, int gameTimeOffset)
        : Base(id, globals::ModelType::player,  pos + glm::vec3(0.5f, 0, 0.5f))
{
    gameTimer.begin(gameTimeOffset);
    player_cam = Camera("playercam");
    player_cam->far = 1.733 * globals::FARPLANE; // Würfeldiagonale

    for(const auto type:INVENTORY_LIST){
        inventory[type] = 0;
    }

    // TODO default inventory
    inventory[globals::ModelType::wood] = 17;
    inventory[globals::ModelType::dirt] = 0;
    inventory[globals::ModelType::grass] = 0;
    inventory[globals::ModelType::leaf] = 17;
    inventory[globals::ModelType::stone] = 17;
    inventory[globals::ModelType::torch] = 7;
    //
    inventory[globals::ModelType::sword] = 1;
    inventory[globals::ModelType::axe] = 1;

    
   player_sword = make_shared<Item>(nextId, pos, globals::ModelType::sword);
   MovableModel::movable_models[nextId] = player_sword;
   nextId++;
   player_axe = make_shared<Item>(nextId, pos, globals::ModelType::axe);
   MovableModel::movable_models[nextId] = player_axe;
   nextId++;
}

void Player::draw_player_gui() {
    if (ImGui::CollapsingHeader("Player")){
        if(ImGui::Button("enemy hit")){
            enemy_hit = true;
        }
        ImGui::SliderFloat("redness time", &HIT_REDNESS_TIME, 500.f, 2500.f);
    }
}


void Player::handleInput(double dt_ms) {
    static const float GRAVITY = -0.00003f;

    static const float BASE_MAX_MOVE_SPEED = 0.005f;
    static float MAX_MOVE_SPEED = BASE_MAX_MOVE_SPEED;
    static const float SPEED_CHANGE = MAX_MOVE_SPEED * 0.1f;

    static const float JUMP_SPEED = 0.01f;
    static const float FORCE_BACK_EPSILON = 0.015;

    static const float MAX_SPRINT_FOV = 85.f;
    static const float FOV_CHANGE = (0.5f * BASE_MAX_MOVE_SPEED) / SPEED_CHANGE;

    static const float MINING_TIME = 750.f;
    // TODO:different mining times depending on block type

    //static const float HIT_REDNESS_TIME = 1000.f;   // time in which being hit is visible with redness

    if (Gui::INGAME_MENU != Gui::getCurrentMenu()) return;

    static auto &im = InputManager::instance();

    if(im.hasMoved(the_bindings->scroll)){
        changeSelectedItem((int) im.getMovement(the_bindings->scroll).y);
    }

    if(im.hasMoved(the_bindings->mouse)){
        static const float MOUSE_SPEED = 0.09f;
        glm::vec2 delta = im.getMovement(the_bindings->mouse) * (MOUSE_SPEED * float(M_PI)  / 180.f);

        yaw(delta.x);
        pitch(delta.y * globals::mouseYFactor);
    }

    if(freezed){
        return;
    }

    // player is being hit by enemy
    if(enemy_hit){
        enemy_hit = false;
        if(!player_hit){
            player_hit = true;
        }
        hit_redness_timer.begin();
    }

    if(player_hit){
        auto dt = float(hit_redness_timer.look());
        if(dt >= HIT_REDNESS_TIME){
            player_hit = false;
            redness_factor = 0.f;
        }
        else{
            redness_factor = 1.f - (dt / HIT_REDNESS_TIME);
        }
    }


    if(selectedItem == globals::ModelType::axe){
        screen_item = the_axe;
    }
    else if (selectedItem == globals::ModelType::sword){
        screen_item = the_sword;
    }
    else{
        screen_item = the_arm;
    }


    // mining and placing of blocks
    glm::ivec2 res = Context::resolution();
    rendering::buffer::geom->bind();

    // get position of the block the player is looking at
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    GLfloat pos[3];
    glReadPixels(res.x / 2, res.y / 2, 1, 1, GL_RGB, GL_FLOAT, pos);

    //getting normal of block to know which side the new block should be placed at
    glReadBuffer(GL_COLOR_ATTACHMENT2);
    float norm[3];
    glReadPixels(res.x / 2, res.y / 2, 1, 1, GL_RGB, GL_FLOAT, norm);

    //getting normal of block to know which side the new block should be placed at
    glReadBuffer(GL_COLOR_ATTACHMENT3);
    float pixel_id;
    glReadPixels(res.x / 2, res.y / 2, 1, 1, GL_RED, GL_FLOAT, &pixel_id);

    rendering::buffer::geom->unbind();

    int enemy_id = int(pixel_id);

    // this is the block the player is currently looking at
    glm::vec3 block_f = { pos[0], pos[1], pos[2] };

    // normal of the block
    glm::vec3 norm_f = {norm[0], norm[1], norm[2]};

    // without this, there are some rounding errors
    block_f += -1.0f * FORCE_BACK_EPSILON * norm_f;
    norm_f += FORCE_BACK_EPSILON * norm_f;

    glm::ivec3 block_pos = { int(block_f.x), int(block_f.y), int(block_f.z)};
    glm::ivec3 block_norm = { int(norm_f[0]), int(norm_f[1]), int(norm_f[2]) };

    Map::BlockPtr block = the_map->get(block_pos);

    const float max_block_dist = 6.0;
    float dist = glm::distance( glm::vec3(block_pos[0], block_pos[1], block_pos[2]), current_camera()->pos);

    //check if enemy is looking at a block within a certain distance
    bool block_in_range = dist < max_block_dist;

    // highlight the block the player is looking at
    // note: this variable will be set to false in the draw call after drawing
    if(block && block_in_range && enemy_id == -1) {
        block->highlighted = true;
    }


    // set current digging animation
    if(im.isActive(the_bindings->act1)){
        if(!screen_item->cooldown_active || screen_item->cooldown_timer.look() >= screen_item->animation_cooldown) {
            screen_item->animation_active = true;
            screen_item->hit_timer.begin();
        }
    }

    mining_progress = 0.f;

    if(enemy_id > 0 ) {
        mining = false;

        // get animated model the player is looking at
        std::shared_ptr<MovableModel> animated_model = MovableModel::movable_models.at(enemy_id);

        // check if player is looking at enemy
        if(animated_model && (   (  (animated_model->type == globals::ModelType::zombie || animated_model->type == globals::ModelType::skeleton)  ) && std::dynamic_pointer_cast<Enemy> (animated_model)->enemy_is_hit == false   ) ){
            float dist_to_player = glm::distance(_pos, animated_model->getPos());
            if(screen_item->hit_timer.look() >= screen_item->hit_time * 0.6f && screen_item->hit_timer.look() <= screen_item->hit_time * 1.4f && dist_to_player <= screen_item->radius) {
                // hit the enemy
                std::dynamic_pointer_cast<Enemy>(animated_model)->enemy_is_hit = true;
                std::dynamic_pointer_cast<Enemy>(animated_model)->hit_redness_timer.begin();

                animated_model->hp -= screen_item->damage;
                msg::client::SvUpdateHp(animated_model->id, -20.f);

                //TODO: damage depending on player weapon
            }

        }

    }
    // no blocks from the boundary should be removed
    else if(block_pos.y != 0 && block_in_range && im.isActive(the_bindings->act1)){
        if(block){
            // mining of the block

            if(!mining || block_pos != mined_block_pos) {
                mining = true;
                mined_block_pos = glm::ivec3(block_pos.x, block_pos.y, block_pos.z);
                mining_timer.begin();


            }
            else{
                if(mining_timer.look() >= MINING_TIME){
                    msg::client::SvDig(block->type, block_pos);
                    the_map->set(block_pos, nullptr);
                    addToInventory(block->type, 1);
                    mining = false;
                }
                else{
                    mining_progress = float(mining_timer.look()) / MINING_TIME;


                    // add particles
                    if (particle_timer.look() >= PARTICLE_EMITTER_TIMESLICE) {

                        rendering::buffer::geom->bind();
                        // get color of block that is currently being mined
                        glReadBuffer(GL_COLOR_ATTACHMENT0);
                        float col[3];
                        glReadPixels(res.x / 2, res.y / 2, 1, 1, GL_RGB, GL_FLOAT, col);
                        rendering::buffer::geom->unbind();

                        glm::vec4 block_col = { col[0], col[1], col[2], 1.0 };


                        particle_timer.begin();
                        glm::vec3 dir = glm::vec3(block_norm.x, block_norm.y, block_norm.z);

                        dir.x = dir.x != 0 ? (dir.x) + 0.8f * float(MathUtil::rand())  : (float)MathUtil::rand();
                        dir.y = dir.y != 0 ? (dir.y) + 0.8f * float(MathUtil::rand())  : (float)MathUtil::rand();
                        dir.z = dir.z != 0 ? (dir.z) + 0.8f * float(MathUtil::rand())  : (float)MathUtil::rand();
                        dir = glm::normalize(dir);
                        dir *= 0.7;

                        for (int i = 0; i < 1; ++i) {
                            particles_small->add(block_f, dir, block_col, (rand() % 1000) + 300);
                        }
                    }
                }
            }
        }
    }
    else if(!im.isActive(the_bindings->act1)){
        mining = false;
    }

    const glm::ivec3 currentBlockPos = glm::floor(_pos);

    if(block_in_range && im.isActive(the_bindings->act2)){
        Map::BlockPtr block = the_map->get(block_pos);
        if(block){
            glm::ivec3 place_block_pos = block_pos + block_norm;

            // check if there is already a block on that position
            // also makes sure that no block is placed inside the player
            if(currentBlockPos != place_block_pos && currentBlockPos + IVEC_UP != place_block_pos && !the_map->get(place_block_pos)){
                // place block
                if(inventory[selectedItem] >= 1){
                    msg::client::SvPlace(selectedItem, place_block_pos);
                    the_map->set(place_block_pos, Block::newBlock(selectedItem, place_block_pos));
                    rmFromInventory(selectedItem, -1);
                }
            }
        }
    }

    if(im.isActive(the_bindings->toggle_camera)) {
        if(playerView == PlayerView::FIRST_PERSON){
            playerView = PlayerView::THIRD_PERSON;
        }else if(playerView == PlayerView::THIRD_PERSON){
            playerView = PlayerView::FRONT_VIEW;
        }else if(playerView == PlayerView::FRONT_VIEW){
            playerView = PlayerView::FIRST_PERSON;
        }
    }

    // running
    if(im.isActive(the_bindings->run)){
        MAX_MOVE_SPEED = 1.3f * BASE_MAX_MOVE_SPEED;
        if(current_camera()->fov_degree < MAX_SPRINT_FOV && (im.isActive(the_bindings->forward) && !im.isActive(the_bindings->backward))){
            current_camera()->fov_degree += 0.5f * FOV_CHANGE;
        }

    }
    else{
        MAX_MOVE_SPEED = BASE_MAX_MOVE_SPEED;

        current_camera()->fov_degree -= 0.5f * FOV_CHANGE;
        current_camera()->fov_degree = glm::clamp(current_camera()->fov_degree, 70.f, MAX_SPRINT_FOV);

    }

    if(im.isActive(the_bindings->forward)){
        velocity += glm::vec3(SPEED_CHANGE, 0, 0);
        velocity[0] = glm::min(MAX_MOVE_SPEED,velocity[0]);
    }
    if(im.isActive(the_bindings->backward)){
        velocity += glm::vec3(-SPEED_CHANGE, 0, 0);
        velocity[0] = glm::max(-MAX_MOVE_SPEED,velocity[0]);
    }
    if(!im.isActive(the_bindings->forward) && !im.isActive(the_bindings->backward) && velocity[0] != 0){
        // slow down

        if(velocity[0]>0){
            velocity += glm::vec3(-SPEED_CHANGE, 0, 0);
            velocity[0] = glm::max(0.f,velocity[0]);
        }else {
            velocity += glm::vec3(SPEED_CHANGE, 0, 0);
            velocity[0] = glm::min(0.f,velocity[0]);
        }
    }

    if(im.isActive(the_bindings->left)){
        velocity += glm::vec3(0, 0, SPEED_CHANGE);
        velocity[2] = glm::min(-MAX_MOVE_SPEED,velocity[2]);
    }
    if(im.isActive(the_bindings->right)){
        velocity += glm::vec3(0,0,-SPEED_CHANGE);
        velocity[2] = glm::max(MAX_MOVE_SPEED,velocity[2]);
    }

    if(!im.isActive(the_bindings->right) && !im.isActive(the_bindings->left) && velocity[2] != 0){
        // slow down

        if(velocity[2]>0){
            velocity += glm::vec3(0 ,0,-SPEED_CHANGE);
            velocity[2] = glm::max(0.f,velocity[2]);
        }else if (velocity[2] < 0){
            velocity += glm::vec3(0, 0, SPEED_CHANGE);
            velocity[2] = glm::min(0.f,velocity[2]);
        }
    }

    // height of the first block, that is underneath the player
    glm::ivec3 block_below_player = the_map->surface_at(currentBlockPos, currentBlockPos.y -1);
    if(block_below_player == IVEC3_INVALID_POS){
        throw std::runtime_error("invalid state");
    }
    int min_player_height = block_below_player[1] + 1;

    bool in_air = _pos[1] > (float) min_player_height;

    static bool jump_sound = false;


    if(!in_air){
        if(im.isActive(the_bindings->jump) && !jump_sound) {
            velocity[1] = JUMP_SPEED;
            jump_sound = true;
        }
        else if(jump_sound){
            // landing
            jump_sound = false;
            //TODO: add sounds when jumping
        }
    }




    if(in_air){
        velocity[1] += GRAVITY * float(dt_ms);
    }

    static glm::vec2 last_pos(-1);

    glm::vec3 rotatedVelocity = glm::rotate(velocity,getRot()[0],glm::vec3(0,1,0));
    glm::vec3 step = rotatedVelocity * float(dt_ms);

    // move player by step
    // limit its position to boundaries of map -> clamp

    float old_y = _pos[1];

    _pos += step;

    if(in_air){
        if(_pos[1] < float(min_player_height)){
            // below ground

            _pos[1] = float(min_player_height);
            velocity[1] = 0;
        }
    }

    using m = globals::ModelType;
    using a = globals::AnimationType;

    // change animation of player
    if( (velocity[0] != 0 || velocity[2] != 0) && velocity[1] == 0){
        if(im.isActive(the_bindings->act1)){
            if(aType == a::WALKING) {
                if (selectedItem != m::sword && selectedItem != m::axe) {
                    changeAnimation(globals::AnimationType::WALKING_AND_DIGGING, 0);
                } else {
                    changeAnimation(globals::AnimationType::WALKING_AND_HITTING, 0);
                    player_sword->changeAnimation(globals::AnimationType::HITTING, 0);
                    player_axe->changeAnimation(globals::AnimationType::HITTING, 0);
                }
            }
            else{
                if(selectedItem != m::sword &&  selectedItem != m::axe){
                    if(aType != a::WALKING_AND_DIGGING){
                        changeAnimation(globals::AnimationType::WALKING_AND_DIGGING, 0);
                    }
                }
                else{
                    if(aType != a::WALKING_AND_HITTING){
                        changeAnimation(globals::AnimationType::WALKING_AND_HITTING, 0);
                        player_sword->changeAnimation(globals::AnimationType::HITTING, 0);
                        player_axe->changeAnimation(globals::AnimationType::HITTING, 0);
                    }
                }
            }
        }
        else{
            if(aType != a::WALKING){
                changeAnimation(globals::AnimationType::WALKING_AND_HITTING, 0);
                player_sword->changeAnimation(globals::AnimationType::HITTING, 0);
                player_axe->changeAnimation(globals::AnimationType::HITTING, 0);
            }
        }
    }
    else {
        if (im.isActive(the_bindings->act1)) {
            if (selectedItem != m::sword && selectedItem != m::axe) {
                if (aType != a::DIGGING) {
                    changeAnimation(globals::AnimationType::DIGGING, 0);
                }
            } else {
                if (aType != a::HITTING) {
                        changeAnimation(globals::AnimationType::HITTING, 0);
                        player_sword->changeAnimation(globals::AnimationType::HITTING, 0);
                        player_axe->changeAnimation(globals::AnimationType::HITTING, 0);
                }
            }
        } else {
            if (aType != a::STANDING) {
                    changeAnimation(globals::AnimationType::STANDING, 0);
                    player_sword->changeAnimation(globals::AnimationType::STANDING, 0);
                    player_axe->changeAnimation(globals::AnimationType::STANDING, 0);
            }
        }
    }


    static const glm::vec3 maxBlockPos = the_map->extent - glm::ivec3(1);
    _pos = glm::clamp(_pos, glm::vec3(0), maxBlockPos);
    glm::ivec3 newBlockPos = glm::floor(_pos);

    bool block_forward  = (currentBlockPos.x >= 0 && currentBlockPos.x < maxBlockPos.x) ? the_map->get(currentBlockPos + IVEC_FORWARD) || the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_UP) : false;
    bool block_backward = (currentBlockPos.x > 0 && currentBlockPos.x <= maxBlockPos.x) ? the_map->get(currentBlockPos + IVEC_BACKWARD) || the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_UP) : false;
    bool block_right    = (currentBlockPos.z >= 0 && currentBlockPos.z < maxBlockPos.z) ? the_map->get(currentBlockPos + IVEC_RIGHT) || the_map->get(currentBlockPos + IVEC_RIGHT + IVEC_UP) : false;
    bool block_left     = (currentBlockPos.z > 0 && currentBlockPos.z <= maxBlockPos.z) ? the_map->get(currentBlockPos + IVEC_LEFT) || the_map->get(currentBlockPos + IVEC_LEFT + IVEC_UP) : false;
    bool block_above    = (currentBlockPos.y >= 0 && currentBlockPos.y < maxBlockPos.y) ? the_map->get(currentBlockPos + IVEC_UP + IVEC_UP)!=0 : false;
    bool block_below    = (currentBlockPos.y > 0 && currentBlockPos.y <= maxBlockPos.y) ? the_map->get(currentBlockPos + IVEC_DOWN)!=0 : false;

    if(newBlockPos != currentBlockPos){
        // Player moved from one block to another
        // Check if player can walk in that direction

        if(newBlockPos.z < currentBlockPos.z){
            // left
            if(block_left){
                float z_forced = ceil(_pos[2]);
                z_forced += FORCE_BACK_EPSILON;
                _pos[2] = z_forced;
                velocity[2] = 0;
            }
        }

        if(newBlockPos.z > currentBlockPos.z){
            // right
            if(block_right){
                float z_forced = floor(_pos[2]);
                z_forced -= FORCE_BACK_EPSILON;
                _pos[2] = z_forced;
                velocity[2] = 0;
            }
        }

        if(newBlockPos.x > currentBlockPos.x) {
            // one block forward

            if (block_forward) {
                // block is in the way:
                // force player to the edge of the block
                float x_forced = floor(_pos[0]);
                x_forced -= FORCE_BACK_EPSILON;
                _pos[0] = x_forced;
            }

            if(newBlockPos.z < currentBlockPos.z) {
                //forward-left

                if (the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_LEFT) ||
                    the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_LEFT + IVEC_UP)) {

                    if(!block_forward && !block_left) {
                        float z_forced = ceil(_pos[2]);
                        z_forced += FORCE_BACK_EPSILON;
                        _pos[2] = z_forced;
                    }

                }
            }

            if (newBlockPos.z > currentBlockPos.z) {
                //forward-right
                if ( the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_RIGHT) ||
                     the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_RIGHT + IVEC_UP) ) {

                    if(!block_forward && !block_right) {
                        float z_forced = floor(_pos[2]);
                        z_forced -= FORCE_BACK_EPSILON;
                        _pos[2] = z_forced;
                    }
                }
            }
        }

        if(newBlockPos.x < currentBlockPos.x){
            // one block backward
            if(block_backward){
                float x_forced = ceil(_pos[0]);
                x_forced += FORCE_BACK_EPSILON;
                _pos[0] = x_forced;
            }

            if(newBlockPos.z < currentBlockPos.z) {
                // backwards-left
                if (the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_LEFT) ||
                    the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_LEFT + IVEC_UP)) {
                    if(!block_backward && !block_left) {
                        float z_forced = ceil(_pos[2]);
                        z_forced += FORCE_BACK_EPSILON;
                        _pos[2] = z_forced;
                    }
                }
            }

            if(newBlockPos.z > currentBlockPos.z){
                // backwards-right
                if(the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_RIGHT) ||
                   the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_RIGHT + IVEC_UP)){
                    if(!block_backward && !block_right) {
                        float z_forced = floor(_pos[2]);
                        z_forced -= FORCE_BACK_EPSILON;
                        _pos[2] = z_forced;
                    }
                }
            }
        }

        if(newBlockPos.y > currentBlockPos.y){
            bool above_left = false;
            bool above_right = false;
            bool above_forward = false;
            bool above_backward = false;

            // one block above
            if(block_above){
                // block is in the way:
                // force player to the edge of the block
                float y_forced = old_y;
                _pos[1] = y_forced;

                velocity.y = 0.f;
            }

            if(newBlockPos.z < currentBlockPos.z){
                // above-left
                if(the_map->get(currentBlockPos + IVEC_LEFT + IVEC_UP + IVEC_UP)){
                    if(!block_left) {
                        above_left = true;
                        float z_forced = ceil(_pos[2]);
                        z_forced += FORCE_BACK_EPSILON;
                        _pos[2] = z_forced;
                    }
                }
            }
            if(newBlockPos.z > currentBlockPos.z){
                // above-right
                if(the_map->get(currentBlockPos + IVEC_RIGHT + IVEC_UP + IVEC_UP)){
                    if(!block_right) {
                        above_right = true;
                        float z_forced = floor(_pos[2]);
                        z_forced -= FORCE_BACK_EPSILON;
                        _pos[2] = z_forced;
                    }
                }
            }

            if(newBlockPos.x > currentBlockPos.x){
                //above-forward
                if(the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_UP + IVEC_UP)){
                    if(!block_forward) {
                        above_forward = true;
                        float x_forced = floor(_pos[0]);
                        x_forced -= FORCE_BACK_EPSILON;
                        _pos[0] = x_forced;
                    }
                }

                if(newBlockPos.z < currentBlockPos.z) {
                    if (the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_UP + IVEC_UP + IVEC_LEFT)) {
                        // forward-above-left
                        if (!block_forward && !block_left && !above_forward && !above_left) {
                            float x_forced = floor(_pos[0]);
                            x_forced -= FORCE_BACK_EPSILON;
                            _pos[0] = x_forced;

                            float z_forced = ceil(_pos[2]);
                            z_forced += FORCE_BACK_EPSILON;
                            _pos[2] = z_forced;
                        }

                    }
                }

                if(newBlockPos.z > currentBlockPos.z) {
                    if (the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_UP + IVEC_UP + IVEC_RIGHT)) {
                        // forward-above-right
                        if (!block_forward && !block_right && !above_forward && !above_right) {
                            float x_forced = floor(_pos[0]);
                            x_forced -= FORCE_BACK_EPSILON;
                            _pos[0] = x_forced;

                            float z_forced = floor(_pos[2]);
                            z_forced -= FORCE_BACK_EPSILON;
                            _pos[2] = z_forced;
                        }

                    }
                }

            }
            if(newBlockPos.x < currentBlockPos.x){
                // above-backward
                if(the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_UP + IVEC_UP)){
                    if(!block_backward) {
                        above_backward = true;
                        float x_forced = ceil(_pos[0]);
                        x_forced += FORCE_BACK_EPSILON;
                        _pos[0] = x_forced;
                    }
                }

                if(newBlockPos.z < currentBlockPos.z) {
                    if (the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_UP + IVEC_LEFT)) {
                        // backward-above-left
                        if (!block_backward && !block_left && !above_backward && !above_left) {
                            float x_forced = ceil(_pos[0]);
                            x_forced += FORCE_BACK_EPSILON;
                            _pos[0] = x_forced;

                            float z_forced = ceil(_pos[2]);
                            z_forced += FORCE_BACK_EPSILON;
                            _pos[2] = z_forced;
                        }
                    }
                }

                if(newBlockPos.z > currentBlockPos.z) {
                    if (the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_UP + IVEC_RIGHT)) {
                        // backward-above-right
                        if (!block_backward && !block_right && !above_backward && !above_right) {
                            float x_forced = ceil(_pos[0]);
                            x_forced += FORCE_BACK_EPSILON;
                            _pos[0] = x_forced;

                            float z_forced = floor(_pos[2]);
                            z_forced -= FORCE_BACK_EPSILON;
                            _pos[2] = z_forced;
                        }

                    }
                }
            }

        }

        if(newBlockPos.y < currentBlockPos.y){
            bool below_left = false;
            bool below_right = false;
            bool below_forward = false;
            bool below_backward = false;

            if(block_below){
                // block is in the way:
                // force player to the edge of the block
                float y_forced = ceil(_pos[1]);
                _pos[1] = y_forced;
            }

            if(newBlockPos.z < currentBlockPos.z){
                // below-left
                if(the_map->get(currentBlockPos + IVEC_LEFT + IVEC_DOWN)){
                    if(!block_left) {
                        below_left = true;
                        float z_forced = ceil(_pos[2]);
                        z_forced += FORCE_BACK_EPSILON;
                        _pos[2] = z_forced;
                    }
                }
            }
            if(newBlockPos.z > currentBlockPos.z){
                // below-right
                if(the_map->get(currentBlockPos + IVEC_RIGHT + IVEC_DOWN)){

                    if(!block_right) {
                        below_right = true;
                        float z_forced = floor(_pos[2]);
                        z_forced -= FORCE_BACK_EPSILON;
                        _pos[2] = z_forced;
                    }
                }
            }

            if(newBlockPos.x > currentBlockPos.x){
                //below-forward
                if(the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_DOWN)){
                    if(!block_forward) {
                        below_forward = true;
                        float x_forced = floor(_pos[0]);
                        x_forced -= FORCE_BACK_EPSILON;
                        _pos[0] = x_forced;
                    }
                }

                if(newBlockPos.z < currentBlockPos.z) {
                    if (the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_DOWN + IVEC_LEFT)) {
                        // forward-below-left
                        if (!block_forward && !block_left && !below_forward && !below_left) {
                            float x_forced = floor(_pos[0]);
                            x_forced -= FORCE_BACK_EPSILON;
                            _pos[0] = x_forced;

                            float z_forced = ceil(_pos[2]);
                            z_forced += FORCE_BACK_EPSILON;
                            _pos[2] = z_forced;
                        }

                    }
                }

                if(newBlockPos.z > currentBlockPos.z) {
                    if (the_map->get(currentBlockPos + IVEC_FORWARD + IVEC_DOWN + IVEC_RIGHT)) {
                        // forward-below-right
                        if (!block_forward && !block_right && !below_forward && !below_right) {
                            float x_forced = floor(_pos[0]);
                            x_forced -= FORCE_BACK_EPSILON;
                            _pos[0] = x_forced;

                            float z_forced = floor(_pos[2]);
                            z_forced -= FORCE_BACK_EPSILON;
                            _pos[2] = z_forced;
                        }

                    }
                }

            }
            if(newBlockPos.x < currentBlockPos.x){
                // below-backward
                if(the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_DOWN)){
                    if(!block_backward) {
                        below_backward = true;
                        float x_forced = ceil(_pos[0]);
                        x_forced += FORCE_BACK_EPSILON;
                        _pos[0] = x_forced;
                    }
                }

                if(newBlockPos.z < currentBlockPos.z) {
                    if (the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_DOWN + IVEC_LEFT)) {
                        // backward-below-left
                        if (!block_backward && !block_left && !below_backward && !below_left) {
                            float x_forced = ceil(_pos[0]);
                            x_forced += FORCE_BACK_EPSILON;
                            _pos[0] = x_forced;

                            float z_forced = ceil(_pos[2]);
                            z_forced += FORCE_BACK_EPSILON;
                            _pos[2] = z_forced;
                        }
                    }
                }

                if(newBlockPos.z > currentBlockPos.z) {
                    if (the_map->get(currentBlockPos + IVEC_BACKWARD + IVEC_DOWN + IVEC_RIGHT)) {
                        // backward-below-right
                        if (!block_backward && !block_right && !below_backward && !below_right) {
                            float x_forced = ceil(_pos[0]);
                            x_forced += FORCE_BACK_EPSILON;
                            _pos[0] = x_forced;

                            float z_forced = floor(_pos[2]);
                            z_forced -= FORCE_BACK_EPSILON;
                            _pos[2] = z_forced;
                        }

                    }
                }

            }
        }
    }

    const auto animationOffset = float(aTimer.look());
    msg::client::SvUpdatePlayer(_pos, getRot(), globals::AnimationType::WALKING, int(animationOffset));
}

void Player::update() {
    doUpdatePlayer();

}

void Player::doUpdatePlayer() {
    const auto& rot = getRot();
    setRot({rot.x, 0});

    // update animations
    Base::update();

    setRot(rot);


    if(playerView == PlayerView::FIRST_PERSON){
        update1stPerson();
    }else if(playerView == PlayerView::THIRD_PERSON){
        update3rdPerson();
    }else if(playerView == PlayerView::FRONT_VIEW){
        updateFrontView();
    }
}

void Player::update3rdPerson() {
    const glm::vec3 headOffset = glm::vec3(0.f,1.7f,0.f);
    const glm::vec3 headPos = _pos + headOffset;

    player_cam->pos = headPos - 5.f * getDir();
    player_cam->dir = getDir();
    player_cam->update();
}

void Player::update1stPerson() {
    const glm::vec3 headOffset = glm::vec3(0.f,1.7f,0.f);
    const glm::vec3 headPos = _pos + headOffset;

    player_cam->pos = headPos;
    player_cam->dir = getDir();
    player_cam->update();
}

void Player::updateFrontView(){
    const glm::vec3 headOffset = glm::vec3(0.f,1.7f,0.f);
    const glm::vec3 headPos = _pos + headOffset;

    player_cam->pos = headPos + 5.f * getDir();
    player_cam->dir = -getDir();
    player_cam->update();
}

void Player::changeSelectedItem(int change) {
    if(change == 0) {
        // no change
        return;
    }

// UNCOMMENT FOR AUTOMATIC ITEM SWITCH
//    if(nonEmptyInventoryItems().size() <= 1) {
//        // can't change to other item type as all others are empty
//        return;
//    }
    if(nonEmptyInventoryItems().empty()) {
        // can't change to other item type as all are empty
        return;
    }

    if(change>0){
        selectedItemIdx = nextItemIdx(selectedItemIdx, change);
    }else if(change<0){
        change *= -1;
        selectedItemIdx = prevItemIdx(selectedItemIdx, change);
    }
    selectedItem = INVENTORY_LIST[selectedItemIdx];
}

int Player::nextItemIdx(int idx, int times) const {
    for(int i = 0; i<times; i++){
        while(true){
            idx++;
            idx %= int(INVENTORY_LIST.size());
            if(inventory.at(INVENTORY_LIST.at(idx)) > 0) break;
        }
    }
    return idx;
}

int Player::prevItemIdx(int idx, int times) const {
    for(int i = 0; i<times; i++){
        while(true){
            idx--;
            if(idx<0) idx += int(INVENTORY_LIST.size());
            if(inventory.at(INVENTORY_LIST.at(idx)) > 0) break;
        }
    }
    return idx;
}

void Player::updateInventory(globals::ModelType block, int change) {
    if(change>0){
        addToInventory(block, change);
    }else if(change<0){
        rmFromInventory(block, change);
    }
}

void Player::rmFromInventory(globals::ModelType block, int change) {
    inventory[block] += change;
    if(inventory[block] < 0){
        throw runtime_error("invalid inventory state");
    }

// UNCOMMENT FOR AUTOMATIC ITEM SWITCH
//    if(selectedItem == block && inventory[block] == 0){
//        // no more of block type in inventory
//        // change to next block type
//
//        if(!nonEmptyInventoryItems().empty()) {
//            selectedItemIdx = nextItemIdx(selectedItemIdx, 1);
//            selectedItem = INVENTORY_LIST[selectedItemIdx];
//        }
//    }
}

void Player::addToInventory(globals::ModelType block, int change) {
    inventory[block] += change;

    if(selectedItem != block && inventory[selectedItem] == 0){
        selectedItemIdx = nextItemIdx(selectedItemIdx, 1);
        selectedItem = INVENTORY_LIST[selectedItemIdx];
    }
}

std::map<globals::ModelType, int> Player::nonEmptyInventoryItems() const {
    std::map<globals::ModelType, int> map;
    for(auto &[key, value] : inventory){
        if(value > 0) {
            map[key] = value;
        }
    }
    return map;
}

