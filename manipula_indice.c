
#include "manipula_arquivo.h"
#include <string.h>

#include "manipula_indice.h"

void Ordena(Pagina* p, int codEscola, int data_reference){
	
	int i;

	for(i =0; i < p->n; i++){
		if(p->elementos[i].chave > codEscola){
			break;
		}
	}
	int pos = i;
	for(i = p->n; i> pos; i--){
		p->ponteiros[i+1] = p->ponteiros[i];
		p->elementos[i].chave = p->elementos[i-1].chave;
		p->elementos[i].RRN = p->elementos[i-1].RRN;
	}




	p->elementos[pos].chave = codEscola;
	p->elementos[pos].RRN = data_reference;

	




}


int split(Buffer_Pool* b, int* codEscola, int* data_reference, int* ultimoRRN, int RRN, int pai, int noRaiz){



	int i;
	//printf("split RRN = %d\n", RRN);
	Pagina* p = get(b, RRN);
	//imprimePagina(p);
	
	Pagina* Father = get(b, pai);
	//printf("split ola\n");



	Pagina* NewPag = NoVazio();

	//Crio um novo no na mesma altura do atual e divido os membros do no entre as duas paginas
	NewPag->n = (p->n)/2;
	for(i = NewPag->n; i < p->n; i++){
		NewPag->ponteiros[i - NewPag->n] = p->ponteiros[i];
		NewPag->elementos[i - NewPag->n].chave = p->elementos[i].chave;
		NewPag->elementos[i - NewPag->n].RRN = p->elementos[i].RRN;
		
	}

	if(*codEscola == 35000152)
	{
		imprimePagina(NewPag);
	}

	p->n = (p->n)/2;
	if(p->elementos[(p->n)].chave < *codEscola){
		NewPag->n++;
		Ordena(NewPag, *codEscola, *data_reference);
	}else{
		p->n++;
		Ordena(p, *codEscola, *data_reference);
		
		Ordena(NewPag,p->elementos[p->n-1].chave, p->elementos[p->n-1].RRN);
		p->n--;
		
	}


	if(Father != NULL){ // se ele tem pai
		// Insiro o primeiro elemento da nova chave do meu NewPag no Pai
		if(Father->n < 9){
			Ordena(Father, NewPag->elementos[0].chave, NewPag->elementos[0].RRN);
			Father->ponteiros[Father->n+1] = (*ultimoRRN)+1;
			Father->n++;
			*(ultimoRRN) += 1;
			NewPag->RRN = (*ultimoRRN);
		}else{
			*(ultimoRRN) += 1;
			NewPag->RRN = (*ultimoRRN);
			(*codEscola) = NewPag->elementos[0].chave;
			(*data_reference) = NewPag->elementos[0].RRN;
		}
		
		
	}else{

		Father = NoVazio();
		Ordena(Father, NewPag->elementos[0].chave, NewPag->elementos[0].RRN);
		Father->n = 1;
		Father->ponteiros[Father->n] = (*ultimoRRN) +1;
		Father->ponteiros[(Father->n)-1] = RRN;
		Father->RRN = (*ultimoRRN) +2;
		*(ultimoRRN) += 2;
		noRaiz = (*ultimoRRN);
		NewPag->RRN  = (*ultimoRRN) -1;
	}

	for(i = 0; i < (NewPag->n); i++){
		NewPag->ponteiros[i] = NewPag->ponteiros[i+1];
		NewPag->elementos[i].chave = NewPag->elementos[i+1].chave;
		NewPag->elementos[i].RRN = NewPag->elementos[i+1].RRN; 
	}
	NewPag->ponteiros[i] = NewPag->ponteiros[i+1];
	NewPag->elementos[i].chave = NewPag->elementos[i+1].chave;
	NewPag->elementos[i].RRN = NewPag->elementos[i+1].RRN;
	put(b, NewPag);

	put(b, p);
	put(b, Father); 


	return noRaiz;
	

}

