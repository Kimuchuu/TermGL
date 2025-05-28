#include "simple3d.h"
#include "3d.h"
#include <math.h>

static char depth[] = { '@', '$', '#', '*', '!', '=', ';', ':', '~', '-', ',', '.' };

Pixel default_fragment_shader(Vec3f color, Vec3f world_pos, Vec3f world_norm, Camera *camera, Light **lights, int n_lights) {
	Vec3f view_direction = vec3f_normalize(vec3f_sub(camera->position, world_pos));
	Vec3f world_norm_normalized = vec3f_normalize(world_norm);

	float total_diffuse_strength = 0;
	Vec3f final_color;
	final_color.x = 0, final_color.y = 0, final_color.z = 0;
	for (int i = 0; i < n_lights; i++) {
		Vec3f light_direction = vec3f_normalize(vec3f_sub(lights[i]->position, world_pos));
		Vec3f reflect_direction = vec3f_reflect(vec3f_scale(light_direction, -1.f), world_norm_normalized);

		Vec3f ambient_ucolor = vec3f_scale(lights[i]->color, lights[i]->ambient_strength);
		float diffuse_strength = MAX(vec3f_dot(world_norm_normalized, light_direction), 0.f);
		total_diffuse_strength += diffuse_strength;
		Vec3f diffuse_ucolor = vec3f_scale(lights[i]->color, diffuse_strength);
		float spec = powf(MAX(vec3f_dot(view_direction, reflect_direction), 0.f), 32.f);
		Vec3f specular_ucolor = vec3f_scale(lights[i]->color, lights[i]->specular_strength * spec);

		Vec3f light_color = vec3f_mult(
			vec3f_add(vec3f_add(ambient_ucolor, diffuse_ucolor), specular_ucolor),
			color);

		final_color.x += light_color.x;
		final_color.y += light_color.y;
		final_color.z += light_color.z;
	}
	final_color.x = MAX(0, MIN(1, (final_color.x)));
	final_color.y = MAX(0, MIN(1, (final_color.y)));
	final_color.z = MAX(0, MIN(1, (final_color.z)));

	Pixel fragment;

	// NORMAL DEBUGGING
	// fragment.c = depth[0];
	// fragment.color.x   = (world_norm_normalized.x + 1.f) / 2.f;
	// fragment.color.y = (world_norm_normalized.y + 1.f) / 2.f;
	// fragment.color.z  = (world_norm_normalized.z + 1.f) / 2.f;

	// ASCII LIGHT
	// short d_index = (1 - total_diffuse_strength) * (sizeof(depth) - 1);
	// fragment.c = depth[d_index];
	// fragment.color = (Color) { 1, 1, 1 };

	// COLOR
	fragment.c = depth[0];
	fragment.color = final_color;

	// ASCII LIGHT AND COLOR
	// short d_index = (1 - total_diffuse_strength) * (sizeof(depth) - 1);
	// fragment.c = depth[d_index];
	// fragment.color = final_color;

	return fragment;
}

