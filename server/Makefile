SRCDIR=./src
COMMONSRCDIR=../lib/src
INCDIR=../lib/include
CC=gcc-13
INCLUDE = -I/usr/local/include -I/usr/local/include/SDL2 -I$(INCDIR)
CFLAGS = -g $(INCLUDE) -c
LDFLAGS = -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net -lm

OUT = gameServer

all: $(OUT)

$(OUT): bullet.o cannon.o enemy_1.o enemy_2.o enemy_3.o serverMain.o menu.o ship.o sound.o stars.o text.o tick.o
	$(CC) bullet.o cannon.o enemy_1.o enemy_2.o enemy_3.o serverMain.o menu.o ship.o sound.o stars.o text.o tick.o -o $(OUT) $(LDFLAGS)

bullet.o: $(COMMONSRCDIR)/bullet.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/bullet.c -o bullet.o

cannon.o: $(COMMONSRCDIR)/cannon.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/cannon.c -o cannon.o

enemy_1.o: $(COMMONSRCDIR)/enemy_1.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/enemy_1.c -o enemy_1.o

enemy_2.o: $(COMMONSRCDIR)/enemy_2.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/enemy_2.c -o enemy_2.o

enemy_3.o: $(COMMONSRCDIR)/enemy_3.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/enemy_3.c -o enemy_3.o

serverMain.o: $(SRCDIR)/serverMain.c
	$(CC) $(CFLAGS) $(SRCDIR)/serverMain.c -o serverMain.o

menu.o: $(COMMONSRCDIR)/menu.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/menu.c -o menu.o

ship.o: $(COMMONSRCDIR)/ship.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/ship.c -o ship.o

sound.o: $(COMMONSRCDIR)/sound.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/sound.c -o sound.o

stars.o: $(COMMONSRCDIR)/stars.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/stars.c -o stars.o

text.o: $(COMMONSRCDIR)/text.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/text.c -o text.o

tick.o: $(COMMONSRCDIR)/tick.c
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/tick.c -o tick.o

clean:
	rm -f *.o $(OUT)