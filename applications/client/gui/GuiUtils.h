#ifndef GRAPA_GUIUTILS_H
#define GRAPA_GUIUTILS_H

#include "texture.h"
#include "imgui/imgui.h"

class GuiUtils {
public:
    /**
     * General-use ImageButton
     *
     * @param R crop from right
     * @param U crop from top/up
     * @param L crop from left
     * @param D crop from down/button
     *
     * @param grayedOut If true, the button can't be clicked and is grayed out. (Well, it is actually tinted red for better visual feedback)
     * @param brighten If true, the button is brighter (as if it is selected).
     */
    static bool addImageButton(const Texture2D &tex, const ImVec2& size,
                               float R=0, float U=0, float L=0, float D=0,
                               bool grayedOut = false, bool brighten = false);

    static ImVec2 calcButtonSize(const std::string &text);
};

#endif //GRAPA_GUIUTILS_H
