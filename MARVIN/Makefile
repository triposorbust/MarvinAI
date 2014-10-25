CC = gcc
CCFLAG = -Wall -Wextra -I/System/Library/Frameworks/Python.framework/Headers
LDFLAG = -lpython -lm

TARGET = MarvinAI.so
DEP = ai.o foreign.o

all: $(TARGET)
	@ cp $(TARGET) ../$(TARGET)

# N.B. w. Apple's GCC, option flag `-soname` is `-install_name`
$(TARGET): $(DEP)
	@ $(CC) $(CCFLAG) -shared -Wl,-install_name,$@ -o $@ $^ $(LDFLAG)

%.o: %.c %.h
	@ $(CC) $(CCFLAG) -fPIC -c $< -o $@

.PHONY: clean
clean:
	@ rm -f $(TARGET) $(DEP)
	@ rm -f ../$(TARGET)