int percorreArvore(Buffer_Pool* b, int* codEscola, int* data_reference, int* ultimoRRN, int RRN, int pai, int* flag, int noRaiz){
	if(RRN == -1){ // tenho que inserir no pai
		Pagina* p = get(b, pai);
		
	
		if(p->n == 9){ // no cheio, tenho que splitar(depois ver como fazer)
			*flag = 1;
		}else{
			Ordena(p, *codEscola, *data_reference);
			p->n++;
			put(b,p);
		}
		return noRaiz;
	}

	int i;
	
	
	Pagina* p = get(b, RRN);
	// imprimePagina(p);
	// exit(0);
	
	for (i = 0; i < p->n; i++){
		if(*codEscola == p->elementos[i].chave){
			return noRaiz;
		}else if(*codEscola < p->elementos[i].chave){

			percorreArvore(b, codEscola, data_reference, ultimoRRN, p->ponteiros[i], RRN, flag, noRaiz);
			if(*flag == 1){ // Tenho que fazer o split
				if(pai != -1){
					Pagina* f = get(b, pai);
					if(f->n < 9)
						(*flag) = 0;

				}else{
					(*flag) = 0;	
				}
				return split(b, codEscola, data_reference, ultimoRRN, RRN, pai, noRaiz);

			}
			return noRaiz;
		} 
	}
	
	percorreArvore(b, codEscola, data_reference, ultimoRRN, p->ponteiros[i], RRN, flag, noRaiz);

	if(*flag == 1){ // Tenho que fazer o split
		if(pai != -1){
			Pagina* f = get(b, pai);
			if(f->n < 9)
				(*flag) = 0;

		}else{
			(*flag) = 0;	
		}
		

		return split(b, codEscola, data_reference, ultimoRRN, RRN, pai, noRaiz);
	}
	
	return noRaiz;

}

void insereIndice(Buffer_Pool* b, int codEscola, int RRN){

	//get(b, RRN);



	// if(noRaiz == -1){ // Primeira inserção no arquivo (Nao existe nenhum no ainda)
	// 	int i;
	// 	//crio a pagina nova
	// 	Pagina* p = (Pagina*) malloc(sizeof(Pagina));
	// 	p->n = 1;
	// 	for(i = 0; i < 9; i++){
	// 		p->ponteiros[i] = -1;
	// 		p->elementos[i].chave = -1;
	// 		p->elementos[i].RRN = -1;

	// 	}
	// 	p->ponteiros[i] = -1;
	// 	p->elementos[0].chave = codEscola;
	// 	p->elementos[0].RRN = RRN;

	// 	//escrevo ela no arquivo
	// 	escrevePagina(fp, p, 0);

	// 	//atualizo o no raiz
	// 	fseek(fp, 0, SEEK_SET);
	// 	fgetc(fp);
	// 	noRaiz = 0;
	// 	fwrite(&noRaiz, sizeof(int), 1, fp);
	// 	ultimoRRN = 0;
	// 	altura=0;
	// 	fwrite(&altura, sizeof(int), 1, fp);
	// 	fwrite(&ultimoRRN, sizeof(int), 1, fp);

	// 	free(p);
	// 	return;
	// }else{
		
	//printf("b->noRaiz =  %d\n", b->noRaiz);

		int flag = 0;
		b->noRaiz = percorreArvore(b, &codEscola, &RRN, &b->UltimoRRN, b->noRaiz, -1, &flag, b->noRaiz);
		// fseek(fp, 1, SEEK_SET);
		// fwrite(&noRaiz, sizeof(int), 1, fp);
		// fseek(fp, 4, SEEK_CUR);
		// fwrite(&ultimoRRN, sizeof(int), 1, fp);
	//}

	return;

}



