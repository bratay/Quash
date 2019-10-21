Quash: quash.o 
	gcc quash.o -o Quash -lreadline

quash.o: quash.c
	gcc -c -g quash.c -lreadline
	
clean:
	rm -f *.o Quash
