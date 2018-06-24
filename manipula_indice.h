#ifndef __MANIPULA_INDICE_H__
#define __MANIPULA_INDICE_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define arquivoIndice "indice.bin"
#define arquivoBuffer "buffer-info.text"
#define TAMANHOPAGINA 116
#define T_CABECALHO_INDICE 13
#define bool int
#define true 1
#define false 0

struct elemento{

	int chave;
	int RRN;
};

typedef struct elemento Elemento;

struct pagina{
	int RRN;
	bool Modified;
	int n;
	int ponteiros[10];
	Elemento elementos[9];
};

typedef struct pagina Pagina;

struct buffer{
	Pagina* pages; // vetor de paginas
	int UltimoRRN; // Valor de RRN da ultima pagina criada
	int noRaiz; // RRN do No Raiz da arvore-b
	int n; // Numero de Paginas no buffer
	int page_hit; // Quantas vezes tentou acessar uma pagina no buffer e ela estava la
	int page_fault; // E quantas ela nao estava

};

typedef struct buffer Buffer_Pool;


void put(Buffer_Pool* b, Pagina p); // Coloca p no buffer(se ela n estiver la insere, se estiver escreve por cima e marca como mdoficada)
void Rearranja(Buffer_Pool* b, Pagina p); // Coloca a pagina p, no fim do vetor no buffer
void Flush(Pagina* p); // Escreve a pagina p no arquivo de indice
void LRU(Buffer_Pool* b, Pagina p); // Aplica a politica de substituição(chama Flush e Rearranja no seu algoritmo)
Pagina get(Buffer_Pool* b, int RRN); // Retorna a pagina do RRN solicitado (chama put() caso a pagina não esteja no buffer)
void ImprimeBuffer(Buffer_Pool* b); // Imprime todas as paginas do buffer


typedef struct pagina Pagina;

void split(Buffer_Pool* b, int* codEscola, int* data_reference, int* ultimoRRN, int RRN, int pai, int* noRaiz, int* propagacao);
void escrevePagina(FILE* fp, Pagina* p, int RRN);
void Ordena(Pagina* p, int codEscola, int data_reference);
void percorreArvore(Buffer_Pool* b, int* codEscola, int* data_reference, int* ultimoRRN, int RRN, int pai, int* flag, int* noRaiz, int* propagacao);
void insereIndice(Buffer_Pool* b, int codEscola, int RRN);
void imprimePagina(Pagina* p);
bool proxPagina(FILE *fp);
Pagina* pag(FILE* fp, int RRN);
Pagina NoVazio();
void ImprimeIndice(FILE* fp);

int buscaIndice(int codEscola);

#endif