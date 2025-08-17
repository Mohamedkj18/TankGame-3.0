CXX        ?= g++
CXXFLAGS   ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic -MMD -MP

ROOT_DIR        ?= ..
USER_COMMON_DIR ?= $(ROOT_DIR)/UserCommon
INCLUDES        := -Iinclude -I$(ROOT_DIR)
ifneq ($(wildcard $(USER_COMMON_DIR)),)
  INCLUDES      += -I$(USER_COMMON_DIR)
endif

IDS      ?= 212497127_324916402
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  LDLIBS  += -pthread
else
  LDLIBS  += -ldl -pthread
  LDFLAGS += -rdynamic
endif

BIN := simulator_$(IDS)

SRCS := $(shell find core -name '*.cpp') $(wildcard *.cpp)
OBJS := $(patsubst core/%.cpp,$(OBJ_DIR)/core/%.o,$(filter core/%,$(SRCS))) \
        $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(filter-out core/%,$(SRCS)))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean
all: $(BIN)

$(BIN): $(OBJS)
	@echo "  [LD]  $@"
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/core/%.o: core/%.cpp
	@mkdir -p $(dir $@)
	@echo "  [C++] $<"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "  [C++] $<"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "  [CLEAN]"
	@rm -rf $(BUILD_DIR) $(BIN)

-include $(DEPS)
