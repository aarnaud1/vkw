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

CXX       := g++ -W -Wall -Wextra -std=c++17
CXX_FLAGS := -O2 -g --pedantic -ffast-math
DEFINEs   :=
IFLAGS    := -I./include
LFLAGS    := -L./output/lib -Wl,-rpath,./output/lib -lVkWrappers -lvulkan -lglfw -ltinyply

SHADERS_SPV := $(patsubst samples/shaders/%.comp,output/spv/%_comp.spv,$(wildcard samples/shaders/*.comp)) \
			   $(patsubst samples/shaders/%.vert,output/spv/%_vert.spv,$(wildcard samples/shaders/*.vert)) \
			   $(patsubst samples/shaders/%.frag,output/spv/%_frag.spv,$(wildcard samples/shaders/*.frag))
OBJ_FILES   := $(patsubst src/%.cpp,output/obj/%.o,$(wildcard src/*.cpp))

MODULE := output/lib/libVkWrappers.so

MAIN_UTILS := $(wildcard samples/utils/*.cpp)
EXEC := output/bin/ArrayAdd \
        output/bin/ArraySaxpy \
		output/bin/BufferCopy \
		output/bin/GaussianBlur \
		output/bin/Triangle \
		# output/bin/MeshDisplay

all: deps $(MODULE) $(SHADERS_SPV) $(EXEC)
lib: deps $(MODULE)
	$(shell) rm -rfd output/obj/ output/spv/ output/bin

output/obj/%.o: src/%.cpp
	$(CXX) $(CXX_FLAGS) -c -fPIC $(IFLAGS) -o $@ $<

$(MODULE): $(OBJ_FILES)
	$(CXX) $(CXX_FLAGS) -shared -o $@ $^

output/bin/%: samples/%.cpp $(MAIN_UTILS)
	$(CXX) $(CXX_FLAGS) -o $@ $(IFLAGS) -I./samples/utils -I./stb/ $^ $(LFLAGS)

output/spv/%_comp.spv: samples/shaders/%.comp
	glslc -std=450core -fshader-stage=compute -o $@ $^
output/spv/%_vert.spv: samples/shaders/%.vert
	glslc -std=450core -fshader-stage=vertex -o $@ $^
output/spv/%_frag.spv: samples/shaders/%.frag
	glslc -std=450core -fshader-stage=fragment -o $@ $^

shaders: $(SHADERS_SPV)

$(EXEC): $(MODULE)

deps:
	$(shell mkdir -p output/spv)
	$(shell mkdir -p output/obj)
	$(shell mkdir -p output/lib)
	$(shell mkdir -p output/bin)

clean:
	$(shell rm -rfd output)