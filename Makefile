# names

nl := lib_emulator

# dependencies

nd += 

# directory paths

db := binary/
do := object/
di := include/
ds := source/

# compilers

cx := g++
cc := gcc

# linkers

lx := g++
lc := gcc

# flags - compiler

fcx := -g -c -O0 -std=c++17 -Wall -MMD
fcc := -g -c -O0 -Wall -MMD

# flags - linker

flx := -lrt
flc :=

# objects

ob := $(patsubst $(ds)%.cpp, $(do)%.o, $(wildcard $(ds)*.cpp))

# targets

all: $(db)$(addsuffix .out, $(nl))

clean:
	-rm $(db)*
	-rm $(do)*

init:
	-mkdir $(db) $(do) $(di) $(ds)
	-touch $(ds)main.cpp

install:
	-cp -r $(di). ../$(di)
	-cp -r $(ds). ../$(ds)
	-cp -r $(db). ../$(db)
	-rm -rf ../$(nl)/

update:
	$(foreach dep, $(nd), $(shell git clone $(dep) && make -C $(basename $(notdir $(dep)))/ install))

# linking rules

$(db)$(addsuffix .out, $(nl)): $(ob)
	$(lx) $^ -o $@ $(flx)

# compilation rules

$(do)%.o: $(ds)%.cpp
	$(cx) $(fcx) $< -o $@ -I $(di)

# helpers

.PHONY: clean dir install
-include $(ob:.o=.d)