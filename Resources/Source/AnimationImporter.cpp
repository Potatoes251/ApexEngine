#include "AnimationImporter.h"

const double ANIMATION_FPS = 30;

void AnimationImporter::Import(string path, Animation& animation)
{
	// Serialize all FBX operations — the SDK is not thread-safe.
	FbxScene* scene = FbxScene::Create(AnimationImporter::m_manager, "");

	FbxImporter* importer = FbxImporter::Create(AnimationImporter::m_manager, "");


	if (!importer->Initialize(path.c_str(), -1, AnimationImporter::m_manager->GetIOSettings()))
	{
		importer->Destroy();
		return;
	}

	if (!importer->Import(scene))
	{
		importer->Destroy();
		return;
	}

	importer->Destroy();

	FbxAxisSystem::OpenGL.ConvertScene(scene);
	//set the unit as meters
	FbxSystemUnit::m.ConvertScene(scene);

	int animStackCount = scene->GetSrcObjectCount<FbxAnimStack>();

	if (animStackCount == 0)
	{
		scene->Destroy();
		return;
	}

	FbxAnimStack* stack = scene->GetSrcObject<FbxAnimStack>(0);
	scene->SetCurrentAnimationStack(stack);

	FbxTakeInfo* takeInfo = scene->GetTakeInfo(stack->GetName());

	if (!takeInfo)
	{
		scene->Destroy();
		return;
	}

	FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
	FbxTime end = takeInfo->mLocalTimeSpan.GetStop();

	//get the duration of the animation
			double duration = end.GetSecondDouble() - start.GetSecondDouble();

	const	double frameCount = duration * ANIMATION_FPS;

	FbxNode* root = scene->GetRootNode();
	if (!root)
	{
		scene->Destroy();
		return;
	}

	animation.m_skeletonAnimation.clear();

	for (int i = 0; i < root->GetChildCount(); ++i)
	{
		ProcessNode(root->GetChild(i), animation, start, frameCount);
	}

	scene->Destroy();
}

void AnimationImporter::ProcessNode(FbxNode* node, Animation& animation, FbxTime start, double frameCount)
{
	SingleBoneAnimation boneAnim;

	boneAnim.m_frames.reserve(frameCount);

	for (double frame = 0; frame < frameCount; frame++)
	{
		FbxTime time;
		time.SetSecondDouble(start.GetSecondDouble() + frame / ANIMATION_FPS);

		FbxAMatrix transform = node->EvaluateLocalTransform(time);

		FbxVector4 t = transform.GetT();
		FbxQuaternion r = transform.GetQ();
		FbxVector4 s = transform.GetS();

		KeyFrame key;

		static_cast<float>(t[0]);

		key.m_position = Vector3(
			static_cast<float>(t[0]),
			static_cast<float>(t[1]),
			static_cast<float>(t[2]));

		key.m_rotation = Quaternion(
			static_cast<float>(r[3]),
			static_cast<float>(r[0]),
			static_cast<float>(r[1]),
			static_cast<float>(r[2]));

		key.m_scale = Vector3(
			static_cast<float>(s[0]),
			static_cast<float>(s[1]),
			static_cast<float>(s[2]));

		boneAnim.m_frames.push_back(key);
	}

	animation.m_skeletonAnimation.push_back(std::move(boneAnim));

	for (int i = 0; i < node->GetChildCount(); ++i)
	{
		ProcessNode(node->GetChild(i), animation, start, frameCount);
	}
}