void escrevePagina(FILE* fp, Pagina* p, int RRN){
	//printf("escrevePagina\n");
	//("RRR = %d\n", RRN);
	int i;
	fseek(fp, (T_CABECALHO_INDICE + (RRN * TAMANHOPAGINA)), SEEK_SET);

	//imprimePagina(p);

	fwrite(&(p->n), sizeof(int), 1, fp);
	
		
	fwrite(&(p->ponteiros[0]), sizeof(int), 1, fp);
	fwrite(&(p->elementos[0].chave), sizeof(int), 1, fp);
	fwrite(&(p->elementos[0].RRN), sizeof(int), 1, fp);

	fwrite(&(p->ponteiros[1]), sizeof(int), 1, fp);
	fwrite(&(p->elementos[1].chave), sizeof(int), 1, fp);
	fwrite(&(p->elementos[1].RRN), sizeof(int), 1, fp);
	
	fwrite(&(p->ponteiros[2]), sizeof(int), 1, fp);
	fwrite(&(p->elementos[2].chave), sizeof(int), 1, fp);
	fwrite(&(p->elementos[2].RRN), sizeof(int), 1, fp);
	
	fwrite(&(p->ponteiros[3]), sizeof(int), 1, fp);
	fwrite(&(p->elementos[3].chave), sizeof(int), 1, fp);
	fwrite(&(p->elementos[3].RRN), sizeof(int), 1, fp);
	
	fwrite(&(p->ponteiros[4]), sizeof(int), 1, fp);
	fwrite(&(p->elementos[4].chave), sizeof(int), 1, fp);
	fwrite(&(p->elementos[4].RRN), sizeof(int), 1, fp);
	
	fwrite(&(p->ponteiros[5]), sizeof(int), 1, fp);
	fwrite(&(p->elementos[5].chave), sizeof(int), 1, fp);
	fwrite(&(p->elementos[5].RRN), sizeof(int), 1, fp);
	
	fwrite(&(p->ponteiros[6]), sizeof(int), 1, fp);
	fwrite(&(p->elementos[6].chave), sizeof(int), 1, fp);
	fwrite(&(p->elementos[6].RRN), sizeof(int), 1, fp);
	
	fwrite(&(p->ponteiros[7]), sizeof(int), 1, fp);
	fwrite(&(p->elementos[7].chave), sizeof(int), 1, fp);
	fwrite(&(p->elementos[7].RRN), sizeof(int), 1, fp);
	
	fwrite(&(p->ponteiros[8]), sizeof(int), 1, fp);
	fwrite(&(p->elementos[8].chave), sizeof(int), 1, fp);
	fwrite(&(p->elementos[8].RRN), sizeof(int), 1, fp);

	fwrite(&(p->ponteiros[9]), sizeof(int), 1, fp);
	//printf("\n");

	return;


}


void imprimePagina(Pagina* p){

	int i;
	printf("|%d|", p->n);
	for(i = 0; i < 9; i++){
		printf("|%d|",p->ponteiros[i]);
		printf("|%d||%d|; ",p->elementos[i].chave,p->elementos[i].RRN);

	}
	printf("|%d| ",p->ponteiros[i]);
	printf("\n");

}

bool proxPagina(FILE *fp){ // avança um registro no arquivo "fp"
				
	// obs: o ponteiro do arquivo deve estar no primeiro byte de um registro !!!

	fseek(fp, TAMANHOPAGINA, SEEK_CUR); // somamos o tamanho do registro no registro atual

	if(ftell(fp) < tamArquivo(fp)){

		return true;
	}else{

		return false;
	}
}



void ImprimeIndice(FILE* fp){


	long int tamanho_arquivo = tamArquivo(fp);
	char status; 
	int noRaiz, altura, ultimoRRN, i;

	fread(&status, sizeof(char), 1, fp);
	fread(&noRaiz, sizeof(int), 1, fp);
	fread(&altura, sizeof(int), 1, fp);
	fread(&ultimoRRN, sizeof(int), 1, fp);
	printf("status = %c\n", status);
	printf("noRaiz = %d\n", noRaiz);
	printf("altura = %d\n", altura);
	printf("ultimoRRN = %d\n", ultimoRRN);

	Pagina* p = NULL;

	printf("tamanho_arquivo = %ld\n",tamanho_arquivo );
	for(i = 0; ftell(fp) < tamanho_arquivo; i++){
		
		printf("RRN = %d\n",i );
		if(i == ultimoRRN+1){
			break;
		}
		p = pag(fp, i);

		imprimePagina(p);
		proxPagina(fp);
		printf("\n\n");
		//p = NULL;
	}


}

