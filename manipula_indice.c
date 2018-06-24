
#include "manipula_arquivo.h"
#include <string.h>

#include "manipula_indice.h"

int busca(Buffer_Pool* b, int codEscola, int RRN){
	int i;
	if(RRN == -1){ // Não encontrei a chave
		return -1;
	}

	Pagina p = get(b, RRN); // Recupero a pagina do RRN que eu estou agora
	
	for (i = 0; i < p.n; i++){
		if(codEscola == p.elementos[i].chave){ // Se a chave ja existe
			return p.elementos[i].RRN;
		}else if(codEscola < p.elementos[i].chave){
			return busca(b, codEscola, p.ponteiros[i]);
		}
	}	
	// se minha chave eh maior que todas do no
	// vou para o ultimo ponteiro valido do meu no
	return busca(b, codEscola, p.ponteiros[i]);
}

int buscaIndice(int codEscola){
	
	// Crio o buffer pool e leio as informações do arquivo
	Buffer_Pool* b =(Buffer_Pool*) malloc(sizeof(Buffer_Pool));
	b->pages = (Pagina*) malloc(sizeof(Pagina)*5);
	b->n = 0;
	
	FILE* indice_file = fopen(arquivoIndice, "rb+");
	if(indice_file == NULL){
		printf("Falha no processamento do arquivo.\n");
		return -2;
	}

	char status_indice;
	int altura;
	fread(&status_indice, sizeof(char), 1, indice_file);
	if(status_indice == '0'){
		printf("Falha no processamento do arquivo.\n");
		return -2;
	}
	fread(&b->noRaiz, sizeof(int), 1, indice_file);
	fread(&altura, sizeof(int), 1, indice_file);
	fread(&b->UltimoRRN, sizeof(int), 1, indice_file);
	fclose(indice_file);

	// busco a chave na arvore
	int RRN = busca(b, codEscola, b->noRaiz);
	// Escrevos page_hit e Page_fault no arquivo txt
	FILE* buffer = fopen(arquivoBuffer, "a");
	fprintf(buffer,"Page Fault: %d; Page Hit: %d\n", b->page_fault, b->page_hit );

	fclose(buffer);
	return RRN;



}


void Ordena(Pagina* p, int codEscola, int data_reference){
	
	int i;
	// Acho a posicao que eu tenho que inserir a chave
	for(i =0; i < p->n; i++){
		if(p->elementos[i].chave > codEscola){
			break;
		}
	}
	int pos = i;
	// Passo os elementos daquela posicao ate o final do no, para a frente
	for(i = p->n; i> pos; i--){
		p->ponteiros[i+1] = p->ponteiros[i];
		p->elementos[i].chave = p->elementos[i-1].chave;
		p->elementos[i].RRN = p->elementos[i-1].RRN;
	}
	// Insiro o elemento na posicao correta
	p->elementos[pos].chave = codEscola;
	p->elementos[pos].RRN = data_reference;

}


