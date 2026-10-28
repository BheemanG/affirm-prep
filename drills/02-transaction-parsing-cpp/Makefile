CXX      := g++
STD      ?= c++20
CXXFLAGS := -std=$(STD) -Wall -Wextra -g -fsanitize=address,undefined -fno-omit-frame-pointer
BUILD    := build/$(STD)

# Hidden tests compile against this file. Override to check another one, e.g.
#   make hidden SOLUTION=path/to/other.cpp
SOLUTION ?= solution.cpp
HIDDEN   := interviewer/hidden_tests/test_hidden.cpp

.PHONY: run test hidden clean

run: $(BUILD)/solution cases/sample.in
	./$(BUILD)/solution < cases/sample.in

test: $(BUILD)/test_parsing
	./$(BUILD)/test_parsing

hidden:
	@test -f $(HIDDEN) || { echo "No hidden tests in this checkout."; exit 1; }
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -Itests -DSOLUTION_FILE='"$(abspath $(SOLUTION))"' $(HIDDEN) -o $(BUILD)/test_hidden
	./$(BUILD)/test_hidden

$(BUILD)/solution: solution.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD)/test_parsing: tests/test_parsing.cpp tests/check.h solution.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf build
