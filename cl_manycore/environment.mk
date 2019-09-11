ORANGE=\033[0;33m
RED=\033[0;31m
NC=\033[0m

__MAKEFILE_ENVIRONMENT = 1

CL_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Check if we are running inside of the BSG Bladerunner repository by searching
# for Makefile.common. If Makefile.common is found, then we are and we should use
# that to define BSG_IP_CORES_DIR/BASEJUMP_STL_DIR, and BSG_MANYCORE_DIR and
# override previous settings. Warn, to provide feedback.
ifneq ("$(wildcard $(CL_DIR)/../../Makefile.common)","")

# Override BSG_IP_CORES_DIR and BSG_MANYCORE_DIR and warn if there is a
# mismatch. := is critical here since it assigns the value of the variable
# immediately, not after the include
ifdef BSG_IP_CORES_DIR
_BSG_IP_CORES_DIR := $(BSG_IP_CORES_DIR)
undefine BSG_IP_CORES_DIR
endif

ifdef BASEJUMP_STL_DIR
_BASEJUMP_STL_DIR := $(BASEJUMP_STL_DIR)
undefine BASEJUMP_STL_DIR
endif

ifdef BSG_MANYCORE_DIR
_BSG_MANYCORE_DIR := $(BSG_MANYCORE_DIR)
undefine BSG_MANYCORE_DIR
endif

include $(CL_DIR)/../../Makefile.common

ifdef _BSG_IP_CORES_DIR
ifneq ($(_BSG_IP_CORES_DIR), $(BSG_IP_CORES_DIR))
$(warning $(shell echo -e "$(ORANGE)BSG MAKE WARN: Overriding BSG_IP_CORES_DIR environment variable with Bladerunner defaults.$(NC)"))
$(warning $(shell echo -e "$(ORANGE)BSG MAKE WARN: BSG_IP_CORES_DIR=$(BSG_IP_CORES_DIR)$(NC)"))
endif
endif

ifdef _BASEJUMP_STL_DIR
ifneq ($(_BASEJUMP_STL_DIR), $(BASEJUMP_STL_DIR))
$(warning $(shell echo -e "$(ORANGE)BSG MAKE WARN: Overriding BASEJUMP_STL_DIR environment variable with Bladerunner defaults.$(NC)"))
$(warning $(shell echo -e "$(ORANGE)BSG MAKE WARN: BASEJUMP_STL_DIR=$(BASEJUMP_STL_DIR)$(NC)"))
endif
endif

ifdef _BSG_MANYCORE_DIR
ifneq ($(_BSG_MANYCORE_DIR), $(BSG_MANYCORE_DIR))
$(warning $(shell echo -e "$(ORANGE)BSG MAKE WARN: Overriding BSG_MANYCORE_DIR environment variable with Bladerunner defaults.$(NC)"))
$(warning $(shell echo -e "$(ORANGE)BSG MAKE WARN: BSG_MANYCORE_DIR=$(BSG_MANYCORE_DIR)$(NC)"))
endif
endif

endif

# If BSG_IP_CORES_DIR is not defined, raise an error.
ifndef BSG_IP_CORES_DIR
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: BSG_IP_CORES_DIR environment variable undefined. Defining is not recommended. Are you running from within Bladerunner?$(NC)"))
endif

# If BASEJUMP_STL_DIR is not defined, raise an error.
ifndef BASEJUMP_STL_DIR
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: BASEJUMP_STL_DIR environment variable undefined. Defining is not recommended. Are you running from within Bladerunner?$(NC)"))
endif

# If BSG_MANYCORE_DIR is not defined, raise an error.
ifndef BSG_MANYCORE_DIR
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: BSG_MANYCORE_DIR environment variable undefined. Defining is not recommended. Are you running from within Bladerunner?$(NC)"))
endif

HARDWARE_PATH    = $(CL_DIR)/hardware/
REGRESSION_PATH  = $(CL_DIR)/regression/
TESTBENCH_PATH   = $(CL_DIR)/testbenches/
LIBRARIES_PATH   = $(CL_DIR)/libraries/
BSG_MACHINE_PATH = $(CL_DIR)