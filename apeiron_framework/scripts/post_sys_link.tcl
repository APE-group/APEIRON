# (c) Copyright 2019 Xilinx, Inc. All rights reserved.
#
# This file contains confidential and proprietary information
# of Xilinx, Inc. and is protected under U.S. and
# international copyright and other intellectual property
# laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any
# rights to the materials distributed herewith. Except as
# otherwise provided in a valid license issued to you by
# Xilinx, and to the maximum extent permitted by applicable
# law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
# WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
# AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
# BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
# INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
# (2) Xilinx shall not be liable (whether in contract or tort,
# including negligence, or under any other theory of
# liability) for any loss or damage of any kind or nature
# related to, arising under or in connection with these
# materials, including for any direct, or any indirect,
# special, incidental, or consequential loss or damage
# (including loss of data, profits, goodwill, or any type of
# loss or damage suffered as a result of any action brought
# by a third party) even if such damage or loss was
# reasonably foreseeable or Xilinx had been advised of the
# possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-
# safe, or for use in any application requiring fail-safe
# performance, such as life-support or safety devices or
# systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any
# other applications that could lead to death, personal
# injury, or severe property or environmental damage
# (individually and collectively, "Critical
# Applications"). Customer assumes the sole risk and
# liability of any use of Xilinx products in Critical
# Applications, subject only to applicable laws and
# regulations governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
# PART OF THIS FILE AT ALL TIMES.
############################################################
#

set __board [current_board]
set __TCLID "(Post-linking ${__board} QDMA QSFP0 Tcl hook): "

# default GT pin names
set __gt_interface "io_gt_gtyquad_00 io_gt_gtyquad_01"
set __gt0_refclk0 "io_clk_gtyquad_refclk0_00"
set __gt0_refclk1 "io_clk_gtyquad_refclk1_00"
set __gt1_refclk0 "io_clk_gtyquad_refclk0_01"
set __gt1_refclk1 "io_clk_gtyquad_refclk1_01"
set __gt_freerun "clk_gt_freerun"


# *************************************************************************
puts "${__TCLID}"

if {[dict exists ${hw_platform_info} hw_platform_vbnv]} {
  set __shell [dict get ${hw_platform_info} hw_platform_vbnv]
  puts "Platform VBNV is ${__shell} - checking if a rename of GT pins is needed"

  switch ${__shell} {
    "xilinx:u200:gen3x16_xdma_2:202110.1" -
    "xilinx:u200:gen3x16_xdma_1:202110.1" {
      puts "Changing GT pin names for platform ${__shell}"
      # this shell has completely different names
      set __gt_interface "io_gt_qsfp_00 io_gt_qsfp_01"
      set __gt0_refclk0 "io_clk_qsfp_refclka_00"
      set __gt0_refclk1 "io_clk_qsfp_refclkb_00"
      set __gt1_refclk0 "io_clk_qsfp_refclka_01"
      set __gt1_refclk1 "io_clk_qsfp_refclkb_01"
      set __gt_freerun "blp_s_aclk_freerun_ref_00"
    }
    "xilinx:u280:xdma:201920.3" {
      puts "Changing GT pin names for platform ${__shell}"
      # this shell has a freerun clock of 50MHz, use hbm one, which is 100MHz
      set __gt_freerun "hbm_ref_clk"
    }
    default {
      puts "Unknown platform, continuing with default pin names"
    }
  }
}


