#include "Animator.h"

#include "Log.h"

using namespace Apex::Rendering;

using Apex::Component;
using Apex::ExposedVar;

void Animator::Serialize(std::ostream& out) const
{
	out << "        \"fps\": " << m_fps << ",\n";
	out << "		\"model\": \"" << m_model->GetPath() << "\"\n";
}


unique_ptr<Component> Animator::Clone()
{
	auto clone = std::make_unique<Animator>(*this);
	return clone;
}

std::vector<ExposedVar> Animator::GetExposedVariables()
{
	return
	{
		{ "FPS", ExposedVar::Float, &m_fps },
	};
}

void Animator::SetModel(ResourceHandle<Model> model)
{
	m_model = model;

	m_boneCount = static_cast<int>(m_model->m_skeleton.size());
	m_skeletonMatrix.resize(m_boneCount);
}

void Animator::SetFPS(float fps)
{
	m_fps = fps;
}

void Animator::OnUpdate(float deltaTime)
{
	switch (m_state)
	{
	case WaitingForLoadedResources:
		WaitForResourcesToBeLoaded();
		break;

	case PlaySingleAnim:
		PlaySingleAnimation(deltaTime);
		break;

	case PlaySingleAnimInterpolated:
		PlaySingleAnimationInterpolated(deltaTime);
		break;

	case PlayCrossFadeAnim:
		PlayCrossFadeAnimation(deltaTime);
		break;

	case PlayCrossFadeAnimInterpolated:
		PlayCrossFadeAnimationInterpolated(deltaTime);
		break;

	case PlayBlendSpaceAnim:
		PlayBlendedAnimation(deltaTime);
		break;

	case PlayBlendSpaceAnimInterpolated:
		PlayBlendedAnimationInterpolated(deltaTime);
		break;

	case PlayLayerBlendSpaceAnim:
		PlayLayerBlendAnimation(deltaTime);
		break;

	case PlayNoAnim:
	default:
		break;
	}
}

void Animator::Stop()
{
	m_nextState = PlayNoAnim;
}

void Animator::PlayRepeat(int animationIndex)
{
	if (m_state == WaitingForLoadedResources)
	{
		m_animationIndexA = animationIndex;
		m_nextState = PlaySingleAnim;
	}
	else
	{
		m_animationA = &m_model->GetAnimations()[animationIndex];
		m_elapse = 0.f;
		m_state = PlaySingleAnim;
	}
}

void Animator::PlayRepeatInterpolated(int animationIndex)
{
	if (m_state == WaitingForLoadedResources)
	{
		m_animationIndexA = animationIndex;
		m_nextState = PlaySingleAnimInterpolated;
	}
	else
	{
		m_animationA = &m_model->GetAnimations()[animationIndex];
		m_elapse = 0.f;
		m_state = PlaySingleAnimInterpolated;
	}
}

void Animator::PlayCrossFade(int animationIndexA, int animationIndexB, float crossfadeTime)
{
	if (crossfadeTime <= 0.f) return;							//check if the cross fade time isn't negatif

	if (m_state == WaitingForLoadedResources)				//if the ressources are not already loaded 
	{
		m_animationIndexA = animationIndexA;
		m_animationIndexB = animationIndexB;

		m_nextState = PlayCrossFadeAnim;
	}
	else
	{
		m_animationA = &m_model->GetAnimations()[animationIndexA];
		m_animationB = &m_model->GetAnimations()[animationIndexB];

		m_state = PlayCrossFadeAnim;

		m_elapse = 0.f;
		m_crossFadeElapseTime = 0;
	}

	m_crossFadeDuration = crossfadeTime;
}

void Animator::PlayCrossFadeInterpolated(int animationIndexA, int animationIndexB, float crossfadeTime)
{
	if (crossfadeTime <= 0.f) return;							//check if the cross fade time isn't negatif

	if (m_state == WaitingForLoadedResources)				//if the ressources are not already loaded 
	{
		m_animationIndexA = animationIndexA;
		m_animationIndexB = animationIndexB;

		m_nextState = PlayCrossFadeAnimInterpolated;
	}
	else
	{
		m_animationA = &m_model->GetAnimations()[animationIndexA];
		m_animationB = &m_model->GetAnimations()[animationIndexB];

		m_state = PlayCrossFadeAnimInterpolated;

		m_elapse = 0.f;
		m_crossFadeElapseTime = 0;
	}

	m_crossFadeDuration = crossfadeTime;
}

