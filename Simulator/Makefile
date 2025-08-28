# ================= Tanks Game 3.0 — Simulator Makefile =================
# Place this file in Simulator/

# ---- Toolchain / Platform ----
CXX      ?= c++
UNAME_S  := $(shell uname -s)

# ---- Paths ----
ROOT_DIR   := .
SRC_DIR    := $(ROOT_DIR)/core
BUILD_DIR  := $(ROOT_DIR)/build
OBJ_DIR    := $(BUILD_DIR)/obj

# ---- Includes (headers only) ----
INC_DIRS   := $(ROOT_DIR)/include .. ../common ../UserCommon
INCS       := $(addprefix -I,$(INC_DIRS))

# ---- Feature toggles ----
SAN ?= 0   # make SAN=1 to enable ASan/UBSan

# ---- Base flags ----
CXXFLAGS  ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
CXXFLAGS  += $(INCS) -pthread
LDFLAGS   ?=
LDFLAGS   += -pthread
LDLIBS    ?=

# ---- Export host symbols & rpath for plugins ----
ifeq ($(UNAME_S),Darwin)
  LDFLAGS += -Wl,-export_dynamic -Wl,-rpath,@loader_path/..
else ifeq ($(UNAME_S),Linux)
  LDFLAGS += -Wl,--export-dynamic -Wl,-rpath,'$$ORIGIN/..'
  LDLIBS  += -ldl
endif

# ---- Sanitizers (optional) ----
ifeq ($(SAN),1)
  CXXFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -O1
  LDFLAGS  += -fsanitize=address,undefined
endif

# ---- Sources (SIMULATOR ONLY) ----
CORE_SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
MAIN_SRCS := $(wildcard $(ROOT_DIR)/main.cpp)
REG_NAMES := PlayerRegistration TankAlgorithmRegistration GameManagerRegistration
REG_SRCS  := $(foreach n,$(REG_NAMES), \
                $(wildcard $(ROOT_DIR)/$(n).cpp) \
                $(wildcard $(SRC_DIR)/$(n).cpp))
SRCS      := $(CORE_SRCS) $(MAIN_SRCS) $(REG_SRCS)

# ---- Objects & deps ----
OBJS := $(patsubst $(ROOT_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(ROOT_DIR)/%,$(SRCS)))
DEPS := $(OBJS:.o=.d)

# ---- Target (executable must be in Simulator/) ----
BIN_NAME := simulator_212788293_212497127
BIN_PATH := $(ROOT_DIR)/$(BIN_NAME)

.PHONY: all clean veryclean print run

all: $(BIN_PATH)

# ---- Link ----
$(BIN_PATH): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# ---- Compile (with header deps) ----
$(OBJ_DIR)/%.o: $(ROOT_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# ---- Utilities ----
print:
	@echo "UNAME_S   = $(UNAME_S)"
	@echo "BIN_PATH  = $(BIN_PATH)"
	@echo "INC_DIRS  = $(INC_DIRS)"
	@echo "REG_SRCS  = $(REG_SRCS)"
	@echo "SAN       = $(SAN)"
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

-include $(DEPS)
