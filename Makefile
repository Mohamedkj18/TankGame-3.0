
STUDENT1 := 212788293
STUDENT2 := 212497127

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  PLUG_EXT  := dylib
  RPATH_VAR := DYLD_LIBRARY_PATH
else
  PLUG_EXT  := so
  RPATH_VAR := LD_LIBRARY_PATH
endif

ALGO_DIR    := Algorithm
GAMEMAN_DIR := GameManager
SIM_DIR     := Simulator

SIM_BIN := $(SIM_DIR)/simulator_$(STUDENT1)_$(STUDENT2)
ALGO_SO := $(ALGO_DIR)/Algorithm_$(STUDENT1)_$(STUDENT2).$(PLUG_EXT)
GM_SO   := $(GAMEMAN_DIR)/GameManager_$(STUDENT1)_$(STUDENT2).$(PLUG_EXT)

.PHONY: all algorithm gamemanager simulator run print clean veryclean submit zipcheck

all: algorithm gamemanager simulator

algorithm:
	@$(MAKE) -C $(ALGO_DIR)

gamemanager:
	@$(MAKE) -C $(GAMEMAN_DIR)

simulator:
	@$(MAKE) -C $(SIM_DIR)

run: all
	@echo ">>> Running simulator (with $(RPATH_VAR)=.. just in case)"
	@$(RPATH_VAR)=.. $(SIM_BIN)

print:
	@echo "Platform   : $(UNAME_S)"
	@echo "PLUG_EXT   : $(PLUG_EXT)"
	@echo "SIM_BIN    : $(SIM_BIN)"
	@echo "ALGO_SO    : $(ALGO_SO)"
	@echo "GM_SO      : $(GM_SO)"
	@echo "RPATH VAR  : $(RPATH_VAR)"

clean:
	@$(MAKE) -C $(ALGO_DIR) clean || true
	@$(MAKE) -C $(GAMEMAN_DIR) clean || true
	@$(MAKE) -C $(SIM_DIR) clean || true
	@echo "Cleaned objects."

veryclean: clean
	@$(MAKE) -C $(ALGO_DIR) veryclean || true
	@$(MAKE) -C $(GAMEMAN_DIR) veryclean || true
	@$(MAKE) -C $(SIM_DIR) veryclean || true
	@echo "Removed libraries and binaries."


SUBMIT_ZIP := ex3_$(STUDENT1)_$(STUDENT2).zip

zipcheck:
	@test -f README.md || (echo "Missing README.md at repo root"; exit 1)
	@test -f students.txt || (echo "Missing students.txt at repo root"; exit 1)

submit: veryclean zipcheck
	@echo "Creating $(SUBMIT_ZIP) (sources only)..."
	@zip -r "$(SUBMIT_ZIP)" \
		Simulator Algorithm GameManager common UserCommon Makefile README.md students.txt \
		-x "*/build/*" \
		-x "$(ALGO_DIR)/*.so" "$(ALGO_DIR)/*.dylib" \
		-x "$(GAMEMAN_DIR)/*.so" "$(GAMEMAN_DIR)/*.dylib" \
		-x "$(SIM_DIR)/simulator_*" \
		> /dev/null
	@echo "Done: $(SUBMIT_ZIP)"
