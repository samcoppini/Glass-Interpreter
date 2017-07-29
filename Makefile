CC=g++
CFLAGS=$(FLAGS) -std=c++17 -Wall -Wextra -Werror -pedantic -g
SOURCES=$(wildcard src/*.cpp)
OBJS=$(SOURCES:src/%.cpp=objs/%.o)

objs/%.o: src/%.cpp
	$(CC) $< -c -o $@$(CFLAGS)

all: $(OBJS)
	$(CC) $(OBJS) -o glass

clean:
	rm objs/*.o
