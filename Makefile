CC 		 = cc
SRCDIR 	 = src
BUILDDIR = build

TARGET   = bin/snake_game
SOURCES  = $(wildcard $(SRCDIR)/*.c)
OBJECTS  = $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:.c=.o)))
CFLAGS 	 = -Wall -g -std=gnu99 
LIB 	 = -lncurses -pthread 

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	rm $(OBJECTS) 
	rm $(TARGET)

.PHONY: clean% 
