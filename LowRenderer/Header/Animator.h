#ifndef ANIMATOR
#define ANIMATOR

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Quaternion.h"
#include "../../Resources/header/Shader.h"
#include "../../Resources/header/Mesh.h"
#include "../../Resources/header/ResourceManager.h"
#include "../../Resources/header/Component.h"
#include <memory>

using std::weak_ptr;
using std::shared_ptr;
using Apex::Resources::ResourceHandle;
using std::vector;
using std::unique_ptr;

namespace Apex::Rendering
{
	class Animator : public Component
	{
	public:
		Animator() = delete;
		Animator(Resources::ResourceManager& resourceManager, ResourceHandle<Mesh> mesh);
		~Animator() = default;
		Animator(const Animator& other);
		Animator& operator=(const Animator&) = default;

		//Component
		void					Serialize(std::ostream& out) const override;
		unique_ptr<Component>	Clone() const override;
		const char*				GetTypeName() const override	{ return "Animator"; };
		std::vector<ExposedVar> GetExposedVariables() override;

		//before update
		void SetMesh(ResourceHandle<Mesh> mesh);
		void SetShader(ResourceHandle<Shader> shader);
		void SetFPS(float fps);				//base fps = 30

		//during update
		void OnUpdate(float deltaTime) override;

		void Stop();

		//Animations
		void PlayRepeat(int animationIndex);															//Play_single_animation
		void PlayRepeatInterpolated(int animationIndex);												//Play_single_animation_interpolated
		void PlayCrossFade(int animationIndexA, int animationIndexB, float crossfadeTime);				//Play_cross_fade_animation
		void PlayCrossFadeInterpolated(int animationIndexA, int animationIndexB, float crossfadeTime);	//Play_cross_fade_animation_interpolated
		void PlayBlended(int animationIndexA, int animationIndexB, float ratio);						//Play_blend_space_animation
		void PlayBlendedInterpolated(int animationIndexA, int animationIndexB, float ratio);			//Play_blend_space_animation_interpolated
		void PlayeLayerBlend(int animationIndexA, int animationIndexB, int origine);					//Play_layer_blend_space_animation

		//IK
		int  AddIKInstance(int endBoneIndex, int chainLenght);	//chainLenght don't include endBone
		void SetIKTargetPosition(int index, LibMath::Vector3 newTargetPosition);
		void ActivateIKInstace(int index);
		void DeactivateIKInstace(int index);

	private:
		enum AnimatorState
		{
			//Play + animation type + repeat? + interpolated?

			Waiting_for_loaded_ressources,
			Play_no_animation,
			Play_single_animation,
			Play_single_animation_interpolated,
			Play_cross_fade_animation,
			Play_cross_fade_animation_interpolated,
			Play_blend_space_animation,
			Play_blend_space_animation_interpolated,
			Play_layer_blend_space_animation
		};

		struct IKInstance
		{
			bool				m_active = true;
			int					m_endBoneIndex = -1;
			int					m_chainLenght = 0;
			LibMath::Vector3	m_targetPosition;
		};

		void WaitForResourcesToBeLoaded();

		//help
		void ExtractFrameSkeleton(vector<KeyFrame>& skeleton_out, const Animation& animation, int currentFrame);
		void GetSkeletonToGlobal(vector<KeyFrame>& skeleton_out, vector<KeyFrame>& localSkeleton);
		void ApplyOffset(vector<Matrix4>& skeletonMatrix_out, const vector<KeyFrame>& globalTransform);
		void InterpoleSkeletons(vector<KeyFrame>& interlopedSkeleton_out, vector<KeyFrame>& skeletonA, vector<KeyFrame>& skeletonB, const float& t);
		void ReplaceBones(vector<KeyFrame>& skeleton_out, vector<KeyFrame>& skeletonA, vector<KeyFrame>& skeletonB);

		LibMath::Vector3 GetPos(Matrix4& m);
		void SetPos(Matrix4& m, const LibMath::Vector3& p);
		void ApllyIKFabrik(vector<KeyFrame>& globalSkeleton, const IKInstance& ik);

		//gpu
		void SendSkeletonMatrixToGPU(vector<Matrix4>& skeletonMatrix);

		//animations
		void PlaySingleAnimation(float deltaTime);						//Play_single_animation
		void PlaySingleAnimationInterpolated(float deltaTime);			//Play_single_animation_interpolated
		void PlayCrossFadeAnimation(float deltaTime);					//Play_cross_fade_animation
		void PlayCrossFadeAnimationInterpolated(float deltaTime);		//Play_cross_fade_animation_interpolated
		void PlayBlendedAnimation(float deltaTime);						//Play_blend_space_animation
		void PlayBlendedAnimationInterpolated(float deltaTime);			//Play_blend_space_animation_interpolated
		void PlayLayerBlendAnimation(float deltaTime);					//Play_layer_blend_space_animation

		//IK
		void ApllyIKInstances(vector<KeyFrame>& globalSkeleton);


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
		float					m_elapse = 0.0f;

		//animations
		const Animation*		m_animationA = nullptr; //in cross fade, the original animation
		const Animation*		m_animationB = nullptr;	//in cross fade, the next animation

		//cross fade
		float					m_crossFadeDuration = 0.f;
		float					m_crossFadeElapseTime = 0.f;

		//blending
		float					m_blendRatio = 0;

		//layer bending
		int						m_layerBendingOrigine = 0;

		//IK
		vector<IKInstance>		m_IKInstances;
	};
}

#endif // !ANIMATOR
