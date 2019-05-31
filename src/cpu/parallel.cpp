#include "./enkiTS/TaskScheduler_c.h"

#include <iostream>
#include <atomic>
#include <algorithm>

#include "parallel.h"

const float kMinT = 0.001f;
const float kMaxT = 1.0e7f;

const int kMaxDepth = 20;

static Sphere s_Spheres[] =
{
	{ float3(0,-100.5,-1), 100.0f },// 巨大球体作为下方背景
	{ float3(2,1,-1), 0.5f },// 后排最右红色
	{ float3(0,0,-1), 0.5f },
	{ float3(-2,0,-1), 0.5f },
	{ float3(2,0,1), 0.5f },
	{ float3(0,0,1), 0.5f },
	{ float3(-2,0,1), 0.5f },
	{ float3(0.5f,1,0.5f), 0.5f },// 玻璃球
	{ float3(-1.5f,1.5f,0.f), 0.3f },// 白色
};
const int kSphereCount = sizeof(s_Spheres) / sizeof(s_Spheres[0]);

struct Material
{
	enum Type { Lambert, Metal, Dielectric };
	Type type;
	float3 albedo;// 反射率
	float3 emissive;// 自发光率，只有白球有这一属性，所以它发光了
	float roughness;
	float ri; // 反射系数
};

static Material s_SphereMats[kSphereCount] =
{
	{ Material::Lambert, float3(0.8f, 0.8f, 0.8f), float3(0,0,0), 0, 0, },
	{ Material::Lambert, float3(0.8f, 0.4f, 0.4f), float3(0,0,0), 0, 0, },
	{ Material::Lambert, float3(0.4f, 0.8f, 0.4f), float3(0,0,0), 0, 0, },
	{ Material::Metal, float3(0.4f, 0.4f, 0.8f), float3(0,0,0), 0, 0 },
	{ Material::Metal, float3(0.4f, 0.8f, 0.4f), float3(0,0,0), 0, 0 },
	{ Material::Metal, float3(0.4f, 0.8f, 0.4f), float3(0,0,0), 0.2f, 0 },
	{ Material::Metal, float3(0.4f, 0.8f, 0.4f), float3(0,0,0), 0.6f, 0 },
	{ Material::Dielectric, float3(0.4f, 0.4f, 0.4f), float3(0,0,0), 0, 1.5f },
	{ Material::Lambert, float3(0.8f, 0.6f, 0.2f), float3(30,25,15), 0, 0 },
};

bool HitWorld(const Ray& r, float tMin, float tMax, Hit& outHit, int& outID)
{
	Hit tmpHit;
	bool ifHit = false;
	float closestT = tMax;

	for (int i = 0; i < kSphereCount; ++i)
	{
		if (HitSphere(r, s_Spheres[i], tMin, closestT, tmpHit))
		{
			ifHit = true;
			outHit = tmpHit;
			closestT = tmpHit.t;
			outID = i;
		}
	}

	return ifHit;
}

static bool Scatter(const Material& mat, const Ray& r_in, const Hit& rec, float3& attenuation, Ray& scattered, float3& outLightE, int& inoutRayCount)
{
	outLightE = float3(0.0, 0.0, 0.0);
	if (mat.type == Material::Lambert)
	{
		float3 target = rec.pos + rec.normal + RandomInUnitSphere();
		scattered = Ray(rec.pos, normalize(target - rec.pos));

		attenuation = mat.albedo;

		for (int i = 0; i < kSphereCount; ++i)
		{
			const Material& smat = s_SphereMats[i];
			if (smat.emissive.x <= 0 && smat.emissive.y <= 0 && smat.emissive.z <= 0)
				continue; // skip non-emissive
			if (&mat == &smat)
				continue; // skip self
			const Sphere& s = s_Spheres[i];

			// create a random direction towards sphere
			// coord system for sampling: sw, su, sv，指前，指左，指上
			float3 sw = normalize(s.center - rec.pos);
			float3 su = normalize(cross(fabs(sw.x)>0.01f ? float3(0, 1, 0) : float3(1, 0, 0), sw));
			float3 sv = cross(sw, su);
			// sample sphere by solid angle
			// sample light与起点到圆心连线的最大夹角cos值，最大情况是光线与球体相切
			float cosAMax = sqrtf(1.0f - s.radius*s.radius / ((rec.pos - s.center).length() * (rec.pos - s.center).length()));
			float eps1 = RandomFloat01(), eps2 = RandomFloat01();
			// ？？？为什么不直接取eps1 * cosAMax？？？
			float cosA = 1.0f - eps1 + eps1 * cosAMax;
			float sinA = sqrtf(1.0f - cosA * cosA);
			// xy平面的随即方向，用于计算uv的值
			float phi = 2 * kPI * eps2;
			float3 l = su * cosf(phi) * sinA + sv * sin(phi) * sinA + sw * cosA;
			l.normalize();

			// shoot shadow ray
			Hit lightHit;
			int hitID;
			++inoutRayCount;
			if (HitWorld(Ray(rec.pos, l), kMinT, kMaxT, lightHit, hitID) && hitID == i)
			{
				float omega = 2 * kPI * (1 - cosAMax);

				float3 rdir = r_in.dir;
				float3 nl = dot(rec.normal, rdir) < 0 ? rec.normal : -rec.normal;
				// 若l与rec.normal夹角大于90，不计算颜色
				// ???omega的作用看不懂???
				outLightE += (mat.albedo * smat.emissive) * (std::max(0.0f, dot(l, nl)) * omega / kPI);
			}
		}

		return true;
	}
	else if (mat.type == Material::Metal)
	{
		float3 refl = reflect(r_in.dir, rec.normal);
		scattered = Ray(rec.pos, normalize(refl + mat.roughness*RandomInUnitSphere()));

		attenuation = mat.albedo;
		
		return dot(scattered.dir, rec.normal) > 0;
	}
	else if (mat.type == Material::Dielectric)
	{
		float3 outwardN;
		float3 rdir = r_in.dir;
		float3 refl = reflect(rdir, rec.normal);
		float nint;
	
		float3 refr;
		float reflProb;
		float cosine;
		if (dot(rdir, rec.normal) > 0)
		{
			outwardN = -rec.normal;
			nint = mat.ri;
			//cosine = mat.ri * dot(rdir, rec.normal);
			cosine = dot(rdir, rec.normal);
		}
		else
		{
			outwardN = rec.normal;
			nint = 1.0f / mat.ri;
			cosine = -dot(rdir, rec.normal);
		}

		if (refract(rdir, outwardN, nint, refr))
		{
			reflProb = schlick(cosine, mat.ri);
		}
		else
		{
			reflProb = 1;
		}

		if (RandomFloat01() < reflProb)
			scattered = Ray(rec.pos, normalize(refl));
		else
			scattered = Ray(rec.pos, normalize(refr));

		attenuation = float3(1, 1, 1);
	}

	return true;
}

