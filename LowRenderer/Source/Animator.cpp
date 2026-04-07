#include "Animator.h"

#include <cmath>

using namespace Apex::Rendering;

Animator::Animator(ResourceHandle<Mesh> mesh, ResourceHandle<Shader> shader, float fps) :
	m_mesh(mesh), m_shader(shader), m_fps(fps) {}

void Animator::SetMesh(ResourceHandle<Mesh> mesh)
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

	case Play_single_animation:
		PlaySingleAnimation(deltaTime);
		break;

	case Play_single_animation_interpolated:
		PlaySingleAnimationInterpolated(deltaTime);
		break;

	case Play_cross_fade_animation:
		PlayCrossFadeAnimation(deltaTime);
		break;

	case Play_cross_fade_animation_interpolated:
		PlayCrossFadeAnimationInterpolated(deltaTime);
		break;

	case Play_blend_space_animation:
		PlayBlendedAnimation(deltaTime);
		break;

	case Play_blend_space_animation_interpolated:
		PlayBlendedAnimationInterpolated(deltaTime);
		break;

	case Play_layer_blend_space_animation:
		PlayLayerBlendAnimation(deltaTime);
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
		m_nextState = Play_single_animation;
	}
	else
	{
		m_animationA = &m_mesh->m_animations[animationIndex];
		m_timeA = 0.f;
		m_state = Play_single_animation;
	}
}

