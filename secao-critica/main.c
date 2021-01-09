#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <omp.h>

int ENFORCE_CRITICAL_SECTION = 1;

int req = 0;
int res = 0;
int SOMA = 0;


void critical_section(int t) {
    int local = SOMA;
    sleep(rand()%2);
    SOMA = local + 1;

    printf("(%d) SOMA: %d\n", t, SOMA);
}


void client(int t) {
    while (1 == 1) {
        if (ENFORCE_CRITICAL_SECTION) while (res != t) req = t;

        critical_section(t);

        res = 0;
    }
}

void server() {
    while (1 == 1) {
        while (req == 0);
        res = req;

        while (res != 0);
        req = 0;
    }
}

int main(int argc, char const *argv[]) {
    int N = 2;
    if (argc > 1) N = atoi(argv[1]);
    if (N <= 0) {
      printf("Minimum number of client threads is 1.\n");
      return 1;
    }

    if (argc > 2 && strcmp("--free", argv[2]) != -1) ENFORCE_CRITICAL_SECTION = 0;

    omp_set_num_threads(N + 1);

    printf("(threads) %d\n", N + 1);
    printf("(server)  t: 0\n");
    printf("(client)  t: 1");
    for(int i = 2; i <= N; i++) printf(", %d", i);
    printf("\n");
    if (ENFORCE_CRITICAL_SECTION == 0) printf("(will not enforce critical section)\n");


    printf("\n");

    #pragma omp parallel
    {
        int t = omp_get_thread_num();

        if (t == 0) server();
        else client(t);
    }

    return 0;
}
