#include <string>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <thread>

#include <cppgl.h>
#include <nlohmann/json.hpp>
#include <boost/program_options.hpp>
#include <irrKlang.h>

#include "rendering/buffer.h"
#include "rendering/pass.h"
#include "rendering/util.h"

#include "map/map.h"
#include "model/movable_model.h"
#include "model/player.h"
#include "globals.h"
#include "gui/Gui.h"

#include "audio/SoundManager.h"
#include "audio/test/SoundTest.h"
#include "audio/test/CircularSoundTest.h"

#include "networking/ambassador.h"
#include "../common/networking/Message.h"
#include "particles.h"
#include "model/enemy.h"
#include "model/cloud.h"
#include "networking/cl_msg.h"
#include "rain.h"
#include "input/InputManager.h"
#include "lightning.h"
#include "model/screen_model.h"

#include "flare.h"
#include "sky.h"
#include "MathUtil.h"

using namespace std;

std::shared_ptr<InputBindings> the_bindings;
std::shared_ptr<Map> the_map;
std::shared_ptr<Sun> the_sun;
std::shared_ptr<FlareHelper> the_flare;
std::shared_ptr<Player> the_player;
std::shared_ptr<Sky> the_sky;

std::shared_ptr<Arm> the_arm;
std::shared_ptr<Sword> the_sword;
std::shared_ptr<Axe> the_axe;

std::shared_ptr<Ambassador> the_ambassador;
std::shared_ptr<Particles> particles_small;
std::shared_ptr<Particles> particles_torch;
std::shared_ptr<Rain> particles_rain;

// TODO move to namespace
float PARTICLE_SIZE = 4.f;
int PARTICLE_EMITTER_TIMESLICE = int(150);
//
float rain_size = 0.003f * 4.0f * PARTICLE_SIZE;
bool rain_on = false;

bool shadow_pass = false;
float xa = 0.416f;
float ya = -0.846f;
float za = -1.276f;

float rx = 3.006f;
float ry = 3.225f;
float rz = 4.295f;

float s = 0.015f;

// anonymous namespace
namespace {
    unsigned int port;
    string address;
    string user;

    void on_resize(int w, int h) {
        rendering::buffer::geom->resize(w, h);
        rendering::buffer::lighting->resize(w, h);
    }


    Camera setup_shadowcam() {
        Camera shadowcam { "shadowcam" };
        shadowcam->pos = glm::vec3 { 3, 34, 16 };
        shadowcam->dir = glm::vec3 { 0, -1, 0 };
        shadowcam->up = glm::vec3 { 0, 0, -1 };
        shadowcam->near = 1.f;
        shadowcam->far = 46.f;
        shadowcam->fov_degree = 110.0f;
        shadowcam->update();
        return shadowcam;
    }

    void parseArgs(int argc, char *argv[]) 
    {
        namespace po = boost::program_options;
        po::options_description desc{"Options"};
        desc.add_options()
                ("help,h", "Help screen")
                ("HIDPI_SCALE", po::value<float>()->default_value(globals::HIDPI_SCALE), "HIDPI scaling")
                ("port,p", po::value<unsigned int>()->default_value(networking::server_port), "Port the server listens on.")
                ("address,a", po::value<string>()->default_value(networking::server_address), "Server address.")
                ("user,u", po::value<string>()->default_value("Player_" + to_string(MathUtil::rand())), "Username of this player.")
        ;

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional({}).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
            exit(EXIT_SUCCESS);
        }

        globals::HIDPI_SCALE = vm["HIDPI_SCALE"].as<float>();
        port = vm["port"].as<unsigned int>();
        address = vm["address"].as<string>();
        user = vm["user"].as<string>();
    }
}

