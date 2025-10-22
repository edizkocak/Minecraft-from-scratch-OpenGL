#version 430
flat in int time;
uniform vec2 near_far;
uniform vec2 screenres;
uniform sampler2D gbuf_depth;
uniform vec3 color;
uniform bool rain;
uniform bool torch;
uniform sampler2D rain_mask;
uniform sampler2D torch_mask;
uniform float random;


out vec4 out_col;

float linear_depth(in float depth, in float near, in float far) { return (2.0 * near) / (far + near - depth * (far - near)); }

void main() {
    vec4 col = vec4(color,1);
    float fade_time = clamp(time / 500.f, 0, 1);


    if(!rain){
        out_col = pow(fade_time, 2) * col;
    }
    vec2 tc = gl_FragCoord.xy / screenres;
    if (gl_FragCoord.z > texture(gbuf_depth,tc).r)
        out_col = vec4(0,1,0,1);    
    float frag_depth = linear_depth(gl_FragCoord.z, near_far.x, near_far.y);
    float geom_depth = linear_depth(texture(gbuf_depth, gl_FragCoord.xy / screenres).r, near_far.x, near_far.y);
    float fade_depth = clamp(abs(frag_depth - geom_depth) / 0.001f, 0, 1);

    if(rain){
        out_col *= vec4(fade_depth * fade_depth);
    }

    out_col = col;

    float alpha = 1; 
    float r = (random * 2 - 1 ) * 0.8;
    
    if(rain){
    alpha = texture(rain_mask, gl_PointCoord).r;

    }else if(torch){
        if(time > 80){
            out_col  *= 3;
            out_col.y = 0.3 + r * 0.3;
        }
        if(time > 160){
            out_col *= vec4(3,5,0,1);
            out_col.y = 1 + r * 0.5;
            
        }
        alpha = 1 - r;


    }
  

    out_col.a *= alpha;
}
