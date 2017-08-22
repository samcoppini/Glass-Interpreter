CC=g++
CFLAGS=$(FLAGS) -std=c++17 -Wall -Wextra -Werror -pedantic -Iinclude -g -O3
SOURCES=$(wildcard src/*.cpp)
OBJS=$(SOURCES:src/%.cpp=objs/%.o)

objs/%.o: src/%.cpp
	@mkdir -p objs
	$(CC) $< -c -o $@$(CFLAGS)

all: $(OBJS)
	$(CC) $(OBJS) -o glass

clean:
	rm $(OBJS)
