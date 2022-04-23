CC=clang++

TARGET=multi_threaded_merge_sort

all: $(TARGET) c_$(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(TARGET).cpp -pthread -o $(TARGET)

c_$(TARGET): c_$(TARGET).cpp
	$(CC) c_$(TARGET).cpp -pthread -o c_$(TARGET)

clean:
	rm $(TARGET)
	rm c_$(TARGET)
