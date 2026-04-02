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

		void Stop();

		void PlayRepeat(int animationIndex);													//Play_single_animation_repeat
		void PlayRepeatInterpolated(int animationIndex);										//Play_single_animation_repeat_interpolated
		void PlayCrossFade(int animationIndexA, int animationIndexB, float crossfadeTime);		//Play_cross_fade_animation

	private:
		enum AnimatorState
		{
			//Play + animation type + repeat or once? + interpolated?

			Waiting_for_loaded_ressources,
			Play_no_animation,
			Play_single_animation_repeat,
			Play_single_animation_repeat_interpolated,
			Play_cross_fade_animation
		};

		void WaitForResourcesToBeLoaded();

		//help
		void ExtractFrameSkeleton(vector<KeyFrame>& skeleton_out, const Animation& animation, int currentFrame);
		void GetSkeletonToGlobal(vector<KeyFrame>& skeleton_out, vector<KeyFrame>& localSkeleton);
		void ApplyOffset(vector<Matrix4>& skeletonMatrix_out, const vector<KeyFrame>& globalTransform);
		void InterlopeSkeletons(vector<KeyFrame>& interlopedSkeleton_out, vector<KeyFrame>& skeletonA, vector<KeyFrame>& skeletonB, const float& t);

		//gpu
		void SendSkeletonMatrixToGPU(vector<Matrix4>& skeletonMatrix);

		//animations
		void PlaySingleAnimation(float deltaTime);						//Play_single_animation_repeat
		void PlaySingleAnimationInterpolated(float deltaTime);			//Play_single_animation_repeat_interpolated
		void PlayCrossFadeAnimation(float deltaTime);	//Play_cross_fade_animation

		//
		AnimatorState			m_state = Waiting_for_loaded_ressources;
		AnimatorState			m_nextState = Play_no_animation;
		int						m_animationIndexA = -1;
		int						m_animationIndexB = -1;

		//shadder
		ResourceHandle<Shader>	m_shader;

		//speed
		float					m_fps = 30.f;

		//mesh infos
		ResourceHandle<Mesh>	m_mesh;
		int						m_boneCount = 0;
		
		//elapse times
		float					m_timeA = 0.0f;

		//animations
		const Animation*		m_animationA = nullptr; //in cross fade, the original animation
		const Animation*		m_animationB = nullptr;	//in cross fade, the next animation

		//cross fade
		float					m_crossFadeDuration = 0.f;
		float					m_crossFadeElapseTime = 0.f;
	};
}

#endif // !ANIMATOR
