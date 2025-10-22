#version 430 core

in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;

out layout(location = 0) vec3 out_col;
out layout(location = 1) vec3 out_pos;
out layout(location = 2) vec3 out_norm;

uniform sampler2D texture_diffuse1;

void main(){
    vec3 diff = texture(texture_diffuse1, tc).rgb;

    out_col = diff;
    out_norm = normalize(norm_wc);
    out_pos = pos_wc.xyz;
}
