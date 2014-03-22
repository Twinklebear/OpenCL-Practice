typedef struct ray_t {
	float3 orig, dir;
	float t;
} ray_t;

typedef struct sphere_t {
	float3 center;
	float radius;
} sphere_t;

bool intersect_sphere(ray_t *ray, const global sphere_t *objects);

/*
 * Cast rays from positions listed in start and test for intersections against the objects
 * start should contain dim.x * dim.y vectors and img should be dim.x * dim.y chars
 */
kernel void cast_rays(const global float3 *start, const global sphere_t *objects, const uint n_objs,
	global char *img, const uint2 dim)
{
	uint2 id = (uint2)(get_global_id(0), get_global_id(1));
	//Should double check that these checks are necessary
	if (id.x >= dim.x || id.y >= dim.y){
		return;
	}
	ray_t ray = { .orig = start[id.y * dim.x + id.x], .dir = (float3)(0, 0, 1), .t = FLT_MAX };
	for (uint i = 0; i < n_objs; ++i){
		if (intersect_sphere(&ray, &objects[i])){
			img[id.y * dim.x + id.x] = '@';
		}
	}
}
bool intersect_sphere(ray_t *ray, const global sphere_t *objects){
	return false;
}

