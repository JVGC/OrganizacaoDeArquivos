#ifndef __MANIPULA_INDICE_H__
#define __MANIPULA_INDICE_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define arquivoIndice "indice.bin"
#define TAMANHOPAGINA 116
#define T_CABECALHO_INDICE 13
#define bool int
#define true 1
#define false 0




struct elemento{

	int chave;
	int RRN;
	int n; // numero de paginas no buffer
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
	Pagina* pages;
	int UltimoRRN;
	int noRaiz;
	int n;
	int page_hit;
	int page_fault;

};

typedef struct buffer Buffer_Pool;
void put(Buffer_Pool* b, Pagina* p);
void Rearranja(Buffer_Pool* b, Pagina p);
void Flush(Pagina* p);
void LRU(Buffer_Pool* b, Pagina* p);
Pagina* get(Buffer_Pool* b, int RRN);



int split(Buffer_Pool* b, int* codEscola, int* data_reference, int* ultimoRRN, int RRN, int pai, int noRaiz);
void escrevePagina(FILE* fp, Pagina* p, int RRN);
void Ordena(Pagina* p, int codEscola, int data_reference);
int percorreArvore(Buffer_Pool* b, int* codEscola, int* data_reference, int* ultimoRRN, int RRN, int pai, int* flag, int noRaiz);
void insereIndice(Buffer_Pool* b, int codEscola, int RRN);
void imprimePagina(Pagina* p);
bool proxPagina(FILE *fp);
Pagina* pag(FILE* fp, int RRN);
Pagina* NoVazio();
void ImprimeIndice(FILE* fp);
#endif