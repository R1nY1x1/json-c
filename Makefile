PROGS = server server_single client

all: $(PROGS)

clean:
	rm -f $(PROGS) *~
