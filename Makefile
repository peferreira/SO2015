EP1SH=ep1sh
SIMULADOR=ep1
CC=gcc
CFLAGS= -ansi -pedantic -Wall

all:	$(EP1SH) $(SIMULADOR)

$(EP1SH):	$(EP1SH).c
		$(CC) -o $(EP1SH) $(EP1SH).c $(CFLAGS) -lreadline
		
$(SIMULADOR):	$(SIMULADOR).c
		$(CC) -o $(SIMULADOR) $(SIMULADOR).c $(CFLAGS) -lpthread		
		
clean:
		rm -f $(SIMULADOR) $(EP1SH) *~ 
