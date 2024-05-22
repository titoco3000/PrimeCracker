/*
Primos conhecidos vão de 1 a N
Cada maquina recebe inicio e fim
Calcula todas as multiplicações de (inicio a fim) * (1 a N)

*/

#include <stdio.h>
#include "distribuicao.h"
int main(int argc, char **argv)
{
    int lider_automatico = 0;
    if(argc>1){
        if(argv[1][0] == '-' && argv[1][1] == 'l')
            iniciar_distribuicao("");   
        else
            iniciar_distribuicao(argv[1]);
    }
    else{
        char ip_buffer[100];

        printf("Se já tem alguma outra máquina na rede, digite o IP dela (vazio caso não tenha): ");
        obter_input(ip_buffer);
        iniciar_distribuicao(ip_buffer);
    }
}