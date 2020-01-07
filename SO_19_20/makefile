all: gestor cliente verificador

gestor: Gestor/Gestor.c
	gcc Gestor/Gestor.c -pthread -o Bin/Gestor

cliente: Cliente/Cliente.c
	gcc Cliente/Cliente.c -pthread -o Bin/Cliente

verificador: Verificador/Verificador.c
	gcc Verificador/Verificador.c -o Bin/Verificador

clean:
	rm Bin/Gestor
	rm Bin/Cliente
	rm Bin/Verificador
