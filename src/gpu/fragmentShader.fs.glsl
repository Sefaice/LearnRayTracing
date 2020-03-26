#version 330 core

//out vec4 fColor;
layout(location = 0) out vec4 fColor;
layout(location = 1) out vec4 fNormal;
layout(location = 2) out vec4 fWorldPos;
layout(location = 3) out vec4 fTexture;
layout(location = 4) out vec4 fColorStdvar;
layout(location = 5) out vec4 fNormalStdvar;
layout(location = 6) out vec4 fWorldPosStdvar;

in vec2 TexCoords;

/* uniforms */
/* -------- */

uniform vec3 origin;
uniform vec3 lowerLeftCorner;
uniform vec3 horizontalVec;
uniform vec3 verticalVec;
uniform vec3 u;
uniform vec3 v;
uniform float lensRadius;

uniform float time;
uniform int frameCount;
uniform sampler2D LastColorTexture;
uniform sampler2D LastNormalTexture;
uniform sampler2D LastWorldPosTexture;
uniform sampler2D LastTexTexture;
uniform sampler2D LastColorStdvarTexture;
uniform sampler2D LastNormalStdvarTexture;
uniform sampler2D LastWorldPosStdvarTexture;

uniform int SCR_WIDTH;
uniform int SCR_HEIGHT;

/* const vars */
/* ---------- */

const int DO_SAMPLES_PER_PIXEL = 1;

const float kMinT = 0.001f;
const float kMaxT = 1.0e7f;
const int kMaxDepth = 20;
const float kPI = 3.1415926f;

/* global vars */
/* ----------- */

int writeFeatures = 1;
vec3 resultNormal = vec3(0, 0, 0);
vec3 resultWorldPos = vec3(0, 0, 0);
vec3 resultTexture = vec3(0, 0, 0);

/* --------------------------------------------------------- */
/* ------------------------ structs ------------------------ */
/* --------------------------------------------------------- */

struct Ray
{
	vec3 orig;
	vec3 dir;
};

struct Hit
{
	vec3 pos;
	vec3 normal;
	float t;
};

struct ScatterData
{
	Ray scatteredRay;
	vec3 lightE;
	vec3 attenuation;
};

struct Sphere
{
	vec3 center;
	float radiusSq;
};

const Sphere s_Spheres[] = Sphere[]
(
	// lights
	Sphere(vec3(0.4f, 5.5f, 1.3f), 0.4f),
	Sphere(vec3(0.05f, 5.0f, 3.0f), 0.1f),
	// walls
	Sphere(vec3(0, -1000.5, -1), 1000000.0f), // 巨大球体作为下方背景
	Sphere(vec3(0, 0, -1010), 1000000.0f), // back wall
	// others
	Sphere(vec3(-3.0f, 0.5f, -0.0f), 0.5f),
	Sphere(vec3(-1.0f, 0.5f, -0.0f), 0.5f),
	Sphere(vec3(1.0f, 0.5f, -0.0f), 0.5f),
	Sphere(vec3(3.0f, 0.5f, -0.0f), 0.5f),
	Sphere(vec3(-3.0f, 2.5f, -0.0f), 0.5f),
	Sphere(vec3(-1.0f, 2.5f, -0.0f), 0.5f),
	Sphere(vec3(1.0f, 2.5f, -0.0f), 0.5f),
	Sphere(vec3(3.0f, 2.5f, -0.0f), 0.5f),

	Sphere(vec3(-2.0f, 0.5f, -2.0f), 1.5f),
	Sphere(vec3(2.0f, 0.5f, -2.0f), 1.5f),
	Sphere(vec3(-2.0f, 2.5f, -2.0f), 1.5f),
	Sphere(vec3(2.0f, 2.5f, -2.0f), 1.5f)
	
	/*Sphere(vec3(-2.5f, 3.5f, -2.5f), 1.0f),
	Sphere(vec3(-2.5f, 3.5f, 0.0f), 1.0f),
	Sphere(vec3(-2.5f, 3.5f, 2.0f), 1.0f),
	Sphere(vec3(2.5f, 3.5f, -2.5f), 1.0f),
	Sphere(vec3(2.5f, 3.5f, 0.0f), 1.0f),
	Sphere(vec3(2.5f, 3.5f, 2.0f), 1.0f),
	Sphere(vec3(-1.2f, 3.5f, 5.0f), 1.0f),
	Sphere(vec3(1.2f, 3.5f, 5.0f), 1.0f),
	Sphere(vec3(0.0f, 3.0f, 0.5f), 2.5f)*/
);

