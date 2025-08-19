# ===== Algorithm/.so build =====

CXX       := c++
CXXFLAGS  := -std=c++20 -O2 -Wall -Wextra -Wpedantic -MMD -MP -fPIC \
             -Iinclude -I../UserCommon
LDFLAGS   :=
LDLIBS    :=

# Force .so as requested (works on Linux; on macOS this still produces a .so with -shared)
SO_EXT    := so
SHARED_LDFLAGS := -shared

# -------- Output name (change the suffix to whatever you like) --------
ID_SUFFIX ?= 212497127_324916402
TARGET    := Algorithm_$(ID_SUFFIX).$(SO_EXT)

# -------- Sources --------
SRC := \
  $(wildcard src/*.cpp) \
  $(wildcard src/Roles/*.cpp)

# Add UserCommon implementation if present
ifneq ($(wildcard ../UserCommon/DirectionUtils.cpp),)
  SRC += ../UserCommon/DirectionUtils.cpp
endif

# -------- Objects / deps --------
OBJDIR := build/obj
OBJS   := $(SRC:%.cpp=$(OBJDIR)/%.o)
DEPS   := $(OBJS:.o=.d)

# -------- Default --------
.PHONY: all
all: $(TARGET)

# -------- Link shared library (.so) --------
$(TARGET): $(OBJS)
	@echo "  [LD]  $@"
	$(CXX) $(SHARED_LDFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# -------- Compile --------
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "  [C++] $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# -------- Utilities --------
.PHONY: clean print
clean:
	@echo "  [CLEAN]"
	@rm -rf $(OBJDIR) $(TARGET) *.dSYM

print:
	@echo "TARGET   = $(TARGET)"
	@echo "SRC      = $(SRC)"
	@echo "OBJS     = $(OBJS)"

# -------- Auto deps --------
-include $(DEPS)
