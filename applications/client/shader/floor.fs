#version 430
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
out layout(location = 0) vec4 out_col;
out layout(location = 1) vec3 out_pos;
out layout(location = 2) vec3 out_norm;
uniform sampler2D diffuse;
uniform sampler2D normalmap;


uniform sampler2D flareTexture;
uniform float brightness;

vec3 align(vec3 axis, vec3 v) {
    float s = sign(axis.z + 0.001f);
    vec3 w = vec3(v.x, v.y, v.z * s);
    vec3 h = vec3(axis.x, axis.y, axis.z + s);
    float k = dot(w, h) / (1.f + abs(axis.z));
    return k * h - w;
}

void main() {
    vec2 TC = fract(tc);
    vec4 diff = texture(diffuse, TC);
    vec3 N = align(norm_wc, 2 * texture(normalmap, TC).xyz - 1);
    out_col = diff;
    out_pos = pos_wc.xyz;
    out_norm = N;

    out_col = texture(flareTexture, tc);
    out_col.a *= brightness * 0.4;
   //  out_colour = vec4(0,1,0,1);
}
/*#version 330

in vec2 pass_textureCoords;

out vec4 out_colour;

uniform sampler2D flareTexture;
uniform float brightness;

void main(void){

   out_colour = texture(flareTexture, pass_textureCoords);
    out_colour.a *= brightness;
     out_colour = vec4(0,1,0,1);

}*/