Pagina* pag(FILE* fp, int RRN){
	int i;
	long int posicao_atual = ftell(fp);

	if(RRN == -1){
		return NULL;
	}

	fseek(fp, (TAMANHOPAGINA*RRN) + T_CABECALHO_INDICE, SEEK_SET);

	Pagina* p = (Pagina*) malloc(sizeof(Pagina));

	fread(&(p->n), sizeof(int), 1, fp);
	fread(&p->ponteiros[0], sizeof(int), 1, fp);
	fread(&(p->elementos[0].chave), sizeof(int), 1, fp);
	fread(&(p->elementos[0].RRN), sizeof(int), 1, fp);

	fread(&p->ponteiros[1], sizeof(int), 1, fp);
	fread(&(p->elementos[1].chave), sizeof(int), 1, fp);
	fread(&(p->elementos[1].RRN), sizeof(int), 1, fp);
	
	fread(&p->ponteiros[2], sizeof(int), 1, fp);
	fread(&(p->elementos[2].chave), sizeof(int), 1, fp);
	fread(&(p->elementos[2].RRN), sizeof(int), 1, fp);
	
	fread(&p->ponteiros[3], sizeof(int), 1, fp);
	fread(&(p->elementos[3].chave), sizeof(int), 1, fp);
	fread(&(p->elementos[3].RRN), sizeof(int), 1, fp);
	
	fread(&p->ponteiros[4], sizeof(int), 1, fp);
	fread(&(p->elementos[4].chave), sizeof(int), 1, fp);
	fread(&(p->elementos[4].RRN), sizeof(int), 1, fp);
	
	fread(&p->ponteiros[5], sizeof(int), 1, fp);
	fread(&(p->elementos[5].chave), sizeof(int), 1, fp);
	fread(&(p->elementos[5].RRN), sizeof(int), 1, fp);
	
	fread(&p->ponteiros[6], sizeof(int), 1, fp);
	fread(&(p->elementos[6].chave), sizeof(int), 1, fp);
	fread(&(p->elementos[6].RRN), sizeof(int), 1, fp);
	
	fread(&p->ponteiros[7], sizeof(int), 1, fp);
	fread(&(p->elementos[7].chave), sizeof(int), 1, fp);
	fread(&(p->elementos[7].RRN), sizeof(int), 1, fp);
	
	fread(&p->ponteiros[8], sizeof(int), 1, fp);
	fread(&(p->elementos[8].chave), sizeof(int), 1, fp);
	fread(&(p->elementos[8].RRN), sizeof(int), 1, fp);
	
	fread(&p->ponteiros[9], sizeof(int), 1, fp);
	fseek(fp, posicao_atual, SEEK_SET);	 // voltamos para posicao inicial da funcao

	return p;

}

Pagina* NoVazio(){
	int i;
	//Crio um novo nó
	Pagina* NewPag = (Pagina*) malloc(sizeof(Pagina));
	for(i = 0; i < 9; i++){
		NewPag->ponteiros[i] = -1;
		NewPag->elementos[i].chave = -1;
		NewPag->elementos[i].RRN = -1;
	}
	NewPag->ponteiros[i] = -1;

	return NewPag;

}

Pagina* get(Buffer_Pool* b, int RRN){

	int i;

	for(i = 0; i < b->n; i++){

		if(b->pages[i].RRN == RRN){
			Rearranja(b, b->pages[i]);
			imprimePagina(&(b->pages[b->n-1]));
			return &(b->pages[b->n-1]);
		}

	}

	printf("RRN = %d\n",RRN);
	
	FILE* fp = fopen(arquivoIndice, "rb+");
	if(fp == NULL){
		printf("Erro na abertura do arquivo\n");
		return NULL;
	}


	Pagina* p = pag(fp, RRN);
	if(p == NULL){
		printf("ola\n");
		return  NULL;
	}

	p->RRN = RRN;

	put(b, p);

	return p;


}

void LRU(Buffer_Pool* b, Pagina* p){
	if(b->n == 5){
		// aplico a politica de substituição
		if(b->pages[0].Modified == true){
			Flush(&b->pages[0]);
		}
		b->pages[0] = *p;	
		Rearranja(b, *p);

	}
}

void Flush(Pagina* p){
	FILE* fp = fopen(arquivoIndice, "r+");

	if(fp == NULL){
		printf("deu ruim\n");
		return ;
	}
	printf("RRN = %d\n", p->RRN);
	imprimePagina(p);
	
	escrevePagina(fp, p, p->RRN);

	fclose(fp);
}

void Rearranja(Buffer_Pool* b, Pagina p){
	
	int i;

	for(i = 0; i < b->n; i++){
		if(p.RRN == b->pages[i].RRN){
			break;
		}

	}

	int pos = i;

	for(i = pos; i < b->n; i++){
		b->pages[i] = b->pages[i+1];
	}
	
	b->pages[(b->n)-1] = p;




}


void put(Buffer_Pool* b, Pagina* p){
	
	int i;
	if(b->n == 5){ // Buffer cheio
		LRU(b, p);		
		return ;

	}else{
		for(i = 0; i < b->n; i++){
			if(b->pages[i].RRN == p->RRN){
				b->pages[i] = *p;
				b->pages[i].Modified = true;
				return ;
			}
		}
		p->Modified = true;
		b->pages[i] = *p;
		b->n++;
		return ;
	}


}

