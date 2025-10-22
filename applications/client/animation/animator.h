#pragma once

#include "cppgl/external/thirdparty/include/glm/glm.hpp"
#include <map>
#include <vector>
#include "cppgl/external/thirdparty/include/assimp/scene.h"
#include "cppgl/external/thirdparty/include/assimp/Importer.hpp"
#include "cppgl/src/cppgl.h"
#include "cppgl/external/thirdparty/include/glm/gtx/quaternion.hpp"
#include "cppgl/external/thirdparty/include/assimp/Importer.hpp"
#include "cppgl/external/thirdparty/include/assimp/scene.h"
#include "cppgl/external/thirdparty/include/assimp/postprocess.h"

#include "animation.h"
#include "bone.h"
#include "model_animation.h"

class Animator
{
public:
    float speedfactor;
	float lTime;
	bool animation_finished = false;
	
    Animator() = default;

	Animator(Animation1* animation, float Speedfactor)
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;

		m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.emplace_back(1.0f);
		speedfactor = Speedfactor;
		lTime = 0.f;
	}

	void render_animation(Shader &shader) {
		// view/projection transformations
		auto transforms = GetPoseTransforms();
		for (long unsigned int i = 0; i < transforms.size(); ++i) {
			shader->uniform("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
		}
	}

	void UpdateAnimation(float curTime)
	{
		float dt = (curTime - lTime) * speedfactor;
		lTime = curTime;
		m_DeltaTime = dt * speedfactor;

		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;

			if(m_CurrentTime >= m_CurrentAnimation->GetDuration() ){
				animation_finished = true;
			}

			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	void PlayAnimation(Animation1* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
        lTime = 0;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (unsigned int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}
	std::vector<glm::mat4> GetPoseTransforms() {
		
		return m_FinalBoneMatrices;
	}

	Animation1* getCurrentAnimation(){
		return m_CurrentAnimation;
	}


private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	Animation1* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;

};