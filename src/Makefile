export CC      = gcc
export LD      = gcc
export CLIB    = ar cq
export CFLAGS  = -Wall -Werror -Wextra
export CDEBUG  = -g
export PROG	   = program		

DIRS= acquisition interface graph

########## TARGETS ########
all: $(patsubst %, _dir_%, $(DIRS))

$(patsubst %,_dir_%,$(DIRS)):
	cd $(patsubst _dir_%,%,$@) && make

########### CLEAN #########
clean: $(patsubst %, _clean_%, $(DIRS))

$(patsubst %,_clean_%,$(DIRS)):
	cd $(patsubst _clean_%,%,$@) && make clean
