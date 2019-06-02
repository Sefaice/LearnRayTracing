#pragma once

#include <math.h>

#define kPI 3.1415926f

/* 定义项目中用到的数据结构 */

// 三维向量，项目中x朝右，y朝上，z朝屏幕外，采用右手系
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

// 求反射向量的方向，v方向是从光源指向碰撞点，注意v和n输入都是方向，因此返回的reflect也只能代表方向
inline float3 reflect(const float3& v, const float3& n)
{
	return v + 2 * (-dot(v, n) * n);
}

// 折射。入射向量v，法向量n，出射向量都要是单位向量，nint折射率（入射材质折射率/折射材质折射率）
inline bool refract(const float3& v, const float3& n, float nint, float3& outRefracted)
{
	// 入射角
	float dt = dot(v, n);
	float discr = 1.0f - nint * nint * (1 - dt * dt);
	// glm的实现也有这个判断，可能是为了排除折射率小于1的情况
	if (discr > 0)
	{
		outRefracted = nint * (v - n * dt) - n * sqrtf(discr);
		return true;
	}
	return false;
}

// https://zhuanlan.zhihu.com/p/31534769
// ri是出射材质折射率/入射材质折射率，但其实ri用哪个计算结果一样，cosine是入射角的cos值
inline float schlick(float cosine, float ri)
{
	float r0 = (1 - ri) / (1 + ri);
	r0 = r0 * r0;
	return r0 + (1 - r0)*powf(1 - cosine, 5);
}

// 射线，起点和方向
struct Ray
{
	Ray() {}
	Ray(const float3 _orig, const float3 _dir)
	{
		orig = _orig;
		dir = normalize(_dir);
	}
	// 距离t处向量
	float3 pointAt(float t) const 
	{
		return orig + dir * t; 
	}

	float3 orig, dir;
};

// 射线和圆的交点，交点法向量，射线起点到交点的距离
struct Hit
{
	float3 pos;
	float3 normal;
	float t;
};

// 球体
struct Sphere
{
	Sphere() : radius(1.0f) {}
	Sphere(float3 center_, float radius_) : center(center_), radius(radius_) {}

	float3 center;
	float radius;
};

// 截径在min和max之间是否和射线有相交
bool HitSphere(const Ray& r, const Sphere& s, float tMin, float tMax, Hit& outHit);

float RandomFloat01();
// 单位圆中随机点
float3 RandomInUnitDisk();
// 单位球中随机点
float3 RandomInUnitSphere();
// 单位球（半径为1）表面随机点
float3 RandomUnitVector();

struct Camera
{
	/**
	* 观察起点，观察终点，坐标系单位朝上向量
	* vfov，纵向field of view，观察视角高度
	* 屏幕宽高比，光圈直径，焦距
	*/
	Camera(const float3& lookFrom, const float3& lookAt, const float3& vup, float vfov, float aspect, float aperture, float focusDist)
	{
		// 光圈半径
		lensRadius = aperture / 2;
		origin = lookFrom;
		a = normalize(lookFrom - lookAt);
		r = normalize(cross(vup, a));
		u = normalize(cross(a, r));

		// 视角高度范围转换成弧度值
		float theta = vfov * kPI / 180;
		// 半高度正切值
		float halfHeightTan = tanf(theta / 2);
		float halfWidthTan = aspect * halfHeightTan;
		// 观察起点在相机z轴（w方向）移动焦距距离，根据参数得到的焦平面左下角
		lowerLeftCorner = origin - halfWidthTan * focusDist * r - halfHeightTan * focusDist * u - focusDist * a;

		horizontalVec = 2 * halfWidthTan * focusDist * r;
		verticalVec = 2 * halfHeightTan * focusDist * u;
	}

	// s和t都是0-1之间的float，分别表示屏幕上某像素点坐标宽度和高度占的比例
	Ray GetRay(float x, float y) const
	{
		// 光圈大小下xy平面上随机一点，RandomInUnitDisk得到的是xy平面上原点为中心圆中的点
		float3 rd = lensRadius * RandomInUnitDisk();
		// 得到xy平面上光圈半径中随机点的向量。看上去像光圈就在视点上
		float3 offset = r * rd.x + u * rd.y;
		// 起点是lookfrom加上光圈中随机的偏移量
		// 终点是焦平面中由s，t得到的随机点
		// 由这两个点得到投向场景中的射线
		return Ray(origin + offset, normalize(lowerLeftCorner + x * horizontalVec + y * verticalVec - origin - offset));
	}

	float3 origin;
	// r相机正右方向（叉积用右手定则），u正上方向，a观察终点指向观察起点
	// 相机坐标系的xyz轴单位向量
	float3 a, u, r;
	float3 lowerLeftCorner;
	float3 horizontalVec;
	float3 verticalVec;
	float lensRadius;
};
