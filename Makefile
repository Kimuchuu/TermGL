3c:
	gcc -o out/3c -Wall -g -lm main.c math.c window.c shapes.c ply.c

clean:
	rm out/*
