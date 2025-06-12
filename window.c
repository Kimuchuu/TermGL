#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "window.h"
#include "internal.h"

static char *print_buffer;

static void clear_buffers() {
	for (int i = 0; i < view_height; i++) {
		for (int j = 0; j < view_width; j++) {
			frame_buffer[i * view_width + j].c = ' ';
			z_buffer[i * view_width + j] = DBL_MAX;
		}
	}
}

static void flush_buffers() {
	printf("\x1b[?2026h\x1b[H\x1b[0J");
	for (int i = 0; i < view_height; i++) {
		printf("\x1b[%d;1H", i + 1);
		for (int j = 0; j < view_width; j++) {
			printf(
				"\x1b[38;2;%d;%d;%dm%c",
				(unsigned char)(frame_buffer[i * view_width + j].color.x * 255.f),
				(unsigned char)(frame_buffer[i * view_width + j].color.y * 255.f),
				(unsigned char)(frame_buffer[i * view_width + j].color.z * 255.f),
				frame_buffer[i * view_width + j].c
			);
		}
	}
	printf("\x1b[?2026l");
	fflush(stdout);
}

void init_window(int width, int height) {
	view_width = width;
	view_height = height;

	frame_buffer = malloc(height * width * sizeof(frame_buffer[0]));
	z_buffer = malloc(height * width * sizeof(z_buffer[0]));
	int print_buffer_size = height * width * 32 * sizeof(print_buffer[0]);
	print_buffer = malloc(print_buffer_size);
	// Prevents writing too early (stdout is line buffered), less flickering
	setvbuf(stdout, print_buffer, _IOFBF, print_buffer_size);
	printf("\x1b[?25l"); // Hide cursor
}

void cleanup_window() {
	printf("\x1b[?25h"); // Show cursor
	fflush(stdout);
	free(print_buffer);
	free(z_buffer);
	free(frame_buffer);
}

double get_time_ms() {
	static struct timespec timer;
	clock_gettime(CLOCK_MONOTONIC, &timer);
	return ((double)timer.tv_nsec / 1000000.0) + ((double)timer.tv_sec * 1000.0);
}

void loop(int fps, void (*fn) (unsigned, double, double)) {
	unsigned int frame = 0;
	double interval_ms = 1000.0 / fps;
	double time_last = get_time_ms();
	double time_frame, time, delta;

	while (1) {
		time = get_time_ms();
		delta = time - time_last;
		// START DRAW
		clear_buffers();
		fn(frame, time, delta);
		flush_buffers();
		// END DRAW
		time_frame = get_time_ms() - time;
		if (time_frame < interval_ms) {
			usleep((interval_ms - time_frame) * 1000);
		}
		time_last = time;
		frame++;
	}
}

