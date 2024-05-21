#ifndef LISTA_H
#define LISTA_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct ItemLista
{
    void *conteudo;
    struct ItemLista *proximo;
} ItemLista;

typedef struct Lista
{
    ItemLista *primeiro;
} Lista;

Lista Lista_new()
{
    Lista l;
    l.primeiro = 0;
    return l;
}

void Lista_inserir(Lista *lista, void *conteudo)
{
    ItemLista **item = &lista->primeiro;
    while (*item != 0)
        item = &((*item)->proximo);

    *item = (ItemLista *)malloc(sizeof(ItemLista));

    (*item)->conteudo = conteudo;
    (*item)->proximo = 0;
}

/*
Remove o item da lista e retorna o conteudo.
Caso não exista na lista, retorna 0.
*/
void *Lista_remover(Lista *lista, ItemLista *item)
{
    void *conteudo = item->conteudo;
    if (lista->primeiro == 0)
        return 0;
    if (lista->primeiro == item)
    {
        lista->primeiro = item->proximo;
        free(item);
        return conteudo;
    }
    ItemLista *lugar = lista->primeiro;
    do
    {
        if (lugar->proximo == item)
        {
            lugar->proximo = lugar->proximo->proximo;
            free(item);
            return conteudo;
        }
        lugar = lugar->proximo;
    } while (lugar);
    return 0;
}

void Lista_liberar(Lista *lista, void (*funcao_liberacao)(void *))
{
    ItemLista *item = lista->primeiro;
    ItemLista *proximo;
    if (item)
    {
        proximo = item->proximo;
        if (funcao_liberacao)
            funcao_liberacao(item->conteudo);
        free(item);
        item = proximo;
    }
}


#define Lista_for_item_em(lista) \
    for (ItemLista *item = (lista)->primeiro; item != NULL; item = item->proximo)

#define Lista_procurar(lista, query) ({                                      \
    ItemLista *result = 0;                                                   \
    for (ItemLista *item = (lista)->primeiro; item != 0; item = item->proximo) \
    {                                                                        \
        if (query)                                                           \
        {                                                                    \
            result = item;                                                   \
            break;                                                           \
        }                                                                    \
    }                                                                        \
    result;                                                                  \
})

int Lista_contar(Lista *lista){
    int n = 0;
    Lista_for_item_em(lista)
        n++;
    return n;
}

int Lista_obter_index(Lista *lista, ItemLista* busca){
    int i = 0;
    Lista_for_item_em(lista){
        if(item == busca)
            return i;
        i++;
    }
    return -1;
}


static void teste_liberar_cada_item(void* arg){
    free(arg);
}
void teste_lista()
{
    //cria lista
    Lista lista = Lista_new();
    
    //aloca valores
    char *s1 = malloc(sizeof(char)*20);
    strcpy(s1, "string1");
    char *s2 = malloc(sizeof(char)*20);
    strcpy(s2, "string2");
    char *s3 = malloc(sizeof(char)*20);
    strcpy(s3, "string3");
    
    //insere na lista
    Lista_inserir(&lista, (void *)s1);
    Lista_inserir(&lista, (void *)s2);
    Lista_inserir(&lista, (void *)s3);

    ItemLista *item_encontrado = Lista_procurar((&lista), strcmp(item->conteudo, "string2")==0);

    printf("Item encontrado no index %d\n", Lista_obter_index(&lista, item_encontrado));

    Lista_for_item_em((&lista))
        printf("%s\n", (char*)item->conteudo);

    Lista_liberar(&lista, teste_liberar_cada_item);
}
#endif