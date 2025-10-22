#version 430 core

in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;

out layout(location = 0) vec3 out_col;
out layout(location = 1) vec3 out_pos;
out layout(location = 2) vec3 out_norm;
out layout(location = 3) vec3 out_id;

uniform sampler2D texture_diffuse1;
uniform float id;
uniform bool enemy_is_hit;
uniform float redness_factor;

void main(){
    vec3 diff = texture(texture_diffuse1, tc).rgb;
    //out_col = diff;
    //diff = pow(diff, vec3(2.2f));

    out_col = diff;
    out_norm = normalize(norm_wc);
    out_pos = pos_wc.xyz;
    out_id = vec3(id);

    if(enemy_is_hit) {
        float redness_scale = 0.4;
        out_col += (vec3(redness_factor, -redness_factor, -redness_factor) * redness_scale);

      //  out_col *= redness_scale;
    }
}
