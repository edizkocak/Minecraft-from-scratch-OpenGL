#version 430
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
out layout(location = 0) vec4 out_col;
out layout(location = 1) vec3 out_pos;
out layout(location = 2) vec3 out_norm;
uniform sampler2D diffuse;
uniform sampler2D normalmap;


uniform sampler2D tex;




void main() {
   

   out_col = texture(tex, tc);
   
   //out_col = vec4(1,0,0,1);
}
