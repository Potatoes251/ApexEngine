#include "Animator.h"

#include <cmath>

using namespace Apex::Rendering;

Animator::Animator(ResourceHandle<Mesh> mesh, ResourceHandle<Shader> shader, float fps) :
	m_mesh(mesh), m_shader(shader), m_fps(fps) {}

void Apex::Rendering::Animator::SetMesh(ResourceHandle<Mesh> mesh)
{
	m_mesh = mesh;

	m_boneCount = static_cast<int>(mesh->m_bones.size());
}

void Animator::SetShader(ResourceHandle<Shader> shader)
{
	m_shader = shader;
}

void Animator::SetFPS(float fps)
{
	m_fps = fps;
}

void Animator::Update(float deltaTime)
{
	switch (m_state)
	{
	case Waiting_for_loaded_ressources:
		WaitForResourcesToBeLoaded();
		break;

	case Play_no_animation:
		break;

	case Play_single_animation_repeat:
		PlaySingleAnimation(deltaTime);
		break;

	case Play_single_animation_repeat_interpolated:
		PlaySingleAnimationInterpolated(deltaTime);
		break;

	case Play_cross_fade_animation:
		PlayCrossFadeAnimation(deltaTime);
		break;
	}
}

void Animator::Stop()
{
	m_nextState = Play_no_animation;
}

void Animator::PlayRepeat(int animationIndex)
{
	if (m_state == Waiting_for_loaded_ressources)
	{
		m_animationIndexA = animationIndex;
		m_nextState = Play_single_animation_repeat;
	}
	else
	{
		m_animationA = &m_mesh->m_animations[animationIndex];
		m_timeA = 0.f;
		m_state = Play_single_animation_repeat;
	}
}

void Animator::PlayRepeatInterpolated(int animationIndex)
{
	if (m_state == Waiting_for_loaded_ressources)
	{
		m_animationIndexA = animationIndex;
		m_nextState = Play_single_animation_repeat_interpolated;
	}
	else
	{
		m_animationA = &m_mesh->m_animations[animationIndex];
		m_timeA = 0.f;
		m_state = Play_single_animation_repeat_interpolated;
	}
}

void Animator::PlayCrossFade(int animationIndexA, int animationIndexB, float crossfadeTime)
{
	if (m_crossFadeDuration <= 0.f) return;							//check if the cross fade time isn't negatif

	if (m_state == Waiting_for_loaded_ressources)				//if the ressources are not already loaded 
	{
		m_animationIndexA = animationIndexA;
		m_animationIndexB = animationIndexB;

		m_nextState = Play_cross_fade_animation;
	}
	else
	{
		m_animationA = &m_mesh->m_animations[animationIndexA];
		m_animationB = &m_mesh->m_animations[animationIndexB];

		m_state = Play_cross_fade_animation;

		m_timeA = 0.f;
		m_crossFadeElapseTime = 0;
	}

	m_crossFadeDuration = crossfadeTime;
}

void Animator::WaitForResourcesToBeLoaded()
{
	//check if the mesh, animations and shader are loaded
	if (m_mesh.IsReady() && m_shader.IsReady())
	{
		m_boneCount = static_cast<int>(m_mesh->m_bones.size());

		m_state = m_nextState;

		if (m_animationIndexA != -1)
		{
			m_animationA = &m_mesh->m_animations[m_animationIndexA];
		}
		if (m_animationIndexB != -1)
		{
			m_animationB = &m_mesh->m_animations[m_animationIndexB];
		}
	}
}

void Animator::ExtractFrameSkeleton(vector<KeyFrame>& skeleton_out, const Animation& animation, int currentFrame)
{
	skeleton_out.clear();
	skeleton_out.resize(m_boneCount);

	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		skeleton_out[bone] = animation.m_boneAnimations[bone].m_frames[currentFrame];
	}
}

void Animator::GetSkeletonToGlobal(vector<KeyFrame>& skeleton_out, vector<KeyFrame>& localSkeleton)
{
	skeleton_out.resize(m_boneCount);

	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		//create local matrix
		Matrix4 local = 
			Matrix4::createTranslation(localSkeleton[bone].m_position) * 
			(Matrix4)localSkeleton[bone].m_rotation * 
			Matrix4::createScale(localSkeleton[bone].m_scale);

		//create the global matrix
		int	parent = m_mesh->m_bones[bone].m_parentIndex;
		if (parent < 0)
		{
			skeleton_out[bone].m_matrix = local;
		}
		else
		{
			skeleton_out[bone].m_matrix = skeleton_out[parent].m_matrix * local;
		}
	}
}

void Animator::ApplyOffset(vector<Matrix4>& skeletonMatrix_out, const vector<KeyFrame>& globalTransform)
{
	skeletonMatrix_out.resize(m_boneCount);

	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		skeletonMatrix_out[bone] = globalTransform[bone].m_matrix * m_mesh->m_bones[bone].m_offsetMatrix;
	}
}

