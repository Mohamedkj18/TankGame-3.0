# ================= Tanks Game 3.0 — Root Makefile =================

# Detect platform
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  SO_EXT := .dylib
  RPATH_VAR := DYLD_LIBRARY_PATH
else
  SO_EXT := .so
  RPATH_VAR := LD_LIBRARY_PATH
endif

# Optional user overrides
CXX      ?= c++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic

# Paths
ALGO_DIR       := Algorithm
GAMEMAN_DIR    := GameManager
SIM_DIR        := Simulator
DIST_DIR       := dist

# Artifacts (as produced by the sub-Makefiles I gave you)
ALGO_LIB       := $(ALGO_DIR)/Algorithm_212497127_324916402$(SO_EXT)
GAMEMAN_LIB    := $(GAMEMAN_DIR)/libGameManager$(SO_EXT)
SIM_BIN        := $(SIM_DIR)/build/simulator

.PHONY: all algorithm gamemanager simulator run print \
        clean veryclean dist package submit

# Build everything
all: algorithm gamemanager simulator

# Build each module by delegating to its own Makefile
algorithm:
	@$(MAKE) -C Algorithm CXX="$(CXX)"

gamemanager:
	@$(MAKE) -C GameManager lib CXX="$(CXX)"

simulator:
	@$(MAKE) -C Simulator CXX="$(CXX)"


# Run simulator with proper library path so it can find the libs at runtime
run: all
	@echo ">>> Running simulator with $(RPATH_VAR) set to repo root"
	@$(RPATH_VAR)=. $(SIM_BIN)

# Bundle deliverables into dist/ (useful for quick testing or handoff)
dist: all
	@mkdir -p $(DIST_DIR)
	@cp -f $(SIM_BIN) $(DIST_DIR)/
	@cp -f $(GAMEMAN_LIB) $(DIST_DIR)/
	@cp -f $(ALGO_LIB) $(DIST_DIR)/
	@echo "Packed into $(DIST_DIR)/:"
	@ls -l $(DIST_DIR)

# If your course asks for “submit” artifacts, customize here.
submit: dist
	@cd $(DIST_DIR) && tar -czf submission.tgz $(notdir $(SIM_BIN)) $(notdir $(GAMEMAN_LIB)) $(notdir $(ALGO_LIB))
	@echo "Created $(DIST_DIR)/submission.tgz"

print:
	@echo "Platform    : $(UNAME_S)"
	@echo "SO_EXT      : $(SO_EXT)"
	@echo "Algorithm   : $(ALGO_LIB)"
	@echo "GameManager : $(GAMEMAN_LIB)"
	@echo "Simulator   : $(SIM_BIN)"
	@echo "RPATH var   : $(RPATH_VAR)"

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
