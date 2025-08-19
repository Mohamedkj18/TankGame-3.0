# ================= Tanks Game 3.0 — Simulator Makefile =================
# Exports host symbols; compiles core + {Player, TankAlgorithm, GameManager}Registration
# Place this file in Simulator/

# ---- Toolchain / Platform ----
CXX      ?= c++
UNAME_S  := $(shell uname -s)

# ---- Paths ----
ROOT_DIR   := .
SRC_DIR    := $(ROOT_DIR)/core
BUILD_DIR  := $(ROOT_DIR)/build
OBJ_DIR    := $(BUILD_DIR)/obj

# ---- Include search paths (headers only) ----
# Keep the simulator lean: do NOT compile ../common or ../UserCommon sources here.
INC_DIRS   := $(ROOT_DIR)/include .. ../common ../UserCommon

CXXFLAGS  ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
CXXFLAGS  += $(addprefix -I,$(INC_DIRS))

# ---- Export the simulator's symbols so plugins can resolve registration APIs ----
ifeq ($(UNAME_S),Darwin)
  EXPORT_DYNAMIC := -Wl,-export_dynamic
  RPATH_FLAGS    := -Wl,-rpath,@loader_path/..
  LDLIBS_EXTRA   :=
else ifeq ($(UNAME_S),Linux)
  EXPORT_DYNAMIC := -Wl,--export-dynamic
  RPATH_FLAGS    := -Wl,-rpath,'$$ORIGIN/..'
  LDLIBS_EXTRA   := -ldl
else
  EXPORT_DYNAMIC :=
  RPATH_FLAGS    :=
  LDLIBS_EXTRA   :=
endif

LDFLAGS   ?=
LDFLAGS   += $(EXPORT_DYNAMIC) $(RPATH_FLAGS)
LDLIBS    ?=
LDLIBS    += $(LDLIBS_EXTRA)

# ---- Sources (SIMULATOR ONLY) ----
# Core sources + main
CORE_SRCS   := $(shell find $(SRC_DIR) -name '*.cpp')
MAIN_SRCS   := $(wildcard $(ROOT_DIR)/main.cpp)

# Explicitly include registration implementations from either root or core:
REG_NAMES   := PlayerRegistration TankAlgorithmRegistration GameManagerRegistration
REG_SRCS    := $(foreach n,$(REG_NAMES), \
                  $(wildcard $(ROOT_DIR)/$(n).cpp) \
                  $(wildcard $(SRC_DIR)/$(n).cpp))

SRCS        := $(CORE_SRCS) $(MAIN_SRCS) $(REG_SRCS)

# ---- Objects ----
OBJS := $(patsubst $(ROOT_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(ROOT_DIR)/%,$(SRCS)))

# ---- Target ----
BIN_NAME := simulator
BIN_PATH := $(BUILD_DIR)/$(BIN_NAME)

.PHONY: all clean veryclean print run

all: $(BIN_PATH)

# ---- Link the simulator ----
$(BIN_PATH): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# ---- Compile rules ----
$(OBJ_DIR)/%.o: $(ROOT_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---- Utilities ----
print:
	@echo "UNAME_S   = $(UNAME_S)"
	@echo "BIN_PATH  = $(BIN_PATH)"
	@echo "INC_DIRS  = $(INC_DIRS)"
	@echo "REG_SRCS  = $(REG_SRCS)"
	@echo "SRCS:"; printf "  %s\n" $(SRCS)

run: $(BIN_PATH)
ifeq ($(UNAME_S),Darwin)
	@echo "Running with DYLD_LIBRARY_PATH=.."
	@DYLD_LIBRARY_PATH=.. $(BIN_PATH)
else
	@echo "Running with LD_LIBRARY_PATH=.."
	@LD_LIBRARY_PATH=.. $(BIN_PATH)
endif

clean:
	@rm -rf $(OBJ_DIR)

veryclean: clean
	@rm -f $(BIN_PATH)
