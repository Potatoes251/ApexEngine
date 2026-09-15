#ifndef ANIMATOR
#define ANIMATOR

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Quaternion.h"

#include "../../Resources/header/ComputeShader.h"
#include "../../Resources/header/Mesh.h"
#include "../../Resources/header/Model.h"
#include "../../Resources/header/ResourceManager.h"
#include "../../Resources/header/Component.h"

#include <memory>

using LibMath::Matrix4;
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
		Animator(ResourceHandle<Model> model) : m_model(model) {}
		~Animator() = default;
		Animator(const Animator&) = default;
		Animator& operator=(const Animator&) = default;

		ResourceHandle<Model>	GetModel() const { return m_model; }
		void SetBlendRatio(float newRatio) { m_blendRatio = newRatio; }

		//Component
		void					Serialize(std::ostream& out) const override;
		unique_ptr<Component>	Clone() override;
		const char*				GetTypeName() const override	{ return "Animator"; };
		std::vector<ExposedVar> GetExposedVariables() override;

		//before update
		void SetModel(ResourceHandle<Model> model);
		void SetFPS(float fps);				//base fps = 30

		//during update
		void OnUpdate(float deltaTime) override;

		void Stop();

		//Animations
		void PlayRepeat(int animationIndex);															//PlaySingleAnim
		void PlayRepeatInterpolated(int animationIndex);												//PlaySingleAnimInterpolated
		void PlayCrossFade(int animationIndexA, int animationIndexB, float crossfadeTime);				//PlayCrossFadeAnim
		void PlayCrossFadeInterpolated(int animationIndexA, int animationIndexB, float crossfadeTime);	//PlayCrossFadeAnimInterpolated
		void PlayBlended(int animationIndexA, int animationIndexB, float ratio);						//PlayBlendSpaceAnim
		void PlayBlendedInterpolated(int animationIndexA, int animationIndexB, float ratio);			//PlayBlendSpaceAnimInterpolated
		void PlayeLayerBlend(int animationIndexA, int animationIndexB, int origine);					//PlayLayerBlendSpaceAnim

		//IK
		int  AddIKInstance(int endBoneIndex, int chainLength);	//chainLength don't include endBone
		void SetIKTargetPosition(int index, LibMath::Vector3 newTargetPosition);
		void ActivateIKInstace(int index);
		void DeactivateIKInstace(int index);

		//gpu
		void SendSkeletonMatrixToGPU(const ResourceHandle<ComputeShader>& shader);
	private:
		struct IKInstance
		{
			bool				m_active = true;
			int					m_endBoneIndex = -1;
			int					m_chainLength = 0;
			LibMath::Vector3	m_targetPosition;
		};

		enum AnimatorState
		{
			//Play + animation type + repeat? + interpolated?

			WaitingForLoadedResources,
			PlayNoAnim,
			PlaySingleAnim,
			PlaySingleAnimInterpolated,
			PlayCrossFadeAnim,
			PlayCrossFadeAnimInterpolated,
			PlayBlendSpaceAnim,
			PlayBlendSpaceAnimInterpolated,
			PlayLayerBlendSpaceAnim
		};

		void WaitForResourcesToBeLoaded();

		//helper
		void ExtractFrameSkeleton(vector<KeyFrame>& skeleton_out, const Animation& animation, int currentFrame) const;
		void RemoveRootMotion(vector<KeyFrame>& localSkeleton);
		void GetSkeletonToGlobal(vector<KeyFrame>& skeleton_out, const vector<KeyFrame>& localSkeleton);
		void ApplyOffset(const vector<KeyFrame>& globalTransform);
		void InterpoleSkeletons(
			vector<KeyFrame>& interlopedSkeleton_out, const vector<KeyFrame>& skeletonA, 
			const vector<KeyFrame>& skeletonB, float t) const;
		void ReplaceBones(
			vector<KeyFrame>& skeleton_out, const vector<KeyFrame>& skeletonA, const vector<KeyFrame>& skeletonB) const;

		LibMath::Vector3 GetPos(Matrix4& m);
		void SetPos(Matrix4& m, const LibMath::Vector3& p);
		void ApllyIKFabrik(vector<KeyFrame>& globalSkeleton, const IKInstance& ik);

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


		vector<Matrix4>			m_skeletonMatrix;

		//
		AnimatorState			m_state = WaitingForLoadedResources;
		AnimatorState			m_nextState = PlayNoAnim;
		int						m_animationIndexA = -1;
		int						m_animationIndexB = -1;

		//speed
		float					m_fps = 30.f;

		//model infos
		ResourceHandle<Model>	m_model;
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
		int						m_layerBendingOrigin = 0;

		//IK
		vector<IKInstance>		m_IKInstances;
	};
}

#endif // !ANIMATOR
