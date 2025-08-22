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

# ---- Platform-specific linker flags & extension ----
ifeq ($(UNAME_S),Darwin)
  PLUG_EXT       := dylib
  # For plugins that resolve some symbols from the host at runtime:
  PLUGIN_LDFLAGS := -dynamiclib -Wl,-undefined,dynamic_lookup
  LDLIBS         :=
else ifeq ($(UNAME_S),Linux)
  PLUG_EXT       := so
  PLUGIN_LDFLAGS := -shared
  LDLIBS         :=
else
  PLUG_EXT       := so
  PLUGIN_LDFLAGS := -shared
  LDLIBS         :=
endif

# ---- Sources ----
# Collect your Algorithm sources
ALG_SRCS := $(shell find $(ALG_DIR)/src -name '*.cpp')

# If your algorithm needs specific sources from UserCommon (example below), list them here:
# e.g., EXTRA_SRCS := $(USERCOMMON_DIR)/DirectionUtils.cpp
EXTRA_SRCS := $(USERCOMMON_DIR)/DirectionUtils.cpp

SRCS := $(ALG_SRCS) $(EXTRA_SRCS)

# ---- Objects (mirror folder structure under build/obj) ----
ALG_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS     := $(ALG_OBJS:.o=.d)

# ---- Targets ----
# Build two copies with the required IDs (matches your simulator’s expected filenames)
ALG_IDS  := 212788293_212497127 212788293_212497128
ALG_BINS := $(addprefix $(OUT_DIR)/Algorithm_,$(addsuffix .$(PLUG_EXT),$(ALG_IDS)))

.PHONY: all clean veryclean print
all: $(ALG_BINS)

# Link rule (same object set, different output names)
$(OUT_DIR)/Algorithm_%.$(PLUG_EXT): $(ALG_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(ALG_OBJS) $(PLUGIN_LDFLAGS) $(LDLIBS) -o $@

# Compile rule: mirror original tree under build/obj
# Handles sources both inside Algorithm/ and any listed in EXTRA_SRCS
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