int main(int argc, char** argv) {
    // EXECUTABLE_DIR set via cmake, paths are now relative to source/executable directory
    filesystem::current_path(EXECUTABLE_DIR);

    parseArgs(argc, argv);

    // init context and set parameters
    ContextParameters params;
    params.title = "The island of two faces";
    params.font_ttf_filename = globals::renderData() / "fonts" / "DroidSansMono.ttf";
    params.icon_font_ttf_filename = globals::renderData() / "fonts" / "forkawesome-webfont.ttf";
    params.font_size_pixels = 15;
    Context::init(params);
    Context::set_resize_callback(&on_resize);
    Gui::init();

    // Init InputManager with default InputBindings
    //   e.g. to pause with ESC key
    // To use different InputBindings, see constants::BINDINGS
    the_bindings = std::make_shared<InputBindings>();
    InputManager &im = InputManager::changeBindings(the_bindings->asList());

    // init stuff
    particles_torch = std::make_shared<Particles>(1000, PARTICLE_SIZE* 0.004f);
    particles_rain = std::make_shared<Rain>(10000, PARTICLE_SIZE * 0.003 * 4.0f);
    particles_small = std::make_shared<Particles>(100, PARTICLE_SIZE * 0.003);
    init_flare();
    the_flare = std::make_shared<FlareHelper>(0.5f);
    the_sky = std::make_shared<Sky>(globals::FARPLANE);
   
    the_sky->setup_skybox();
    SoundManager sMgr;

    the_ambassador = std::make_shared<Ambassador>(port, address);
    msg::client::SvLogin(user);

    // start network thread after login message
    std::thread networkThread = std::thread([](){
        the_ambassador->run();
    });
    // wait until reply of login message was processed
    // -> the_player, the_map created
    while(!msg::client::handleIncoming()){
    }
    // update which blocks are visible after setting initial blocks
    the_map->updateInitialBlockVisibility();
     the_sun = std::make_shared<Sun>();

    /* Set clear color to black to avoid confusion between light and unshaded
     * areas
     */
    glClearColor(-2, -2, -2, 1);

    Camera shadowcam = setup_shadowcam();

    rendering::buffer::init();

    AnimatedModel::add_animated_model(glm::ivec3(5, 1, 6), globals::ModelType::horse);
    AnimatedModel::add_animated_model(glm::ivec3(1, 1, 3), globals::ModelType::dog);

    Context::capture_mouse(true);

    float fov = current_camera()->fov_degree;

    Cloud::darken_clouds();
    Lightning::init_lightning();

    // init rain
    Cloud::rain_around_player(the_player->getPos());

    // init on screen models / weapons
    the_arm = std::make_shared<Arm>(the_player->nextId);
    the_player->nextId++;
    the_sword = std::make_shared<Sword>(the_player->nextId);
    the_player->nextId++;
    the_axe = std::make_shared<Axe>(the_player->nextId);
    the_player->nextId++;

    the_player->screen_item = the_arm;




    while (Context::running()) {
        static uint32_t reload_counter = 0;
        if (reload_counter++ % 128 == 0) {
            reload_counter = 1;
            reload_modified_shaders();
        }
        im.update();

        the_sun->update();
        the_player->screen_item->update();

        the_player->inventory[globals::ModelType::dirt] = 500;
        the_player->inventory[globals::ModelType::wood] = 500;
        the_player->inventory[globals::ModelType::leaf] = 500;
        the_player->inventory[globals::ModelType::stone] = 500;
        the_player->inventory[globals::ModelType::torch] = 500;

        if(current_camera()->name == "default" && CameraImpl::default_input_handler(Context::frame_time())){
            current_camera()->update();
        }else{
            the_player->handleInput(Context::frame_time());
        }

        the_sky->update();

        // after player moved, update visible chunks
        // TODO: delete function CHUNKvisibility, the frustum already determines which chunks are visible, visible "loading" of previously unloaded chunks will not happen with frustum culling
        //the_map->updateChunkVisibility();

        // update clouds
        Cloud::updateDarkness();
        for (auto &[id, cloud] : Cloud::all_clouds) {
            cloud->update();
            // after player and clouds have moved, update their visibility
            cloud->updateVisibility();
        }

        // update frustum for frustum culling
        Chunk::frustum = Chunk::createFrustumFromCamera();

        for (auto &[id, m] : MovableModel::movable_models) {
            m->update();

            // update visibility of each model after
            // player movement and updating of model
            m->updateVisibility();
        }

        if(rain_on) {
            particles_rain->update();
        }
        particles_small->update();
        particles_torch->update();

        msg::client::handleIncoming();

        rendering::pass::shadow();

        rendering::pass::geometry();
        rendering::pass::geomAnimated();
        rendering::pass::geomScreenObj();
        //rendering::pass::geomScreenObj_test();


        rendering::pass::deferred_lighting();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rendering::pass::final();

        Gui::draw();

        if (ImGui::Begin("debug")){
            ImGui::SliderFloat("fov", &fov, 0.1f, 180.f);
            ImGui::SliderFloat("rain size", &rain_size, 0.0001f, 0.08f);

            ImGui::SliderFloat("arm x pos", &xa, -4.f, 4.f);
            ImGui::SliderFloat("arm y pos", &ya, -4.f, 4.f);
            ImGui::SliderFloat("arm z pos", &za, -4.f, 4.f);

            ImGui::SliderFloat("arm rot x", &rx, -0.7f, 5.f);
            ImGui::SliderFloat("arm rot y", &ry, -0.7f, 5.f);
            ImGui::SliderFloat("arm rot z", &rz, -0.7f, 6.f);

            ImGui::SliderFloat("speed_factor", &s, 0.f, 0.1f);

            ImGui::Checkbox("rain_on", &rain_on);

            //current_camera()->fov_degree = fov;
            the_player->draw_player_gui();
            rendering::pass::draw_pass_gui();
        }
        ImGui::End();

        Context::swap_buffers();
    }

    msg::client::SvLogout();
    networkThread.join();
    return EXIT_SUCCESS;
}
