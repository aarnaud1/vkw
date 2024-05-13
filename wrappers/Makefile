 # Copyright (C) 2024 Adrien ARNAUD
 #
 # This program is free software: you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation, either version 3 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program. If not, see <https://www.gnu.org/licenses/>.

CC := g++ -std=c++11
CXX_FLAGS := -O3 --pedantic -ffast-math
IFLAGS := -I./include 
LFLAGS := -lvulkan -lglfw -lm
OBJ_FILES := $(patsubst src/%.cpp,obj/%.o,$(wildcard src/*.cpp))

all: lib/libvk-wrappers.so

lib/libvk-wrappers.so: $(OBJ_FILES)
	$(CC) -shared $(CXX_FLAGS) -o $@ $(IFLAGS) $^ $(LFLAGS)

obj/%.o: src/%.cpp
	$(CC) -fPIC -c $(CXX_FLAGS) -o $@ $(IFLAGS) $^

clean:
	rm -f lib/*
	rm -f obj/*
