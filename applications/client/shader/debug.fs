#version 330

#include "types.fs"
#include "util.fs"

/* Also modify rendering/util.h */
#define DEBUG_BUFFER_GEOM_DEPTH 0
#define DEBUG_BUFFER_GEOM_DIFF  1
#define DEBUG_BUFFER_GEOM_POS   2
#define DEBUG_BUFFER_GEOM_NORM  3
#define DEBUG_BUFFER_SSAO_PREBLUR 4
#define DEBUG_BUFFER_SSAO         5
#define DEBUG_BUFFER_LIGHTING_DEPTH 6
#define DEBUG_BUFFER_LIGHTING_COLOR 7
#define DEBUG_BUFFER_BLOOM          8
#define DEBUG_BUFFER_COC            9
#define DEBUG_BUFFER_DOF            10
#define DEBUG_BUFFER_SHADOW         11

in vec2 tc;

out vec4 out_col;

uniform int debug_buffer;
uniform Camera cam;
uniform Geom geom;
uniform sampler2D ssao_preblur;
uniform sampler2D ssao;
uniform Lighting lighting;
uniform sampler2D bloom;
uniform sampler2D coc;
uniform sampler2D dof;
uniform sampler2DShadow shadow;
uniform mat4 shadow_proj_view;

vec4 map_depth(sampler2D sampler) {
    float frag_depth = texture(sampler, tc).r;
    return vec4(linear_depth(frag_depth, cam.near, cam.far));
}

void main() {
    switch (debug_buffer) {
        case DEBUG_BUFFER_GEOM_DEPTH:
            out_col = map_depth(geom.depth);
            break;
        case DEBUG_BUFFER_GEOM_DIFF:
            out_col = texture(geom.diff, tc);
            break;
        case DEBUG_BUFFER_GEOM_POS:
            out_col = texture(geom.pos, tc);
            break;
        case DEBUG_BUFFER_GEOM_NORM:
            out_col = texture(geom.norm, tc);
            break;
        case DEBUG_BUFFER_SSAO_PREBLUR:
            out_col = vec4(vec3(texture(ssao_preblur, tc).r), 1.0f);
            break;
        case DEBUG_BUFFER_SSAO:
            out_col = vec4(vec3(texture(ssao, tc).r), 1.0f);
            break;
        case DEBUG_BUFFER_LIGHTING_DEPTH:
            out_col = map_depth(lighting.depth);
            break;
        case DEBUG_BUFFER_LIGHTING_COLOR:
            out_col = texture(lighting.color, tc);
            break;
        case DEBUG_BUFFER_BLOOM:
            out_col = texture(bloom, tc);
            break;
        case DEBUG_BUFFER_COC:
            vec3 coc = abs(vec3(texture(coc, tc).r));
           // if(coc<0) coc = abs(coc);
            out_col = vec4(coc, 1.0f);
            break;
        case DEBUG_BUFFER_DOF:
            out_col = texture(dof, tc);
            break;
        case DEBUG_BUFFER_SHADOW: {
            vec4 pos = vec4(texture(geom.pos, tc).xyz, 1.0f);
            vec4 pos_vc = shadow_proj_view * pos;
            pos_vc /= pos_vc.w;
            pos_vc = pos_vc * 0.5f + 0.5f;
            float shadow_val = texture(shadow, pos_vc.xyz);
            out_col = vec4(vec3(shadow_val), 1);
            break;
        }
    }
}
