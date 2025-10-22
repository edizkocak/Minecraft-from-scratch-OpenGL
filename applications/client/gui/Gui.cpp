#include "Gui.h"
#include "context.h"
#include "imgui/imgui.h"
#include "../input/InputManager.h"
#include "../model/player.h"
#include "GuiUtils.h"
#include "IconFont.h"
#include "daytime.h"

extern std::shared_ptr<InputBindings> the_bindings;
extern std::shared_ptr<Player> the_player;
extern int the_score;

namespace {
    Gui::Menu currentMenu = Gui::INGAME_MENU;
    std::map<globals::ModelType, Texture2D> icons;

    void drawMainMenu() {
        if (ImGui::Button("START")) {
            Context::capture_mouse(true);
            currentMenu = Gui::INGAME_MENU;
        }
        if (ImGui::Button("EXIT")) {
            glfwSetWindowShouldClose(Context::instance().glfw_window, 1);
        }
    }

    void drawSettingsMenu() {
        if (ImGui::Button("BACK")) {
            currentMenu = Gui::MAIN_MENU;
        }
    }

    void drawIngameMenu() {
        static const float BUTTON_HEIGHT = GuiUtils::calcButtonSize("foo").y;

        // draw top bar

        int n = 3;
        string score = "Score: " + to_string(the_score) + " " + string(ICON_FK_LINE_CHART);
        string hp = "HP: " + to_string((int) the_player->hp) + " " + string(ICON_FK_HEARTBEAT);
        string hour = to_string(daytime::hour(daytime::dayFract(the_player->gameTimer.look()))) + ":00 " + string(ICON_FK_CLOCK_O);

        ImGui::BeginMainMenuBar();

        // center-align
        {
            float contentWidth = GuiUtils::calcButtonSize(score).x + GuiUtils::calcButtonSize(hp).x +
                                 GuiUtils::calcButtonSize(hour).x + ImGui::GetStyle().ItemSpacing.x * float(n-1);
            ImGui::Dummy({
                                 (ImGui::GetContentRegionAvailWidth() - contentWidth - ImGui::GetStyle().ItemSpacing.x) / 2,
                                 BUTTON_HEIGHT
                         });
        }

        ImGui::Text("%s", score.c_str());
        ImGui::Separator(); // vertical line
        ImGui::Text("%s", hp.c_str());
        ImGui::Separator(); // vertical line
        ImGui::Text("%s", hour.c_str());
        ImGui::EndMainMenuBar();

        // draw left sidebar

        const auto& inventory = the_player->inventory;

        const int numImages = (int) inventory.size();
        static const ImVec2 imgSize = {30 * globals::HIDPI_SCALE, 30 * globals::HIDPI_SCALE};
        const ImVec2 windowSize = {
                imgSize.x + 2 * ImGui::GetStyle().WindowPadding.x + 2 * ImGui::GetStyle().FramePadding.x,

                (float) numImages * imgSize.y + 2 * ImGui::GetStyle().WindowPadding.y +
                ((float) numImages - 1) * ImGui::GetStyle().ItemSpacing.y +
                ((float) numImages * 2) * ImGui::GetStyle().FramePadding.y
        };
        static bool *p_open = nullptr;
        ImGui::SetNextWindowPos({
                                        0,
                                        (ImGui::GetIO().DisplaySize.y - windowSize.y) * 0.5f
                                });
        ImGui::SetNextWindowSize(windowSize);
        if (ImGui::Begin("IngameLeftSidebar", p_open,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
            // Window is NOT collapsed / fully clipped

            for(auto &[key, value] : inventory){
                bool grayedOut = value<=0;
                bool brighten = the_player->selectedItem == key;
                GuiUtils::addImageButton(icons.at(key), imgSize, 0, 0, 0, 0, grayedOut, brighten);
            }
        }
        ImGui::End();
    }
}

void Gui::init() {
    icons = {
            {globals::ModelType::water, {"waterIcon",
                                                globals::renderData() / "Cubes" / "water" / "water.png"}},
            {globals::ModelType::wood,  {"woodIcon",
                                                globals::renderData() / "Cubes" / "wood" / "wood.png"}},
            {globals::ModelType::dirt,  {"dirtIcon",
                                                globals::renderData() / "Cubes" / "dirt" / "dirt.png"}},
            {globals::ModelType::grass, {"grassIcon",
                                                globals::renderData() / "Cubes" / "grass" / "grass.png"}},
            {globals::ModelType::leaf,  {"leafIcon",
                                                globals::renderData() / "Cubes" / "leafs" / "leafs.png"}},
            {globals::ModelType::torch, {"torchIcon",
                                                globals::renderData() / "Icons" / "torchIcon.png"}},
            {globals::ModelType::stone, {"stoneIcon",
                                                globals::renderData() / "Cubes" / "stone" / "stone.png"}},
            {globals::ModelType::sword, {"swordIcon",
                                        globals::renderData() / "Icons" / "swordIcon.png"}},
            {globals::ModelType::axe, {"axeIcon",
                                         globals::renderData() / "Icons" / "axeIcon.png"}},
    };
}

void Gui::draw() {
    // Toggle currently shown menu.
    // Don't check for ImGui::GetIO().WantCaptureKeyboard as we want to open the menu
    // when in GAME-context and close it when in GUI-context.

    if (InputManager::instance().isActive(the_bindings->pause)) {
        switch (currentMenu) {
            case MAIN_MENU:
                break;
            case SETTINGS_MENU:
                currentMenu = MAIN_MENU;
                break;
            case INGAME_MENU:
                Context::capture_mouse(false);
                currentMenu = MAIN_MENU;
                break;
        }
    }

    switch (currentMenu) {
        case MAIN_MENU:
            drawMainMenu();
            break;
        case SETTINGS_MENU:
            drawSettingsMenu();
            break;
        case INGAME_MENU:
            drawIngameMenu();
            break;
    }
}

Gui::Menu Gui::getCurrentMenu() {
    return currentMenu;
}
