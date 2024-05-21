#ifndef PRIMOS_H
#define PRIMOS_H
#include "prime_list.c"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ull unsigned long long

typedef struct Primos
{
    FILE *arquivo;
    int index;
    int numbers_on_file;
} Primos;

Primos *Primos_abrir(char *arquivo)
{
    // FILE *f = fopen(arquivo, "rb");
    // if (!f)
    //     return 0;
    Primos *p = (Primos *)malloc(sizeof(Primos));
    // if (fseek(f, 0, SEEK_END) == -1)
    // {
    //     printf("failed to fseek primes\n");
    //     exit(EXIT_FAILURE);
    // }
    // fclose(f);

    p->arquivo = 0;
    p->index = 0;
    p->numbers_on_file = 3000;//30000;
    return p;
}

void Primos_fechar(Primos *p)
{
    fclose(p->arquivo);
}

ull Primos_obter_no_index(Primos *p, int index)
{
    // fseek(p->arquivo, index * sizeof(ull), SEEK_SET);
    // ull number_read;
    // size_t elements_read = fread(&number_read, sizeof(ull), 1, p->arquivo);
    // if (elements_read != 1)
    // {
    //     perror("Error reading from file");
    //     exit(1);
    // }
    return prime_array[index];
}

ull Primos_obter_primo_aleatorio(Primos *p)
{
    ull max_num_index = p->numbers_on_file;
    return Primos_obter_no_index(p, rand() % 1000);
}

ull Primos_obter_proximo_primo(Primos *p)
{
    return Primos_obter_no_index(p, p->index++);
}

int Primos_obter_index_aproximado(Primos *p, ull num){
    int inicio = 0; int fim = p->numbers_on_file;
    int m;
    for (int i = 0; i < 10; i++)
    {
        m = (inicio + fim )/2;
        if(prime_array[m]>num)
            inicio = m;
        else if(prime_array[m]<num)
            fim = m;
        else
            return m;
    }
    while (prime_array[m]<num)
        m++;
    
    return m;
    
}
#endif

/*
    Primos *arquivo_primos = Primos_abrir("encoded_primes.p");
    Primos_obter_proximo_primo(arquivo_primos);
    puts("");
    Primos_obter_proximo_primo(arquivo_primos);
    
    srand(time(NULL));
    Primos_obter_primo_aleatorio(arquivo_primos);

    Primos_fechar(arquivo_primos);
*/