# ================= Tanks Game 3.0 — GameManager Plugin Makefile =================

# ---- IDs ----
STUDENT1 := 212788293
STUDENT2 := 212497127
GM_BASENAME := GameManager_$(STUDENT1)_$(STUDENT2)

# ---- Platform ----
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  PLUG_EXT := dylib
  PLUGIN_LDFLAGS := -dynamiclib -Wl,-undefined,dynamic_lookup
else
  PLUG_EXT := so
  PLUGIN_LDFLAGS := -shared
endif

# ---- Paths ----
ROOT_DIR   := .
PROJ_ROOT  := ..
SRC_DIR    := $(ROOT_DIR)/src
COMMON_DIR := $(PROJ_ROOT)/common
USERC_DIR  := $(PROJ_ROOT)/UserCommon

BUILD_DIR  := $(ROOT_DIR)/build
OBJ_DIR    := $(BUILD_DIR)/obj

# Final plugin must be in GameManager/ (per assignment)
OUTPUT := $(ROOT_DIR)/$(GM_BASENAME).$(PLUG_EXT)

# ---- Includes ----
INC_DIRS := $(ROOT_DIR)/include $(PROJ_ROOT) $(COMMON_DIR) $(USERC_DIR)
INCS     := $(addprefix -I,$(INC_DIRS))
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -fPIC

# ---- Sources ----
GM_SRCS        := $(shell find $(SRC_DIR) -name '*.cpp')
USERCOMMON_SRCS:= $(shell find $(USERC_DIR) -name '*.cpp')
SRCS           := $(GM_SRCS) $(USERCOMMON_SRCS)

# Normalize to project-root-relative paths (avoid ../ in obj tree)
PROJ_ROOT_ABS := $(abspath $(PROJ_ROOT))
SRCS_ABS      := $(abspath $(SRCS))
SRCS_REL      := $(patsubst $(PROJ_ROOT_ABS)/%,%,$(SRCS_ABS))

# Objects mirrored under build/obj/<relative path>
OBJS := $(addprefix $(OBJ_DIR)/,$(SRCS_REL:.cpp=.o))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean veryclean print
all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) $(PLUGIN_LDFLAGS) -o $@

# Compile: map back to project-root-relative sources
$(OBJ_DIR)/%.o: $(PROJ_ROOT)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCS) -MMD -MP -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)

veryclean: clean
	@rm -f $(OUTPUT)

print:
	@echo "OUTPUT = $(OUTPUT)"
	@printf "SRCS:\n  %s\n" $(SRCS_REL)

-include $(DEPS)