const int kSphereCount = 16;

struct SpheresSoA
{
    vec3[kSphereCount]  center;
    float[kSphereCount] radiusSq;
};

SpheresSoA s_SpheresSoA;

const uint Lambert     = 0x00000001u;
const uint Metal       = 0x00000002u;
const uint Dielectric  = 0x00000004u;
struct Material
{
    uint type;
    vec3 albedo; // 自身颜色
    vec3 emissive; // 自发光率，只有白球有这一属性，所以它发光了
    float roughness;
    float ri; // 折射率
};

const Material s_SphereMats[] = Material[]
(
	// lights
	Material(Lambert, vec3(0.8f, 0.7f, 0.2f), vec3(40, 40, 40), 0, 0),
	Material(Lambert, vec3(0.1f, 0.2f, 0.5f), vec3(30, 30, 30), 0, 0),
	// walls
	Material(Lambert, vec3(0.6f, 0.6f, 0.6f), vec3(0, 0, 0), 0, 0),
	Material(Lambert, vec3(0.6f, 0.6f, 0.6f), vec3(0, 0, 0), 0, 0),
	// others
	Material(Lambert, vec3(0.4195, 0.9936, 0.1768), vec3(0, 0, 0), 0, 0.0f),
	Material(Dielectric, vec3(0.4f, 0.4f, 0.4f), vec3(0, 0, 0), 0, 1.1f),
	Material(Dielectric, vec3(0.4f, 0.4f, 0.4f), vec3(0, 0, 0), 0, 1.2f),
	Material(Lambert, vec3(0.2050, 0.6987, 0.9338), vec3(0, 0, 0), 0, 0.0f),
	Material(Lambert, vec3(0.7028, 0.9109, 0.7888), vec3(0, 0, 0), 0, 0.0f),
	Material(Dielectric, vec3(0.4f, 0.4f, 0.4f), vec3(0, 0, 0), 0, 1.3f),
	Material(Dielectric, vec3(0.4f, 0.4f, 0.4f), vec3(0, 0, 0), 0, 1.4f),
	Material(Lambert, vec3(0.4127, 0.4738, 0.6997), vec3(0, 0, 0), 0, 0.0f),
	Material(Metal, vec3(0.1987, 0.7430, 0.8941), vec3(0, 0, 0), 0, 0.0f),
	Material(Metal, vec3(0.1719, 0.9611, 0.7890), vec3(0, 0, 0), 0, 0.0f),
	Material(Metal, vec3(0.5666, 0.9189, 0.7103), vec3(0, 0, 0), 0, 0.0f),
	Material(Metal, vec3(0.9774, 0.0989, 0.7561), vec3(0, 0, 0), 0, 0.0f)
	//Material(Dielectric, vec3(0.4f, 0.4f, 0.4f), vec3(0, 0, 0), 0, 1.6f)
);

const int emissiveCount = 2;
const int emissiveSpheres[] = int[](0, 1); // emissive spheres' index

/* ------------------------------------------------------- */
/* ------------------------ funcs ------------------------ */
/* ------------------------------------------------------- */

//uint s_RndState = 1;

uint XorShift32(inout uint state)
{
	uint x = state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 15;
	state = x;
	return x;
}

float RandomFloat01(inout uint state)
{
	return (XorShift32(state) & uint(0xFFFFFF)) / 16777216.0f;
	//return XorShift32(state) / 4294967296.0f;
}

vec3 RandomInUnitDisk(inout uint state)
{
    vec3 p;
    do
    {
        p = 2.0 * vec3(RandomFloat01(state), RandomFloat01(state), 0) - vec3(1, 1, 0);
    } while (dot(p,p) >= 1.0);
    return p;
}

vec3 RandomInUnitSphere(inout uint state)
{
	vec3 p;
	do {
		p = vec3(2.0) * vec3(RandomFloat01(state), RandomFloat01(state), RandomFloat01(state)) - vec3(1, 1, 1);
	} while (dot(p, p) >= 1.0);
	return p;
}

vec3 RandomUnitVector(inout uint state)
{
    float z = RandomFloat01(state) * 2.0f - 1.0f;
    float a = RandomFloat01(state) * 2.0f * kPI;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
}

