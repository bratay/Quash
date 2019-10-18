Quash: quash.o 
	gcc quash.o -o Quash -lreadline

quash.o: quash.c quash.h
	gcc -c -g quash.c -lreadline

#backgroundProcess.o: backgroundProcess.c backgroundProcess.h
#	gcc -c -g backgroundProcess.c -lreadline

clean:
	rm -f *.o Quash
