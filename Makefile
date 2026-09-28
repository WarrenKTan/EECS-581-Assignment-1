# Program name
PROGRAM = extract_ipv4.cpp
# Executable name
TARGET = extract_ipv4

# Default rule
all: run

# Rule to build the executable
$(TARGET): $(PROGRAM)
# 	@echo
# 	@echo Compiling...
	@g++ -std=c++11 $(PROGRAM) -o $(TARGET)
# 	@echo
# 	@echo '$(PROGRAM)' Successfully Compiled

# Rule to run the program
run: $(TARGET)
# 	@echo
# 	@echo Running Program...
	./$(TARGET)
# 	@echo

# Rule to run the program
test: $(TARGET)
# 	@echo
# 	@echo Running Program...
	./$(TARGET) < test_inputs.txt > test_output.txt
# 	@echo

# Clean rule
clean:
# 	@echo
# 	@echo Cleaning...
	@rm -f $(TARGET)
# 	@echo
# 	@echo Successfully Cleaned
# 	@echo
