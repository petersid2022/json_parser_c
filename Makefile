default:
	gcc -ggdb -std=c99 -Wall -Wextra -pedantic -o parser main.c
	./parser test.json

debug:
	gcc -ggdb -std=c99 -Wall -Wextra -pedantic -o parser main.c
	gf2 --args ./parser test.json