void Animator::PlayBlended(int animationIndexA, int animationIndexB, float ratio)
{
	if (ratio < 0.f || ratio > 1.f) return;							//check if the ratio is between 0% and 100%

	if (m_state == WaitingForLoadedResources)					//if the ressources are not already loaded 
	{
		m_animationIndexA = animationIndexA;
		m_animationIndexB = animationIndexB;

		m_nextState = PlayBlendSpaceAnim;
	}
	else
	{
		m_animationA = &m_model->GetAnimations()[animationIndexA];
		m_animationB = &m_model->GetAnimations()[animationIndexB];

		m_state = PlayBlendSpaceAnim;

		m_elapse = 0.f;
	}

	m_blendRatio = ratio;
}

void Animator::PlayBlendedInterpolated(int animationIndexA, int animationIndexB, float ratio)
{
	if (ratio < 0.f || ratio > 1.f) return;							//check if the ratio is between 0% and 100%

	if (m_state == WaitingForLoadedResources)					//if the ressources are not already loaded 
	{
		m_animationIndexA = animationIndexA;
		m_animationIndexB = animationIndexB;

		m_nextState = PlayBlendSpaceAnimInterpolated;
	}
	else
	{
		m_animationA = &m_model->GetAnimations()[animationIndexA];
		m_animationB = &m_model->GetAnimations()[animationIndexB];

		m_state = PlayBlendSpaceAnimInterpolated;

		m_elapse = 0.f;
	}

	m_blendRatio = ratio;
}

void Animator::PlayeLayerBlend(int animationIndexA, int animationIndexB, int origine)
{
	if (m_state == WaitingForLoadedResources)					//if the ressources are not already loaded 
	{
		m_animationIndexA = animationIndexA;
		m_animationIndexB = animationIndexB;

		m_nextState = PlayLayerBlendSpaceAnim;

		m_layerBendingOrigin = origine;
	}
	else
	{
		if (origine < 0 || origine > m_boneCount) return;		//check if the origine of the layer blend is valid

		m_layerBendingOrigin = origine;

		m_animationA = &m_model->GetAnimations()[animationIndexA];
		m_animationB = &m_model->GetAnimations()[animationIndexB];

		m_state = PlayLayerBlendSpaceAnim;

		m_elapse = 0.f;
	}
}

int Animator::AddIKInstance(int endBoneIndex, int chainLength)
{
	if (endBoneIndex < 0 || chainLength <= 0) return -1;

	//check if the lenght is right
	int currentBoneIndex = endBoneIndex;

	for (int chainNode = 0; chainNode < chainLength; chainNode++)
	{
		if (m_model->m_skeleton[currentBoneIndex].m_parentIndex <= -1)
		{
			return -1;
		}

		currentBoneIndex = m_model->m_skeleton[currentBoneIndex].m_parentIndex;
	}

	//create the IKInstace
	IKInstance ikInstance = {true, endBoneIndex, chainLength, LibMath::Vector3(0.f)};
	m_IKInstances.push_back(ikInstance);

	//return the IKIstance index
	return m_IKInstances.size() - 1;		
}

void Animator::SetIKTargetPosition(int index, LibMath::Vector3 newTargetPosition)
{
	if (index < 0 || index >= m_IKInstances.size()) return;

	m_IKInstances[index].m_targetPosition = newTargetPosition;
}

void Animator::ActivateIKInstace(int index)
{
	if (index < 0 || index >= m_IKInstances.size()) return;

	m_IKInstances[index].m_active = true;
}

void Animator::DeactivateIKInstace(int index)
{
	if (index < 0 || index >= m_IKInstances.size()) return;

	m_IKInstances[index].m_active = false;
}

void Animator::WaitForResourcesToBeLoaded()
{
	//check if the mesh is loaded
	if (m_model.IsReady())
	{
		m_boneCount = static_cast<int>(m_model->m_skeleton.size());
		m_skeletonMatrix.resize(m_boneCount);
		for (size_t i = 0; i < m_boneCount; i++)
		{
			m_skeletonMatrix[i] = m_model->m_skeleton[i].m_offsetMatrix;
		}

		m_state = m_nextState;

		if (m_animationIndexA != -1)
		{
			m_animationA = &m_model->GetAnimations()[m_animationIndexA];
		}
		if (m_animationIndexB != -1)
		{
			m_animationB = &m_model->GetAnimations()[m_animationIndexB];
		}
		if (m_layerBendingOrigin < 0 || m_layerBendingOrigin > m_boneCount)
		{
			m_state = PlayNoAnim;
		}
	}
}

