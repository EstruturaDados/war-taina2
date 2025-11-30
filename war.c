/*
    Jogo de Estratégia - TechNova War
    ---------------------------------
    Demonstra:
    - Structs (Territory, Player, Mission, Game)
    - Alocação dinâmica: malloc, calloc, free
    - Ponteiros e ponteiro para função (missões)
    - Estrutura não linear (grafo de territórios via vizinhos)
    - Modularização por funções
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ==========================
   DEFINIÇÕES BÁSICAS
   ========================== */

#define MAX_NOME 30

/* Forward declaration de Game para usar em ponteiros de função */
struct Game;
struct Player;

/* ==========================
   STRUCTS PRINCIPAIS
   ========================== */

/* Território: nó de um grafo (tem ponteiros para vizinhos) */
typedef struct Territory {
    int id;
    char nome[MAX_NOME];
    int dono;                 // id do jogador (-1 se neutro)
    int tropas;               // quantidade de tropas no território

    int numVizinhos;
    struct Territory **vizinhos; // lista de ponteiros para territórios vizinhos
} Territory;

/* Jogador */
typedef struct Player {
    int id;
    char nome[MAX_NOME];
    int numTerritorios;
} Player;

/* Tipos de missão possíveis */
typedef enum {
    MISSION_DOMINAR_N_TERRITORIOS
    // Poderia ter outros tipos: eliminar jogador, dominar região, etc.
} MissionType;

/* Ponteiro para função que verifica missão: retorna 1 se cumpriu, 0 se não */
typedef int (*MissionCheckFunc)(struct Game *game, struct Player *p);

/* Missão associada a um jogador */
typedef struct Mission {
    MissionType tipo;
    int alvo;               // aqui será o número mínimo de territórios a dominar
    MissionCheckFunc check; // função que verifica se a missão foi cumprida
} Mission;

/* Estrutura principal do jogo (TAD Game) */
typedef struct Game {
    Territory *territorios;  // vetor dinâmico de territórios
    int numTerritorios;

    Player *jogadores;       // vetor dinâmico de jogadores
    int numJogadores;

    Mission *missoes;        // uma missão por jogador
} Game;

/* ==========================
   PROTÓTIPOS DAS FUNÇÕES
   ========================== */

/* Criação / destruição do jogo */
Game *criarJogo(int numTerr, int numPlayers);
void destruirJogo(Game *game);

/* Inicializações */
void inicializarTerritoriosPadrao(Game *game);
void inicializarJogadores(Game *game);
void distribuirTerritorios(Game *game);
void inicializarMissoes(Game *game);

/* Lógica principal */
int atacar(Game *game, int idJogador, int idOrigem, int idAlvo, int tropas);

int missionDominarNTerritorios(Game *game, Player *p);
int verificarVitoria(Game *game, Player *p);

/* Utilitárias de exibição e entrada */
void mostrarEstadoJogador(Game *game, Player *p);
void listarTodosTerritorios(Game *game);
int lerInteiro(const char *msg);
int rolarDado();

/* ==========================
   IMPLEMENTAÇÕES
   ========================== */

/* Cria e aloca toda a estrutura Game dinamicamente */
Game *criarJogo(int numTerr, int numPlayers) {
    Game *game = (Game *) malloc(sizeof(Game));
    if (!game) {
        printf("Erro ao alocar Game\n");
        exit(1);
    }

    game->numTerritorios = numTerr;
    game->territorios = (Territory *) calloc(numTerr, sizeof(Territory));
    if (!game->territorios) {
        printf("Erro ao alocar territorios\n");
        free(game);
        exit(1);
    }

    game->numJogadores = numPlayers;
    game->jogadores = (Player *) calloc(numPlayers, sizeof(Player));
    if (!game->jogadores) {
        printf("Erro ao alocar jogadores\n");
        free(game->territorios);
        free(game);
        exit(1);
    }

    game->missoes = (Mission *) calloc(numPlayers, sizeof(Mission));
    if (!game->missoes) {
        printf("Erro ao alocar missoes\n");
        free(game->jogadores);
        free(game->territorios);
        free(game);
        exit(1);
    }

    return game;
}

/* Libera toda a memória alocada dinamicamente */
void destruirJogo(Game *game) {
    if (!game) return;

    /* libera o vetor de vizinhos de cada território */
    for (int i = 0; i < game->numTerritorios; i++) {
        if (game->territorios[i].vizinhos != NULL) {
            free(game->territorios[i].vizinhos);
        }
    }

    free(game->territorios);
    free(game->jogadores);
    free(game->missoes);
    free(game);
}

