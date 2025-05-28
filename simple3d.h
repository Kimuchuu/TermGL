#ifndef LIB3C_SIMPLE3D_H
#define LIB3C_SIMPLE3D_H

#include "3d.h"
#include "model.h"

Pixel default_fragment_shader(Vec3f color, Vec3f world_pos, Vec3f world_norm, Camera *camera, Light **lights, int n_lights);

#endif
