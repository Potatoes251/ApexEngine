#ifndef ANIMATOR
#define ANIMATOR

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Quaternion.h"
#include "../../Resources/header/Shader.h"
#include "../../Resources/header/Mesh.h"
#include "../../Resources/header/ResourceManager.h"
#include <memory>

using std::weak_ptr;
using std::shared_ptr;
using Apex::Resources::ResourceHandle;
using std::vector;

namespace Apex::Rendering
{
	enum AnimatorState
	{
		WAITING_FOR_LOADED_RESSOURCE,
		PLAY_NO_ANIMATION,
		PLAY_SINGLE_ANIMATION
	};

	class Animator
	{
	public:
		Animator() = default;
		Animator(ResourceHandle<Mesh> mesh, ResourceHandle<Shader> shader, float fps);
		~Animator() = default;
		Animator(const Animator& other) = delete;
		Animator& operator=(const Animator&) = delete;

		//before update
		void SetMesh(ResourceHandle<Mesh> mesh);
		void SetShader(ResourceHandle<Shader> shader);
		void SetFPS(float fps);				//base fps = 30

		//during update
		void Update(float deltaTime);

		void Play(int animationIndex);

	private:

		void ExtractTransforms(vector<KeyFrame>& keyframe_out, const Animation& animation, int currentFrame);
		void ConvertSkeletonToMatrix(vector<Matrix4>& skeletonMatrix_out, const vector<KeyFrame>& globalTransform);

		void SendSkeletonToGPU(vector<Matrix4>& skeletonMatrix);

		void PlaySingleAnimation();

		AnimatorState			m_state = WAITING_FOR_LOADED_RESSOURCE;
		AnimatorState			m_nextState = WAITING_FOR_LOADED_RESSOURCE;
		int						m_animationIndex = -1;

		ResourceHandle<Shader>	m_shader;
		float					m_fps = 30.f;

		ResourceHandle<Mesh>	m_mesh;
		int						m_boneCount = 0;

		float					m_timeA = 0.0f;
		float					m_timeB = 0.0f;

		const Animation*		m_animationA = nullptr; //in cross fade, the original animation
		const Animation*		m_animationB = nullptr;	//in cross fade, the next animation

		bool					m_crossFade = false;
		bool					m_layerBlend = false;
	};
}

#endif // !ANIMATOR
