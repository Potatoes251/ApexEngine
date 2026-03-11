#ifndef ANIMATION_IMPORTER
#define ANIMATION_IMPORTER

#include <string>
#include "Animation.h"
#include <fbxsdk.h>
#include <mutex>

using std::string; 

class AnimationImporter
{
public:
	AnimationImporter() = default;
	~AnimationImporter() = default;

	void Import(string path, Animation& animation);
	void ProcessNode(FbxNode* node, Animation& animation, FbxTime start, double frameCount);

	static FbxManager* m_manager;

private:
};

#endif // !ANIMATION_IMPORTER
