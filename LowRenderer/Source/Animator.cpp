#include "Animator.h"

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
	if (m_state == WAITING_FOR_LOADED_RESSOURCE)
	{
		if (m_mesh.IsReady() && m_shader.IsReady())
		{
			m_boneCount = static_cast<int>(m_mesh->m_bones.size());

			if (m_nextState != WAITING_FOR_LOADED_RESSOURCE)
			{
				m_state = m_nextState;
			}
			else
			{
				m_state = PLAY_NO_ANIMATION;
			}

			if (m_animationIndex != -1)
			{
				m_animationA = &m_mesh->m_animations[m_animationIndex];
			}
		}
		else
		{
			return;
		}
	}

	if (m_state == PLAY_SINGLE_ANIMATION)
	{
		printf("ready");

		m_timeA += deltaTime;

		PlaySingleAnimation();
	}

	//add cross fade here

	//add callable functions in function of the animation called 1 animation / 2 animations / cross fade
}

void Animator::Play(int animationIndex)
{
	if (m_state == WAITING_FOR_LOADED_RESSOURCE)
	{
		m_nextState = PLAY_SINGLE_ANIMATION;
		m_animationIndex = animationIndex;
	}
	else
	{
		m_animationA = &m_mesh->m_animations[animationIndex];
		m_timeA = 0.f;

		m_state = PLAY_SINGLE_ANIMATION;
	}
}

void Animator::ExtractTransforms(vector<KeyFrame>& keyframe_out, const Animation& animation, int currentFrame)
{
	keyframe_out.resize(m_boneCount);

	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		keyframe_out[bone] = animation.m_boneAnimations[bone].m_frames[currentFrame];
	}
}

void Animator::ConvertSkeletonToMatrix(vector<Matrix4>& skeletonMatrix_out, const vector<KeyFrame>& globalTransform)
{
	skeletonMatrix_out.resize(m_boneCount);

	Matrix4 scaleMatrix = Matrix4::createScale(Vector3(100.f));

	for (int bone = 0; bone < m_boneCount; ++bone)
	{
		skeletonMatrix_out[bone] = scaleMatrix * globalTransform[bone].m_matrix * m_mesh->m_bones[bone].offsetMatrix;
	}
}

void Animator::SendSkeletonToGPU(vector<Matrix4>& skeletonMatrix)
{	
	m_shader->SetUniformMat4Array("u_Bones[0]", skeletonMatrix);
}

void Animator::PlaySingleAnimation()
{
	//get current frame
	int currentFrame = static_cast<int>(m_timeA * m_fps) % m_animationA->m_boneAnimations[0].m_frames.size();

	//extract transforms
	vector<KeyFrame>	localTransforms;
	ExtractTransforms(localTransforms, *m_animationA, currentFrame);

	//transforms -> matrix
	vector<Matrix4>		SkeletonMatrices;
	ConvertSkeletonToMatrix(SkeletonMatrices, localTransforms);

	//send to shader (GPU)
	SendSkeletonToGPU(SkeletonMatrices);
}
