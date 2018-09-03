CC=gcc
TARGET=run

CFLAGS=-Wall -std=c99

# We need to include some source files from unity into the build
UNITY=Unity
UNITY_FILES=$(UNITY)/src/unity.c $(UNITY)/extras/fixture/src/unity_fixture.c

# Test sources. Add additional tests and runners here!
TESTS=test/TestLightScheduler.c
RUNNERS=test/TestLightScheduler_Runner.c

# Source files. Add additional sources here!
SRCS=$(UNITY_FILES) $(TESTS) $(RUNNERS) src/LightScheduler.c 

# Include directories
INCL=-I include -I$(UNITY)/src -I$(UNITY)/extras/fixture/src

.PHONY: all clean run_tests

default all: clean run_tests

test/%_Runner.c: test/%.c
	ruby $(UNITY)/auto/generate_test_runner.rb $< $@

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCL) $(SRCS) -o $@

run_tests: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(TARGET) $(RUNNERS) *~ src/*~ test/*~ include/*~