static float3 Trace(const Ray& r, int depth, int& inoutRayCount)
{
	Hit rec;
	int id = 0;
	++inoutRayCount;
	if (HitWorld(r, kMinT, kMaxT, rec, id)) // 碰到球体 
	{
		Ray scattered;
		float3 attenuation;
		float3 lightE;
		const Material& mat = s_SphereMats[id];
		float3 matE = mat.emissive;
		if (depth < kMaxDepth && Scatter(mat, r, rec, attenuation, scattered, lightE, inoutRayCount))
		{
			return matE + lightE + attenuation * Trace(scattered, depth + 1, inoutRayCount);
		}
		else
		{
			return matE;
		}
	}
	else
	{
		return float3(0.5f, 0.7f, 1.0f);
	}
}

static enkiTaskScheduler* g_TS;

void InitializeTest()
{
	g_TS = enkiNewTaskScheduler();
	enkiInitTaskScheduler(g_TS);
}

void ShutdownTest()
{
	enkiDeleteTaskScheduler(g_TS);
}

struct JobData
{
	float time;
	int frameCount;
	int screenWidth, screenHeight;
	float* backbuffer;
	Camera* cam;
	// 原子类型对象，从不同线程访问不会导致数据竞争(data race)，而其他类型从不同线程访问通常会导致undefined
	std::atomic<int> rayCount;
};

static void TraceRowJob(uint32_t start, uint32_t end, uint32_t threadnum, void* data_)
{
	JobData& data = *(JobData*)data_;

	// backbuffer是用于存储像素色值的缓存
	float* backbuffer = data.backbuffer + start * data.screenWidth * 4;
	float invWidth = 1.0f / data.screenWidth;
	float invHeight = 1.0f / data.screenHeight;
	float lerpFac = float(data.frameCount) / float(data.frameCount + 1);

	int rayCount = 0;
	for (uint32_t y = start; y < end; ++y)
	{
		//uint32_t state = (y * 9781 + data.frameCount * 6271) | 1;
		for (int x = 0; x < data.screenWidth; ++x)
		{
			float3 col(0, 0, 0);
			float u = float(x) * invWidth;
			float v = float(y) * invHeight;
			Ray r = data.cam->GetRay(u, v);

			col = Trace(r, 0, rayCount);
			// ？？？为什么要开根号sqrt？？？
			// col = float3(sqrtf(col.x), sqrtf(col.y), sqrtf(col.z));

			float3 prev(backbuffer[0], backbuffer[1], backbuffer[2]);
			// 每一帧与上一帧进行插值，prev是上一帧的像素值
			col = prev * lerpFac + col * (1 - lerpFac);
			backbuffer[0] = col.x;
			backbuffer[1] = col.y;
			backbuffer[2] = col.z;
			backbuffer += 4;
			/*backbuffer[0] = 0.8;
			backbuffer[1] = 0.4;
			backbuffer[2] = 0.2;
			backbuffer += 4;*/
		}
	}
	data.rayCount += rayCount;
}

void DrawTest(float time, int frameCount, int screenWidth, int screenHeight, float* backbuffer, int& outRayCount)
{
	float3 lookfrom(0, 2, 3);
	float3 lookat(0, 0, 0);
	float distToFocus = 3;
	float aperture = 0.1f;

	/*for (int i = 0; i < kSphereCount; ++i)
		s_Spheres[i].UpdateDerivedData();*/

	Camera cam(lookfrom, lookat, float3(0, 1, 0), 60, float(screenWidth) / float(screenHeight), aperture, distToFocus);

	JobData args;
	args.time = time;
	args.frameCount = frameCount;
	args.screenWidth = screenWidth;
	args.screenHeight = screenHeight;
	args.backbuffer = backbuffer;
	args.cam = &cam;
	args.rayCount = 0;
	enkiTaskSet* task = enkiCreateTaskSet(g_TS, TraceRowJob);
	bool threaded = true;
	enkiAddTaskSetToPipeMinRange(g_TS, task, &args, screenHeight, threaded ? 4 : screenHeight);
	enkiWaitForTaskSet(g_TS, task);
	enkiDeleteTaskSet(task);
	outRayCount = args.rayCount;
}