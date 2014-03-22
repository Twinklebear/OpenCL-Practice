typedef struct ray_t {
	float3 orig, dir;
	float t;
} ray_t;

typedef struct sphere_t {
	float3 center;
	float radius;
} sphere_t;

//Check the ray for intersection against the sphere, true if intersects
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
			if (ray.t < 0.5){
				img[id.y * dim.x + id.x] = '@';
			}
			else if (ray.t < 1){
				img[id.y * dim.x + id.x] = '0';
			}
			else {
				img[id.y * dim.x + id.x] = '.';
			}
		}
	}
}
bool intersect_sphere(ray_t *ray, const global sphere_t *sphere){
	float3 l = sphere->center - ray->orig;
	float l_sqr = dot(l, l);
	float s = dot(l, ray->dir);
	float r_sqr = pow(sphere->radius, 2);

	if (s < 0 && l_sqr > r_sqr){
		return false;
	}
	float m_sqr = l_sqr - pow(s, 2);
	if (m_sqr > r_sqr){
		return false;
	}

	float q = sqrt(r_sqr - m_sqr);
	float t = l_sqr > r_sqr ? s - q : s + q;
	if (t < ray->t){
		ray->t = t;
		return true;
	}	
	return false;
}

