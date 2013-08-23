CURRENT_DIR = `pwd`
OBJECT_DIR = obj
EXEC_DIR = bin

SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=$(OBJECT_DIR)/%.o)
OUT = $(EXEC_DIR)/executable

INC = 
CFLAGS =  -Wall -std=c99
RESINC = 
LIBDIR = 
LIB =  -lglfw -lGL -lGLU -lpthread -lXxf86vm -lm -lpng
LDFLAGS =

all: compile

before:
	test -d $(EXEC_DIR) || mkdir -p $(EXEC_DIR)
	test -d $(OBJECT_DIR) || mkdir -p $(OBJECT_DIR)

after: 

compile: before clean out after

out: $(OBJ) $(DEP)
	gcc $(LDFLAGS) $(LIBDIR) $(OBJ) $(LIB) -o $(OUT)

$(OBJECT_DIR)/%.o: %.c
	gcc $(CFLAGS) $(INC) -c $< -o $@

clean: 
	rm -rf $(OBJ) $(OUT)
