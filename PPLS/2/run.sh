echo "Compiling:"

/usr/lib64/openmpi/bin/mpicc -o aquadPartB aquadPartB.c

echo "Running:"
/usr/lib64/openmpi/bin/mpirun -c 5 ./aquadPartB
