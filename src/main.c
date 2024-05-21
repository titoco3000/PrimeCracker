/*
Primos conhecidos vão de 1 a N
Cada maquina recebe inicio e fim
Calcula todas as multiplicações de (inicio a fim) * (1 a N)

*/

#include <stdio.h>
#include "distribuicao.h"
int main()
{
    char ip_buffer[100];

    printf("Se já tem alguma outra máquina na rede, digite o IP dela (vazio caso não tenha): ");
    obter_input(ip_buffer);
    iniciar_distribuicao(ip_buffer);

}