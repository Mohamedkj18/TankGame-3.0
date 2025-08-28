# ================= Tanks Game 3.0 — Algorithm Plugin Makefile =================

# ---- Toolchain / Platform ----
CXX      ?= c++
UNAME_S  := $(shell uname -s)

# ---- IDs (edit if needed) ----
STUDENT1 := 212788293
STUDENT2 := 212497127

# ---- Paths (project-relative) ----
ROOT_DIR        := ..
COMMON_DIR      := $(ROOT_DIR)/common
USERCOMMON_DIR  := $(ROOT_DIR)/UserCommon
ALG_DIR         := $(ROOT_DIR)/Algorithm

# Outputs must live inside Algorithm/ (per assignment)
OUT_DIR         := $(ALG_DIR)

# Build folders for objects
BUILD_DIR       := $(ALG_DIR)/build
OBJ_DIR         := $(BUILD_DIR)/obj

# ---- Include search paths ----
# Note: DO NOT include Simulator here.
INC_DIRS := $(ALG_DIR)/include $(ROOT_DIR) $(COMMON_DIR) $(USERCOMMON_DIR)
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -fPIC
CXXFLAGS += $(addprefix -I,$(INC_DIRS))

# ---- Platform-specific linker flags & extension ----
ifeq ($(UNAME_S),Darwin)
  PLUG_EXT       := dylib
  PLUGIN_LDFLAGS := -dynamiclib -Wl,-undefined,dynamic_lookup
else
  PLUG_EXT       := so
  PLUGIN_LDFLAGS := -shared
endif
LDLIBS :=

# ---- Sources ----
# All Algorithm cpp files + any UserCommon cpp files used by the algorithm
ALG_SRCS        := $(shell find $(ALG_DIR)/src -name '*.cpp')
USERCOMMON_SRCS := $(shell find $(USERCOMMON_DIR) -name '*.cpp')
SRCS            := $(ALG_SRCS) $(USERCOMMON_SRCS)

# Normalize to absolute, then make paths relative to project root to avoid ../ in obj tree
ROOT_ABS  := $(abspath $(ROOT_DIR))
SRCS_ABS  := $(abspath $(SRCS))
SRCS_REL  := $(patsubst $(ROOT_ABS)/%,%,$(SRCS_ABS))

# Map e.g. Algorithm/src/Foo.cpp -> build/obj/Algorithm/src/Foo.o
OBJS      := $(addprefix $(OBJ_DIR)/,$(SRCS_REL:.cpp=.o))
DEPS      := $(OBJS:.o=.d)

# ---- Final plugin name ----
PLUGIN := $(OUT_DIR)/Algorithm_$(STUDENT1)_$(STUDENT2).$(PLUG_EXT)

.PHONY: all clean veryclean print
all: $(PLUGIN)

$(PLUGIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) $(PLUGIN_LDFLAGS) $(LDLIBS) -o $@

# Compile rule mirrored under build/obj/<relative path>
$(OBJ_DIR)/%.o: $(ROOT_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

print:
	@echo "UNAME_S  = $(UNAME_S)"
	@echo "PLUG_EXT = $(PLUG_EXT)"
	@echo "PLUGIN   = $(PLUGIN)"
	@echo "INC_DIRS = $(INC_DIRS)"
	@echo "SRCS:"; printf "  %s\n" $(SRCS_REL)

clean:
	@rm -rf $(BUILD_DIR)

veryclean: clean
	@rm -f $(PLUGIN)

-include $(DEPS)
