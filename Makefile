#
# GTUltra root makefile
#

SRC_DIR := src
MAC_DIR := mac
LINUX_DIR := linux
WIN_DIR := win32

.PHONY: all clean mac-build mac-rebuild mac-run linux-build linux-rebuild win-build win-rebuild

all: mac-build

clean:
	rm -f $(SRC_DIR)/*.o $(SRC_DIR)/*.d $(SRC_DIR)/asm/*.o $(SRC_DIR)/asm/*.d $(SRC_DIR)/bme/*.o $(SRC_DIR)/bme/*.d $(SRC_DIR)/resid/*.o $(SRC_DIR)/resid/*.d $(SRC_DIR)/resid-fp/*.o $(SRC_DIR)/resid-fp/*.d
	rm -f $(SRC_DIR)/goatdata.c $(SRC_DIR)/gt2stereo.dat
	rm -f $(SRC_DIR)/bme/datafile $(SRC_DIR)/bme/dat2inc $(SRC_DIR)/bme/bme_end.o
	rm -rf $(MAC_DIR)/ $(LINUX_DIR)/

# --- Common build tool compilation ---
build-tools:
	@echo "=== Compiling build tools ==="
	cc -I$(SRC_DIR)/bme -I$(SRC_DIR)/bme/SDL -c -o $(SRC_DIR)/bme/bme_end.o $(SRC_DIR)/bme/bme_end.c
	cc -I$(SRC_DIR)/bme/SDL -o $(SRC_DIR)/bme/datafile $(SRC_DIR)/bme/datafile.c $(SRC_DIR)/bme/bme_end.o
	cc -I$(SRC_DIR)/bme/SDL -o $(SRC_DIR)/bme/dat2inc $(SRC_DIR)/bme/dat2inc.c $(SRC_DIR)/bme/bme_end.o
	@echo "=== Generating data files ==="
	cd $(SRC_DIR) && ./bme/datafile gt2stereo.dat gt2stereo.seq
	cd $(SRC_DIR) && ./bme/dat2inc gt2stereo.dat goatdata.c

# --- macOS ---
mac-build: build-tools
	@mkdir -p $(MAC_DIR)
	@echo "=== Building GTUltra (macOS) ==="
	cd $(SRC_DIR) && make -f makefile.mac
	@echo "=== Done: $(MAC_DIR)/gtultra ==="

mac-rebuild: clean mac-build

mac-run:
	@echo "Running with HOME=$(CURDIR)/$(MAC_DIR) (config in $(MAC_DIR)/.goattrk/)"
	HOME=$(CURDIR)/$(MAC_DIR) $(MAC_DIR)/gtultra

# --- Linux ---
linux-build: build-tools
	@mkdir -p $(LINUX_DIR)
	@echo "=== Building GTUltra (Linux) ==="
	cd $(SRC_DIR) && make -f makefile
	@echo "=== Done: $(LINUX_DIR)/gtultra ==="

linux-rebuild: clean linux-build

# --- Windows (cross-compile or MSYS2) ---
win-build: build-tools
	@mkdir -p $(WIN_DIR)
	@echo "=== Building GTUltra (Windows) ==="
	cd $(SRC_DIR) && make -f makefile.win
	@echo "=== Done: $(WIN_DIR)/gtultra.exe ==="

win-rebuild: clean win-build
