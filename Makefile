CC=gcc
CFLAGS=a
FILES_SERVER=server.c
FILES_CLIENT=client.c

OUTPUT_FILE=hinfosvc
OUTPUT_CLIENT=client

$(OUTPUT_FILE):
	$(CC) -o $@ $(FILES_SERVER)

$(OUTPUT_CLIENT):
	$(CC) -o $@ $(FILES_CLIENT)

clean:
	rm -f $(OUTPUT_FILE)
	rm -f $(OUTPUT_CLIENT)
