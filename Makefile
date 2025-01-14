# Copyright (C) 2025 Adrien ARNAUD
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

CXX         := g++ -W -Wall -Wextra -Wmissing-field-initializers -Wconversion
CXX_FLAGS   := -std=c++17 -O2 -g --pedantic -ffast-math
GLSLC_FLAGS := -O --target-env=vulkan1.3
DEFINES     :=
IFLAGS      := -I./include
LFLAGS      := -L./build/lib -Wl,-rpath,./build/lib -lVkWrappers -lvulkan -lglfw -ltinyply

SHADERS_SPV := $(patsubst samples/shaders/%.comp,build/spv/%_comp.spv,$(wildcard samples/shaders/*.comp)) \
			   $(patsubst samples/shaders/%.vert,build/spv/%_vert.spv,$(wildcard samples/shaders/*.vert)) \
			   $(patsubst samples/shaders/%.frag,build/spv/%_frag.spv,$(wildcard samples/shaders/*.frag)) \
			   $(patsubst samples/shaders/%.mesh,build/spv/%_task.spv,$(wildcard samples/shaders/*.task)) \
			   $(patsubst samples/shaders/%.mesh,build/spv/%_mesh.spv,$(wildcard samples/shaders/*.mesh))
OBJ_FILES   := $(patsubst src/%.cpp,build/obj/%.o,$(wildcard src/*.cpp))

MODULE := build/lib/libVkWrappers.so

MAIN_UTILS := $(wildcard samples/utils/*.cpp)
SAMPLES := build/bin/ArrayAdd 			 \
           build/bin/ArraySaxpy 	   	 \
		   build/bin/GaussianBlur 		 \
		   build/bin/Triangle 			 \
		   build/bin/MeshShader

all: deps $(MODULE) $(SHADERS_SPV) $(SAMPLES)
lib: deps $(MODULE)
	$(shell) rm -rfd build/obj/ build/spv/ build/bin

build/obj/%.o: src/%.cpp
	$(CXX) $(CXX_FLAGS) $(DEFINES) -c -fPIC $(IFLAGS) -o $@ $<

$(MODULE): $(OBJ_FILES)
	$(CXX) $(CXX_FLAGS) -shared -o $@ $^

build/bin/%: samples/%.cpp $(MAIN_UTILS)
	$(CXX) $(CXX_FLAGS) -o $@ $(IFLAGS) -I./samples/utils -I./stb/ $^ $(LFLAGS)

build/spv/%_comp.spv: samples/shaders/%.comp
	glslc -std=450core $(GLSLC_FLAGS) -fshader-stage=compute -o $@ $^
build/spv/%_vert.spv: samples/shaders/%.vert
	glslc -std=450core $(GLSLC_FLAGS) -fshader-stage=vertex -o $@ $^
build/spv/%_frag.spv: samples/shaders/%.frag
	glslc -std=450core $(GLSLC_FLAGS) -fshader-stage=fragment -o $@ $^
build/spv/%_mesh.spv: samples/shaders/%.mesh
	glslc -std=450 $(GLSLC_FLAGS) -fshader-stage=mesh -o $@ $^
shaders: $(SHADERS_SPV)

$(SAMPLES): $(MODULE)

.PHONY: deps clean
deps:
	$(shell mkdir -p build/spv)
	$(shell mkdir -p build/obj)
	$(shell mkdir -p build/lib)
	$(shell mkdir -p build/bin)
clean:
	$(shell rm -rfd build)