void Animator::ExtractFrameSkeleton(vector<KeyFrame>& skeleton_out, const Animation& animation, int currentFrame) const
{
	skeleton_out.resize(m_boneCount);

	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		skeleton_out[bone] = animation.m_boneAnimations[bone].m_frames[currentFrame];
	}
}

void Animator::RemoveRootMotion(vector<KeyFrame>& localSkeleton)
{
	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		if (m_model->m_skeleton[bone].m_parentIndex == -1)
		{
			localSkeleton[bone].m_position = { 0.f, 0.f, 0.f };
			// optionally also lock rotation:
			// localSkeleton[bone].m_rotation = Quaternion::identity();
		}
	}
}

void Animator::GetSkeletonToGlobal(vector<KeyFrame>& skeleton_out, const vector<KeyFrame>& localSkeleton)
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
		int	parent = m_model->m_skeleton[bone].m_parentIndex;

		assert(parent < bone);

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

void Animator::ApplyOffset(const vector<KeyFrame>& globalTransform)
{
	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		m_skeletonMatrix[bone] = globalTransform[bone].m_matrix * m_model->m_skeleton[bone].m_offsetMatrix;
	}
}

void Animator::InterpoleSkeletons(
	vector<KeyFrame>& interlopedSkeleton_out, const vector<KeyFrame>& skeletonA, 
	const vector<KeyFrame>& skeletonB, float t) const
{
	interlopedSkeleton_out.resize(m_boneCount);

	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		//position
		interlopedSkeleton_out[bone].m_position = LibMath::Vector3::lerp(skeletonA[bone].m_position, skeletonB[bone].m_position, t);

		//rotation
		interlopedSkeleton_out[bone].m_rotation = LibMath::Quaternion::slerp(skeletonA[bone].m_rotation, skeletonB[bone].m_rotation, t);

		//scale
		interlopedSkeleton_out[bone].m_scale = LibMath::Vector3::lerp(skeletonA[bone].m_scale, skeletonB[bone].m_scale, t);
	}
}

void Animator::ReplaceBones(
	vector<KeyFrame>& skeleton_out, const vector<KeyFrame>& skeletonA, const vector<KeyFrame>& skeletonB) const
{
	skeleton_out.resize(m_boneCount);

	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		if (bone >= m_layerBendingOrigin)
		{
			skeleton_out[bone] = skeletonB[bone];
		}
		else
		{
			skeleton_out[bone] = skeletonA[bone];
		}
	}
}

LibMath::Vector3 Animator::GetPos(Matrix4& m)
{
	return LibMath::Vector3(m[3][0], m[3][1], m[3][2]);
}

void Animator::SetPos(Matrix4& m, const LibMath::Vector3& p)
{	
	m[3][0] = p[0];	//x
	m[3][1] = p[1];	//y
	m[3][2] = p[2];	//z
}

void Animator::ApllyIKFabrik(vector<KeyFrame>& globalSkeleton, const IKInstance& ik)
{
	if (!ik.m_active) return;

	//build chain
	std::vector<int>	chain;
	int					current = ik.m_endBoneIndex;

	for (int i = 0; i < ik.m_chainLength; i++)
	{
		if (current < 0)
			return;

		chain.push_back(current);
		current = m_model->m_skeleton[current].m_parentIndex;
	}

	const int count = (int)chain.size();

	//extract positions
	std::vector<LibMath::Vector3> position(count);
	for (int i = 0; i < count; i++)
	{
		position[i] = GetPos(globalSkeleton[chain[i]].m_matrix);
	}

	//get lenght
	std::vector<float> lengths(count - 1);
	float totalLength = 0.f;

	for (int i = 0; i < count - 1; i++)
	{
		lengths[i] = (position[i] - position[i + 1]).magnitude();
		totalLength += lengths[i];
	}

	//Fabrix
	LibMath::Vector3 rootPos = position[count - 1];
	LibMath::Vector3 target = ik.m_targetPosition;

	const int iterations = 10;

	if ((target - rootPos).magnitude() > totalLength)
	{
		// unreachable → stretch
		for (int i = count - 2; i >= 0; i--)
		{
			LibMath::Vector3 dir = (target - position[i + 1]).normalized();
			position[i] = position[i + 1] + dir * lengths[i];
		}
	}
	else
	{
		for (int iter = 0; iter < iterations; iter++)
		{
			// backward
			position[0] = target;
			for (int i = 1; i < count; i++)
			{
				LibMath::Vector3 dir = (position[i] - position[i - 1]).normalized();
				position[i] = position[i - 1] + dir * lengths[i - 1];
			}

			// forward
			position[count - 1] = rootPos;
			for (int i = count - 2; i >= 0; i--)
			{
				LibMath::Vector3 dir = (position[i] - position[i + 1]).normalized();
				position[i] = position[i + 1] + dir * lengths[i];
			}
		}
	}

	//replace POsition
	for (int i = 0; i < count; i++)
	{
		SetPos(globalSkeleton[chain[i]].m_matrix, position[i]);
	}

}