void Animator::PlayRepeatInterpolated(int animationIndex)
{
	if (m_state == Waiting_for_loaded_ressources)
	{
		m_animationIndexA = animationIndex;
		m_nextState = Play_single_animation_interpolated;
	}
	else
	{
		m_animationA = &m_mesh->m_animations[animationIndex];
		m_timeA = 0.f;
		m_state = Play_single_animation_interpolated;
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

void Animator::PlayCrossFadeInterpolated(int animationIndexA, int animationIndexB, float crossfadeTime)
{
	if (m_crossFadeDuration <= 0.f) return;							//check if the cross fade time isn't negatif

	if (m_state == Waiting_for_loaded_ressources)				//if the ressources are not already loaded 
	{
		m_animationIndexA = animationIndexA;
		m_animationIndexB = animationIndexB;

		m_nextState = Play_cross_fade_animation_interpolated;
	}
	else
	{
		m_animationA = &m_mesh->m_animations[animationIndexA];
		m_animationB = &m_mesh->m_animations[animationIndexB];

		m_state = Play_cross_fade_animation_interpolated;

		m_timeA = 0.f;
		m_crossFadeElapseTime = 0;
	}

	m_crossFadeDuration = crossfadeTime;
}

void Animator::PlayBlended(int animationIndexA, int animationIndexB, float ratio)
{
	if (ratio < 0.f || ratio > 1.f) return;							//check if the ratio is between 0% and 100%

	if (m_state == Waiting_for_loaded_ressources)					//if the ressources are not already loaded 
	{
		m_animationIndexA = animationIndexA;
		m_animationIndexB = animationIndexB;

		m_nextState = Play_blend_space_animation;
	}
	else
	{
		m_animationA = &m_mesh->m_animations[animationIndexA];
		m_animationB = &m_mesh->m_animations[animationIndexB];

		m_state = Play_blend_space_animation;

		m_timeA = 0.f;
	}

	m_blendRatio = ratio;
}

void Animator::PlayBlendedInterpolated(int animationIndexA, int animationIndexB, float ratio)
{
	if (ratio < 0.f || ratio > 1.f) return;							//check if the ratio is between 0% and 100%

	if (m_state == Waiting_for_loaded_ressources)					//if the ressources are not already loaded 
	{
		m_animationIndexA = animationIndexA;
		m_animationIndexB = animationIndexB;

		m_nextState = Play_blend_space_animation_interpolated;
	}
	else
	{
		m_animationA = &m_mesh->m_animations[animationIndexA];
		m_animationB = &m_mesh->m_animations[animationIndexB];

		m_state = Play_blend_space_animation_interpolated;

		m_timeA = 0.f;
	}

	m_blendRatio = ratio;
}

void Animator::PlayeLayerBlend(int animationIndexA, int animationIndexB, int origine)
{
	if (m_state == Waiting_for_loaded_ressources)					//if the ressources are not already loaded 
	{
		m_animationIndexA = animationIndexA;
		m_animationIndexB = animationIndexB;

		m_nextState = Play_layer_blend_space_animation;

		m_layerBendingOrigine = origine;
	}
	else
	{
		if (origine < 0 || origine > m_boneCount) return;		//check if the origine of the layer blend is valid

		m_layerBendingOrigine = origine;

		m_animationA = &m_mesh->m_animations[animationIndexA];
		m_animationB = &m_mesh->m_animations[animationIndexB];

		m_state = Play_layer_blend_space_animation;

		m_timeA = 0.f;
	}
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
		if (m_layerBendingOrigine < 0 || m_layerBendingOrigine > m_boneCount)
		{
			m_state = Play_no_animation;
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

void Animator::ReplaceBonese(vector<KeyFrame>& skeleton_out, vector<KeyFrame>& skeletonA, vector<KeyFrame>& skeletonB)
{
	skeleton_out.resize(m_boneCount);

	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		if (bone >= m_layerBendingOrigine)
		{
			skeleton_out[bone] = skeletonB[bone];
		}
		else
		{
			skeleton_out[bone] = skeletonA[bone];
		}
	}
}

void Animator::SendSkeletonMatrixToGPU(vector<Matrix4>& skeletonMatrix)
{	
	m_shader->SetUniform("u_Bones[0]", skeletonMatrix[0], (int)skeletonMatrix.size());
}

void Animator::PlaySingleAnimation(float deltaTime)
{
	m_timeA += deltaTime;

	//get current frame
	int currentFrame = static_cast<int>(m_timeA * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();

	//extract transforms
	vector<KeyFrame> animationSkeleton;
	ExtractFrameSkeleton(animationSkeleton, *m_animationA, currentFrame);

	//transforms -> matrix
	vector<Matrix4>	SkeletonMatrices;
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
	vector<KeyFrame> animationSkeleton, nextAnimationSkeleton;
	ExtractFrameSkeleton(animationSkeleton, *m_animationA, currentFrame);
	ExtractFrameSkeleton(nextAnimationSkeleton, *m_animationA, nextFrame);

	vector<KeyFrame> interlopedSkeleton;
	InterlopeSkeletons(interlopedSkeleton, animationSkeleton, nextAnimationSkeleton, ratio);

	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, interlopedSkeleton);

	//transforms -> matrix
	vector<Matrix4>	SkeletonMatrices;
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
	float	ratio = frame - std::floor(frame);

	vector<KeyFrame> animationSkeletonA;
	ExtractFrameSkeleton(animationSkeletonA, *m_animationA, prevIndex);

	//get the second animation skeleton
	frame = phase * nbFrameAnimationB;
	prevIndex = static_cast<int>(frame) % nbFrameAnimationB;
	ratio = frame - std::floor(frame);

	vector<KeyFrame> animationSkeletonB;
	ExtractFrameSkeleton(animationSkeletonB, *m_animationB, prevIndex);

	//cross fade
	std::vector<KeyFrame> crossFadedSkeleton;
	InterlopeSkeletons(crossFadedSkeleton, animationSkeletonA, animationSkeletonB, crossFadePercentage);

	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, crossFadedSkeleton);

	//transforms -> matrix
	vector<Matrix4>	SkeletonMatrices;
	ApplyOffset(SkeletonMatrices, globalSkeleton);

	//send to shader (GPU)
	SendSkeletonMatrixToGPU(SkeletonMatrices);
}

void Animator::PlayCrossFadeAnimationInterpolated(float deltaTime)
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

	vector<KeyFrame> animationSkeleton, nextAnimationSkeleton;
	ExtractFrameSkeleton(animationSkeleton, *m_animationA, prevIndex);
	ExtractFrameSkeleton(nextAnimationSkeleton, *m_animationA, nextIndex);

	std::vector<KeyFrame> AnimationASkeleton;
	InterlopeSkeletons(AnimationASkeleton, animationSkeleton, nextAnimationSkeleton, ratio);

	//get the second animation skeleton
	frame = phase * nbFrameAnimationB;
	prevIndex = static_cast<int>(frame) % nbFrameAnimationB;
	nextIndex = (prevIndex + 1) % nbFrameAnimationB;
	ratio = frame - std::floor(frame);

	ExtractFrameSkeleton(animationSkeleton, *m_animationB, prevIndex);
	ExtractFrameSkeleton(nextAnimationSkeleton, *m_animationB, nextIndex);

	std::vector<KeyFrame> AnimationBSkeleton;
	InterlopeSkeletons(AnimationBSkeleton, animationSkeleton, nextAnimationSkeleton, ratio);

	//cross fade
	std::vector<KeyFrame> crossFadedSkeleton;
	InterlopeSkeletons(crossFadedSkeleton, AnimationASkeleton, AnimationBSkeleton, crossFadePercentage);

	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, crossFadedSkeleton);

	//transforms -> matrix
	vector<Matrix4> SkeletonMatrices;
	ApplyOffset(SkeletonMatrices, globalSkeleton);

	//send to shader (GPU)
	SendSkeletonMatrixToGPU(SkeletonMatrices);
}

