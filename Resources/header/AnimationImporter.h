#ifndef ANIMATION_IMPORTER
#define ANIMATION_IMPORTER

#include <string>
#include "Animation.h"
#include <fbxsdk.h>

using std::string; 

class AnimationImporter
{
public:
	AnimationImporter() = default;
	~AnimationImporter() = default;

	void Import(string path, Animation& animation);

private:
};

#endif // !ANIMATION_IMPORTER
