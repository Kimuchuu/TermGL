#ifndef LIB3C_WINDOW_H
#define LIB3C_WINDOW_H

void cleanup_window(void);
void init_window(int width, int height);
void loop(int fps, void (*fn) (unsigned, double, double));

#endif

