CC := gcc
SOURCE := core/salis.c
LIB_DEB := core/lib/libsalis-deb.so
LIB_REL := core/lib/libsalis-rel.so
VENV_DIR := venv
VENV_REQS := requirements.txt

# Flags
CFLAGS := \
	-Icore \
	-Wall \
	-Wextra \
	-Wmissing-prototypes \
	-Wno-unused-function \
	-Wno-unused-result \
	-Wno-unused-variable \
	-Wold-style-definition \
	-Wstrict-prototypes \
	-pedantic-errors \
	-shared \
	-std=c17
DEB_FLAGS := -Og -ggdb
REL_FLAGS := -O3 -DNDEBUG

# Targets
all: debug release venv

debug:
	$(CC) $(CFLAGS) $(DEB_FLAGS) $(SOURCE) -o $(LIB_DEB)

release:
	$(CC) $(CFLAGS) $(REL_FLAGS) $(SOURCE) -o $(LIB_REL)

venv:
	virtualenv -p python3 $(VENV_DIR)
	. $(VENV_DIR)/bin/activate && pip install -r $(VENV_REQS)

clean:
	-rm -f $(LIB_DEB)
	-rm -f $(LIB_REL)
	-rm -rf $(VENV_DIR)
