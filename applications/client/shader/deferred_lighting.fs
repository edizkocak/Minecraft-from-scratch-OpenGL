#version 430

#include "types.fs"

in vec2 tc;

out vec3 out_col;

uniform Camera cam;
uniform PointLight p_light;
uniform Geom geom;
uniform sampler2DShadow shadow;
uniform mat4 shadow_proj_view;
uniform bool directional_light;
uniform sampler2D screen_depth;

uniform float screen_hit_radius;
uniform float max_hit_dist;
uniform float redness_factor;
uniform bool player_hit;
uniform bool rayleigh_scattering_on;

uniform vec3 sun_diffuse;
uniform vec3 sun_ambient;
uniform vec3 sun_dir;

uniform vec3 lightPos;
uniform float far_plane;
uniform float bias;
uniform float off;

const int MAX_POINT_LIGHTS = 26; // TODO Set to <=26 on Ryzen 4800H

uniform int num_p_lights;
uniform vec3 p_light_pos[MAX_POINT_LIGHTS];
uniform vec3 p_light_ambient;
uniform vec3 p_light_diffuse;
//uniform vec3 p_light_specular[MAX_POINT_LIGHTS];
uniform samplerCube p_light_depthMap[MAX_POINT_LIGHTS];

uniform float amb_col;

uniform bool lightning_on;
uniform int lightning_index1;
uniform int lightning_index2;
uniform int lightning_index3;

uniform float far_plane_lightning;
uniform vec3 lightning_diffuse;


float get_shadow_val(vec2 at_tc) {
    vec4 pos = vec4(texture(geom.pos, at_tc).xyz, 1.0f);
    vec4 pos_vc = shadow_proj_view * pos;
    pos_vc /= pos_vc.w;
    pos_vc = pos_vc * 0.5f + 0.5f;
    float shadow_val = texture(shadow, pos_vc.xyz);
    float shadow_factor = smoothstep(1.0, 0.95, pos_vc.x);
    shadow_factor *= smoothstep(1.0, 0.95, pos_vc.y);
    shadow_factor *= smoothstep(0.0, 0.05, pos_vc.x);
    shadow_factor *= smoothstep(0.0, 0.05, pos_vc.y);
    shadow_factor *= smoothstep(35, 34, pos.y);
    return clamp((1-shadow_factor) + shadow_val, 0.0, 1.0);
}


float ShadowCalculation(vec3 fragPos, vec3 frag_norm, int i)
{
    float far = far_plane;
    if(lightning_on && (i == lightning_index1 || i == lightning_index2 || i == lightning_index3)){
        far = far_plane_lightning;
    }

    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - p_light_pos[i];
    // use the light to fragment vector to sample from the depth map
    float closestDepth = texture(p_light_depthMap[i], fragToLight).r;

    float depth_0_1 = closestDepth;

    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= far;

    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);

    if(currentDepth >= far_plane - bias && depth_0_1 >= 1.0){
        return 0.f;
    }

    // now test for shadows
    float b = bias;
    b = max(0.05 * (1.0 - dot(frag_norm, sun_dir)), b);
    if(currentDepth < 6.0 * b){
        //b = 0.03;
    }
    float shadow = currentDepth -  b > closestDepth ? 1.0 : 0.0;
    //return shadow;

    //TODO: depending on daylight
    return shadow * (1.f - smoothstep(0.f, 2.3f * far, currentDepth));
}


vec3 CalcPointLight(vec3 frag_diff, vec3 frag_norm, vec3 frag_pos, vec3 viewDir, int i)
{
    const float l_constant = 1.f;
    // distance 7
    //const float l_linear = .7f;
    //const float l_quadratic = 1.8f;
    // distance 13
    //const float l_linear = .35f;
    //const float l_quadratic = .44f;
    // distance 20
    float l_linear = .22f;
    float l_quadratic = .2f;

    vec3 p_amb = p_light_ambient;
    vec3 p_diff = p_light_diffuse;

    if(lightning_on && (i == lightning_index1 || i == lightning_index2 || i == lightning_index3) ){
        // this is the lightning
        p_amb = vec3(amb_col);
        p_diff = lightning_diffuse;


        //distance 100
        //l_linear = .045f;
        //l_quadratic = .0075f;

        //distance 160
        //l_linear = .027f;
        //l_quadratic = .0028f;

        //distance 200
        l_linear = .022f;
        l_quadratic = .0019f;
    }


    vec3 lightDir = normalize(p_light_pos[i] - frag_pos);
    // diffuse shading
    float diff = max(dot(frag_norm, lightDir), 0.0);
    // attenuation
    float distance    = length(p_light_pos[i] - frag_pos);
    float attenuation = 1.0 / (l_constant + l_linear * distance +
  			     l_quadratic * (distance * distance));
    // combine results
    //vec3 ambient  = p_light_ambient[i]  * frag_diff;
    vec3 ambient  = p_amb  * frag_diff;
    vec3 diffuse  = p_diff  * diff * frag_diff;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    return (ambient + diffuse);
}