/* Inicializa territórios com um mapa simples em anel (grafo) */
void inicializarTerritoriosPadrao(Game *game) {
    int n = game->numTerritorios;

    for (int i = 0; i < n; i++) {
        game->territorios[i].id = i;
        sprintf(game->territorios[i].nome, "Territorio %d", i);
        game->territorios[i].dono = -1;
        game->territorios[i].tropas = 0;

        /* Cada território terá 2 vizinhos: anterior e próximo (em anel) */
        game->territorios[i].numVizinhos = 2;
        game->territorios[i].vizinhos = (Territory **) malloc(2 * sizeof(Territory *));
        if (!game->territorios[i].vizinhos) {
            printf("Erro ao alocar vizinhos do territorio %d\n", i);
            exit(1);
        }

        int vizEsq = (i - 1 + n) % n;
        int vizDir = (i + 1) % n;

        game->territorios[i].vizinhos[0] = &game->territorios[vizEsq];
        game->territorios[i].vizinhos[1] = &game->territorios[vizDir];
    }
}

/* Lê nomes dos jogadores e zera contadores */
void inicializarJogadores(Game *game) {
    printf("=== Cadastro de Jogadores ===\n");
    for (int i = 0; i < game->numJogadores; i++) {
        game->jogadores[i].id = i;
        game->jogadores[i].numTerritorios = 0;

        printf("Nome do jogador %d: ", i);
        /* " %29[^\n]" lê até 29 caracteres incluindo espaços */
        scanf(" %29[^\n]", game->jogadores[i].nome);
    }
}

/* Distribui territórios de forma simples: alternando entre os jogadores */
void distribuirTerritorios(Game *game) {
    printf("\nDistribuindo territorios...\n");

    for (int i = 0; i < game->numTerritorios; i++) {
        int idDono = i % game->numJogadores;
        game->territorios[i].dono = idDono;
        game->territorios[i].tropas = 3; /* tropas iniciais por território */

        game->jogadores[idDono].numTerritorios++;
    }
}

/* Inicializa missoes: cada jogador deve dominar pelo menos T territórios
   T = (numTerritorios / numJogadores) + 1  */
void inicializarMissoes(Game *game) {
    int base = game->numTerritorios / game->numJogadores;
    int alvo = base + 1;

    printf("\n=== Missoes ===\n");
    for (int i = 0; i < game->numJogadores; i++) {
        game->missoes[i].tipo = MISSION_DOMINAR_N_TERRITORIOS;
        game->missoes[i].alvo = alvo;
        game->missoes[i].check = missionDominarNTerritorios;

        printf("Jogador %s deve dominar pelo menos %d territorios.\n",
               game->jogadores[i].nome, alvo);
    }
}

/* Função utilitária: rola um dado de 6 lados */
int rolarDado() {
    return rand() % 6 + 1;
}

/* Ataque entre territórios */
int atacar(Game *game, int idJogador, int idOrigem, int idAlvo, int tropas) {
    if (idOrigem < 0 || idOrigem >= game->numTerritorios ||
        idAlvo   < 0 || idAlvo   >= game->numTerritorios) {
        printf("ID de territorio invalido.\n");
        return 0;
    }

    Territory *origem = &game->territorios[idOrigem];
    Territory *alvo   = &game->territorios[idAlvo];

    if (origem->dono != idJogador) {
        printf("Voce nao controla o territorio de origem!\n");
        return 0;
    }
    if (origem->tropas <= tropas || tropas <= 0) {
        printf("Tropas insuficientes para atacar!\n");
        return 0;
    }
    if (origem == alvo) {
        printf("Nao pode atacar o mesmo territorio.\n");
        return 0;
    }

    /* Verificar se é vizinho */
    int vizinho = 0;
    for (int i = 0; i < origem->numVizinhos; i++) {
        if (origem->vizinhos[i] == alvo) {
            vizinho = 1;
            break;
        }
    }
    if (!vizinho) {
        printf("Territorio alvo nao eh vizinho.\n");
        return 0;
    }

    printf("\n--- Ataque ---\n");
    printf("Origem: %s (tropas: %d)\n", origem->nome, origem->tropas);
    printf("Alvo:   %s (tropas: %d)\n", alvo->nome, alvo->tropas);

    /* Soma de dados para ataque e defesa (simplificado) */
    int somaAtaque = 0, somaDefesa = 0;
    for (int i = 0; i < tropas; i++) {
        somaAtaque += rolarDado();
    }
    for (int i = 0; i < alvo->tropas; i++) {
        somaDefesa += rolarDado();
    }

    printf("Resultado dados -> Ataque: %d | Defesa: %d\n", somaAtaque, somaDefesa);

    if (somaAtaque > somaDefesa) {
        printf("Territorio conquistado!\n");

        int antigoDono = alvo->dono;

        /* Atualiza contagem de territórios dos jogadores */
        if (antigoDono != -1 && antigoDono != idJogador) {
            game->jogadores[antigoDono].numTerritorios--;
        }
        game->jogadores[idJogador].numTerritorios++;

        /* Transfere tropas */
        origem->tropas -= tropas;
        alvo->dono = idJogador;
        alvo->tropas = tropas;

        return 1;
    } else {
        printf("Ataque fracassou. Tropas perdidas.\n");
        origem->tropas -= tropas;
        if (origem->tropas < 0) origem->tropas = 0;
        return 0;
    }
}

