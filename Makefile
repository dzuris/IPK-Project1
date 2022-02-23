CC=gcc
FILES_SERVER=server.c

OUTPUT_FILE=hinfosvc

$(OUTPUT_FILE):
	$(CC) -o $@ $(FILES_SERVER)

clean:
	rm -f $(OUTPUT_FILE)
