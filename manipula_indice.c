
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


void split(Buffer_Pool* b, int* codEscola, int* data_reference, int* ultimoRRN, int RRN, int pai, int* noRaiz, int* propagacao){

	int i;
	Pagina Father;

	Pagina p = get(b, RRN);

	Pagina NewPag = NoVazio();

	//Crio um novo no na mesma altura do atual e divido os membros do no entre as duas paginas
	NewPag.n = (p.n)/2;
	for(i = NewPag.n; i < p.n; i++){
		NewPag.ponteiros[i - NewPag.n] = p.ponteiros[i];
		NewPag.elementos[i - NewPag.n].chave = p.elementos[i].chave;
		NewPag.elementos[i - NewPag.n].RRN = p.elementos[i].RRN;
		
	}
	NewPag.ponteiros[NewPag.n+1] = p.ponteiros[p.n];
	if((*propagacao) == 1){
		NewPag.ponteiros[NewPag.n+2] = (*ultimoRRN);
		(*propagacao) = 0;
	}
	

	p.n = (p.n)/2;

	if(p.elementos[(p.n)].chave < *codEscola){
		NewPag.n++;
		Ordena(&NewPag, *codEscola, *data_reference);
		
	
		

	}else{
		p.n++;
		Ordena(&p, *codEscola, *data_reference);
		
		Ordena(&NewPag,p.elementos[p.n-1].chave, p.elementos[p.n-1].RRN);
		p.n--;
		
	}


	if(pai != -1){
		Father = get(b, pai);

		// Insiro o primeiro elemento da nova chave do meu NewPag no Pai
		if(Father.n < 9){
			Ordena(&Father, NewPag.elementos[0].chave, NewPag.elementos[0].RRN);
			//imprimePagina(&Father);
			Father.ponteiros[Father.n+1] = (*ultimoRRN)+1;
			Father.n++;
			*(ultimoRRN) += 1;
			NewPag.RRN = (*ultimoRRN);
		}else{
			*(ultimoRRN) += 1;
			NewPag.RRN = (*ultimoRRN);
			(*codEscola) = NewPag.elementos[0].chave;
			(*data_reference) = NewPag.elementos[0].RRN;


		}
		
		
	}else{

		Father = NoVazio();
		//printf("chave a ser promovida = %d\n", NewPag.elementos[0].chave);
		Father.n = 0;
		Ordena(&Father, NewPag.elementos[0].chave, NewPag.elementos[0].RRN);
		imprimePagina(&NewPag);
		Father.n = 1;
		Father.ponteiros[Father.n] = (*ultimoRRN) +1;
		Father.ponteiros[(Father.n)-1] = RRN;
		Father.RRN = (*ultimoRRN) +2;
		*(ultimoRRN) += 2;
		*noRaiz = (*ultimoRRN);
		NewPag.RRN = (*ultimoRRN) -1;
	}

	for(i = 0; i < (NewPag.n); i++){
		NewPag.ponteiros[i] = NewPag.ponteiros[i+1];
		NewPag.elementos[i].chave = NewPag.elementos[i+1].chave;
		NewPag.elementos[i].RRN = NewPag.elementos[i+1].RRN; 
	}
	NewPag.ponteiros[i] = NewPag.ponteiros[i+1];
	NewPag.elementos[i].chave = NewPag.elementos[i+1].chave;
	NewPag.elementos[i].RRN = NewPag.elementos[i+1].RRN;


	put(b, NewPag);
	put(b, p);
	put(b, Father);


	

}

