#pragma once

#include <math.h>

#define kPI 3.1415926f

/* ������Ŀ���õ������ݽṹ */

// ��ά��������Ŀ��x���ң�y���ϣ�z����Ļ�⣬��������ϵ
struct float3
{
	float3() : x(0), y(0), z(0) {}
	float3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	float length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}
	void normalize()
	{
		float l = length();
		x = x / l;
		y = y / l;
		z = z / l;
	}

	float3 operator-() const
	{
		return float3(-x, -y, -z);
	}
	float3& operator+=(const float3& o)
	{
		x += o.x;
		y += o.y;
		z += o.z;
		return *this;
	}
	float3& operator-=(const float3& o)
	{
		x -= o.x;
		y -= o.y;
		z -= o.z;
		return *this;
	}
	float3& operator*=(const float3& o)
	{
		x *= o.x;
		y *= o.y;
		z *= o.z;
		return *this;
	}
	float3& operator*=(float o)
	{
		x *= o;
		y *= o;
		z *= o;
		return *this;
	}

	float x, y, z;
};

inline float3 operator+(const float3& a, const float3& b)
{
	return float3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline float3 operator-(const float3& a, const float3& b) 
{ 
	return float3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline float3 operator*(const float3& a, const float3& b) 
{
	return float3(a.x*b.x, a.y*b.y, a.z*b.z);
}
inline float3 operator*(const float3& a, float b) 
{ 
	return float3(a.x*b, a.y*b, a.z*b); 
}
inline float3 operator*(float a, const float3& b) 
{
	return float3(a*b.x, a*b.y, a*b.z); 
}
inline float dot(const float3& a, const float3& b) 
{
	return a.x*b.x + a.y*b.y + a.z*b.z; 
}
inline float3 cross(const float3& a, const float3& b)
{
	return float3(a.y*b.z - a.z*b.y,
		-(a.x*b.z - a.z*b.x),
		a.x*b.y - a.y*b.x);
}
inline float3 normalize(const float3& v) 
{ 
	float k = 1.0f / v.length(); 
	return float3(v.x*k, v.y*k, v.z*k); 
}

// ���������ķ���v�����Ǵӹ�Դָ����ײ�㣬ע��v��n���붼�Ƿ�����˷��ص�reflectҲֻ�ܴ�����
inline float3 reflect(const float3& v, const float3& n)
{
	return v + 2 * (-dot(v, n) * n);
}

// ���䡣��������v��������n������������Ҫ�ǵ�λ������nint�����ʣ��������������/������������ʣ�
inline bool refract(const float3& v, const float3& n, float nint, float3& outRefracted)
{
	// �����
	float dt = dot(v, n);
	float discr = 1.0f - nint * nint * (1 - dt * dt);
	// glm��ʵ��Ҳ������жϣ�������Ϊ���ų�������С��1�����
	if (discr > 0)
	{
		outRefracted = nint * (v - n * dt) - n * sqrtf(discr);
		return true;
	}
	return false;
}

// https://zhuanlan.zhihu.com/p/31534769
// ri�ǳ������������/������������ʣ�����ʵri���ĸ�������һ����cosine������ǵ�cosֵ
inline float schlick(float cosine, float ri)
{
	float r0 = (1 - ri) / (1 + ri);
	r0 = r0 * r0;
	return r0 + (1 - r0)*powf(1 - cosine, 5);
}

// ���ߣ����ͷ���
struct Ray
{
	Ray() {}
	Ray(const float3 _orig, const float3 _dir)
	{
		orig = _orig;
		dir = normalize(_dir);
	}
	// ����t������
	float3 pointAt(float t) const 
	{
		return orig + dir * t; 
	}

	float3 orig, dir;
};

// ���ߺ�Բ�Ľ��㣬���㷨������������㵽����ľ���
struct Hit
{
	float3 pos;
	float3 normal;
	float t;
};

// ����
struct Sphere
{
	Sphere() : radius(1.0f) {}
	Sphere(float3 center_, float radius_) : center(center_), radius(radius_) {}

	float3 center;
	float radius;
};

// �ؾ���min��max֮���Ƿ���������ཻ
bool HitSphere(const Ray& r, const Sphere& s, float tMin, float tMax, Hit& outHit);

float RandomFloat01();
// ��λԲ�������
float3 RandomInUnitDisk();
// ��λ���������
float3 RandomInUnitSphere();
// ��λ�򣨰뾶Ϊ1�����������
float3 RandomUnitVector();

struct Camera
{
	/**
	* �۲���㣬�۲��յ㣬����ϵ��λ��������
	* vfov������field of view���۲��ӽǸ߶�
	* ��Ļ��߱ȣ���Ȧֱ��������
	*/
	Camera(const float3& lookFrom, const float3& lookAt, const float3& vup, float vfov, float aspect, float aperture, float focusDist)
	{
		// ��Ȧ�뾶
		lensRadius = aperture / 2;
		origin = lookFrom;
		a = normalize(lookFrom - lookAt);
		r = normalize(cross(vup, a));
		u = normalize(cross(a, r));

		// �ӽǸ߶ȷ�Χת���ɻ���ֵ
		float theta = vfov * kPI / 180;
		// ��߶�����ֵ
		float halfHeightTan = tanf(theta / 2);
		float halfWidthTan = aspect * halfHeightTan;
		// �۲���������z�ᣨw�����ƶ�������룬���ݲ����õ��Ľ�ƽ�����½�
		lowerLeftCorner = origin - halfWidthTan * focusDist * r - halfHeightTan * focusDist * u - focusDist * a;

		horizontalVec = 2 * halfWidthTan * focusDist * r;
		verticalVec = 2 * halfHeightTan * focusDist * u;
	}

	// s��t����0-1֮���float���ֱ��ʾ��Ļ��ĳ���ص������Ⱥ͸߶�ռ�ı���
	Ray GetRay(float x, float y) const
	{
		// ��Ȧ��С��xyƽ�������һ�㣬RandomInUnitDisk�õ�����xyƽ����ԭ��Ϊ����Բ�еĵ�
		float3 rd = lensRadius * RandomInUnitDisk();
		// �õ�xyƽ���Ϲ�Ȧ�뾶������������������ȥ���Ȧ�����ӵ���
		float3 offset = r * rd.x + u * rd.y;
		// �����lookfrom���Ϲ�Ȧ�������ƫ����
		// �յ��ǽ�ƽ������s��t�õ��������
		// ����������õ�Ͷ�򳡾��е�����
		return Ray(origin + offset, normalize(lowerLeftCorner + x * horizontalVec + y * verticalVec - origin - offset));
	}

	float3 origin;
	// r������ҷ��򣨲�������ֶ��򣩣�u���Ϸ���a�۲��յ�ָ��۲����
	// �������ϵ��xyz�ᵥλ����
	float3 a, u, r;
	float3 lowerLeftCorner;
	float3 horizontalVec;
	float3 verticalVec;
	float lensRadius;
};