void main() {
    { // crosshair
        const float aspect_ratio = 16.0 / 9.0;
        const float crosshair_length = 0.01;
        const float crosshair_width = 0.0005;

        // horizontal line
        if (tc.x < 0.5 + crosshair_length
        && tc.x > 0.5 - crosshair_length
        && tc.y < 0.5 + aspect_ratio * crosshair_width
        && tc.y > 0.5 - aspect_ratio * crosshair_width){
            out_col = vec3(1);
            return;
        }

        // vertical line
        if (tc.y < 0.5 + aspect_ratio * crosshair_length
        && tc.y > 0.5 - aspect_ratio * crosshair_length
        && tc.x < 0.5 + crosshair_width
        && tc.x > 0.5 - crosshair_width){
            out_col = vec3(1);
            return;
        }
    }



    float frag_depth = texture(geom.depth, tc).r;
    float depth_screen = texture(screen_depth, tc).r;


    gl_FragDepth = frag_depth;

    if(frag_depth > depth_screen){gl_FragDepth = depth_screen;}


    if (frag_depth == 1.f ) {
        discard;
        return;
    }

    vec3 frag_diff = texture(geom.diff, tc).rgb;
    vec3 frag_pos = texture(geom.pos, tc).xyz;
    vec3 frag_norm = normalize(texture(geom.norm, tc).xyz);

    const float shadow_val = get_shadow_val(tc);
    float shadow = 0.f;

    if(num_p_lights > 0){
        for(int i = 0; i < num_p_lights; i++){
            shadow += ShadowCalculation(frag_pos, frag_norm, i);
        }
    }

    float n_dot_l;

    if(sun_dir.y >= -0.2f){
        n_dot_l = max(0, dot(frag_norm, -vec3(sun_dir.x, -0.5f, sun_dir.z)));
    }
    else{
        n_dot_l = max(0, dot(frag_norm, -sun_dir));
    }

    float clamp_amb = max(0.2f, sun_ambient.x);

    // phase 1: directional lighting
    vec3 result =  ( vec3(clamp_amb) + ( (1.0 - shadow) * shadow_val) * (vec3(clamp_amb) * n_dot_l) ) * frag_diff;

    vec3 viewDir = normalize(cam.pos - frag_pos);

    // phase 2: point lights
    for(int i = 0; i < num_p_lights; i++){
        result += CalcPointLight(frag_diff, frag_norm, frag_pos, viewDir, i);
    }

    out_col = result;

    //out_col =  ( light.ambient + ( (1.0 - shadow) * shadow_val) * (light.ambient * n_dot_l) ) * frag_diff;


    // imitate rayleigh scattering
    // turned off for now
    if(rayleigh_scattering_on){
        float dist = distance(cam.pos, frag_pos);
        const float max_dist = 50.0;
        if(dist >= max_dist && frag_diff.z < 0.6){
            const float color_reduce = 1.f - smoothstep(max_dist, 2.0 * max_dist, dist);
            //out_col *= 1.5 * color_reduce;

            // add blue tint
            out_col += vec3(0,0, 0.3 * color_reduce);
        }
    }


    // .4f  , .8f

    // player screen turns red when being hit
    vec2 base_tc = vec2(.5f, .5f);
    float dist = distance(tc, base_tc);
    //const float screen_hit_distance = .35f;
    if(dist >= screen_hit_radius && player_hit){
        float red_attenuate = smoothstep(screen_hit_radius, max_hit_dist, dist);
        //red_attenuate = clamp(red_attenuate, 0.f, .2f);
        //red_attenuate *= 0.5f;
        //out_col += vec3(red_attenuate, -red_attenuate, -red_attenuate);

        //red_attenuate *= hit_factor;
        red_attenuate *= redness_factor;

        out_col += vec3(red_attenuate, -red_attenuate, -red_attenuate);
        //out_col.x = clamp(out_col.x, 0.f, 0.5f);
    }

//     //out_col = vec3(1 - shadow);
vec3 fragToLight = frag_pos - p_light_pos[0];
//     // use the light to fragment vector to sample from the depth map
   float closestDepth = texture(p_light_depthMap[0], fragToLight).r;
//    out_col = vec3(closestDepth);
     //out_col = vec3(num_p_lights );
     //out_col = vec3(1);
//

}
