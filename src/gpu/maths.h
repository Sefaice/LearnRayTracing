#pragma once

#include <glm/glm.hpp>
#include <assert.h>

#define kPI 3.1415926f

struct Camera
{
	Camera(const glm::vec3& lookFrom, const glm::vec3& lookAt, const glm::vec3& vup, float vfov, float aspect, float aperture, float focusDist)
	{
		lensRadius = aperture / 2;
		float theta = vfov * kPI / 180;
		float halfHeight = tanf(theta / 2);
		float halfWidth = aspect * halfHeight;
		origin = lookFrom;
		w = normalize(lookFrom - lookAt);
		u = normalize(cross(vup, w));
		v = cross(w, u);

		lowerLeftCorner = origin - halfWidth * focusDist*u - halfHeight * focusDist*v - focusDist * w;
		horizontalVec = 2 * halfWidth*focusDist * u;
		verticalVec = 2 * halfHeight*focusDist * v;
	}

	//Ray GetRay(float s, float t, uint32_t& state) const
	//{
	//	glm::vec3 rd = lensRadius * RandomInUnitDisk();
	//	glm::vec3 offset = u * rd.x + v * rd.y;
	//	return Ray(origin + offset, normalize(lowerLeftCorner + s * horizontalVec + t * verticalVec - origin - offset));
	//}

	glm::vec3 origin;
	glm::vec3 lowerLeftCorner;
	glm::vec3 horizontalVec;
	glm::vec3 verticalVec;
	glm::vec3 u, v, w;
	float lensRadius;
};