#version 430 core

#include "types.fs"


in VS_OUT {
    vec4 pos_wc;
    vec3 norm_wc;
    vec2 tc;
} fs_in;

out layout(location = 0) vec3 out_col;
out layout(location = 1) vec3 out_pos;
out layout(location = 2) vec3 out_norm;
out layout(location = 3) vec3 out_id;

uniform sampler2D diffuse;
uniform Camera cam;
uniform bool highlighted;
uniform float mining_progress;
uniform bool is_cloud;
uniform float cloud_darkness_factor;

void main() {
    vec3 diff = texture(diffuse, fs_in.tc).rgb;

    if(!is_cloud){
        diff = pow(diff, vec3(2.2f));
    }

    out_col = diff;
    out_pos = fs_in.pos_wc.xyz;
    out_norm = fs_in.norm_wc;
    out_id = vec3(-1.f);//vec3(-0.0001f);

    if(highlighted){
        //out_col *= vec3(0.85, 0.85, 0.85); make block darker
        out_col *= vec3(1.3, 1.3, 1.3);
        out_col *= 1.f + 0.25 * smoothstep(0, 1, mining_progress);

    }

    if(is_cloud){
        const float max_darkness_factor = .2f;

        out_col *= (1.f - max_darkness_factor) * smoothstep(0, 1, 1.f - cloud_darkness_factor) + max_darkness_factor;
    }
}
