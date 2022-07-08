env.so: env.o
	gcc -shared -o env.so env.o
	
env.o:
	gcc -c -fPIC env.c
