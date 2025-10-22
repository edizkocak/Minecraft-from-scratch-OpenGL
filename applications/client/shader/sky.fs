#version 430 core

#include "types.fs"


in VS_OUT {
    vec4 pos_wc;
    vec3 norm_wc;
    vec3 tc;
} fs_in;

out layout(location = 0) vec3 out_col;
out layout(location = 1) vec3 out_pos;
out layout(location = 2) vec3 out_norm;
out layout(location = 3) vec3 out_id;

uniform samplerCube day;
uniform samplerCube night;
uniform Camera cam;
uniform bool sun;
uniform float time;
uniform vec3 sun_pos;

void main() {
    vec3 diff;
    vec3 day_diff = texture(day, fs_in.tc).rgb * 1.7f;
     vec3 night_diff = texture(night, fs_in.tc).rgb;
     float blend = 7;
     float y = sun_pos.y;
     float x = sun_pos.x;

    if(sun){ 
        
        
        diff = vec3(1,0.9,0.9);}
        
        else{

        if(sun_pos.y > 0){
            diff = day_diff;

        }else{
              diff = night_diff;
        }

        if(y > blend){
            diff = day_diff;
        }
        else if(y > -blend && y < blend){
            float fac = (y + blend) / (blend * 2);
            if(x < 0){
               diff = mix(night_diff, day_diff, fac);
            }else{
                diff = mix(day_diff, night_diff, 1-fac);
            }

        }else{
              diff = night_diff;
        }
    }
  

    out_col = diff;
  //  out_col = vec3(mix_x,mix_y,mix_z);
   // out_pos = fs_in.pos_wc.xyz;
    //out_norm = fs_in.norm_wc;
    out_id = vec3(-1.f);//vec3(-0.0001f);


}