void split(Buffer_Pool* b, int* codEscola, int* data_reference, int* ultimoRRN, int RRN, int pai, int* noRaiz, int* propagacao){

	int i;
	Pagina Father; // Pagina pai

	Pagina p = get(b, RRN); // Pego a pagina que irei splitar
	Pagina NewPag = NoVazio(); // Crio um no Vazio

	// Divido os membros de p, entre p e NewPag
	NewPag.n = (p.n)/2;
	for(i = NewPag.n; i < p.n; i++){
		NewPag.ponteiros[i - NewPag.n] = p.ponteiros[i];
		NewPag.elementos[i - NewPag.n].chave = p.elementos[i].chave;
		NewPag.elementos[i - NewPag.n].RRN = p.elementos[i].RRN;
		
	}
	// Coloco o ultimo ponteiro do No Cheio(o que estou splitando)
	// no ultimo ponteiro de NewPag
	NewPag.ponteiros[NewPag.n+1] = p.ponteiros[p.n];
	// Se houver propagacao do split(i.e houver pelo menos um split abaixo do nivel atual)
	if((*propagacao) == 1){
		// referencio no novo no que estou criando, o RRN no No que eu criei no nivel debaixo
		NewPag.ponteiros[NewPag.n+2] = (*ultimoRRN);
		(*propagacao) = 0;
	}
	
	// p tambem passa a ter metade dos elementos
	p.n = (p.n)/2;
	
	//Se a chave que quero inserir for maior que todas as chaves restantes em p
	// insiro a minha chave em NewPag 
	if(p.elementos[(p.n)].chave < *codEscola){
		NewPag.n++;
		Ordena(&NewPag, *codEscola, *data_reference);
		
	
		

	}else{ // Se houver pelo menos uma chave maior que a minha no No P
		// Aumento o numero de chaves em p
		// e insiro a minha chave la
		p.n++;
		Ordena(&p, *codEscola, *data_reference);
		// retiro o ultimo elemento de p, e insiro em NewPag
		Ordena(&NewPag,p.elementos[p.n-1].chave, p.elementos[p.n-1].RRN);
		p.n--;
		
	}

	// Se o pai existir
	if(pai != -1){
		Father = get(b, pai); // Pego a pagina dele

		// Se o pai nao estiver cheio
		// Insiro o primeiro elemento da nova chave do meu NewPag no Pai
		if(Father.n < 9){
			Ordena(&Father, NewPag.elementos[0].chave, NewPag.elementos[0].RRN);
			//imprimePagina(&Father);
			Father.ponteiros[Father.n+1] = (*ultimoRRN)+1; // Referencio o ultimo ponteiro do No pai, como o novo No que eu criei
			Father.n++; // Aumento o Numero de elemento valido em father
			*(ultimoRRN) += 1; // E atualizo o valor do ultimo RRN criado
			NewPag.RRN = (*ultimoRRN); // Que eh o RRN da minha nova pagina
		}else{ // Se o pai estiver cheio, tenho que splitar ele em cima
			*(ultimoRRN) += 1; // atualizo o valor do ultimo RRN criado
			NewPag.RRN = (*ultimoRRN); //Que eh o RRN da minha nova pagina
			
			// Aviso o proximo split que a chave que ele tem que inserir no No a ser splitado(num nivel acima)
			// Eh a primeira chave da Pag que estou criando nesse nivel
			(*codEscola) = NewPag.elementos[0].chave; // 
			(*data_reference) = NewPag.elementos[0].RRN;


		}
		
		
	}else{ // Se o pai nao existir

		// Crio um no Vazio
		Father = NoVazio();
		Father.n = 0;
		// Promovo o primeiro elemento de NewPag para o pai
		Ordena(&Father, NewPag.elementos[0].chave, NewPag.elementos[0].RRN);
		Father.n = 1;
		// Referencio os ponteiros
		Father.ponteiros[Father.n] = (*ultimoRRN) +1;
		Father.ponteiros[(Father.n)-1] = RRN;
		// e atualizo os valores dos RRN das paginas que estao sendo criadas
		Father.RRN = (*ultimoRRN) +2;
		*(ultimoRRN) += 2;
		
		// se o pai nao existe, eh porque estou splitando a raiz, entao modifico o RRN do meu no raiz
		*noRaiz = (*ultimoRRN);
		NewPag.RRN = (*ultimoRRN) -1;
	}


	// Como retirei uma chave do meu no New Pag, trago todos os elementos uma posicao para tras
	for(i = 0; i < (NewPag.n); i++){
		NewPag.ponteiros[i] = NewPag.ponteiros[i+1];
		NewPag.elementos[i].chave = NewPag.elementos[i+1].chave;
		NewPag.elementos[i].RRN = NewPag.elementos[i+1].RRN; 
	}
	NewPag.ponteiros[i] = NewPag.ponteiros[i+1];
	NewPag.elementos[i].chave = NewPag.elementos[i+1].chave;
	NewPag.elementos[i].RRN = NewPag.elementos[i+1].RRN;

	// Dou Put nas 3 paginas modificadas
	put(b, NewPag);
	put(b, p);
	put(b, Father);


	

}

