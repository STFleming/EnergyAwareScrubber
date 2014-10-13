CC = g++
INC_DIR = ./includes
SRC_DIR = ./src
BIN_DIR = ./bin
COPTS = -lpthread


power_reader:
	$(CC) -I$(INC_DIR) $(SRC_DIR)/continuous_power_monitor.cpp -o $(BIN_DIR)/power_reader 	

scrubber:
	$(CC) -I$(INC_DIR) $(SRC_DIR)/scrubber.cpp -o $(BIN_DIR)/scrubber $(COPTS) 	
