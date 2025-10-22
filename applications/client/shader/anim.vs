#version 430 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 model_normal;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec4 pos_wc;
out vec3 norm_wc;
out vec2 tc;

void main()
{
    vec3 localNormal;
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1)
            continue;
        if(boneIds[i] >=MAX_BONES)
        {
            totalPosition = vec4(pos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos,1.0f);
        totalPosition += localPosition * weights[i];
        localNormal = mat3(finalBonesMatrices[boneIds[i]]) * norm;
        //localNormalusum *= localNormal;
    }

    gl_Position =  projection * view * model * totalPosition;

    pos_wc = model * totalPosition;
    norm_wc = normalize(mat3(model_normal) * localNormal);
    tc = tex;
}
