// scene 1

const Sphere s_Spheres[] = Sphere[]
(
	Sphere(vec3(0,-100.5,-1), 10000.0f),// 巨大球体作为下方背景
	Sphere(vec3(0, 0, -104), 10000.0f),
	Sphere(vec3(2,1,-1), 0.25f),// 后排最右红色
	Sphere(vec3(0,0,-1), 0.25f),
	Sphere(vec3(-2,0,-1), 0.25f),
	Sphere(vec3(2,0,1), 0.25f),
	Sphere(vec3(0,0,1), 0.25f),
	Sphere(vec3(-2,0,1), 0.25f),
	Sphere(vec3(0.5f,1,0.5f), 0.25f),// 玻璃球
	Sphere(vec3(-1.5f,3.0f,0.f), 0.09f)// 白色
);

const int kSphereCount = 10;

const Material s_SphereMats[] = Material[]
(
    Material( Lambert, vec3(0.8f, 0.8f, 0.8f), vec3(0,0,0), 0, 0),
	Material( Lambert, vec3(0.4f, 0.4f, 0.4f), vec3(0, 0, 0), 0, 0),
    Material( Lambert, vec3(0.8f, 0.4f, 0.4f), vec3(0,0,0), 0, 0),
    Material( Lambert, vec3(0.4f, 0.8f, 0.4f), vec3(0,0,0), 0, 0),
    Material( Metal, vec3(0.4f, 0.4f, 0.8f), vec3(0,0,0), 0, 0 ),
    Material( Metal, vec3(0.4f, 0.8f, 0.4f), vec3(0,0,0), 0, 0 ),
    Material( Metal, vec3(0.4f, 0.8f, 0.4f), vec3(0,0,0), 0.2f, 0 ),
    Material( Metal, vec3(0.4f, 0.8f, 0.4f), vec3(0,0,0), 0.6f, 0 ),
    Material( Dielectric, vec3(0.4f, 0.4f, 0.4f), vec3(0,0,0), 0, 1.5f ),
    Material( Lambert, vec3(0.8f, 0.6f, 0.2f), vec3(40,30,20), 0, 0 )
);

const int emissiveCount = 1;
const int emissiveSpheres[] = int[](9);

// scene 2