#include "buffer.h"

#include <cppgl.h>

using namespace glm;
using namespace rendering;

Framebuffer buffer::geom;
Framebuffer buffer::lighting;
Framebuffer buffer::shadow_fb;
Framebuffer buffer::screen;

static void clamp_to_edge(const Texture2D& tex) {
    glTextureParameteri(tex->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void buffer::init() {
    const ivec2 res = Context::resolution();

    {
        geom = Framebuffer { "geom", res.x, res.y };

        geom->attach_depthbuffer(
            Texture2D { "geom.depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT }
        );
        geom->attach_colorbuffer(Texture2D { "geom.diff", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT });
        geom->attach_colorbuffer(Texture2D { "geom.pos",  res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT });
        geom->attach_colorbuffer(Texture2D { "geom.norm", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT });
        geom->attach_colorbuffer(Texture2D { "geom.enemy_id", res.x, res.y, GL_R32F, GL_RED, GL_FLOAT });

        clamp_to_edge(geom->depth_texture);
        for (const auto& tex : geom->color_textures) {
            clamp_to_edge(tex);
        }

        geom->check();
    }

    {
        lighting = Framebuffer { "lighting", res.x, res.y };

        lighting->attach_depthbuffer(
            Texture2D { "lighting.depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT }
        );
        lighting->attach_colorbuffer(Texture2D { "lighting.color", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT });

        clamp_to_edge(lighting->depth_texture);
        for (const auto& tex : lighting->color_textures) {
            clamp_to_edge(tex);
        }

        lighting->check();
    }

    {
        shadow_fb = { "shadow", 4096, 4096 };
        shadow_fb->attach_depthbuffer(
            Texture2D { "shadow.depth", 4096, 4096, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT }
        );

        glTextureParameteri(shadow_fb->depth_texture->id, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTextureParameteri(shadow_fb->depth_texture->id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTextureParameteri(shadow_fb->depth_texture->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(shadow_fb->depth_texture->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        shadow_fb->check();
    }

    {
        screen = Framebuffer{"screen", res.x, res.y};

        screen->attach_depthbuffer(
                Texture2D{"screen.depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT}
        );
        screen->attach_colorbuffer(geom->color_textures[0]);
        screen->attach_colorbuffer(geom->color_textures[1]);
        screen->attach_colorbuffer(geom->color_textures[2]);

        clamp_to_edge(screen->depth_texture);
        for (const auto &tex: screen->color_textures) {
            clamp_to_edge(tex);
        }

        screen->check();
    }
}