//bool HitSphere(const Ray r, const Sphere s, float tMin, float tMax, inout Hit outHit)
//{
//	vec3 rs = s.center - r.orig;
//
//	float rsProj = dot(rs, r.dir);
//	float ifHit = dot(rs, rs) - rsProj * rsProj - s.radiusSq;
//
//	if (ifHit < 0.0f)
//	{
//		float halfCut = sqrt(-ifHit);
//		float t;
//
//		t = rsProj - halfCut;
//		if (t > tMin && t < tMax)
//		{
//			outHit.pos = r.orig + r.dir * t;
//			outHit.normal = normalize(outHit.pos - s.center);
//			outHit.t = t;
//
//			return true;
//		}
//
//		t = rsProj + halfCut;
//		if (t > tMin && t < tMax)
//		{
//			outHit.pos = r.orig + r.dir * t;
//			outHit.normal = normalize(outHit.pos - s.center);
//			outHit.t = t;
//
//			return true;
//		}
//	}
//
//	return false;
//}

int HitSpheres(const Ray r, const SpheresSoA spheres, float tMin, float tMax, inout Hit outHit)
{
	Hit tmpHit;
    int id = -1;
    float closest = tMax;
    for (int i = 0; i < kSphereCount; ++i)
    {
		vec3 rs = spheres.center[i] - r.orig;

		float rsProj = dot(rs, r.dir);
		float ifHit = dot(rs, rs) - rsProj * rsProj - spheres.radiusSq[i];

        if (ifHit < 0.0f)
        {
			float halfCut = sqrt(-ifHit);
			float t = rsProj - halfCut;

            if (t > tMin && t < tMax && t < closest)
            {
                id = i;
                closest = t;
                tmpHit.pos = r.orig + r.dir * t;
                tmpHit.normal = normalize(tmpHit.pos - spheres.center[i]);
                tmpHit.t = t;
            }

            t = rsProj + halfCut;
			if (t > tMin && t < tMax && t < closest)
            {
                id = i;
                closest = t;
                tmpHit.pos = r.orig + r.dir * t;
                tmpHit.normal = normalize(tmpHit.pos - spheres.center[i]);
                tmpHit.t = t;
            }
        }
    }

	outHit = tmpHit;
    return id;
}

float schlick(float cosine, float ri)
{
	float r0 = (1 - ri) / (1 + ri);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow(1 - cosine, 5);
}

//bool HitWorld(const Ray r, float tMin, float tMax, inout Hit outHit, inout int outID)
//{
//	Hit tmpHit;
//	bool ifHit = false;
//	float closestT = tMax;
//
//	for (int i = 0; i < kSphereCount; ++i)
//	{
//		if (HitSphere(r, s_Spheres[i], tMin, closestT, tmpHit))
//		{
//			ifHit = true;
//			outHit = tmpHit;
//			closestT = tmpHit.t;
//			outID = i;
//		}
//	}
//
//	return ifHit;
//}

bool HitWorld(const Ray r, float tMin, float tMax, inout Hit outHit, inout int outID)
{
	outID = HitSpheres(r, s_SpheresSoA, tMin, tMax, outHit);
	return outID != -1;
}

