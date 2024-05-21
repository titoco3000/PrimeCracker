#include <stdlib.h>
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

void teste_lista()
{
    Lista lista = Lista_new();
    Lista_inserir(&lista, (void *)0);
    Lista_inserir(&lista, (void *)1);
    Lista_inserir(&lista, (void *)2);

    ItemLista *item_encontrado = Lista_procurar((&lista), item->conteudo == (void *)42);

    printf("Item encontrado: %lld\n", (long long)item_encontrado);

    Lista_for_item_em((&lista))
        printf("%lld\n", (long long)item->conteudo);

    Lista_liberar(&lista, 0);
}