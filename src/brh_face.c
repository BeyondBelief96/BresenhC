#include "brh_face.h"


brh_vector3 get_face_normal(brh_vector3 a, brh_vector3 b, brh_vector3 c)
{
	brh_vector3 ab = vec3_subtract(b, a);
	brh_vector3 ac = vec3_subtract(c, a);
	brh_vector3 normal = vec3_cross(ab, ac);
	vec3_normalize(&normal);
    return normal;
}