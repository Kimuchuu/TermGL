3d:
	gcc -o out/3d.o -c -Wall -g 3d.c
internal:
	gcc -o out/internal.o -c -Wall -g internal.c
math:
	gcc -o out/math.o -c -Wall -g math.c
ply:
	gcc -o out/ply.o -c -Wall -g ply.c
shapes:
	gcc -o out/shapes.o -c -Wall -g shapes.c
simple3d:
	gcc -o out/simple3d.o -c -Wall -g simple3d.c
window:
	gcc -o out/window.o -c -Wall -g window.c

3c: 3d internal math ply shapes simple3d window
	ar rcs out/lib3c.a out/math.o out/internal.o out/window.o out/3d.o out/simple3d.o out/shapes.o out/ply.o

# Examples
basic.o: 3c
	gcc -c -o out/basic.o -Wall -g examples/basic.c
basic: basic.o
	gcc -o out/basic -Wall -g -Lout -lm out/basic.o -l3c

# Utils
clean:
	rm out/*
