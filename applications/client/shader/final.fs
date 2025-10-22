#version 430

#include "types.fs"

in vec2 tc;

out vec4 out_col;

uniform Lighting lighting;

void main() {
    float frag_depth = texture(lighting.depth, tc).r;
    gl_FragDepth = frag_depth;

    vec3 frag_col = texture(lighting.color, tc).rgb;
    out_col = vec4(frag_col, 1.0f);
}