void percorreArvore(Buffer_Pool* b, int* codEscola, int* data_reference, int* ultimoRRN, int RRN, int pai, int* flag, int* noRaiz, int* propagacao){
	
	int i;
	// tenho que inserir no pai
	if(RRN == -1){
		// Pego a pagina do pai
		Pagina p = get(b, pai);
		if(p.n == 9){ // Se o pai estiver cheio, tenho que fazer split nele
			*flag = 1; // aviso a chamada anterior que tenho que splitar o no
		}else{
			// Se nao estiver cheio
			// Insiro o codEscola e sua referencia no No e insiro o no No buffer
			Ordena(&p, *codEscola, *data_reference);
			p.n++;
			put(b, p);
			
		}
		return ;
	}

	// Continuo aqui se o meu RRN existe
	Pagina p = get(b, RRN); // Recupero a pagina do RRN que eu estou agora
	
	for (i = 0; i < p.n; i++){
		if(*codEscola == p.elementos[i].chave){ // Se a chave ja existe
			return ;
		}else if(*codEscola < p.elementos[i].chave){ // Se eu achar uma chave no No que eh maior que a chave que eu quero inserir
			// vou para o ponteiro da esquerda da chave
			percorreArvore(b, codEscola, data_reference, ultimoRRN, p.ponteiros[i], RRN, flag, noRaiz, propagacao);
			// Ao voltar da recursao, se eu tiver que fazer um split
			if(*flag == 1){ 
				if(pai != -1){ // Se o pai existir
					// Pego o pai
					Pagina f = get(b, pai);
					// Se o pai estiver cheio tambem, mantenho a flag em 1 para splitar no No de cima também
					// Caso contrário, já seto a flag para 0
					if(f.n < 9)
						(*flag) = 0;

				}else{
					// Se ele nao tiver pai, entao a flag vai para 0 tambem
					(*flag) = 0;	
				}
				// Splito o no
				split(b, codEscola, data_reference, ultimoRRN, RRN, pai, noRaiz, propagacao);
				// Se a flag for 1 aqui, entao quer dizer que eu tenho q fazer um split no pai tambem
				// Entao aviso a chamada anterior, que o split foi feito num nivel abaixo da arvore

				if(*flag == 1)
					*propagacao=1;

			}
			return ;
		} 
	}

	// Se eu chegar aqui quer dizer que eu tenho que acesso o ultimo ponteiro do No, 
	// Pois a minha chave eh maior que todas as que estao no no
	percorreArvore(b, codEscola, data_reference, ultimoRRN, p.ponteiros[i], RRN, flag, noRaiz, propagacao);
	
	// Ao voltar da recursao, se eu tiver que fazer um split
	if(*flag == 1){ 
		if(pai != -1){ // Se o pai existir
			// Pego o pai
			Pagina f = get(b, pai);
			// Se o pai estiver cheio tambem, mantenho a flag em 1 para splitar no No de cima também
			// Caso contrário, já seto a flag para 0
			if(f.n < 9)
				(*flag) = 0;

		}else{
			// Se ele nao tiver pai, entao a flag vai para 0 tambem
			(*flag) = 0;	
		}
		// Splito o no
		split(b, codEscola, data_reference, ultimoRRN, RRN, pai, noRaiz, propagacao);
		
		// Se a flag for 1 aqui, entao quer dizer que eu tenho q fazer um split no pai tambem
		// Entao aviso a chamada anterior, que o split foi feito num nivel abaixo da arvore
		if(*flag == 1)
			*propagacao=1;

	}
	
	return;
}

// Insere o codEscola e o seu RRN na arvore-b
void insereIndice(Buffer_Pool* b, int codEscola, int RRN){
	
	
	int flag = 0; // flag usada para indicar se haverá split ou nao
	int propagacao =-1; // flag usada para indicar se houver split no nivel abaixo
	percorreArvore(b, &codEscola, &RRN, &b->UltimoRRN, b->noRaiz, -1, &flag, &b->noRaiz, &propagacao);
	return;
}


