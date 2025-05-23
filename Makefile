CC = c++

CFLAGS = -Wall -Wextra -Werror -g  --std=c++98 -I./includes -I./includes/utils 

TARGET = webserver
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

SRCS = 	main.cpp \
		src/Server.cpp \
		src/Client.cpp \
		src/Response.cpp \
		src/Request.cpp \
		src/Parser.cpp \
		src/Booter.cpp \
		src/ServerManager.cpp \
		src/Utils.cpp \
		src/ConfigParser.cpp \
		src/Logger.cpp \
		src/utils/utilsServerManager.cpp \
		src/utils/utilsParser.cpp \
		src/Treenode.cpp \
		src/Cgi.cpp 

all: $(TARGET)

tester: $(SRCS)
	@echo "--------------------------------------------------"
	@echo "Compiling tester..."
	@echo "--------------------------------------------------"
	g++ $(SRCS) -I. -I./includes -g -Wall -Wextra -Wextra --std=c++98 -o tester

launch_test: tester
	@echo "--------------------------------------------------"
	@echo "Running tests for all config files..."
	@echo "--------------------------------------------------"
	@for config_file in ./config/invalid/*.conf; do \
		echo "Testing $$config_file..."; \
		./tester $$config_file; \
		echo "--------------------------------------------------"; \
	done

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

val: $(TARGET)
	$(VALGRIND) ./$(TARGET) config/valid/example1.conf

clean:
	rm -f $(TARGET)

re: clean
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

.PHONY: all clean testParser
