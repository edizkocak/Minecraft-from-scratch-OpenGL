#version 430
layout(location = 0) in vec3 in_pos;

layout(location = 1) in vec2 in_tc;

vec3 cam_right;
vec3 cam_up;
uniform mat4 model;

uniform mat4 view;
uniform mat4 proj;

uniform mat4 modelview;
out vec4 pos_wc;
out vec3 norm_wc;
out vec2 tc;



void main() {
    pos_wc =  vec4(in_pos, 1.0);
    tc = in_tc;
 

   //pos_wc = vec4(vec3(0,0,0)+cam_right * in_pos + cam_up * in_pos,1);


    gl_Position = proj *  view * model* pos_wc;//proj * view *
}



