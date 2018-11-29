#pragma once
#include <glm/glm.hpp>

#define PI 3.14159265359

//inverting helper function for transforms.
//used primarily for wordspace to objectspace transforms
glm::mat4 invertT(glm::mat4 mat){
	mat[3][0] = -mat[3][0];
	mat[3][1] = -mat[3][1];
	mat[3][2] = -mat[3][2];
	return mat;
}

//angle axis rotations using rodriguez's equation  for efficieny
glm::mat4 rodriguezRot(glm::vec3 k, float angle){
	float rad = angle * 3.145/(float) 180;
	glm::mat4 I = glm::mat4(1.0f);
	glm::mat4 K = glm::mat4(glm::vec4(0.0, k.z, -k.y, 0.0),
				glm::vec4(-k.z, 0.0, k.x, 0.0),
				glm::vec4(k.y, -k.x, 0.0, 0.0),
				glm::vec4(0.0, 0.0, 0.0, 0.0));

	return I + glm::sin(rad)*K +(1-glm::cos(rad))*K*K;
}
