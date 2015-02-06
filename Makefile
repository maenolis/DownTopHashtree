OBJS = filesync.o MD5.o components.o
EXEC = filesync
GCCCX = g++ -g -Wall -c
GCCOX = g++ -g -Wall -o

$(EXEC): $(OBJS)
	$(GCCOX) $(EXEC) $(OBJS)

filesync.o: filesync.cpp
	$(GCCCX) filesync.cpp
	
components.o: components.cpp
	$(GCCCX) components.cpp

MD5.o: MD5.c
	$(GCCCX) MD5.c

clean:
	rm -rf $(OBJS) $(EXEC)
