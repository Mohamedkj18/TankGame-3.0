CXX        ?= g++
CXXFLAGS   ?= -std=c++20 -O2 -fPIC -Wall -Wextra -Wpedantic -MMD -MP

ROOT_DIR        ?= ..
USER_COMMON_DIR ?= $(ROOT_DIR)/UserCommon
INCLUDES        := -Iinclude -I$(ROOT_DIR)
ifneq ($(wildcard $(USER_COMMON_DIR)),)
  INCLUDES      += -I$(USER_COMMON_DIR)
endif

IDS        ?= 212497127_324916402

BUILD_DIR  := build
OBJ_DIR    := $(BUILD_DIR)/obj

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  SHARED_EXT := dylib
  LDFLAGS   += -dynamiclib -undefined dynamic_lookup
else
  SHARED_EXT := so
  LDFLAGS   += -shared
endif

LIB_NAME   := GameManager_$(IDS).$(SHARED_EXT)

SRCS := $(shell find src -name '*.cpp')
OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean
all: $(LIB_NAME)

$(LIB_NAME): $(OBJS)
	@echo "  [LD]  $@"
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@echo "  [C++] $<"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "  [CLEAN]"
	@rm -rf $(BUILD_DIR) $(LIB_NAME)

-include $(DEPS)
