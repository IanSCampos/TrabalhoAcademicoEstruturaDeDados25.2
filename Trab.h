#ifndef REDEFARMA_H
#define REDEFARMA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Catálogo de Produtos 
typedef struct Produto {
    int codigo;
    char descricao[64];
    float preco;
    struct Produto *prox;
} Produto;

// Estoque 
typedef struct ItemEstoque {
    int codigo_produto;
    int quantidade;
    struct ItemEstoque *prox;
} ItemEstoque;

// Filial
typedef struct Filial {
    int id_filial;
    char nome[50]; 
    ItemEstoque *estoque; 
    struct Filial *prox;
} Filial;

// Item do Carrinho
typedef struct ItemCarrinho {
    int codigo_produto;
    int qtd;
    float preco_unit;
    struct ItemCarrinho *prox;
} ItemCarrinho;

// Carrinho
typedef struct Carrinho {
    ItemCarrinho *itens; 
    float total;
} Carrinho;


Produto* carregar_produtos(const char *nome_arquivo);
Filial* carregar_filiais(const char *nome_arquivo);
void carregar_estoques(const char *nome_arquivo, Filial *cabeca_filial);
Produto* inserir_produto(Produto *cabeca, int codigo, const char *descricao, float preco);
ItemEstoque* inserir_estoque(ItemEstoque *cabeca, int codigo_produto, int quantidade);

void adicionar_item_carrinho(Carrinho *carrinho, Produto *catalogo, int codigo, int quantidade);
void remover_item_carrinho(Carrinho *carrinho, int codigo, int quantidade);
void listar_carrinho(Carrinho *carrinho, Produto *catalogo);
void recalcular_total(Carrinho *carrinho);


void verificar_atendimento_geral(Filial *lista_filiais, Carrinho *carrinho, Produto *catalogo);
void finalizar_compra(Filial *lista_filiais, Produto *catalogo, Carrinho *carrinho, int id_filial);

Filial* buscar_filial_por_id(Filial *cabeca, int id);
Produto* buscar_produto_por_codigo(Produto *cabeca, int codigo);
ItemEstoque* buscar_item_estoque(ItemEstoque *cabeca_estoque, int codigo);
ItemCarrinho* buscar_item_carrinho(Carrinho *carrinho, int codigo);

void desalocar_memoria(Produto *produtos, Filial *filiais, Carrinho *carrinho);

#endif