set __gt_k_list {}
set __gt_intf_width 0
# Make sure the kernel key in the config_info dict exists
if {[dict exists $config_info kernels]} {
  puts "${__TCLID} got config_info"
  set __k_list [dict get $config_info kernels]
  # Make sure that list of kernels is populated
  if {[llength $__k_list] > 0} {
    # Iterate over each kernel
    foreach __k_inst $__k_list {
      puts "${__TCLID} K Inst: ${__k_inst}"
      set __cu_bd_cell_list [get_bd_cells -quiet -filter "VLNV=~*:*:${__k_inst}:*"]
      # Iterate over each compute unit for the current kernel
      foreach __cu_bd_cell $__cu_bd_cell_list {
        puts "${__TCLID} CU Cell: ${__cu_bd_cell}"
        set __cu_bd_cell_sub [string range $__cu_bd_cell 1 [string length $__cu_bd_cell]]
        #Create a list of GT capable kernels.
        set __gt_pins [get_bd_intf_pins -quiet -of_objects [get_bd_cells $__cu_bd_cell_sub] -filter {VLNV=~*gt_rtl*}]
        if {[llength ${__gt_pins} ] > 0} {
          puts "${__TCLID} found gt interface on $__cu_bd_cell_sub"
          lappend __gt_k_list $__cu_bd_cell_sub
        }
      }
    }
  } else {
    puts "${__TCLID} kernel list 0"
  }


  if {[llength ${__gt_k_list}] > 1} {
    puts "${__TCLID} More than 1 GT interface is not supported. A single GT interface of max width 4 must be provided."
    exit
  }

  if {[llength $__gt_k_list] > 0} {
    puts "${__TCLID} Iterating over kernels"

    puts "${__TCLID} GT Kernel List $__gt_k_list"
    foreach __k_inst $__gt_k_list {
      set __gt_intf [get_bd_intf_pins -quiet -of_objects [get_bd_cells $__k_inst] -filter {VLNV=~*gt_rtl*}]

      if {[llength $__gt_intf] > [llength $__gt_interface]} {
        puts "${__TCLID} ${__k_inst} has more GT interfaces than available"
        exit
      }

      for {set i 0} {$i < [llength $__gt_intf]} {incr i} {
        puts "${__TCLID} connecting GT${i}"
        connect_bd_intf_net [get_bd_intf_ports [lindex ${__gt_interface} $i]] [lindex ${__gt_intf} $i]
      }

      # Raw connection to "sub-nets" of clock bd_intf_port (as kernel cannot leave a clock bd_intf_pin hanging after
      # System Linker due to validate_bd_design)
      puts "${__TCLID} TEMPORARY: Not connecting reference clock as diff clock due to post-System Linker validate error"
      set __gt0_refclk0_pins [get_bd_pins -of_objects [get_bd_cells ${__k_inst}] -filter {NAME =~ "gt0_refclk0*"}]
      if {[llength $__gt0_refclk0_pins] > 0} {
        puts "${__TCLID} connecting ref gt0 clk 0"
        connect_bd_net [get_bd_ports ${__gt0_refclk0}_clk_n] [get_bd_pins ${__k_inst}/gt0_refclk0_n]
        connect_bd_net [get_bd_ports ${__gt0_refclk0}_clk_p] [get_bd_pins ${__k_inst}/gt0_refclk0_p]
      }
      set __gt0_refclk1_pins [get_bd_pins -of_objects [get_bd_cells ${__k_inst}] -filter {NAME =~ "gt0_refclk1*"}]
      if {[llength $__gt0_refclk1_pins] > 0} {
        puts "${__TCLID} connecting ref gt0 clk 1"
        connect_bd_net [get_bd_ports ${__gt0_refclk1}_clk_n] [get_bd_pins ${__k_inst}/gt0_refclk1_n]
        connect_bd_net [get_bd_ports ${__gt0_refclk1}_clk_p] [get_bd_pins ${__k_inst}/gt0_refclk1_p]
      }

      set __gt1_refclk0_pins [get_bd_pins -of_objects [get_bd_cells ${__k_inst}] -filter {NAME =~ "gt1_refclk0*"}]
      if {[llength $__gt1_refclk0_pins] > 0} {
        puts "${__TCLID} connecting ref gt1 clk 0"
        connect_bd_net [get_bd_ports ${__gt1_refclk0}_clk_n] [get_bd_pins ${__k_inst}/gt1_refclk0_n]
        connect_bd_net [get_bd_ports ${__gt1_refclk0}_clk_p] [get_bd_pins ${__k_inst}/gt1_refclk0_p]
      }
      set __gt1_refclk1_pins [get_bd_pins -of_objects [get_bd_cells ${__k_inst}] -filter {NAME =~ "gt1_refclk1*"}]
      if {[llength $__gt1_refclk1_pins] > 0} {
        puts "${__TCLID} connecting ref gt1 clk 1"
        connect_bd_net [get_bd_ports ${__gt1_refclk1}_clk_n] [get_bd_pins ${__k_inst}/gt1_refclk1_n]
        connect_bd_net [get_bd_ports ${__gt1_refclk1}_clk_p] [get_bd_pins ${__k_inst}/gt1_refclk1_p]
      }

      set __freerunclk_pins [get_bd_pins -of_objects [get_bd_cells ${__k_inst}] -filter {NAME =~ "clk_gt_freerun"}]
      puts __freerunclk_pins
      if {[llength $__freerunclk_pins] ne 1} {
        puts "${__TCLID} ERROR - No clk_gt_freerun pin found"
        exit
      } else {
        connect_bd_net [get_bd_ports ${__gt_freerun}] [get_bd_pins ${__k_inst}/clk_gt_freerun]
      }
    }
  }
}