void Animator::SendSkeletonMatrixToGPU(const ResourceHandle<ComputeShader>& shader)
{
	if (m_skeletonMatrix.size() > 0)
		shader->SetUniform("u_Bones", m_skeletonMatrix[0], (int)m_skeletonMatrix.size());
}

void Animator::PlaySingleAnimation(float deltaTime)
{
	m_elapse += deltaTime;
	
	//get current frame
	int currentFrame = static_cast<int>(m_elapse * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();

	//extract transforms
	vector<KeyFrame> animationSkeleton;
	ExtractFrameSkeleton(animationSkeleton, *m_animationA, currentFrame);

	RemoveRootMotion(animationSkeleton);

	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, animationSkeleton);
	//transforms -> matrix
	ApplyOffset(globalSkeleton);
}

void Animator::PlaySingleAnimationInterpolated(float deltaTime)
{
	m_elapse += deltaTime;

	//get current frame
	int currentFrame = static_cast<int>(m_elapse * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();
	int nextFrame = (currentFrame + 1) % m_animationA->m_boneAnimations[0].m_frames.size();
	float ratio = (m_elapse * m_fps) - std::floor(m_elapse * m_fps);

	//extract transforms
	vector<KeyFrame> animationSkeleton, nextAnimationSkeleton;
	ExtractFrameSkeleton(animationSkeleton, *m_animationA, currentFrame);
	ExtractFrameSkeleton(nextAnimationSkeleton, *m_animationA, nextFrame);

	vector<KeyFrame> interlopedSkeleton;
	InterpoleSkeletons(interlopedSkeleton, animationSkeleton, nextAnimationSkeleton, ratio);

	RemoveRootMotion(interlopedSkeleton);

	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, interlopedSkeleton);

	//transforms -> matrix
	ApplyOffset(globalSkeleton);
}

void Animator::PlayCrossFadeAnimation(float deltaTime)
{
	m_elapse += deltaTime;

	//get cross fade data
	float crossFadePercentage = m_elapse / m_crossFadeDuration;
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
	InterpoleSkeletons(crossFadedSkeleton, animationSkeletonA, animationSkeletonB, crossFadePercentage);

	RemoveRootMotion(crossFadedSkeleton);

	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, crossFadedSkeleton);

	//transforms -> matrix
	ApplyOffset(globalSkeleton);
}

void Animator::PlayCrossFadeAnimationInterpolated(float deltaTime)
{
	m_elapse += deltaTime;

	//get cross fade data
	float crossFadePercentage = m_elapse / m_crossFadeDuration;
	if (crossFadePercentage > 1.f) crossFadePercentage = 1.f; 

	int nbFrameAnimationA = m_animationA->m_boneAnimations[0].m_frames.size();
	int nbFrameAnimationB = m_animationB->m_boneAnimations[0].m_frames.size();

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
	InterpoleSkeletons(AnimationASkeleton, animationSkeleton, nextAnimationSkeleton, ratio);

	//get the second animation skeleton
	frame = phase * nbFrameAnimationB;
	prevIndex = static_cast<int>(frame) % nbFrameAnimationB;
	nextIndex = (prevIndex + 1) % nbFrameAnimationB;
	ratio = frame - std::floor(frame);

	ExtractFrameSkeleton(animationSkeleton, *m_animationB, prevIndex);
	ExtractFrameSkeleton(nextAnimationSkeleton, *m_animationB, nextIndex);

	std::vector<KeyFrame> AnimationBSkeleton;
	InterpoleSkeletons(AnimationBSkeleton, animationSkeleton, nextAnimationSkeleton, ratio);

	//cross fade
	std::vector<KeyFrame> crossFadedSkeleton;
	InterpoleSkeletons(crossFadedSkeleton, AnimationASkeleton, AnimationBSkeleton, crossFadePercentage);

	RemoveRootMotion(crossFadedSkeleton);

	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, crossFadedSkeleton);

	//transforms -> matrix
	ApplyOffset(globalSkeleton);
}

