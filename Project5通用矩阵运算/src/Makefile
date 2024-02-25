main: main.c matmul.c
	gcc -o main main.c matmul.c -I /usr/include/ -L/usr/lib -lopenblas -lpthread -lgfortran -mavx2 -fopenmp -O3