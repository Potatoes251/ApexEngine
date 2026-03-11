#ifndef ANIMATION
#define ANIMATION

#include "IResource.h"
#include "LibMath/Vector/Vector3.h"
#include "LibMath/Quaternion.h"
#include <vector>
#include <string>

using LibMath::Vector3;
using LibMath::Quaternion;
using std::vector;
using std::string;

struct KeyFrame
{
	Vector3		m_position;
	Quaternion	m_rotation;
	Vector3		m_scale;
};

struct SingleBoneAnimation
{
	std::vector<KeyFrame>	m_frames;
};

class Animation : public IResource
{
public:
	Animation(string name);
	Animation(const Animation&) = delete;
	Animation& operator=(const Animation&) = delete;
	~Animation() = default;

	void LoadFromFile() override;

	vector<SingleBoneAnimation> m_skeletonAnimation;

private:
	string m_name;
};

#endif // !ANIMATION
