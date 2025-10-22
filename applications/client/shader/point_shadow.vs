#version 330 core
layout (location = 0) in vec3 aPos;
layout(location = 1) in vec3 norm;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 model;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool animated_model;


void main()
{
    gl_Position = model * vec4(aPos, 1.0);

    if(animated_model){
        vec3 localNormal;
        vec4 totalPosition = vec4(0.0f);
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(boneIds[i] == -1)
                continue;
            if(boneIds[i] >=MAX_BONES)
            {
                totalPosition = vec4(aPos,1.0f);
                break;
            }
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos,1.0f);
            totalPosition += localPosition * weights[i];
            localNormal = mat3(finalBonesMatrices[boneIds[i]]) * norm;
            //localNormalusum *= localNormal;
        }

        gl_Position =  model * totalPosition;
        //gl_Position =  model * totalPosition;
    }

}