bool Scatter(const Material mat, const Ray r_in, const Hit rec, inout vec3 attenuation, inout Ray scattered, inout vec3 outLightE, inout uint state)
{
    outLightE = vec3(0,0,0);
    if (mat.type == Lambert)
    {
        vec3 target = rec.pos + rec.normal + RandomUnitVector(state);
        //vec3 target = rec.pos + rec.normal;
        scattered = Ray(rec.pos, normalize(target - rec.pos));

        attenuation = mat.albedo;

		// light sampling
		for (int j = 0; j < emissiveCount; ++j)
		{
			int i = emissiveSpheres[j];
			Material smat = s_SphereMats[i];
			if (mat == smat)
				continue;
			Sphere s = s_Spheres[i];

			vec3 sw = normalize(s.center - rec.pos);
			vec3 su = normalize(cross(abs(sw.x) > 0.01f ? vec3(0, 1, 0) : vec3(1, 0, 0), sw));
			vec3 sv = cross(sw, su);

			float cosAMax = sqrt(1.0f - s.radiusSq / dot(rec.pos - s.center, rec.pos - s.center));
			float eps1 = RandomFloat01(state), eps2 = RandomFloat01(state);
			float cosA = 1.0f - eps1 + eps1 * cosAMax;
			float sinA = sqrt(1.0f - cosA * cosA);
			float phi = 2 * kPI * eps2;
			vec3 l = su * cos(phi) * sinA + sv * sin(phi) * sinA + sw * cosA;
			//l = normalize(l); l already normalized

			Hit lightHit;
			int hitID;
			if (HitWorld(Ray(rec.pos, l), kMinT, kMaxT, lightHit, hitID) && hitID == i)
			{
				float omega = 2 * kPI * (1 - cosAMax);
				vec3 rdir = r_in.dir;
				vec3 nl = dot(rec.normal, rdir) < 0 ? rec.normal : -rec.normal;

				outLightE += (mat.albedo * smat.emissive) * (max(0.0f, dot(l, nl)) * omega / kPI);
			}
		}

		return true;
    }
	else if (mat.type == Metal)
	{
		vec3 refl = reflect(r_in.dir, rec.normal);
		scattered = Ray(rec.pos, normalize(refl + mat.roughness*RandomInUnitSphere(state)));

		attenuation = mat.albedo;

		return dot(scattered.dir, rec.normal) > 0;
	}
	else if (mat.type == Dielectric)
	{
		vec3 outwardN;
		vec3 rdir = r_in.dir;
		vec3 refl = reflect(rdir, rec.normal);
		float nint;
	
		vec3 refr;
		float reflProb;
		float cosine;
		if (dot(rdir, rec.normal) > 0)
		{
			outwardN = -rec.normal;
			nint = mat.ri;
			cosine = dot(rdir, rec.normal);
		}
		else
		{
			outwardN = rec.normal;
			nint = 1.0f / mat.ri;
			cosine = -dot(rdir, rec.normal);
		}

		refr = refract(rdir, outwardN, nint);
		reflProb = schlick(cosine, mat.ri);

		if (RandomFloat01(state) < reflProb)
			scattered = Ray(rec.pos, normalize(refl));
		else
			scattered = Ray(rec.pos, normalize(refr));

		attenuation = vec3(1, 1, 1);

		return true;
	}

	return true;
}

vec3 TraceLoop(Ray r, int depth, inout uint state)
{
	vec3 resultColor = vec3(0.0);
	vec3 iteAttenuation = vec3(1.0);

	bool doMaterialE = true;

	while(true)
	{
		Hit rec;
		int id = 0;
	
		if (HitWorld(r, kMinT, kMaxT, rec, id))
		{
			Ray scattered;
			vec3 attenuation;
		    vec3 lightE;
			Material mat = s_SphereMats[id];
			vec3 matE = mat.emissive;
			// write to normal texture
			if (writeFeatures == 1)
			{
				resultNormal = rec.normal;
				resultWorldPos = rec.pos;
				resultTexture = mat.albedo;
				writeFeatures = 0;
			}


			if (depth < kMaxDepth && Scatter(mat, r, rec, attenuation,  scattered, lightE, state))
			{
				if (!doMaterialE) matE = vec3(0.0);
				doMaterialE = (mat.type != Lambert);

				resultColor += iteAttenuation * (matE + lightE);
				iteAttenuation *= attenuation;
				r = scattered;
	
				depth++;
			}
			else
			{	
				resultColor += iteAttenuation * matE;
				break;
			}
		}
		else // miss
		{
			vec3 unitDir = r.dir;
			float t = 0.5f * (unitDir.y + 1.0f);
			// the same as dot product below
			// float t = 0.5f * (dot(vec3(0, 1, 0), unitDir) + 1.0f);
			resultColor += iteAttenuation * ((1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f)) * 0.3f;

			//resultColor += iteAttenuation * vec3(0.5f, 0.7f, 1.0f);
			break;
		}
	}

	return resultColor;
}

float LinearToSRGB(float x)
{
	x = max(x, 0.0f);
	x = max(1.055f * pow(x, 0.416666667f) - 0.055f, 0.0f);
	return x;
}

float AdaptiveStdvar(float lastStdvar, float lastMean, int n, float newVal, float newMean)
{
	return sqrt((n * pow(lastStdvar, 2) + n * pow(lastMean - newMean, 2) + pow(newVal - newMean, 2)) / (n + 1));
}

