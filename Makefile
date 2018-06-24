# Compilador gcc
CP = gcc
# Parâmetros de compilação
P_CO = -c -lm -g3 -std=gnu99

main: manipula_arquivo.o  manipula_indice.o main.o 
	$(CP) manipula_arquivo.o manipula_indice.o main.o -o main 

all: manipula_arquivo.o  main.o 
	$(CP) manipula_arquivo.o main.o -o main 

manipula_arquivo.o: manipula_arquivo.c manipula_indice.h
	$(CP) $(P_CO) manipula_arquivo.c
manipula_indice.o: manipula_indice.c manipula_arquivo.h
	$(CP) $(P_CO) manipula_indice.c

main.o: main.c
	$(CP) $(P_CO) main.c

run:
	./main
clean: 
	rm -rf *o main *bin

clean_all:	
	rm -rf *o
