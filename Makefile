CC=gcc
CFLAGS=-g -Wall -lreadline -llua -lm -ldl
INCLUDE=-I 3rd/lua/src -L 3rd/lua/src
SRC_DIR=src
BIN_DIR=bin

SOURCE=$(SRC_DIR)/ldb.c $(SRC_DIR)/lcommand.c $(SRC_DIR)/lfile.c

sherldb: $(SOURCE)
	$(CC) -o $(BIN_DIR)/sherldb $(SOURCE) $(INCLUDE) $(CFLAGS)

clean:
	-rm $(BIN_DIR)/sherldb
	-rm ./core
