IDS ?= 212497127_324916402

ALGO_DIR := Algorithm
GM_DIR   := GameManager
SIM_DIR  := Simulator

.PHONY: all algorithm gamemanager simulator clean

all: algorithm gamemanager simulator

algorithm:
	@$(MAKE) -C $(ALGO_DIR) IDS="$(IDS)" ROOT_DIR=".."

gamemanager:
	@$(MAKE) -C $(GM_DIR) IDS="$(IDS)" ROOT_DIR=".."

simulator:
	@$(MAKE) -C $(SIM_DIR) IDS="$(IDS)" ROOT_DIR=".."

clean:
	-@$(MAKE) -C $(ALGO_DIR) clean
	-@$(MAKE) -C $(GM_DIR) clean
	-@$(MAKE) -C $(SIM_DIR) clean
