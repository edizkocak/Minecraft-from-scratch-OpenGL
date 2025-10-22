#pragma once
#include <cppgl.h>


namespace rendering::pass {
    void draw_pass_gui();

    void geometry();
    void geomAnimated();
    void geomScreenObj();

    void shadow();

    void deferred_lighting();
    void final();

}
