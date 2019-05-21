bin=crash

# Set the following to '0' to disable log messages:
debug=1

CFLAGS += -Wall -g -DDEBUG=$(debug)
LDFLAGS +=

src=history.c shell.c timer.c tokenizer.c leetify.c expansion.c
obj=$(src:.c=.o)

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

shell.o: shell.c history.h timer.h debug.h tokenizer.h leetify.h expansion.h
history.o: history.c history.h
timer.o: timer.c timer.h
tokenizer.o: tokenizer.c tokenizer.h
leetify.o: leetify.c leetify.h
expansion.o: expansion.c expansion.h

clean:
	rm -f $(bin) $(obj)