// Escreve a pagina p no arquivo de indices, no RRN solicitado
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
	return;


}

// Imprime a pagina p
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

bool proxPagina(FILE *fp){ // avança uma pagina no arquivo "fp"
				
	// obs: o ponteiro do arquivo deve estar no primeiro byte de uma pagina !!!

	fseek(fp, TAMANHOPAGINA, SEEK_CUR); // somamos o tamanho da pagina na pagina atual

	if(ftell(fp) < tamArquivo(fp)){

		return true;
	}else{

		return false;
	}
}


// Imprime o Arquivo de Indices
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

	for(i = 0; ftell(fp) < tamanho_arquivo; i++){
		
		printf("RRN = %d\n",i );
		if(i == ultimoRRN+1){
			break;
		}
		p = pag(fp, i);

		imprimePagina(p);
		proxPagina(fp);
		printf("\n\n");
	}


}

// Acessa o RRN no arquivo de indices, e retorna a pagina daquele RRN
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

// Cria um no Vazio(com todos os elementos -1)
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
// Retorna a pagina com o RRN solicitado
Pagina get(Buffer_Pool* b, int RRN){

	int i;

	for(i = 0; i < b->n; i++){
		// Se minha pagina esta no buffer pool
		if(b->pages[i].RRN == RRN){
			// Ela foi utilizada, entao Rearranjo as posicoes na minha lista do buffer
			Rearranja(b, b->pages[i]);
			Pagina p = b->pages[b->n-1];
			b->page_hit++;
			return p;
		}

	}

	// Se ela nao esta no buffer
	// Acesso o Arquivo, recupero a pagina e insiro no buffer chamando put


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
	fclose(fp);
	b->page_fault++;
	
	p->RRN = RRN;

	put(b, *p);

	//Retorno p
	return *p;


}

void put(Buffer_Pool* b, Pagina p){
	
	int i;

	// Verifico se a pagina ja esta no buffer 
	for(i = 0; i < b->n; i++){
		if(b->pages[i].RRN == p.RRN){ // se tiver, atualizo ela, e mando ela pro final da lista
			b->pages[i] = p;
			b->pages[i].Modified = true;
			Rearranja(b, b->pages[i]);
			return ;
		}
	}
	// Se nao estiver no buff

	if(b->n == 5){ // Buffer cheio
		LRU(b, p);		
		return ;

	}else{ // Ha espaco no buffer
		b->pages[i] = p;
		b->n++;
		return ;
	}


}


void Rearranja(Buffer_Pool* b, Pagina p){
	
	int i;
	// Acha a posicao da pagina
	for(i = 0; i < b->n; i++){
		if(p.RRN == b->pages[i].RRN){
			break;
		}

	}
	int pos = i;

	// Reordeno as posicoes
	for(i = pos; i < b->n; i++){
		b->pages[i] = b->pages[i+1];
	}
	
	b->pages[(b->n)-1] = p;

}


void LRU(Buffer_Pool* b, Pagina p){

	if(b->pages[0].RRN == b->noRaiz){ // Se pagina que eu for tirar for a raiz
			
		// tiro a proxima (pois o no raiz tem q ficar no buffer)
		if(b->pages[1].Modified == true){ // se o no a ser retirado tiver sido modificado
			Flush(&b->pages[1]); // dou flush nele
		}
		b->pages[1] = p;	
		Rearranja(b, p);
	}else{// Se nao,
		// aplico a politica de substituição
		if(b->pages[0].Modified == true){ // se o no a ser retirado tiver sido modificado
			Flush(&b->pages[0]); // dou flush nele
		}
		b->pages[0] = p;	
		Rearranja(b, p);
	}
}

void Flush(Pagina* p){
	// Escrevo p no arquivo
	FILE* fp = fopen(arquivoIndice, "r+");

	if(fp == NULL){
		return ;
	}
	
	escrevePagina(fp, p, p->RRN);

	fclose(fp);
}

// Imprimo as paginas do buffer
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