void Animator::PlayBlendedAnimation(float deltaTime)
{
	m_elapse += deltaTime;

	//get animations frame
	int animationAFrame = static_cast<int>(m_elapse * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();
	int animationBFrame = static_cast<int>(m_elapse * m_fps) % m_animationB->m_boneAnimations[0].m_frames.size();

	//extract the animations
	vector<KeyFrame> animationASkeleton, animationBSkeleton;
	ExtractFrameSkeleton(animationASkeleton, *m_animationA, animationAFrame);
	ExtractFrameSkeleton(animationBSkeleton, *m_animationB, animationBFrame);

	//blend
	std::vector<KeyFrame> blendedSkeleton;
	InterpoleSkeletons(blendedSkeleton, animationASkeleton, animationBSkeleton, m_blendRatio);

	RemoveRootMotion(blendedSkeleton);

	//local -> global
	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, blendedSkeleton);

	//transforms -> matrix
	ApplyOffset(globalSkeleton);
}

void Animator::PlayBlendedAnimationInterpolated(float deltaTime)
{
	m_elapse += deltaTime;

	//get animations frame
	int animationACurrentFrame = static_cast<int>(m_elapse * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();
	int animationANextFrame = (animationACurrentFrame + 1) % m_animationA->m_boneAnimations[0].m_frames.size();
	int animationBCurrentFrame = static_cast<int>(m_elapse * m_fps) % m_animationB->m_boneAnimations[0].m_frames.size();
	int animationBNextFrame = (animationBCurrentFrame + 1) % m_animationB->m_boneAnimations[0].m_frames.size();

	float ratio = (m_elapse * m_fps) - std::floor(m_elapse * m_fps);

	//extract the animations
	vector<KeyFrame> animationACurrentSkeleton, animationANextSkeleton, animationBCurrentSkeleton, animationBNextSkeleton;
	ExtractFrameSkeleton(animationACurrentSkeleton, *m_animationA, animationACurrentFrame);
	ExtractFrameSkeleton(animationANextSkeleton, *m_animationA, animationANextFrame);
	ExtractFrameSkeleton(animationBCurrentSkeleton, *m_animationB, animationBCurrentFrame);
	ExtractFrameSkeleton(animationBNextSkeleton, *m_animationB, animationBNextFrame);

	//interpolate skeletons
	vector<KeyFrame> animationAInterpolatedSkeleton, animationBInterpolatedSkeleton;
	InterpoleSkeletons(animationAInterpolatedSkeleton, animationACurrentSkeleton, animationANextSkeleton, ratio);
	InterpoleSkeletons(animationBInterpolatedSkeleton, animationBCurrentSkeleton, animationBNextSkeleton, ratio);

	//blend
	std::vector<KeyFrame> blendedSkeleton;
	InterpoleSkeletons(blendedSkeleton, animationAInterpolatedSkeleton, animationBInterpolatedSkeleton, m_blendRatio);

	RemoveRootMotion(blendedSkeleton);

	//local -> global
	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, blendedSkeleton);

	//transforms -> matrix
	ApplyOffset(globalSkeleton);
}

void Animator::PlayLayerBlendAnimation(float deltaTime)
{
	m_elapse += deltaTime;

	//get animations frame
	int animationAFrame = static_cast<int>(m_elapse * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();
	int animationBFrame = static_cast<int>(m_elapse * m_fps) % m_animationB->m_boneAnimations[0].m_frames.size();

	//extract the animations
	vector<KeyFrame> animationASkeleton, animationBSkeleton;
	ExtractFrameSkeleton(animationASkeleton, *m_animationA, animationAFrame);
	ExtractFrameSkeleton(animationBSkeleton, *m_animationB, animationBFrame);

	//layer blend
	vector<KeyFrame> LayberBlendedSkeleton;
	ReplaceBones(LayberBlendedSkeleton, animationASkeleton, animationBSkeleton);

	//local -> global
	vector<KeyFrame> globalSkeleton;
	GetSkeletonToGlobal(globalSkeleton, LayberBlendedSkeleton);

	//transforms -> matrix
	ApplyOffset(globalSkeleton);
}

void Animator::ApllyIKInstances(vector<KeyFrame>& globalSkeleton)
{
	for (const IKInstance& instance : m_IKInstances)
	{
		if (instance.m_active)
		{
			ApllyIKFabrik(globalSkeleton, instance);
		}
	}
}

