CC=clang++

TARGET=multi_threaded_merge_sort

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(TARGET).cpp -pthread -o $(TARGET)

clean:
	rm $(TARGET)
