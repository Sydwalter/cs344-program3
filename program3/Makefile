CFLAGS = -Wall -fpic -coverage -lm
GCOV = -ftest-coverage -fprofile-arcs

smallsh: smallsh.c
	gcc -o smallsh -g $(CFLAGS)

all: smallsh

clean:
	rm -f *.o smallsh *.gcov *.gcda *.gcno *.so

#######################################

# rngs.o: rngs.h rngs.c
# 	gcc -c rngs.c -g  $(CFLAGS)

# dominion.o: dominion.h dominion.c rngs.o
# 	gcc -c dominion.c -g  $(CFLAGS) $(GCOV)

# playdom: dominion.o playdom.c
# 	gcc -o playdom playdom.c -g dominion.o rngs.o $(CFLAGS)

# testDrawCard: testDrawCard.c dominion.o rngs.o
# 	gcc -o testDrawCard -g  testDrawCard.c dominion.o rngs.o $(CFLAGS)

# badTestDrawCard: badTestDrawCard.c dominion.o rngs.o
# 	gcc -o badTestDrawCard -g  badTestDrawCard.c dominion.o rngs.o $(CFLAGS)

# testBuyCard: testDrawCard.c dominion.o rngs.o
# 	gcc -o testDrawCard -g  testDrawCard.c dominion.o rngs.o $(CFLAGS)

# testAll: dominion.o testSuite.c
# 	gcc -o testSuite testSuite.c -g  dominion.o rngs.o $(CFLAGS)

# interface.o: interface.h interface.c
# 	gcc -c interface.c -g  $(CFLAGS)

# runtests: testDrawCard 
# 	./testDrawCard &> unittestresult.out
# 	gcov dominion.c >> unittestresult.out
# 	cat dominion.c.gcov >> unittestresult.out

# player: player.c interface.o
# 	gcc -o player player.c -g  dominion.o rngs.o interface.o $(CFLAGS)
