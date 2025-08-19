# ================= Tanks Game 3.0 — Algorithm Plugin Makefile =================
# Place this file in Algorithm/

# ---- Toolchain / Platform ----
CXX      ?= c++
UNAME_S  := $(shell uname -s)

# ---- Paths (adjust if your tree differs) ----
ROOT_DIR        := ..
SIM_DIR         := $(ROOT_DIR)/Simulator
COMMON_DIR      := $(ROOT_DIR)/common
USERCOMMON_DIR  := $(ROOT_DIR)/UserCommon
ALG_DIR         := $(ROOT_DIR)/Algorithm

# Output: where the simulator expects plugin .dylib/.so files
OUT_DIR         := $(ROOT_DIR)/tests/Algorithms

# Build folders for objects
BUILD_DIR       := $(ALG_DIR)/build
OBJ_DIR         := $(BUILD_DIR)/obj

# ---- Include search paths ----
INC_DIRS := $(SIM_DIR)/include $(ROOT_DIR) $(COMMON_DIR) $(USERCOMMON_DIR) $(ALG_DIR)/include
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -fPIC
CXXFLAGS += $(addprefix -I,$(INC_DIRS))

# ---- Platform-specific linker flags for plugins ----
# macOS: allow undefined symbols to resolve from the host executable at runtime
ifeq ($(UNAME_S),Darwin)
  PLUGIN_LDFLAGS := -shared -Wl,-undefined,dynamic_lookup
  LDLIBS         :=
else ifeq ($(UNAME_S),Linux)
  PLUGIN_LDFLAGS := -shared -fPIC
  LDLIBS         :=            # (no special libs needed for the plugin itself)
else
  PLUGIN_LDFLAGS := -shared
  LDLIBS         :=
endif

# ---- Sources ----
# Add your algorithm .cpp files here (recursively collects from Algorithm/src)
ALG_SRCS := $(shell find $(ALG_DIR)/src -name '*.cpp')

# ---- Objects (mirror folder structure under build/obj) ----
ALG_OBJS := $(patsubst $(ALG_DIR)/%.cpp,$(OBJ_DIR)/$(ALG_DIR)/%.o,$(ALG_SRCS))

# ---- Targets ----
# Build two copies with the required IDs (matches your simulator’s expected filenames)
ALG_IDS := 212788293_212497127 212788293_212497128
ALG_BINS := $(addprefix $(OUT_DIR)/Algorithm_,$(addsuffix .dylib,$(ALG_IDS)))

.PHONY: all clean veryclean print

all: $(ALG_BINS)

# Link rule (same object set, different output names)
$(OUT_DIR)/Algorithm_%.dylib: $(ALG_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(ALG_OBJS) $(PLUGIN_LDFLAGS) $(LDLIBS) -o $@

# Compile rule: keep original tree under build/obj
$(OBJ_DIR)/$(ALG_DIR)/%.o: $(ALG_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

print:
	@echo "UNAME_S     = $(UNAME_S)"
	@echo "OUT_DIR     = $(OUT_DIR)"
	@echo "INC_DIRS    = $(INC_DIRS)"
	@echo "ALG_SRCS:"; printf "  %s\n" $(ALG_SRCS)

clean:
	@rm -rf $(BUILD_DIR)

veryclean: clean
	@rm -f $(ALG_BINS)
