# ===== Simulator Makefile =====

CXX       := c++
CXXFLAGS  := -std=c++20 -O2 -Wall -Wextra -Wpedantic -MMD -MP \
             -Iinclude -I.. -I../UserCommon
LDFLAGS   :=
LDLIBS    := -pthread

# --- dlopen linkage only on Linux ---
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  LDLIBS += -ldl
endif

# --- Output name (edit ID if you want) ---
ID_SUFFIX ?= 212497127_324916402
SIM_BIN   := simulator_$(ID_SUFFIX)

# --- Sources ---
CORE_SRC := $(wildcard core/*.cpp)
REG_SRC  := GameManagerRegistration.cpp PlayerRegistration.cpp TankAlgorithmRegistration.cpp
MAIN_SRC := main.cpp
SRC      := $(CORE_SRC) $(REG_SRC) $(MAIN_SRC)

# --- Objects / deps ---
OBJDIR := build/obj
OBJS   := $(SRC:%.cpp=$(OBJDIR)/%.o)
DEPS   := $(OBJS:.o=.d)

# --- Default ---
.PHONY: all
all: $(SIM_BIN)

# --- Link executable ---
$(SIM_BIN): $(OBJS)
	@echo "  [LD]  $@"
	$(CXX) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# --- Compile objects (creates nested dirs) ---
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "  [C++] $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Convenience targets ---
.PHONY: clean print run run-comparative run-competition
clean:
	@echo "  [CLEAN]"
	@rm -rf $(OBJDIR) $(SIM_BIN) *.dSYM

print:
	@echo "SIM_BIN = $(SIM_BIN)"
	@echo "SRC     = $(SRC)"
	@echo "OBJS    = $(OBJS)"

# Example run targets (edit paths to your libs/maps)
run: $(SIM_BIN)
	./$(SIM_BIN) -comparative \
	  game_map=../maps/sample.map \
	  game_managers_folder=../GameManager \
	  algorithm1=../Algorithm/Algorithm_$(ID_SUFFIX).so \
	  algorithm2=../Algorithm/Algorithm_$(ID_SUFFIX).so \
	  -verbose

run-comparative: $(SIM_BIN)
	./$(SIM_BIN) -comparative \
	  game_map=../maps/sample.map \
	  game_managers_folder=../GameManager \
	  algorithm1=../algorithms/AlgoA.so \
	  algorithm2=../algorithms/AlgoB.so \
	  -verbose

run-competition: $(SIM_BIN)
	./$(SIM_BIN) -competition \
	  game_maps_folder=../maps \
	  game_manager=../GameManager/GameManager_$(ID_SUFFIX).so \
	  algorithms_folder=../algorithms \
	  -verbose

# --- Auto-include deps ---
-include $(DEPS)
