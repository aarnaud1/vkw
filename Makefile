 # Copyright (C) 2022 Adrien ARNAUD
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
IFLAGS := -I./wrappers/include \
	-I./utils/
LFLAGS := -L./wrappers/lib/ -lvulkan -lglfw \
	-Wl,-rpath,./wrappers/lib/ -lvk-wrappers \
	-L./utils/lib/ -lutils \
	-Wl,-rpath,./utils/lib/ -lutils

SHADERS_SPV := $(patsubst shaders/%.comp,spv/%.spv,$(wildcard shaders/*.comp))
EXEC := $(patsubst main/%.cpp,bin/%,$(wildcard main/*.cpp))

all: 
	$(MAKE) vk-wrappers 
	$(MAKE) lib-utils 
	$(MAKE) shaders 
	$(MAKE) $(EXEC)

bin/%: main/%.cpp
	$(CC) $(CXX_FLAGS) -o $@ $(IFLAGS) $^ $(LFLAGS)

vk-wrappers:
	$(MAKE) -C wrappers/ all

lib-utils:
	$(MAKE) -C utils/ all

shaders: $(SHADERS_SPV)

spv/%.spv: shaders/%.comp
	glslc -std=450core -fshader-stage=compute -o $@ $^

clean:
	$(MAKE) -C wrappers/ clean
	$(MAKE) -C utils/ clean
	rm -f bin/*
