TARGET = zenCheckerboard

OBJS = zenCheckerboard.o

CFLAGS += -Wall -O3

all : clean $(TARGET)
	$(STRIP) $(TARGET)

$(TARGET): $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)