void Animator::InterlopeSkeletons(vector<KeyFrame>& interlopedSkeleton_out, vector<KeyFrame>& skeletonA, vector<KeyFrame>& skeletonB, const float& t)
{
	interlopedSkeleton_out.resize(m_boneCount);

	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		//position
		interlopedSkeleton_out[bone].m_position = Vector3::lerp(skeletonA[bone].m_position, skeletonB[bone].m_position, t);

		//rotation
		interlopedSkeleton_out[bone].m_rotation = Quaternion::slerp(skeletonA[bone].m_rotation, skeletonB[bone].m_rotation, t);

		//scale
		interlopedSkeleton_out[bone].m_scale = Vector3::lerp(skeletonA[bone].m_scale, skeletonB[bone].m_scale, t);
	}
}

void Animator::SendSkeletonMatrixToGPU(vector<Matrix4>& skeletonMatrix)
{	
	m_shader->SetUniformMat4Array("u_Bones[0]", skeletonMatrix);
}

void Animator::PlaySingleAnimation(float deltaTime)
{
	m_timeA += deltaTime;

	//get current frame
	int currentFrame = static_cast<int>(m_timeA * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();

	//extract transforms
	vector<KeyFrame>	animationSkeleton;
	ExtractFrameSkeleton(animationSkeleton, *m_animationA, currentFrame);

	//transforms -> matrix
	vector<Matrix4>		SkeletonMatrices;
	ApplyOffset(SkeletonMatrices, animationSkeleton);

	//send to shader (GPU)
	SendSkeletonMatrixToGPU(SkeletonMatrices);
}

void Animator::PlaySingleAnimationInterpolated(float deltaTime)
{
	m_timeA += deltaTime;

	//get current frame
	int currentFrame = static_cast<int>(m_timeA * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();
	int nextFrame = (currentFrame + 1) % m_animationA->m_boneAnimations[0].m_frames.size();
	float ratio = (m_timeA * m_fps) - std::floor(m_timeA * m_fps);

	//extract transforms
	vector<KeyFrame>	animationSkeleton, nextAnimationSkeleton;
	ExtractFrameSkeleton(animationSkeleton, *m_animationA, currentFrame);
	ExtractFrameSkeleton(nextAnimationSkeleton, *m_animationA, nextFrame);

	vector<KeyFrame> interlopedSkeleton;
	InterlopeSkeletons(interlopedSkeleton, animationSkeleton, nextAnimationSkeleton, ratio);

	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, interlopedSkeleton);

	//transforms -> matrix
	vector<Matrix4>		SkeletonMatrices;
	ApplyOffset(SkeletonMatrices, globalSkeleton);

	//send to shader (GPU)
	SendSkeletonMatrixToGPU(SkeletonMatrices);
}

void Animator::PlayCrossFadeAnimation(float deltaTime)
{
	m_timeA += deltaTime;

	//get cross fade data
	float crossFadePercentage = m_timeA / m_crossFadeDuration;
	if (crossFadePercentage > 1.f) crossFadePercentage = 1.f; 

	int nbFrameAnimationA = m_animationA->m_boneAnimations.size();
	int nbFrameAnimationB = m_animationB->m_boneAnimations.size();

	//get phase
	float	aps = 1.0f + (nbFrameAnimationA / nbFrameAnimationB - 1.0f) * crossFadePercentage;
	m_crossFadeElapseTime += deltaTime * aps;
	float	elapseFrame = m_crossFadeElapseTime * 30.0f;
	float	phase = fmod(elapseFrame, nbFrameAnimationA) / nbFrameAnimationA;

	//get the first animation skeleton
	float	frame = phase * nbFrameAnimationA;
	int		prevIndex = static_cast<int>(frame) % nbFrameAnimationA;
	int		nextIndex = (prevIndex + 1) % nbFrameAnimationA;
	float	ratio = frame - std::floor(frame);

	vector<KeyFrame>	animationSkeleton, nextAnimationSkeleton;
	ExtractFrameSkeleton(animationSkeleton, *m_animationA, prevIndex);
	ExtractFrameSkeleton(nextAnimationSkeleton, *m_animationA, nextIndex);

	std::vector<KeyFrame>	AnimationASkeleton;
	InterlopeSkeletons(AnimationASkeleton, animationSkeleton, nextAnimationSkeleton, ratio);

	//get the second animation skeleton
	frame = phase * nbFrameAnimationB;
	prevIndex = static_cast<int>(frame) % nbFrameAnimationB;
	nextIndex = (prevIndex + 1) % nbFrameAnimationB;
	ratio = frame - std::floor(frame);

	ExtractFrameSkeleton(animationSkeleton, *m_animationB, prevIndex);
	ExtractFrameSkeleton(nextAnimationSkeleton, *m_animationB, nextIndex);

	std::vector<KeyFrame>	AnimationBSkeleton;
	InterlopeSkeletons(AnimationBSkeleton, animationSkeleton, nextAnimationSkeleton, ratio);

	//cross fade
	std::vector<KeyFrame>	crossFadedSkeleton;
	InterlopeSkeletons(crossFadedSkeleton, AnimationASkeleton, AnimationBSkeleton, crossFadePercentage);

	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, crossFadedSkeleton);

	//transforms -> matrix
	vector<Matrix4>		SkeletonMatrices;
	ApplyOffset(SkeletonMatrices, globalSkeleton);

	//send to shader (GPU)
	SendSkeletonMatrixToGPU(SkeletonMatrices);
}

