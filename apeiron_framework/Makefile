PLATFORM ?= xilinx_u200_gen3x16_xdma_2_202110_1
#PLATFORM ?= xilinx_u280_xdma_201920_3
XCLBIN_NAME ?= test.xclbin
TARGET ?= hw
#TARGET ?= hw_emu
AUTOGEN_YAML ?= config.yaml

TARGET_APP ?= CNN_apeiron_preprocessing_node

LDFLAGS += -L$(XILINX_XRT)/lib -lxrt_coreutil -pthread
CXXFLAGS += -std=c++1y -I$(XILINX_XRT)/include -Wall

PRODUCTS := $(XCLBIN_NAME) bandwidth_test latency_test host_test emconfig.json 

VPP ?= v++
VPP_FLAGS += -t $(TARGET) --platform $(PLATFORM)
VPP_FLAGS += -s --temp_dir $(TMP_BUILD_DIR) --log_dir $(TMP_BUILD_DIR)/logs --report_dir $(TMP_BUILD_DIR)/reports 
VPP_CFLAGS += -I include
VPP_LINKER_CONFIG ?= vpp_linker.cfg
POSTSYSLINKTCL ?= $(shell readlink -f ./scripts/post_sys_link.tcl)
CONSTRAINTSTCL ?= $(shell readlink -f ./scripts/textarossa_switch.tcl)

RM := rm -rf

EMCONFIG_DIR := emconfig
XO_DIR := dev_apps/$(TARGET_APP)/hw_hdl
TMP_BUILD_DIR := _tmp_build

HLS_CONFIGS := $(shell find dev_apps/$(TARGET_APP)/hw_hls -name '*.cfg' -not -path 'dev_apps/$(TARGET_APP)/hw_hls/autogen/*')
HLS_AUTOGEN_KERNELS := $(shell ./generate.py -c $(AUTOGEN_YAML) -d $(TARGET_APP) --list-autogen)
HLS_AUTOGEN_CONFIGS := $(foreach k,$(HLS_AUTOGEN_KERNELS), dev_apps/$(TARGET_APP)/hw_hls/autogen/$(k)/$(k).cfg)
HLS_AUTOGEN_SRCS := $(patsubst %.cfg,%.cpp,$(HLS_AUTOGEN_CONFIGS))
HLS_CONFIGS += $(HLS_AUTOGEN_CONFIGS)
SWITCH_NAME := $(shell ./generate.py  -c $(AUTOGEN_YAML) -d $(TARGET_APP) --switch-name)
SWITCH_XO := hw_hdl/$(SWITCH_NAME).xo

HLS_OBJECTS := $(patsubst %.cfg,%.xo,$(HLS_CONFIGS))
RTL_OBJECTS := $(wildcard $(XO_DIR)/*.xo) $(SWITCH_XO) 
VOBJECTS := $(RTL_OBJECTS) $(HLS_OBJECTS)

ifeq ($(TARGET), hw)
VPP_LDFLAGS += --xp param:compiler.userPostDebugProfileOverlayTcl=$(POSTSYSLINKTCL) --vivado.prop run.impl_1.STEPS.INIT_DESIGN.TCL.POST=$(CONSTRAINTSTCL)
else
VPP_CFLAGS += -g
endif

all: $(PRODUCTS)

hw: $(XCLBIN_NAME)

hw-req:
	@which $(VPP) > /dev/null 2>&1 || (echo "Missing $(VPP) compiler (forgot to source Vitis setup.sh?)"; exit 1)

sw-req:
	@[ ! -z "${XILINX_XRT}" ] || (echo "XILINX_XRT variable not defined (forgot to source xrt setup.sh?)"; exit 1)


.PHONY: all clean distclean prepare hw hw-req sw-req

$(XCLBIN_NAME): $(VOBJECTS) $(VPP_LINKER_CONFIG) | hw-req
	mkdir -p $(TMP_BUILD_DIR)
	$(VPP) $(VPP_FLAGS) $(VPP_CFLAGS) --link --config $(VPP_LINKER_CONFIG) $(VPP_LDFLAGS) --messageDb $(TMP_BUILD_DIR)/$(notdir $@).mdb -o $@ $(VOBJECTS)

emconfig.json:
	emconfigutil --platform $(PLATFORM) --od $(EMCONFIG_DIR)
	cp $(EMCONFIG_DIR)/$@ $@

%.xo: %.cpp %.cfg | hw-req
	mkdir -p $(TMP_BUILD_DIR)
	$(VPP) $(VPP_FLAGS) $(VPP_CFLAGS) --config $(@:.xo=.cfg) --messageDb $(TMP_BUILD_DIR)/$(notdir $@).mdb -o $@ $<

%: dev_apps/$(TARGET_APP)/host/%.cpp | sw-req
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

prepare $(HLS_AUTOGEN_CONFIGS) $(HLS_AUTOGEN_SRCS) $(VPP_LINKER_CONFIG) $(SWITCH_XO): dev_apps/$(TARGET_APP)/$(AUTOGEN_YAML)
	./generate.py -c $(AUTOGEN_YAML) -d $(TARGET_APP)
	./scripts/update_xo.sh $(SWITCH_XO) $(PLATFORM)

clean:
	$(RM) $(HLS_OBJECTS)
	$(RM) $(HLS_OBJECTS:=.compile_summary)
	$(RM) $(SWITCH_XO)
	$(RM) $(EMCONFIG_DIR)
	$(RM) .run
	$(RM) $(TMP_BUILD_DIR)
	$(RM) *.log
	$(RM) .Xil .ipcache
	$(RM) profile_kernels.csv timeline_kernels.csv *.protoinst *.wcfg *.wdb
	$(RM) dev_apps/$(TARGET_APP)/hw_hls/autogen
	$(RM) *.jou
	$(RM) *.csv
	$(RM) *.str
	$(RM) sample_link.ini
	$(RM) xrt.run_summary

distclean: clean
	$(RM) $(PRODUCTS)
	$(RM) $(XCLBIN_NAME).info $(XCLBIN_NAME).link_summary	$(XCLBIN_NAME:.xclbin=.ltx)
