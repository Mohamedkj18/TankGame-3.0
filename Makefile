# ===== Top-level Makefile =====
# Default artifact suffix for all subprojects
ID_SUFFIX ?= 212788293_212497127

SUBS := Algorithm GameManager Simulator

.PHONY: all algorithm gamemanager simulator clean print run run-comparative run-competition rebuild

# Build everything (libs first, then simulator)
all: algorithm gamemanager simulator

algorithm:
	$$(MAKE) -C Algorithm ID_SUFFIX=$(ID_SUFFIX)

gamemanager:
	$$(MAKE) -C GameManager ID_SUFFIX=$(ID_SUFFIX)

simulator: algorithm gamemanager
	$$(MAKE) -C Simulator ID_SUFFIX=$(ID_SUFFIX)

clean:
	@for d in $(SUBS); do \
		echo "[CLEAN] $$d"; \
		$$(MAKE) -C $$d clean ID_SUFFIX=$(ID_SUFFIX); \
	done

print:
	@echo "ID_SUFFIX = $(ID_SUFFIX)"
	@for d in $(SUBS); do \
		echo "[PRINT] $$d"; \
		$$(MAKE) -C $$d print ID_SUFFIX=$(ID_SUFFIX) || true; \
	done

rebuild: clean all

run: simulator
	$$(MAKE) -C Simulator run ID_SUFFIX=$(ID_SUFFIX)

run-comparative: simulator
	$$(MAKE) -C Simulator run-comparative ID_SUFFIX=$(ID_SUFFIX)

run-competition: simulator
	$$(MAKE) -C Simulator run-competition ID_SUFFIX=$(ID_SUFFIX)
