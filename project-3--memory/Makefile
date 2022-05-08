CC=clang++

TARGET_1=file_sys_example
TARGET_2=mmap
RELATIVE_PATH=../../../../../../../../Home/Desktop/

all: $(TARGET_1) $(TARGET_2)

fyi: $(TARGET_1)
	./resources/$(TARGET_1) resources/example_input.txt resources/example_output.txt

test: $(TARGET_2)
	./$(TARGET_2) testFile.txt outFile.txt

large: $(TARGET_2)
	./$(TARGET_2) $(RELATIVE_PATH)inFile.txt $(RELATIVE_PATH)outFile.txt

$(TARGET_1): resources/$(TARGET_1).cpp
	$(CC) resources/$(TARGET_1).cpp -o resources/$(TARGET_1)

$(TARGET_2): $(TARGET_2).cpp
	$(CC) $(TARGET_2).cpp -o $(TARGET_2)

clean:
	rm $(TARGET_1) $(TARGET_2)
	