
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/lib

LIBRARY = lib/libhttp.so
TARGET = tests/programa_test
all: $(LIBRARY) $(TARGET)

$(LIBRARY): src/http_lib.c
	gcc -Wall -g -fPIC -shared -o $@ $< -Iinclude -ljansson -lcurl

$(TARGET): tests/main.c
	gcc -Wall -g -o $@ $< -Llib -lhttp -ljansson -lcurl -Iinclude

clean:
	rm -f $(LIBRARY) $(TARGET)

.PHONY: clean all
