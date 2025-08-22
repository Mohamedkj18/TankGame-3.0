# ================= Tanks Game 3.0 — Root Makefile =================

# Detect platform
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  PLUG_EXT  := dylib
  RPATH_VAR := DYLD_LIBRARY_PATH
else
  PLUG_EXT  := so
  RPATH_VAR := LD_LIBRARY_PATH
endif

# Optional user overrides
CXX      ?= c++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic

# Paths
ALGO_DIR    := Algorithm
GAMEMAN_DIR := GameManager
SIM_DIR     := Simulator
DIST_DIR    := dist

# Built artifacts (from sub-Makefiles)
SIM_BIN := $(SIM_DIR)/build/simulator
# Plugins are emitted under tests/* (per the updated sub-Makefiles)
ALGO_GLOB := tests/Algorithms/Algorithm_*.$(PLUG_EXT)
GM_GLOB   := tests/GameManagers/GameManager_*.$(PLUG_EXT)

.PHONY: all algorithm gamemanager simulator run print clean veryclean dist package submit

# Build everything
all: algorithm gamemanager simulator

# Delegate to subprojects (use their default targets)
algorithm:
	@$(MAKE) -C $(ALGO_DIR) CXX="$(CXX)"

gamemanager:
	@$(MAKE) -C $(GAMEMAN_DIR) CXX="$(CXX)"

simulator:
	@$(MAKE) -C $(SIM_DIR) CXX="$(CXX)"


# Run simulator with proper search path (usually not needed due to rpath, but safe)
run: all
	@echo ">>> Running simulator with $(RPATH_VAR)=."
	@$(RPATH_VAR)=. $(SIM_BIN)

# Bundle deliverables into dist/
dist: all
	@mkdir -p $(DIST_DIR)
	@cp -f $(SIM_BIN) $(DIST_DIR)/
	# copy all algorithm plugins
	@cp -f $(ALGO_GLOB) $(DIST_DIR)/ 2>/dev/null || true
	# copy all game manager plugins
	@cp -f $(GM_GLOB)   $(DIST_DIR)/ 2>/dev/null || true
	@echo "Packed into $(DIST_DIR)/:"
	@ls -l $(DIST_DIR)

# Example submission tarball (adjust if your course needs specific names)
submit: dist
	@cd $(DIST_DIR) && tar -czf submission.tgz simulator Algorithm_*.$(PLUG_EXT) GameManager_*.$(PLUG_EXT) 2>/dev/null || true
	@echo "Created $(DIST_DIR)/submission.tgz"

print:
	@echo "Platform   : $(UNAME_S)"
	@echo "PLUG_EXT   : $(PLUG_EXT)"
	@echo "Simulator  : $(SIM_BIN)"
	@echo "Algo glob  : $(ALGO_GLOB)"
	@echo "GM glob    : $(GM_GLOB)"
	@echo "RPATH var  : $(RPATH_VAR)"

# Cleanup
clean:
	@$(MAKE) -C $(ALGO_DIR) clean || true
	@$(MAKE) -C $(GAMEMAN_DIR) clean || true
	@$(MAKE) -C $(SIM_DIR) clean || true
	@echo "Cleaned objects."

veryclean: clean
	@$(MAKE) -C $(ALGO_DIR) veryclean || true
	@$(MAKE) -C $(GAMEMAN_DIR) veryclean || true
	@$(MAKE) -C $(SIM_DIR) veryclean || true
	@rm -rf $(DIST_DIR)
	@echo "Removed libraries, binaries, and dist/."
