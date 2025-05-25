CC       = c++
CFLAGS   = -Wall -Wextra -Werror -g --std=c++98 -I./includes -I./includes/utils

TARGET   = webserver
TESTER   = tester
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

SRCS = 	main.cpp \
		src/Server.cpp \
		src/Client.cpp \
		src/Response.cpp \
		src/Request.cpp \
		src/ResourceValidator.cpp \
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

$(TARGET): $(SRCS)
	@echo "Building $(TARGET)..."
	$(CC) $(CFLAGS) -o $@ $^

$(TESTER): $(SRCS)
	@echo "--------------------------------------------------"
	@echo "Compiling $(TESTER)..."
	@echo "--------------------------------------------------"
	$(CC) $(CFLAGS) -o $@ $^

launch_test: $(TESTER)
	@echo "--------------------------------------------------"
	@echo "Running tests for all config files in ./config/invalid..."
	@echo "--------------------------------------------------"
	@for config_file in ./config/invalid/*.conf; do \
		echo "Testing $$config_file..."; \
		./$(TESTER) $$config_file; \
		echo "--------------------------------------------------"; \
	done

val: $(TARGET)
	$(VALGRIND) ./$(TARGET) config/valid/example1.conf

clean:
	@echo "Cleaning up..."
	rm -f $(TARGET) $(TESTER)

re: clean all

.PHONY: all clean re launch_test
