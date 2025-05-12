#include <stdlib.h>
#include <string.h>
#include "ply.h"

// bad parser of .ply ascii files for testing models
// elements: vertex, face
// properties: x,y,z nx,ny,nz, red,green,blue r,g,b vertex_index

Polygon parse_ply(FILE *fp) {
	char characters[300];
	int section_count;
	Polygon obj;

	// index positions of relevant properties
	int i_x  = -1, i_y  = -1, i_z  = -1,
	 	i_nx = -1, i_ny = -1, i_nz = -1,
	 	i_r  = -1, i_g  = -1, i_b  = -1;

	if (fgets(characters, 300, fp) == NULL) {
		perror("Unable to parse file");
		exit(1);
	}
	if (strcmp(characters, "ply\n") != 0) {
		fprintf(stderr, "File must start with ply");
		exit(1);
	}
	if (fgets(characters, 300, fp) == NULL) {
		perror("Unable to parse file");
		exit(1);
	}
	if (strcmp(characters, "format ascii 1.0\n") != 0) {
		fprintf(stderr, "Format must be ascii 1.0");
		exit(1);
	}

	int property_index = 0;
	while (fgets(characters, 300, fp) != NULL) {
		if (strncmp("end_header", characters, 10) == 0) {
			break;
		}
		if (strncmp("comment", characters, 7) == 0) {
			continue;
		}
		if (strncmp("element", characters, 7) == 0) {
			char element[10];
			int value;
			property_index = 0;
			if (sscanf(characters, "element %s %d", element, &value) != 2) {
				fprintf(stderr, "Error parsing element '%s' with value '%d'\n", element, value);
				exit(1);
			} else if (strcmp("vertex", element) == 0) {
				obj.n_points = value;
			} else if (strcmp("face", element) == 0) {
				obj.n_faces = value;
			} else {
				fprintf(stderr, "Invalid element type '%s'", element);
				exit(1);
			}
		} else if (strncmp("property", characters, 7) == 0) {
			char type[10];
			char name[10];
			if (sscanf(characters, "property %s\n", type) != 1) {
				fprintf(stderr, "Error parsing line '%s'\n", characters);
				exit(1);
			} else if (strncmp("list", type, 4) == 0) {
				// TODO: list
			} else {
				sscanf(characters, "property %s %s\n", type, name);
				if (strcmp("x", name) == 0) {
					i_x = property_index;
				} else if (strcmp("y", name) == 0) {
					i_y = property_index;
				} else if (strcmp("z", name) == 0) {
					i_z = property_index;
				} else if (strcmp("nx", name) == 0) {
					i_nx = property_index;
				} else if (strcmp("ny", name) == 0) {
					i_ny = property_index;
				} else if (strcmp("nz", name) == 0) {
					i_nz = property_index;
				} else if (strcmp("r", name) == 0 || strcmp("red", name) == 0) {
					i_r = property_index;
				} else if (strcmp("g", name) == 0 || strcmp("green", name) == 0) {
					i_g = property_index;
				} else if (strcmp("b", name) == 0 || strcmp("blue", name) == 0) {
					i_b = property_index;
				}
			}
			property_index++;
		}
	}

	obj.points = malloc(obj.n_points * sizeof(Point3D));
	section_count = 0;
	float x, y, z;
	float nx, ny, nz;
	unsigned char r = 255, g = 255, b = 255; // default
	while (section_count++ < obj.n_points && fgets(characters, 300, fp) != NULL) {
		property_index = 0;
		char *end, *start = characters;
		while ((end = index(start, ' ')) != NULL) {
			*end = '\0';
			if (i_x == property_index) {
				x = atof(start);
			} else if (i_y == property_index) {
				y = atof(start);
			} else if (i_z == property_index) {
				z = atof(start);
			} else if (i_nx == property_index) {
				nx = atof(start);
			} else if (i_ny == property_index) {
				ny = atof(start);
			} else if (i_nz == property_index) {
				nz = atof(start);
			} else if (i_r == property_index) {
				r = atoi(start);
			} else if (i_g == property_index) {
				g = atoi(start);
			} else if (i_b == property_index) {
				b = atoi(start);
			}
			property_index++;
			start = end + 1;
		}

		obj.points[section_count-1] = (Point3D) {
			{ x, y, z },
			{ r, g, b },
			{ nx, ny, nz }
		};
	}

	int face_vertices;
	int i1;
	int i2;
	int i3;
	int i4;
	obj.faces = malloc(obj.n_faces * 3 * sizeof(int));
	section_count = 0;
	int order_index = 0;
	while (section_count++ < obj.n_faces && fgets(characters, 300, fp) != NULL) {
		sscanf(characters, "%d %d %d %d %d", &face_vertices, &i1, &i2, &i3, &i4);
		obj.faces[order_index++] = i1;
		obj.faces[order_index++] = i2;
		obj.faces[order_index++] = i3;

		if (face_vertices == 4) {
			obj.n_faces++;
			obj.faces = realloc(obj.faces, obj.n_faces * 3 * sizeof(int));
			obj.faces[order_index++] = i1;
			obj.faces[order_index++] = i3;
			obj.faces[order_index++] = i4;
		}
	}

	return obj;
}

