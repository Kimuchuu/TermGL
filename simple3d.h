#ifndef LIB3C_SIMPLE3D_H
#define LIB3C_SIMPLE3D_H

#include "3d.h"
#include "model.h"

Pixel fragment_shader_ascii(Vec3f color, Vec3f world_pos, Vec3f world_norm, Camera *camera, Light **lights, int n_lights);
Pixel fragment_shader_ascii_color(Vec3f color, Vec3f world_pos, Vec3f world_norm, Camera *camera, Light **lights, int n_lights);
Pixel fragment_shader_color(Vec3f color, Vec3f world_pos, Vec3f world_norm, Camera *camera, Light **lights, int n_lights);
Pixel fragment_shader_debug(Vec3f color, Vec3f world_pos, Vec3f world_norm, Camera *camera, Light **lights, int n_lights);

#endif
