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
	// Բ�ĵ�������������
	float3 rs = s.center - r.orig;

	// ����������������������߷����ͶӰ���ȣ������Ǹ���
	// ��ͶӰ���ȴ����е�������������߳��ȣ������ߴ��������壬����û�д���
	float rsProj = dot(rs, r.dir);
	float ifHit = dot(rs, rs) - rsProj * rsProj - s.radius * s.radius;

	if (ifHit < 0.0f)
	{
		// �ؾ��볤
		float halfCut = sqrtf(-ifHit);
		float t;

		// ����㣬t��������㵽����ľ���
		t = rsProj - halfCut;
		if (t > tMin && t < tMax)
		{
			// �����
			outHit.pos = r.pointAt(t);
			// ��λ������
			outHit.normal = normalize(outHit.pos - s.center);
			outHit.t = t;

			return true;
		}

		// ���������ⲿ�����Ĺ���һ���᷵������㣬���Ǵ������ϻ����ڲ������ģ���������Ĺ�����᷵�س����
		// ע������������䷵�صķ���������ָ�������s
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