#include "Trab.h"
#include <limits.h>

//Insere no estoque
ItemEstoque* inserir_estoque(ItemEstoque *cabeca, int codigo_produto, int quantidade) {
    ItemEstoque *novo_item = (ItemEstoque *)malloc(sizeof(ItemEstoque));
    novo_item->codigo_produto = codigo_produto;
    novo_item->quantidade = quantidade;
    novo_item->prox = NULL;

    if (cabeca == NULL || codigo_produto < cabeca->codigo_produto) {
        novo_item->prox = cabeca;
        return novo_item;
    }

    ItemEstoque *atual = cabeca;
    while (atual->prox != NULL && atual->prox->codigo_produto < codigo_produto) {
        atual = atual->prox;
    }

    if (atual->codigo_produto == codigo_produto) {
        atual->quantidade += quantidade;
        free(novo_item);
    } else {
        novo_item->prox = atual->prox;
        atual->prox = novo_item;
    }

    return cabeca;
}

// Insere Produto
Produto* inserir_produto(Produto *cabeca, int codigo, const char *descricao, float preco) {
    Produto *novo_produto = (Produto *)malloc(sizeof(Produto));
    novo_produto->codigo = codigo;
    strncpy(novo_produto->descricao, descricao, 64);
    novo_produto->preco = preco;
    novo_produto->prox = NULL;

    if (cabeca == NULL || codigo < cabeca->codigo) {
        novo_produto->prox = cabeca;
        return novo_produto;
    }

    Produto *atual = cabeca;
    while (atual->prox != NULL && atual->prox->codigo < codigo) {
        atual = atual->prox;
    }

    if (atual->codigo == codigo) {
        // Produto já existe, atualizar informações
        strncpy(atual->descricao, descricao, 64);
        atual->preco = preco;
        free(novo_produto);
    } else {
        novo_produto->prox = atual->prox;
        atual->prox = novo_produto;
    }

    return cabeca;
}

//Carrega produtos
Produto* carregar_produtos(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de produtos.\n");
        return NULL;
    }

    Produto *cabeca = NULL;
    int codigo;
    char descricao[64];
    float preco;

    while (fscanf(arquivo, "%d %63s %f\n", &codigo, descricao, &preco) == 3) {
        cabeca = inserir_produto(cabeca, codigo, descricao, preco);
    }

    fclose(arquivo);
    return cabeca;
}

//Carrega filiais
Filial* carregar_filiais(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de filiais.\n");
        return NULL;
    }

    Filial *cabeca = NULL;
    int id_filial;
    char nome[19];
    char nome2[29];

    while(fscanf(arquivo, "%d %19s %29s \n", &id_filial, nome, nome2) == 3) {
        Filial *novo_filial = (Filial *)malloc(sizeof(Filial));
        novo_filial->id_filial = id_filial;
        snprintf(novo_filial->nome, 50, "%s %s", nome, nome2);
        novo_filial->estoque = NULL;
        novo_filial->prox = cabeca;
        cabeca = novo_filial;
    }

    fclose(arquivo);
    return cabeca;
}

//Carrega estoques
void carregar_estoques(const char *nome_arquivo, Filial *cabeca_filial) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de estoques.\n");
        return;
    }

    int id_filial, codigo_produto, quantidade;
    while (fscanf(arquivo, "%d %d %d\n", &id_filial, &codigo_produto, &quantidade) == 3) {
        Filial *filial = buscar_filial_por_id(cabeca_filial, id_filial);
        if (filial != NULL) {
            filial->estoque = inserir_estoque(filial->estoque, codigo_produto, quantidade);
        }
    }

    fclose(arquivo);
}