void Animator::PlayBlendedAnimation(float deltaTime)
{
	m_timeA += deltaTime;

	//get animations frame
	int animationAFrame = static_cast<int>(m_timeA * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();
	int animationBFrame = static_cast<int>(m_timeA * m_fps) % m_animationB->m_boneAnimations[0].m_frames.size();

	//extract the animations
	vector<KeyFrame> animationASkeleton, animationBSkeleton;
	ExtractFrameSkeleton(animationASkeleton, *m_animationA, animationAFrame);
	ExtractFrameSkeleton(animationBSkeleton, *m_animationB, animationBFrame);

	//blend
	std::vector<KeyFrame> blendedSkeleton;
	InterlopeSkeletons(blendedSkeleton, animationASkeleton, animationBSkeleton, m_blendRatio);

	//local -> global
	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, blendedSkeleton);

	//transforms -> matrix
	vector<Matrix4>	SkeletonMatrices;
	ApplyOffset(SkeletonMatrices, globalSkeleton);

	//send to shader (GPU)
	SendSkeletonMatrixToGPU(SkeletonMatrices);
}

void Animator::PlayBlendedAnimationInterpolated(float deltaTime)
{
	m_timeA += deltaTime;

	//get animations frame
	int animationACurrentFrame = static_cast<int>(m_timeA * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();
	int animationANextFrame = (animationACurrentFrame + 1) % m_animationA->m_boneAnimations[0].m_frames.size();
	int animationBCurrentFrame = static_cast<int>(m_timeA * m_fps) % m_animationB->m_boneAnimations[0].m_frames.size();
	int animationBNextFrame = (animationACurrentFrame + 1) % m_animationB->m_boneAnimations[0].m_frames.size();

	float ratio = (m_timeA * m_fps) - std::floor(m_timeA * m_fps);

	//extract the animations
	vector<KeyFrame> animationACurrentSkeleton, animationANextSkeleton, animationBCurrentSkeleton, animationBNextSkeleton;
	ExtractFrameSkeleton(animationACurrentSkeleton, *m_animationA, animationACurrentFrame);
	ExtractFrameSkeleton(animationANextSkeleton, *m_animationA, animationANextFrame);
	ExtractFrameSkeleton(animationBCurrentSkeleton, *m_animationB, animationBCurrentFrame);
	ExtractFrameSkeleton(animationBNextSkeleton, *m_animationB, animationBNextFrame);

	//interpolate skeletons
	vector<KeyFrame> animationAInterpolatedSkeleton, animationBInterpolatedSkeleton;
	InterlopeSkeletons(animationAInterpolatedSkeleton, animationACurrentSkeleton, animationANextSkeleton, ratio);
	InterlopeSkeletons(animationBInterpolatedSkeleton, animationBCurrentSkeleton, animationBNextSkeleton, ratio);

	//blend
	std::vector<KeyFrame> blendedSkeleton;
	InterlopeSkeletons(blendedSkeleton, animationAInterpolatedSkeleton, animationBInterpolatedSkeleton, m_blendRatio);

	//local -> global
	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, blendedSkeleton);

	//transforms -> matrix
	vector<Matrix4>	SkeletonMatrices;
	ApplyOffset(SkeletonMatrices, globalSkeleton);

	//send to shader (GPU)
	SendSkeletonMatrixToGPU(SkeletonMatrices);
}

void Animator::PlayLayerBlendAnimation(float deltaTime)
{
	m_timeA += deltaTime;

	//get animations frame
	int animationAFrame = static_cast<int>(m_timeA * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();
	int animationBFrame = static_cast<int>(m_timeA * m_fps) % m_animationB->m_boneAnimations[0].m_frames.size();

	//extract the animations
	vector<KeyFrame> animationASkeleton, animationBSkeleton;
	ExtractFrameSkeleton(animationASkeleton, *m_animationA, animationAFrame);
	ExtractFrameSkeleton(animationBSkeleton, *m_animationB, animationBFrame);

	//layer blend
	vector<KeyFrame> LayberBlendedSkeleton;
	ReplaceBonese(LayberBlendedSkeleton, animationASkeleton, animationBSkeleton);

	//local -> global
	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, LayberBlendedSkeleton);

	//transforms -> matrix
	vector<Matrix4>	SkeletonMatrices;
	ApplyOffset(SkeletonMatrices, globalSkeleton);

	//send to shader (GPU)
	SendSkeletonMatrixToGPU(SkeletonMatrices);
}

