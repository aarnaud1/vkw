# Copyright (c) 2025 Adrien ARNAUD
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

CXX         := g++ -W -Wall -Wextra -Wmissing-field-initializers -Wconversion
CXX_FLAGS   := -std=c++17 -O2 -g --pedantic -ffast-math
GLSLC_FLAGS := -std=460 --target-env=vulkan1.3 --target-spv=spv1.4 -O
DEFINES     := -DVK_NO_PROTOTYPES -DDEBUG
IFLAGS      := -I./include \
			   -I./thidrparty/VulkanMemoryAllocator/include \
			   -I./thidrparty/volk
LFLAGS      := -L./build/lib -Wl,-rpath,./build/lib -lvkw -lglfw

SHADERS_SPV := $(patsubst samples/shaders/%.comp,build/spv/%.comp.spv,$(wildcard samples/shaders/*.comp)) \
			   $(patsubst samples/shaders/%.vert,build/spv/%.vert.spv,$(wildcard samples/shaders/*.vert)) \
			   $(patsubst samples/shaders/%.frag,build/spv/%.frag.spv,$(wildcard samples/shaders/*.frag)) \
			   $(patsubst samples/shaders/%.mesh,build/spv/%.task.spv,$(wildcard samples/shaders/*.task)) \
			   $(patsubst samples/shaders/%.mesh,build/spv/%.mesh.spv,$(wildcard samples/shaders/*.mesh))
OBJ_FILES   := $(patsubst src/%.cpp,build/obj/%.o,$(wildcard src/*.cpp))

MODULE := build/lib/libvkw.a

SAMPLES_SRCS := samples/main_desktop.cpp \
				samples/IGraphicsSample.cpp \
				samples/SimpleTriangle.cpp \
				samples/RayQueryTriangle.cpp

all: deps $(MODULE) $(SHADERS_SPV) build/bin/samples
lib: deps $(MODULE)
	$(shell) rm -rfd build/obj/ build/spv/ build/bin

build/obj/%.o: src/%.cpp
	$(CXX) $(CXX_FLAGS) $(DEFINES) -c $(IFLAGS) -o $@ $<

$(MODULE): $(OBJ_FILES)
	ar rcs $@ $^

build/bin/samples: $(MODULE)
	$(CXX) $(CXX_FLAGS) -o $@ $(DEFINES) $(IFLAGS) $(SAMPLES_SRCS) $(LFLAGS)

build/spv/%.comp.spv: samples/shaders/%.comp
	glslc $(GLSLC_FLAGS) -fshader-stage=compute -o $@ $^
build/spv/%.vert.spv: samples/shaders/%.vert
	glslc $(GLSLC_FLAGS) -fshader-stage=vertex -o $@ $^
build/spv/%.frag.spv: samples/shaders/%.frag
	glslc $(GLSLC_FLAGS) -fshader-stage=fragment -o $@ $^
build/spv/%.mesh.spv: samples/shaders/%.mesh
	glslc $(GLSLC_FLAGS) -fshader-stage=mesh -o $@ $^
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