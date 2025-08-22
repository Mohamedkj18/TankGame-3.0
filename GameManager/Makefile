# IDs
STUDENT1 := 212788293
STUDENT2 := 212497127
GM_BASENAME := GameManager_$(STUDENT1)_$(STUDENT2)

# Platform
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  PLUG_EXT := dylib
  PLUGIN_LDFLAGS := -dynamiclib -Wl,-undefined,dynamic_lookup
else
  PLUG_EXT := so
  PLUGIN_LDFLAGS := -shared
endif

# Paths
ROOT_DIR  := .
SRC_DIR   := $(ROOT_DIR)/src
BUILD_DIR := $(ROOT_DIR)/build
OBJ_DIR   := $(BUILD_DIR)/obj
OUT_DIR   := ../tests/GameManagers
OUTPUT    := $(OUT_DIR)/$(GM_BASENAME).$(PLUG_EXT)

# Includes
INC_DIRS := $(ROOT_DIR)/include .. ../common ../UserCommon
INCS     := $(addprefix -I,$(INC_DIRS))
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -fPIC

# Sources
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
EXTRA_SRCS := ../UserCommon/DirectionUtils.cpp   # if you actually need it
SRCS += $(EXTRA_SRCS)

# Objects / deps
OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean veryclean print
all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) $(PLUGIN_LDFLAGS) -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCS) -MMD -MP -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)

veryclean: clean
	@rm -f $(OUTPUT)

print:
	@echo "OUTPUT = $(OUTPUT)"
	@printf "SRCS:\n  %s\n" $(SRCS)

-include $(DEPS)
