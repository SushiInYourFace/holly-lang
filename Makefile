UNAME_S := $(shell uname -s)

CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Wno-c23-extensions -g 
OUTNAME = holly

ifeq ($(UNAME_S),Darwin)
	CC = clang
	CODESIGN = 	codesign -s - -f --options runtime --entitlements ./ent.plist ./holly
else ifeq ($(UNAME_S),Linux)
	CC = gcc
	CODESIGN = 
endif


SRCS = $(wildcard src/*.c)
SRCS += $(wildcard src/parsing/*.c)

$(OUTNAME): $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LIBS) -o $(OUTNAME)
	$(CODESIGN)


clean: 
	rm -f $(OUTNAME)

remake:
	$(MAKE) clean
	$(MAKE) $(OUTNAME)