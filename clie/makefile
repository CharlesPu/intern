EXEC = clie
DIR_APP	=./APP
DIR_TASK=./TASK
DIR_CURR=.
DIR_OBJ=./OBJ
ALL_DIR= -I${DIR_APP} \
         -I${DIR_TASK} \
         -I${DIR_CURR} 

C_SRC  =${wildcard ${DIR_APP}/*.c}\
		${wildcard ${DIR_TASK}/*.c}\
		${wildcard ${DIR_CURR}/*.c}
		
CPP_SRC=${wildcard ${DIR_APP}/*.cpp}\
		${wildcard ${DIR_TASK}/*.cpp}\
		${wildcard ${DIR_CURR}/*.cpp}
		
CPP_OBJS = ${patsubst %.cpp, ${DIR_OBJ}/%.o, ${notdir ${CPP_SRC}}}
C_OBJS = ${patsubst %.c, ${DIR_OBJ}/%.o, ${notdir ${C_SRC}}}

CC = arm-none-linux-gnueabi-g++
CFLAGS = -O2 -Wall ${ALL_DIR} -std=c++0x
LDFLAGS = -pthread

all:$(EXEC)

$(EXEC):$(CPP_OBJS)	$(C_OBJS)	
	$(CC) $(LDFLAGS) -o $@ $(CPP_OBJS)	$(C_OBJS) 
	@echo "Make done."

${DIR_OBJ}/%.o:${DIR_APP}/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@	
${DIR_OBJ}/%.o:${DIR_TASK}/%.cpp	
	$(CC) $(CFLAGS) -c $< -o $@	
${DIR_OBJ}/%.o:${DIR_CURR}/%.cpp	
	$(CC) $(CFLAGS) -c $< -o $@	
	
${DIR_OBJ}/%.o:${DIR_APP}/%.c
	$(CC) $(CFLAGS) -c $< -o $@	
${DIR_OBJ}/%.o:${DIR_TASK}/%.c	
	$(CC) $(CFLAGS) -c $< -o $@	
${DIR_OBJ}/%.o:${DIR_CURR}/%.c	
	$(CC) $(CFLAGS) -c $< -o $@	


clean:
	@rm -rf ${DIR_OBJ}/*.o
	@echo "Clean done."
clean_all:
	@rm -rf ${DIR_OBJ}/*.o
	@rm -rf $(EXEC)
	@echo "Clean all done."