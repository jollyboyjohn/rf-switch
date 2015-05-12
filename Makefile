OBJS = rf-switch.o hw.o

all : rf-switch

rf-switch: $(OBJS)
	$(CC) $^ -o $@

rf-switch.o: rf-switch.h hw.h
hw.o: hw.h

.PHONY: clean
clean:
	rm rf-switch $(OBJS)
