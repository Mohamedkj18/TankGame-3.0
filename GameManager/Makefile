# ================= Tanks Game 3.0 — GameManager Makefile (with common + UserCommon) =================
# Place this file in GameManager/

# ---- Toolchain / Platform ----
STUDENT1 := 212788293
STUDENT2 := 212497127

CXX      ?= c++
UNAME_S  := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    SO_EXT     := .dylib
    SO_FLAG    := -dynamiclib
    LDFLAGS_SO := $(SO_FLAG) -undefined dynamic_lookup
else
    SO_EXT     := .so
    SO_FLAG    := -shared
    LDFLAGS_SO := $(SO_FLAG)
endif

# ---- Paths ----
ROOT_DIR   := .
SRC_DIR    := $(ROOT_DIR)/src
BUILD_DIR  := $(ROOT_DIR)/build
OBJ_DIR    := $(BUILD_DIR)/obj

# ---- Include search paths ----
# - ./include       → local headers
# - ..              → repo root so #include "common/..."/"UserCommon/..." works
# - ../common       → direct includes without "common/"
# - ../UserCommon   → direct includes without "UserCommon/"
INC_DIRS   := $(ROOT_DIR)/include .. ../common ../UserCommon

# ---- Compiler flags ----
CXXFLAGS  ?= -std=c++20 -O2 -fPIC -Wall -Wextra -Wpedantic
CXXFLAGS  += $(addprefix -I,$(INC_DIRS))

# ---- Sources ----
GM_SRCS        := $(shell find $(SRC_DIR) -name '*.cpp')
REG_SRC        := $(wildcard $(ROOT_DIR)/GameManagerRegistration.cpp)
COMMON_SRCS    := $(wildcard ../common/*.cpp)
USERCOMMON_SRCS:= $(wildcard ../UserCommon/*.cpp)

LIB_SRCS       := $(GM_SRCS) $(REG_SRC) $(COMMON_SRCS) $(USERCOMMON_SRCS)

MAIN_SRC       := $(wildcard $(ROOT_DIR)/main.cpp)

# ---- Objects ----
OBJS       := $(patsubst $(ROOT_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(ROOT_DIR)/%,$(LIB_SRCS)))
OBJS       += $(patsubst ../%.cpp,$(OBJ_DIR)/../%.o,$(filter ../%,$(LIB_SRCS)))
MAIN_OBJ   := $(patsubst $(ROOT_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(MAIN_SRC))

# ---- Targets ----
LIB_NAME   := GameManager_$(STUDENT1)_$(STUDENT2)$(SO_EXT)
LIB_PATH   := $(ROOT_DIR)/$(LIB_NAME)

BIN_NAME   := game_manager_demo
BIN_PATH   := $(ROOT_DIR)/$(BIN_NAME)

.PHONY: default all lib exe clean veryclean print

default: lib
all: lib exe

# ---- Build the shared library ----
lib: $(LIB_PATH)

$(LIB_PATH): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) $(LDFLAGS_SO) -o $@

# ---- Optional demo executable ----
exe: $(BIN_PATH)

$(BIN_PATH): $(MAIN_OBJ) $(LIB_PATH)
	@mkdir -p $(dir $@)
	$(CXX) $(MAIN_OBJ) -L$(ROOT_DIR) -lGameManager -o $@
ifeq ($(UNAME_S),Darwin)
	@install_name_tool -add_rpath @loader_path $(BIN_PATH) || true
else
	@echo "Run with: LD_LIBRARY_PATH=. ./$(BIN_NAME)"
endif

# ---- Compile rules ----
$(OBJ_DIR)/%.o: $(ROOT_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/../%.o: ../%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---- Utilities ----
print:
	@echo "UNAME_S   = $(UNAME_S)"
	@echo "LIB_PATH  = $(LIB_PATH)"
	@echo "INC_DIRS  = $(INC_DIRS)"
	@echo "SRCS (lib):"
	@printf "  %s\n" $(LIB_SRCS)

clean:
	@rm -rf $(OBJ_DIR)

veryclean: clean
	@rm -f $(LIB_PATH) $(BIN_PATH)