void percorreArvore(Buffer_Pool* b, int* codEscola, int* data_reference, int* ultimoRRN, int RRN, int pai, int* flag, int* noRaiz, int* propagacao){
	
	int i;
	if(RRN == -1){// tenho que inserir no pai
		
		Pagina p = get(b, pai);
		if(p.n == 9){ // no cheio, tenho que splitar(depois ver como fazer)
			*flag = 1;
		}else{
			
			Ordena(&p, *codEscola, *data_reference);
			p.n++;
			put(b, p);
			
		}
		return ;
	}

	
	Pagina p = get(b, RRN);
	
	for (i = 0; i < p.n; i++){
		if(*codEscola == p.elementos[i].chave){
			return ;
		}else if(*codEscola < p.elementos[i].chave){

			percorreArvore(b, codEscola, data_reference, ultimoRRN, p.ponteiros[i], RRN, flag, noRaiz, propagacao);
			if(*flag == 1){ // Tenho que fazer o split
				if(pai != -1){
					Pagina f = get(b, pai);
					if(f.n < 9)
						(*flag) = 0;

				}else{
					(*flag) = 0;	
				}
				
				split(b, codEscola, data_reference, ultimoRRN, RRN, pai, noRaiz, propagacao);
				if(*flag == 1)
					*propagacao=1;

			}
			return ;
		} 
	}
	percorreArvore(b, codEscola, data_reference, ultimoRRN, p.ponteiros[i], RRN, flag, noRaiz, propagacao);

	if(*flag == 1){ // Tenho que fazer o split
		if(pai != -1){
			Pagina f = get(b, pai);
			if(f.n < 9)
				(*flag) = 0;

		}else{
			(*flag) = 0;	
		}
		

		split(b, codEscola, data_reference, ultimoRRN, RRN, pai, noRaiz, propagacao);
		if(*flag == 1)
			*propagacao=1;
	}
	
	return;
}

void insereIndice(Buffer_Pool* b, int codEscola, int RRN){
	
	
	int flag = 0;
	int propagacao =-1;
	percorreArvore(b, &codEscola, &RRN, &b->UltimoRRN, b->noRaiz, -1, &flag, &b->noRaiz, &propagacao);
	return;
}



void escrevePagina(FILE* fp, Pagina* p, int RRN){
	
	int i;
	fseek(fp, (T_CABECALHO_INDICE + (RRN * TAMANHOPAGINA)), SEEK_SET);

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

Pagina NoVazio(){
	int i;
	//Crio um novo nó
	Pagina NewPag;
	for(i = 0; i < 9; i++){
		NewPag.ponteiros[i] = -1;
		NewPag.elementos[i].chave = -1;
		NewPag.elementos[i].RRN = -1;
	}
	NewPag.ponteiros[i] = -1;

	return NewPag;

}

Pagina get(Buffer_Pool* b, int RRN){

	int i;

	for(i = 0; i < b->n; i++){

		if(b->pages[i].RRN == RRN){
			//printf("ola\n");
			Rearranja(b, b->pages[i]);
			b->page_hit++;
			Pagina pag = b->pages[b->n-1];
			return pag;
		}

	}
	
	FILE* fp = fopen(arquivoIndice, "rb+");
	if(fp == NULL){
		printf("Erro na abertura do arquivo\n");
		Pagina p;
		return p;
	}


	Pagina* p = pag(fp, RRN);
	if(p == NULL){
		printf("pai nao existe\n");
		p->n = -1;
		return  *p;
	}

	b->page_fault++;
	
	p->RRN = RRN;

	put(b, *p);

	return *p;


}

void put(Buffer_Pool* b, Pagina p){
	
	int i;
	for(i = 0; i < b->n; i++){
		if(b->pages[i].RRN == p.RRN){
			b->pages[i] = p;
			b->pages[i].Modified = true;
			Rearranja(b, b->pages[i]);
			return ;
		}
	}

	if(b->n == 5){ // Buffer cheio
		LRU(b, p);	
		//printf("buffer cheio\n");	
		return ;

	}else{
		b->pages[i] = p;
		b->n++;
		return ;
	}


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


void LRU(Buffer_Pool* b, Pagina p){
	if(b->n == 5){

		if(b->pages[0].RRN == b->noRaiz){
			// aplico a politica de substituição
			if(b->pages[1].Modified == true){
				Flush(&b->pages[1]);
			}
			b->pages[1] = p;	
			Rearranja(b, p);
		}else{
			// aplico a politica de substituição
			if(b->pages[0].Modified == true){
				Flush(&b->pages[0]);
			}
			b->pages[0] = p;	
			Rearranja(b, p);
		}
		

	}
}

void Flush(Pagina* p){
	FILE* fp = fopen(arquivoIndice, "r+");

	if(fp == NULL){
		return ;
	}
	
	escrevePagina(fp, p, p->RRN);

	fclose(fp);
}
void ImprimeBuffer(Buffer_Pool* b){


	printf("imprimindo buffer\n");
	printf("b->n = %d\n", b->n);
	printf("b->noRaiz = %d\n", b->noRaiz);
			//exit(0);
	for(int i = 0; i < b->n; i++){
		printf("RRN: %d\n",b->pages[i].RRN );
		imprimePagina(&b->pages[i]);
	}

}