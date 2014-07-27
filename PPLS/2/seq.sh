echo "Compiling:"

gcc -o aquadsequential aquadsequential.c -lm

echo "Running:"
#/usr/lib64/openmpi/bin/mpirun -c 5 ./aquadPartA
./aquadsequential
