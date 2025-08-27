# ================= Tanks Game 3.0 — Algorithm Plugin Makefile =================
# Place this file in Algorithm/

# ---- Toolchain / Platform ----
CXX      ?= c++
UNAME_S  := $(shell uname -s)

# ---- Paths ----
ROOT_DIR        := ..
SIM_DIR         := $(ROOT_DIR)/Simulator
COMMON_DIR      := $(ROOT_DIR)/common
USERCOMMON_DIR  := $(ROOT_DIR)/UserCommon
ALG_DIR         := $(ROOT_DIR)/Algorithm

# Output: where the simulator expects plugin .dylib/.so/.dll files
OUT_DIR         := $(ROOT_DIR)/tests/Algorithms

# Build folders for objects
BUILD_DIR       := $(ALG_DIR)/build
OBJ_DIR         := $(BUILD_DIR)/obj

# ---- Include search paths ----
INC_DIRS := $(SIM_DIR)/include $(ROOT_DIR) $(COMMON_DIR) $(USERCOMMON_DIR) $(ALG_DIR)/include
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -fPIC
CXXFLAGS += $(addprefix -I,$(INC_DIRS))

# ---- Platform-specific linker flags & extension ----
ifeq ($(UNAME_S),Darwin)
  PLUG_EXT       := dylib
  PLUGIN_LDFLAGS := -dynamiclib -Wl,-undefined,dynamic_lookup
  LDLIBS         :=
else ifeq ($(UNAME_S),Linux)
  PLUG_EXT       := so
  PLUGIN_LDFLAGS := -shared
  LDLIBS         :=
else ifneq (,$(findstring MINGW,$(UNAME_S)))
  # MinGW / Git Bash on Windows
  PLUG_EXT       := dll
  PLUGIN_LDFLAGS := -shared -Wl,--export-all-symbols
  LDLIBS         :=
else
  # Fallback (other *nix); adjust if your simulator needs something else
  PLUG_EXT       := so
  PLUGIN_LDFLAGS := -shared
  LDLIBS         :=
endif

# ---- Sources ----
# All .cpp under Algorithm/src (includes registration.cpp if present)
ALG_SRCS := $(shell find $(ALG_DIR)/src -name '*.cpp')

# Optional extras from UserCommon if they exist
EXTRA_SRCS := $(wildcard $(USERCOMMON_DIR)/DirectionUtils.cpp)

SRCS := $(ALG_SRCS) $(EXTRA_SRCS)

# ---- Objects (mirror folder structure under build/obj) ----
ALG_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS     := $(ALG_OBJS:.o=.d)

# ---- Targets ----
# Build two copies with the required IDs (matches the simulator’s expected filenames)
ALG_IDS  := 212788293_212497127 212788293_212497128
ALG_BINS := $(addprefix $(OUT_DIR)/Algorithm_,$(addsuffix .$(PLUG_EXT),$(ALG_IDS)))

.PHONY: all clean veryclean print sim

all: $(ALG_BINS)

# Link rule (same object set, different output names)
$(OUT_DIR)/Algorithm_%.$(PLUG_EXT): $(ALG_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(ALG_OBJS) $(PLUGIN_LDFLAGS) $(LDLIBS) -o $@

# Compile rule: mirror original tree under build/obj
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

print:
	@echo "UNAME_S   = $(UNAME_S)"
	@echo "OUT_DIR   = $(OUT_DIR)"
	@echo "PLUG_EXT  = $(PLUG_EXT)"
	@echo "INC_DIRS  = $(INC_DIRS)"
	@echo "SRCS:"; printf "  %s\n" $(SRCS)

clean:
	@rm -rf $(BUILD_DIR)

veryclean: clean
	@rm -f $(ALG_BINS)

-include $(DEPS)

# ---- (Optional) Simulator convenience target ----
# Build sim (if it has its own makefile) and remind how to run a match.
sim:
	@echo "Build your plugins with:  mingw32-make -C Algorithm  (or make -C Algorithm)"
	@echo "Your plugins will be at:  $(OUT_DIR)/Algorithm_<ID>.$(PLUG_EXT)"
	@echo "Now run the Simulator with those two plugins on the desired map."
	@echo "Example (adjust path/flags to your Simulator CLI):"
	@echo "  Simulator/bin/TanksGame --alg1 $(OUT_DIR)/Algorithm_212788293_212497127.$(PLUG_EXT) \\"
	@echo "                          --alg2 $(OUT_DIR)/Algorithm_212788293_212497128.$(PLUG_EXT) \\"
	@echo "                          --map  tests/Boards/empty.board --verbose"