//Busca filial
Filial* buscar_filial_por_id(Filial *cabeca, int id) {
    Filial *atual = cabeca;
    while (atual != NULL) {
        if (atual->id_filial == id) {
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;
}

//Busca produto
Produto* buscar_produto_por_codigo(Produto *cabeca, int codigo) {
    Produto *atual = cabeca;
    while (atual != NULL) {
        if (atual->codigo == codigo) {
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;
}

//Busca no estoque
ItemEstoque* buscar_item_estoque(ItemEstoque *cabeca_estoque, int codigo) {
    ItemEstoque *atual = cabeca_estoque;
    while (atual != NULL) {
        if (atual->codigo_produto == codigo) {
            return atual;
        }
        if (atual->codigo_produto > codigo) {
            return NULL;
        }
        atual = atual->prox;
    }
    return NULL;
}

//Busca no carrinho
ItemCarrinho* buscar_item_carrinho(Carrinho *carrinho, int codigo) {
    if (carrinho == NULL) return NULL;
    
    ItemCarrinho *atual = carrinho->itens;
    while (atual != NULL) {
        if (atual->codigo_produto == codigo) {
            return atual;
        }
        if (atual->codigo_produto > codigo) { // Otimização
            return NULL;
        }
        atual = atual->prox;
    }
    return NULL;
}

//recalcula total
void recalcular_total(Carrinho *carrinho) {
    carrinho->total = 0.0;
    ItemCarrinho *atual = carrinho->itens;
    
    while (atual != NULL) {
        carrinho->total += (atual->preco_unit * atual->qtd);
        atual = atual->prox;
    }
}

//Adiciona item ao carrinho
void adicionar_item_carrinho(Carrinho *carrinho, Produto *catalogo, int codigo, int quantidade) {
    if (carrinho == NULL) {
        printf("  ADD ERRO: Nenhum carrinho ativo.\n");
        return;
    }
    if (quantidade <= 0) {
        printf("  ADD ERRO: Quantidade invalida (%d).\n", quantidade);
        return;
    }

    // 1. Buscar produto no catálogo (para pegar preço e validar)
    Produto *p = buscar_produto_por_codigo(catalogo, codigo);
    if (p == NULL) {
        printf("  ADD ERRO: Produto %d nao encontrado no catalogo.\n", codigo);
        return;
    }

    // 2. Verificar se já existe no carrinho
    ItemCarrinho *item_existente = buscar_item_carrinho(carrinho, codigo);

    if (item_existente != NULL) {
        item_existente->qtd += quantidade;
        printf("  ADD: %d %s (Qtd: +%d -> Total no carrinho: %d)\n", codigo, p->descricao, quantidade, item_existente->qtd);
    } else {
        ItemCarrinho *novo_item = (ItemCarrinho*) malloc(sizeof(ItemCarrinho));
        novo_item->codigo_produto = codigo;
        novo_item->qtd = quantidade;
        novo_item->preco_unit = p->preco;
        novo_item->prox = NULL;

        if (carrinho->itens == NULL || carrinho->itens->codigo_produto > codigo) {
            novo_item->prox = carrinho->itens;
            carrinho->itens = novo_item;
        } else {
            ItemCarrinho *ant = carrinho->itens;
            while (ant->prox != NULL && ant->prox->codigo_produto < codigo) {
                ant = ant->prox;
            }
            novo_item->prox = ant->prox;
            ant->prox = novo_item;
        }
        printf("  ADD: %d %s (Qtd: %d)\n", codigo, p->descricao, quantidade);
    }

    // 4. Atualizar total
    recalcular_total(carrinho);
}

//remover item do carrinho
void remover_item_carrinho(Carrinho *carrinho, int codigo, int quantidade) {
    if (carrinho == NULL || carrinho->itens == NULL) {
        printf("  REMOVE ERRO: Carrinho esta vazio.\n");
        return;
    }
    if (quantidade <= 0) {
        printf("  REMOVE ERRO: Quantidade invalida (%d).\n", quantidade);
        return;
    }

    // 1. Encontrar o item e seu anterior
    ItemCarrinho *ant = NULL;
    ItemCarrinho *atual = carrinho->itens;

    while (atual != NULL && atual->codigo_produto != codigo) {
        ant = atual;
        atual = atual->prox;
    }

    // 2. Se não encontrou
    if (atual == NULL) {
        printf("  REMOVE ERRO: Produto %d nao esta no carrinho.\n", codigo);
        return;
    }

    // 3. Se encontrou, subtrair
    printf("  REMOVE: %d (Qtd: -%d)\n", codigo, quantidade);
    
    // 4. Verificar se a remoção é total ou parcial
    if (quantidade >= atual->qtd) {
        if (ant == NULL) {
            carrinho->itens = atual->prox;
        } else { 
            ant->prox = atual->prox;
        }
        free(atual);
        printf("  INFO: Produto %d removido completamente do carrinho.\n", codigo);
    } else {
        // Apenas subtrai a quantidade
        atual->qtd -= quantidade;
        printf("  INFO: Qtd restante de %d no carrinho: %d\n", codigo, atual->qtd);
    }

    // 5. Atualizar total
    recalcular_total(carrinho);
}

//listar carrinho
void listar_carrinho(Carrinho *carrinho, Produto *catalogo) {
    if (carrinho == NULL) {
        printf("Nenhum carrinho ativo.\n");
        return;
    }
    if (carrinho->itens == NULL) {
        printf("Carrinho esta vazio. Total: R$ 0.00\n");
        return;
    }

    printf("--- ITENS NO CARRINHO ---\n");
    ItemCarrinho *atual = carrinho->itens;
    
    while (atual != NULL) {
        Produto *p = buscar_produto_por_codigo(catalogo, atual->codigo_produto);
        char *descricao = (p != NULL) ? p->descricao : "DESCONHECIDO";
        
        printf("  - [%d] %s (Qtd: %d, Unit: R$%.2f, Sub: R$%.2f)\n",
               atual->codigo_produto, descricao, atual->qtd,
               atual->preco_unit, (atual->qtd * atual->preco_unit));
        
        atual = atual->prox;
    }
    printf("-------------------------\n");
    printf("TOTAL NO CARRINHO: R$ %.2f\n", carrinho->total);
}

//Verificar 
void verificar_atendimento_geral(Filial *lista_filiais, Carrinho *carrinho, Produto *catalogo) {
    if (carrinho == NULL || carrinho->itens == NULL) {
        printf("--- VERIFICAR ATENDIMENTO (Carrinho vazio) ---\n");
        return;
    }

    printf("--- VERIFICAR ATENDIMENTO ---\n");
    
    printf("Filiais 100%% Aptas:\n");
    int aptas_count = 0;
    Filial *f_atual = lista_filiais;
    
    while (f_atual != NULL) {
        int filial_apta = 1;
        ItemCarrinho *item = carrinho->itens;
        
        while (item != NULL) {
            ItemEstoque *est = buscar_item_estoque(f_atual->estoque, item->codigo_produto);
            int disponivel = (est == NULL) ? 0 : est->quantidade;
            
            if (disponivel < item->qtd) {
                filial_apta = 0;
                break; 
            }
            item = item->prox;
        }
        
        if (filial_apta) {
            printf("  - %d: %s\n", f_atual->id_filial, f_atual->nome);
            aptas_count++;
        }
        f_atual = f_atual->prox;
    }
    if (aptas_count == 0) {
        printf("  (Nenhuma filial atende 100%% do pedido)\n");
    }

    printf("\nFiliais Nao Aptas (Diagnostico de Faltas):\n");
    f_atual = lista_filiais;
    int nao_aptas_count = 0;

    while (f_atual != NULL) {
        int tem_falta = 0;
        
        ItemCarrinho *item_check = carrinho->itens;
        while(item_check != NULL) {
            ItemEstoque *est = buscar_item_estoque(f_atual->estoque, item_check->codigo_produto);
            int disponivel = (est == NULL) ? 0 : est->quantidade;
            if (disponivel < item_check->qtd) {
                tem_falta = 1;
                break;
            }
            item_check = item_check->prox;
        }
        
        if (tem_falta) {
            nao_aptas_count++;
            printf("  - Filial %d (%s):\n", f_atual->id_filial, f_atual->nome);
            
            ItemCarrinho *item_diag = carrinho->itens; 
            while (item_diag != NULL) {
                 ItemEstoque *est = buscar_item_estoque(f_atual->estoque, item_diag->codigo_produto);
                 int disponivel = (est == NULL) ? 0 : est->quantidade;
                 
                 if (disponivel < item_diag->qtd) {
                     Produto *p = buscar_produto_por_codigo(catalogo, item_diag->codigo_produto);
                     printf("    - FALTA: %d %s (Solicitado: %d, Disponivel: %d, Faltam: %d)\n",
                            item_diag->codigo_produto, p->descricao, 
                            item_diag->qtd, disponivel, item_diag->qtd - disponivel);
                 }
                item_diag = item_diag->prox;
            }
        }
        f_atual = f_atual->prox;
    }
    
    if (nao_aptas_count == 0 && aptas_count > 0) {
        printf("  (Todas as filiais estao aptas)\n");
    }

    printf("------------------------------\n");
}

//Finalizar
void finalizar_compra(Filial *lista_filiais, Produto *catalogo, Carrinho *carrinho, int id_filial) {
    if (carrinho == NULL || carrinho->itens == NULL) {
        printf("--- FINALIZAR ERRO (Carrinho vazio) ---\n");
        return;
    }

    Filial *filial_escolhida = buscar_filial_por_id(lista_filiais, id_filial);
    if (filial_escolhida == NULL) {
        printf("--- FINALIZAR ERRO (Filial %d nao encontrada) ---\n", id_filial);
        return;
    }
    
    printf("--- FINALIZAR COMPRA (Filial: %d %s) ---\n", id_filial, filial_escolhida->nome);

    printf("Verificando e ajustando carrinho para estoque da filial...\n");
    
    ItemCarrinho *ant = NULL;
    ItemCarrinho *atual = carrinho->itens;
    int itens_ajustados = 0;

    while (atual != NULL) {
        ItemEstoque *est = buscar_item_estoque(filial_escolhida->estoque, atual->codigo_produto);
        int disponivel = (est == NULL) ? 0 : est->quantidade;
        Produto *p = buscar_produto_por_codigo(catalogo, atual->codigo_produto);

        if (disponivel < atual->qtd) {
            itens_ajustados = 1;
            
            if (disponivel == 0) {
                printf("  AJUSTE: %d %s REMOVIDO (Disponivel: 0, Solicitado: %d)\n",
                       atual->codigo_produto, p->descricao, atual->qtd);
                
                ItemCarrinho *temp = atual;
                if (ant == NULL) {
                    carrinho->itens = atual->prox;
                    atual = carrinho->itens;
                } else {
                    ant->prox = atual->prox;
                    atual = atual->prox;
                }
                free(temp);
                continue;
                
            } else {
                printf("  AJUSTE: %d %s REDUZIDO (Disponivel: %d, Solicitado: %d)\n",
                       atual->codigo_produto, p->descricao, disponivel, atual->qtd);
                atual->qtd = disponivel;
            }
        }
        
        ant = atual;
        atual = atual->prox;
    }

    if (itens_ajustados) {
        recalcular_total(carrinho);
    }

    if (carrinho->itens == NULL) {
        printf("\nVENDA CANCELADA: Carrinho ficou vazio apos ajustes de estoque.\n");
        printf("Total: R$ %.2f\n", carrinho->total);
        printf("------------------------------\n");
        return;
    }

    printf("\nVENDA CONFIRMADA (Estoque Abatido):\n");
    atual = carrinho->itens;
    
    while (atual != NULL) {
        ItemEstoque *est = buscar_item_estoque(filial_escolhida->estoque, atual->codigo_produto);

        est->quantidade -= atual->qtd;

        Produto *p = buscar_produto_por_codigo(catalogo, atual->codigo_produto);
        printf("  - %d %s (Qtd: %d, Preco: R$%.2f, Sub: R$%.2f)\n",
               atual->codigo_produto, p->descricao, atual->qtd,
               atual->preco_unit, (atual->qtd * atual->preco_unit));
        
        atual = atual->prox;
    }

    printf("\nTOTAL DA COMPRA: R$ %.2f\n", carrinho->total);
    printf("------------------------------\n");
}

//Desalocar memoria
void desalocar_memoria(Produto *catalogo, Filial *filiais, Carrinho *carrinho_atual) {
    
    printf("\n--- INICIANDO LIMPEZA GERAL DA MEMORIA ---\n");

    // 1. Desalocar o Catálogo de Produtos
    printf("Liberando catalogo de produtos...\n");
    Produto *prod_atual = catalogo;
    while (prod_atual != NULL) {
        Produto *prod_prox = prod_atual->prox;
        free(prod_atual);
        prod_atual = prod_prox;
    }

    // 2. Desalocar Filiais e seus Estoques Internos
    printf("Liberando filiais e estoques...\n");
    Filial *filial_atual = filiais;
    while (filial_atual != NULL) {
        
        ItemEstoque *est_atual = filial_atual->estoque;
        while (est_atual != NULL) {
            ItemEstoque *est_prox = est_atual->prox;
            free(est_atual);
            est_atual = est_prox;
        }
        
        Filial *filial_prox = filial_atual->prox;
        printf("  - Filial %d liberada.\n", filial_atual->id_filial);
        free(filial_atual);
        filial_atual = filial_prox;
    }

    // 3. Desalocar o Carrinho Atual (se ele existir)
    printf("Liberando carrinho de compras pendente...\n");
    if (carrinho_atual != NULL) {
        
        ItemCarrinho *item_atual = carrinho_atual->itens;
        while (item_atual != NULL) {
            ItemCarrinho *item_prox = item_atual->prox;
            free(item_atual);
            item_atual = item_prox;
        }
        
        free(carrinho_atual);
        printf("  - Carrinho pendente liberado.\n");
    } else {
        printf("  - Nenhum carrinho pendente.\n");
    }
    
    printf("--- MEMORIA LIBERADA COM SUCESSO ---\n");
}

int main(int argc, char const *argv[])
{
    printf("Iniciando Simulador RedeFarma...\n");

    Produto *catalogo = carregar_produtos("produtos_v2.txt");
    if (catalogo == NULL) return 1;
    
    Filial *filiais = carregar_filiais("filiais_v2.txt");
    if (filiais == NULL) {
        desalocar_memoria(catalogo, NULL, NULL);
        return 1;
    }
    
    carregar_estoques("estoques_v2.txt", filiais);
    
    printf("Dados carregados. Processando simulacao...\n\n");

    FILE *sim_file = fopen("carrinhos_clientes_v2.txt", "r");
    if (sim_file == NULL) {
        printf("ERRO FATAL: Nao foi possivel abrir 'carrinhos_clientes_v2.txt'\n");
        desalocar_memoria(catalogo, filiais, NULL);
        return 1;
    }

    Carrinho *carrinho_atual = NULL;
    char comando[100];
    int id_carrinho_ativo = 0;
    
    int id_carrinho, codigo_produto, quantidade, id_filial;

    while (fscanf(sim_file, "%99s", comando) != EOF) {
        
        if (comando[0] == '#') {
            char buffer_comentario[1024];
            fgets(buffer_comentario, 1024, sim_file);
        }

        if (strcmp(comando, "NOVO_CARRINHO") == 0) {
            fscanf(sim_file, "%d", &id_carrinho);
            
            if (carrinho_atual != NULL) {

                printf("AVISO: Carrinho %d anterior descartado (sem 'FIM').\n", id_carrinho_ativo);
                ItemCarrinho *temp_item = carrinho_atual->itens;
                while (temp_item != NULL) {
                    ItemCarrinho *prox_item = temp_item->prox;
                    free(temp_item);
                    temp_item = prox_item;
                }
                free(carrinho_atual);
            }
            
            carrinho_atual = (Carrinho*) malloc(sizeof(Carrinho));
            carrinho_atual->itens = NULL;
            carrinho_atual->total = 0.0;
            id_carrinho_ativo = id_carrinho;
            
            printf("--- NOVO CARRINHO %d --- (R$ %.2f)\n", id_carrinho_ativo, carrinho_atual->total);

        } else if (strcmp(comando, "ADD") == 0) {
            fscanf(sim_file, "%d %d", &codigo_produto, &quantidade);
            adicionar_item_carrinho(carrinho_atual, catalogo, codigo_produto, quantidade);

        } else if (strcmp(comando, "REMOVE") == 0) {
            fscanf(sim_file, "%d %d", &codigo_produto, &quantidade);
            remover_item_carrinho(carrinho_atual, codigo_produto, quantidade);

        } else if (strcmp(comando, "VERIFICAR_ATENDIMENTO") == 0) {
            verificar_atendimento_geral(filiais, carrinho_atual, catalogo);

        } else if (strcmp(comando, "FINALIZAR") == 0) {
            fscanf(sim_file, "%d", &id_filial);
            finalizar_compra(filiais, catalogo, carrinho_atual, id_filial);

        } else if (strcmp(comando, "FIM") == 0) {
            printf("--- FIM DO CARRINHO %d --- (Total Final: R$ %.2f)\n\n", 
                   id_carrinho_ativo, (carrinho_atual != NULL ? carrinho_atual->total : 0.0));
            
            if (carrinho_atual != NULL) {
                ItemCarrinho *temp_item = carrinho_atual->itens;
                while (temp_item != NULL) {
                    ItemCarrinho *prox_item = temp_item->prox;
                    free(temp_item);
                    temp_item = prox_item;
                }
                free(carrinho_atual);
                carrinho_atual = NULL;
            }
            id_carrinho_ativo = 0;
        }
    }

    fclose(sim_file);
    
    printf("\nSimulacao concluida. Liberando memoria principal...\n");
    
    desalocar_memoria(catalogo, filiais, carrinho_atual);

    printf("Fim do programa.\n");
    return 0;
}
