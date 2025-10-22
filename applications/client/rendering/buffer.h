#pragma once

#include <cppgl.h>

namespace rendering::buffer {
    void init();

    /* Buffer used as output during the geometry pass. Has
     * - depth
     * - diff (color 0)
     * - pos  (color 1)
     * - norm (color 2)
     */
    extern Framebuffer geom;

    /* Buffer used as output during the deferred lighting pass (and e.g.
     * particle lighting)
     */
    extern Framebuffer lighting;

    extern Framebuffer shadow_fb;

    extern Framebuffer screen;
}
