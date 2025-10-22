#include "GuiUtils.h"

using namespace std;

bool GuiUtils::addImageButton(const Texture2D &tex, const ImVec2& size, float R, float U, float L, float D,
                              bool grayedOut, bool brighten) {
    static const ImVec4 unchanged = {1, 1, 1, 1}; // unchanged tint
    static const ImVec4 lessColor = {1, 1, 1, 0.5}; // grayed-out tint
    static const ImVec4 gray = {50.f/255.f, 50.f/255.f, 50.f/255.f, 0.8f};
    static const ImVec4 unchanged_hover_col = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);

    ImVec4 tint_col = unchanged;
    if (grayedOut) {
        // reduce color intensity
        tint_col = lessColor;

        // gray-out button color
        ImGui::PushStyleColor(ImGuiCol_Button, gray);
        // button does not look "hovered"
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, gray);
    }
    if (brighten) {
        // tint_col = unchanged;

        // button looks as if "hovered"
        ImGui::PushStyleColor(ImGuiCol_Button, unchanged_hover_col);
    };

    bool pressed = ImGui::ImageButton((void *) tex->id, size,
                                      ImVec2(0 + L, 1 - U), ImVec2(1 - R, 0 + D),
                                      -1, {0, 0, 0, 0}, tint_col);

    if (grayedOut) {
        ImGui::PopStyleColor(2);
    }
    if (brighten) {
        ImGui::PopStyleColor(1);
    }

    return !grayedOut && pressed;
}

ImVec2 GuiUtils::calcButtonSize(const string &text) {
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 framePadding = ImGui::GetStyle().FramePadding;
    return {
            textSize.x + 2 * framePadding.x,
            textSize.y + 2 * framePadding.y
    };
}
