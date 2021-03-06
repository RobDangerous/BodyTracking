#pragma once

#include <Kore/Graphics2/Graphics.h>
#include <Kore/Math/Quaternion.h>

struct BoneNode;

using namespace Kore;

class InverseKinematics {
	
public:
	InverseKinematics(std::vector<BoneNode*> bones);
	bool inverseKinematics(Kore::vec4 desiredPos, BoneNode* targetBone);
	
private:
	int boneCount;
	int maxSteps;
	float maxError;
	int rootIndex;
	
	std::vector<BoneNode*> bones;
	
	static const int maxBones = 8;
	typedef Matrix<3, maxBones, float> mat3x;
	typedef Matrix<3, 1, float> mat3x1;
	typedef Matrix<1, maxBones, float> mat1x;
	
	void setJointConstraints();
	
	mat3x calcJacobian(BoneNode* targetBone, Kore::vec4 rotAxis);
	mat3x getPsevdoInverse(mat3x jacobian);
	
	void applyChanges(std::vector<float> theta, BoneNode* targetBone);
	void updateBonePosition(BoneNode* targetBone);
	
	float getRadians(float degree);

};
