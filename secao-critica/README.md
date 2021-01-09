# Atividade  1 - Seção Crítica por espera ocupada

# Compile
*gcc -fopenmp main.c -o run.o*

or

*./compile.sh*


# Run

## Number of client threads
Algorithms total number of threads will be NUM_OF_CLIENT_THREADS + 1 (thread "0" is the server)

Default value is **2**

*./run.o <NUM_OF_CLIENT_THREADS>*

## Testing
Will not enforce critical sector (tests only)
*./run.o <NUM_OF_CLIENT_THREADS> --free*
