#include "maths.h"

#include <iostream>

static uint32_t s_RndState = 1;

static uint32_t XorShift32()
{
	uint32_t x = s_RndState;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 15;
	s_RndState = x;
	return x;
}

float RandomFloat01()
{
	return (XorShift32() & 0xFFFFFF) / 16777216.0f;
}

float3 RandomInUnitDisk()
{
	float3 p;
	do
	{
		p = 2.0 * float3(RandomFloat01(), RandomFloat01(), 0) - float3(1, 1, 0);
	} while (dot(p, p) >= 1.0);
	return p;
}

float3 RandomUnitVector()
{
	float z = RandomFloat01() * 2.0f - 1.0f;
	float a = RandomFloat01() * 2.0f * kPI;
	float r = sqrtf(1.0f - z * z);
	float x = r * cosf(a);
	float y = r * sinf(a);
	return float3(x, y, z);
}

float3 RandomInUnitSphere()
{
	float3 p;
	do {
		p = 2.0*float3(RandomFloat01(), RandomFloat01(), RandomFloat01()) - float3(1, 1, 1);
	} while (p.length() >= 1.0);
	return p;
}

bool HitSphere(const Ray& r, const Sphere& s, float tMin, float tMax, Hit& outHit)
{
	// 圆心到射线起点的向量
	float3 rs = s.center - r.orig;

	// 射线起点与球心连线在射线方向的投影长度，这里是负数
	// 若投影长度大于切点与射线起点连线长度，则射线穿过了球体，否则没有穿过
	float rsProj = dot(rs, r.dir);
	float ifHit = dot(rs, rs) - rsProj * rsProj - s.radius * s.radius;

	if (ifHit < 0.0f)
	{
		// 截径半长
		float halfCut = sqrtf(-ifHit);
		float t;

		// 入射点，t是射线起点到交点的距离
		t = rsProj - halfCut;
		if (t > tMin && t < tMax)
		{
			// 入射点
			outHit.pos = r.pointAt(t);
			// 单位法向量
			outHit.normal = normalize(outHit.pos - s.center);
			outHit.t = t;

			return true;
		}

		// 若在球体外部触发的光线一定会返回入射点，但是从球面上或球内部出发的，经过折射的光线则会返回出射点
		// 注意这里入射出射返回的法向量都是指向球外的s
		t = rsProj + halfCut;
		if (t > tMin && t < tMax)
		{
			outHit.pos = r.pointAt(t);
			outHit.normal = normalize(outHit.pos - s.center);
			outHit.t = t;

			return true;
		}
	}

	return false;
}