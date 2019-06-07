#version 330 core

out vec4 fColor;

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

/* const vars */
/* -------- */

const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

const int DO_SAMPLES_PER_PIXEL = 10;

const float kMinT = 0.001f;
const float kMaxT = 1.0e7f;
const int kMaxDepth = 20;
const float kPI = 3.1415926f;

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

//struct Sphere
//{
//	vec3 center;
//	float radiusSq;
//};

//const Sphere s_Spheres[] = Sphere[]
//(
//	Sphere(vec3(0,-100.5,-1), 10000.0f),// �޴�������Ϊ�·�����
//	Sphere(vec3(2,1,-1), 0.25f),// �������Һ�ɫ
//	Sphere(vec3(0,0,-1), 0.25f),
//	Sphere(vec3(-2,0,-1), 0.25f),
//	Sphere(vec3(2,0,1), 0.25f),
//	Sphere(vec3(0,0,1), 0.25f),
//	Sphere(vec3(-2,0,1), 0.25f),
//	Sphere(vec3(0.5f,1,0.5f), 0.25f),// ������
//	Sphere(vec3(-1.5f,1.5f,0.f), 0.09f)// ��ɫ
//);

const int kSphereCount = 9;

struct SpheresSoA
{
    vec3[kSphereCount]  center;
    float[kSphereCount] radiusSq;
};

const SpheresSoA s_SpheresSoA = SpheresSoA
(
	vec3[](vec3(0,-100.5,-1), vec3(2,1,-1), vec3(0,0,-1), vec3(-2,0,-1), vec3(2,0,1), vec3(0,0,1), vec3(-2,0,1), vec3(0.5f,1,0.5f), vec3(-1.5f,1.5f,0.f)),
	float[](10000.0f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.09f)
);

const uint Lambert     = 0x00000001u;
const uint Metal       = 0x00000002u;
const uint Dielectric  = 0x00000004u;
struct Material
{
    uint type;
    vec3 albedo; // ������ɫ
    vec3 emissive; // �Է����ʣ�ֻ�а�������һ���ԣ�������������
    float roughness;
    float ri; // ������
};

const Material s_SphereMats[] = Material[]
(
    Material( Lambert, vec3(0.8f, 0.8f, 0.8f), vec3(0,0,0), 0, 0),
    Material( Lambert, vec3(0.8f, 0.4f, 0.4f), vec3(0,0,0), 0, 0),
    Material( Lambert, vec3(0.4f, 0.8f, 0.4f), vec3(0,0,0), 0, 0),
    Material( Metal, vec3(0.4f, 0.4f, 0.8f), vec3(0,0,0), 0, 0 ),
    Material( Metal, vec3(0.4f, 0.8f, 0.4f), vec3(0,0,0), 0, 0 ),
    Material( Metal, vec3(0.4f, 0.8f, 0.4f), vec3(0,0,0), 0.2f, 0 ),
    Material( Metal, vec3(0.4f, 0.8f, 0.4f), vec3(0,0,0), 0.6f, 0 ),
    Material( Dielectric, vec3(0.4f, 0.4f, 0.4f), vec3(0,0,0), 0, 1.5f ),
    Material( Lambert, vec3(0.8f, 0.6f, 0.2f), vec3(30,25,15), 0, 0 )
);

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

		for (int i = 0; i < kSphereCount; ++i)
		{
			Material smat = s_SphereMats[i];
			if (smat.emissive.x <= 0 && smat.emissive.y <= 0 && smat.emissive.z <= 0)
				continue;
			if (mat == smat)
				continue;

			vec3 sw = normalize(s_SpheresSoA.center[i] - rec.pos);
			vec3 su = normalize(cross(abs(sw.x) > 0.01f ? vec3(0, 1, 0) : vec3(1, 0, 0), sw));
			vec3 sv = cross(sw, su);

			float cosAMax = sqrt(1.0f - s_SpheresSoA.radiusSq[i] / dot(rec.pos - s_SpheresSoA.center[i], rec.pos - s_SpheresSoA.center[i]));
			float eps1 = RandomFloat01(state), eps2 = RandomFloat01(state);
			float cosA = 1.0f - eps1 + eps1 * cosAMax;
			float sinA = sqrt(1.0f - cosA * cosA);
			float phi = 2 * kPI * eps2;
			vec3 l = su * cos(phi) * sinA + sv * sin(phi) * sinA + sw * cosA;
			l = normalize(l);

			Hit lightHit;
			int hitID;
			//++inoutRayCount;
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

void main()
{
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

	float lerpFac = float(frameCount) / float(frameCount + 1);
	vec3 lastColor = texture(LastColorTexture, TexCoords).rgb;
	resultColor = lastColor * lerpFac + resultColor * (1 - lerpFac);

	fColor = vec4(resultColor, 1.0);

}