PROJECT := tracer

DIR_SRC += .
DIR_SRC += ./src

DIR_INC += -I./inc 
DIR_INC += $(addprefix -I, $(DIR_SRC))

SRC_C += $(wildcard $(addsuffix /*.c, $(DIR_SRC)))
OBJ := $(patsubst %.c, %.o, $(SRC_C))
EXE := $(PROJECT)

CC := $(CC_PREFIX)gcc
CFLAG = -pthread -g

.PHONY:all

all:$(OBJ) $(EXE)

%.o: %.c
	$(CC) $(CFLAG) -c $(DIR_INC) $< -o $@ 

$(EXE): $(OBJ)
	$(CC) $(CFLAG) $(OBJ) -o $@ 

clean:
	rm -r $(EXE) $(OBJ) 
