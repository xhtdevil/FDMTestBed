#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file $(TARGET)
# 'make clean'  removes all .o and executable files
# 'make run'	build executalbe file $(TARGET) and run it ./$(TARGET)
#

# define the C compiler to use
CC = g++

# define thr remove command
RM = rm

# define object files directory
OBJS_DIR = obj

# define any compile-time flags
CFLAGS = -Wall -g -std=c++11

# define any directories containing header files other than /usr/include
# eg: INCLUDES = -I/home/newhall/include  -I../include
# 
INCLUDES = 

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
#	eg: LFLAGS = -L/home/newhall/lib  -L../lib
# 
LFLAGS = 

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
#	eg: LIBS = -lmylib -lm
#
LIBS =

# define the C source files
SRCS = FDM.cpp \
	   utils.cpp

# define the C source files
SRCS_HUB = FDM_HUB.cpp \
	   utils.cpp

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:%.cpp=$(OBJS_DIR)/%.o)
OBJS_HUB = $(SRCS_HUB:%.cpp=$(OBJS_DIR)/%.o)

# define the executable file 
TARGET = fdm
TARGET_HUB = fdm_hub

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean run

all: $(TARGET) $(TARGET_HUB)
	@echo $(TARGET) has been succesfully compiled
	@echo $(TARGET_HUB) has been succesfully compiled

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LFLAGS) $(LIBS)

$(TARGET_HUB): $(OBJS_HUB) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET_HUB) $(OBJS_HUB) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
#
$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(OBJS_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# $(OBJS): $(OBJS_DIR)/%.o : %.c  
# 	@mkdir -p $(OBJS_DIR)
# 	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

run: $(TARGET)
	./$(TARGET) < in.txt
	@echo "-------------------------"
	@open alloc.txt

runhub: $(TARGET_HUB)
	./$(TARGET_HUB) < in_hub.txt
	@echo "-------------------------"
	@open alloc_hub.txt

clean:
	$(RM) -f $(OBJS_DIR)/*.o $(TARGET) $(TARGET_HUB)
	$(RM) -r $(OBJS_DIR)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it