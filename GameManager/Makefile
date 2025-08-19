# ===== GameManager/.so build =====

CXX       := c++
CXXFLAGS  := -std=c++20 -O2 -Wall -Wextra -Wpedantic -MMD -MP -fPIC \
             -Iinclude -I../UserCommon
LDFLAGS   :=
LDLIBS    :=

# --- Platform: build a .so on both macOS and Linux ---
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SO_EXT          := so
    SHARED_LDFLAGS  := -dynamiclib -undefined dynamic_lookup
else
    SO_EXT          := so
    SHARED_LDFLAGS  := -shared
endif

# --- Output name (edit the suffix if you like) ---
ID_SUFFIX ?= 212497127_324916402
TARGET    := GameManager_$(ID_SUFFIX).$(SO_EXT)

# --- Sources ---
SRC := \
  $(wildcard src/*.cpp) \
  $(wildcard src/*/*.cpp)

# Exclude any standalone runner
SRC := $(filter-out src/main.cpp,$(SRC))

# Pull in UserCommon impl if present
ifneq ($(wildcard ../UserCommon/DirectionUtils.cpp),)
  SRC += ../UserCommon/DirectionUtils.cpp
endif

# --- Objects / deps ---
OBJDIR := build/obj
OBJS   := $(SRC:%.cpp=$(OBJDIR)/%.o)
DEPS   := $(OBJS:.o=.d)

# --- Default ---
.PHONY: all
all: $(TARGET)

# --- Link shared library (.so) ---
$(TARGET): $(OBJS)
	@echo "  [LD]  $@"
	$(CXX) $(SHARED_LDFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# --- Compile rule ---
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "  [C++] $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Optional: local test runner that uses src/main.cpp ---
# Builds a small binary linking the same objects + main.cpp (not needed for the .so)
TEST_BIN := gm_test
.PHONY: test
test: $(TEST_BIN)
$(TEST_BIN): $(filter-out $(OBJDIR)/src/main.o,$(OBJS)) $(OBJDIR)/src/main.o
	@echo "  [LD]  $@"
	$(CXX) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# Ensure main.o exists for test target only
$(OBJDIR)/src/main.o: src/main.cpp
	@mkdir -p $(dir $@)
	@echo "  [C++] $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Utils ---
.PHONY: clean print
clean:
	@echo "  [CLEAN]"
	@rm -rf $(OBJDIR) $(TARGET) $(TEST_BIN) *.dSYM

print:
	@echo "TARGET = $(TARGET)"
	@echo "SRC    = $(SRC)"
	@echo "OBJS   = $(OBJS)"

# --- Auto deps ---
-include $(DEPS)