vec3 AdaptiveStdvar_Vec3(vec3 lastStdvar, vec3 lastMean, int n, vec3 newVal, vec3 newMean) 
{
	float resultx = AdaptiveStdvar(lastStdvar.x, lastMean.x, n, newVal.x, newMean.x);
	float resulty = AdaptiveStdvar(lastStdvar.y, lastMean.y, n, newVal.y, newMean.y);
	float resultz = AdaptiveStdvar(lastStdvar.z, lastMean.z, n, newVal.z, newMean.z);
	return vec3(resultx, resulty, resultz);
}

void main()
{
	// init spheresSoA
	for(int i = 0; i < kSphereCount; i++)
	{
		s_SpheresSoA.center[i] = s_Spheres[i].center;
		s_SpheresSoA.radiusSq[i] = s_Spheres[i].radiusSq;
	}

	uint state = uint(int(gl_FragCoord.x * time * 9781 + gl_FragCoord.y * time * 6271) | 1);
	//state = uint(int(gl_FragCoord.x * 1973 + gl_FragCoord.y * 9277 + frameCount * 26699) | 1);

	float x = (gl_FragCoord.x + RandomFloat01(state)) / SCR_WIDTH;
	float y = (gl_FragCoord.y + RandomFloat01(state))  / SCR_HEIGHT;

	vec3 rd = lensRadius * RandomInUnitDisk(state);
	vec3 offset = u * rd.x + v * rd.y;
	vec3 rayOrigin = origin + offset;
	vec3 rayDir = normalize(lowerLeftCorner + x * horizontalVec + y * verticalVec - origin - offset);
	Ray r = Ray(rayOrigin, rayDir);

	vec3 resultColor = vec3(0.0);
	for(int i = 0; i < DO_SAMPLES_PER_PIXEL; i++)
	{
		resultColor += TraceLoop(r, 0, state);
	}
	resultColor *= 1.0f / float(DO_SAMPLES_PER_PIXEL); 

	resultColor = vec3(LinearToSRGB(resultColor.x), LinearToSRGB(resultColor.y), LinearToSRGB(resultColor.z));

	// lerp color
	float lerpFac = float(frameCount) / float(frameCount + 1);
	vec3 lastColor = texture(LastColorTexture, TexCoords).rgb;
	vec3 colorMean = lastColor * lerpFac + resultColor * (1 - lerpFac);
	fColor = vec4(colorMean, 1.0);

	// the rest is for generating grad proj data
	if (frameCount <= 4) { // only calc it when spp<=4
		// lerp normal
		vec3 lastNormal = texture(LastNormalTexture, TexCoords).rgb;
		vec3 normalMean = lastNormal * lerpFac + resultNormal * (1 - lerpFac);
		fNormal = vec4(normalMean, 1.0);
		// lerp worldpos
		vec3 lastWorldPos = texture(LastWorldPosTexture, TexCoords).rgb;
		vec3 worldPosMean = lastWorldPos * lerpFac + resultWorldPos * (1 - lerpFac);
		fWorldPos = vec4(worldPosMean, 1.0);
		// lerp texture color
		vec3 lastTexture = texture(LastTexTexture, TexCoords).rgb;
		vec3 textureMean = lastTexture * lerpFac + resultTexture * (1 - lerpFac);
		fTexture = vec4(textureMean, 1.0);
		// calc color std var
		vec3 lastColorStdvar = texture(LastColorStdvarTexture, TexCoords).rgb;
		vec3 resultColorStdvar = AdaptiveStdvar_Vec3(lastColorStdvar, lastColor, frameCount, resultColor, colorMean);
		fColorStdvar = vec4(resultColorStdvar, 1.0);
		// calc normal std var
		vec3 lastNormalStdvar = texture(LastNormalStdvarTexture, TexCoords).rgb;
		vec3 resultNormalStdvar = AdaptiveStdvar_Vec3(lastNormalStdvar, lastNormal, frameCount, resultNormal, normalMean);
		fNormalStdvar = vec4(resultNormalStdvar, 1.0);
		// calc worldpos std var
		vec3 lastWorldPosStdvar = texture(LastWorldPosStdvarTexture, TexCoords).rgb;
		vec3 resultWorldPosStdvar = AdaptiveStdvar_Vec3(lastWorldPosStdvar, lastWorldPos, frameCount, resultWorldPos, worldPosMean);
		fWorldPosStdvar = vec4(resultWorldPosStdvar, 1.0);
	}
}