#ifndef GRAPA_GUI_H
#define GRAPA_GUI_H

namespace Gui {
    enum Menu {
        INGAME_MENU,
        MAIN_MENU,
        SETTINGS_MENU,
    };

    void init();
    void draw();
    Menu getCurrentMenu();
}

#endif //GRAPA_GUI_H