/* Missão: dominar N territórios */
int missionDominarNTerritorios(Game *game, Player *p) {
    int count = 0;

    for (int i = 0; i < game->numTerritorios; i++) {
        if (game->territorios[i].dono == p->id) {
            count++;
        }
    }

    int alvo = game->missoes[p->id].alvo;
    return (count >= alvo);
}

/* Chama a função de checagem associada à missão do jogador */
int verificarVitoria(Game *game, Player *p) {
    Mission *m = &game->missoes[p->id];
    if (m->check != NULL) {
        return m->check(game, p);
    }
    return 0;
}

/* Exibe os territórios controlados pelo jogador */
void mostrarEstadoJogador(Game *game, Player *p) {
    printf("\n=== Estado do Jogador: %s ===\n", p->nome);
    printf("Territorios controlados:\n");
    for (int i = 0; i < game->numTerritorios; i++) {
        Territory *t = &game->territorios[i];
        if (t->dono == p->id) {
            printf("  ID %d - %s | Tropas: %d\n", t->id, t->nome, t->tropas);
        }
    }
}

/* Lista todos os territórios do jogo */
void listarTodosTerritorios(Game *game) {
    printf("\n=== Todos os territorios ===\n");
    for (int i = 0; i < game->numTerritorios; i++) {
        Territory *t = &game->territorios[i];
        printf("ID %d - %s | Dono: %d | Tropas: %d\n",
               t->id, t->nome, t->dono, t->tropas);
    }
}

/* Lê um inteiro da entrada com mensagem */
int lerInteiro(const char *msg) {
    int x;
    printf("%s", msg);
    scanf("%d", &x);
    return x;
}

/* ==========================
   FUNÇÃO PRINCIPAL (main)
   ========================== */

int main() {
    srand((unsigned) time(NULL));

    int numTerritorios = 6;  /* pode alterar */
    int numJogadores   = 2;  /* pode alterar */

    Game *game = criarJogo(numTerritorios, numJogadores);

    inicializarTerritoriosPadrao(game);
    inicializarJogadores(game);
    distribuirTerritorios(game);
    inicializarMissoes(game);

    int jogadorAtual = 0;
    int venceu = 0;

    printf("\n=== INICIO DO JOGO TECHNOVA WAR ===\n");

    while (!venceu) {
        Player *p = &game->jogadores[jogadorAtual];

        printf("\n--------------------------------------\n");
        printf("Turno do jogador: %s (id %d)\n", p->nome, p->id);

        mostrarEstadoJogador(game, p);
        listarTodosTerritorios(game);

        printf("\nMenu:\n");
        printf(" 1 - Atacar\n");
        printf(" 0 - Pular turno\n");
        printf(" 9 - Encerrar jogo\n");

        int opcao = lerInteiro("Escolha: ");

        if (opcao == 9) {
            printf("Jogo encerrado pelo usuario.\n");
            break;
        } else if (opcao == 1) {
            int idOrigem = lerInteiro("ID do territorio de origem: ");
            int idAlvo   = lerInteiro("ID do territorio alvo: ");
            int tropas   = lerInteiro("Quantidade de tropas para atacar: ");

            atacar(game, p->id, idOrigem, idAlvo, tropas);
        } else {
            printf("Turno pulado.\n");
        }

        /* Verificação de vitória após a ação do jogador */
        if (verificarVitoria(game, p)) {
            printf("\n=====================================\n");
            printf("Jogador %s CUMPRIU A MISSAO e venceu!\n", p->nome);
            printf("=====================================\n");
            venceu = 1;
        } else {
            /* Passa a vez para o próximo jogador */
            jogadorAtual = (jogadorAtual + 1) % game->numJogadores;
        }
    }

    destruirJogo(game);
    return 0;
}
