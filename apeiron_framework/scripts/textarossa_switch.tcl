

set switch_inst [get_cells -hierarchical -filter {ORIG_REF_NAME == Textarossa_switch_synt}]

create_clock -period 6.200 -name gt0_refclk0 [get_pins -of_objects $switch_inst -filter {NAME =~ *gt0_refclk0_p}]
create_clock -period 6.200 -name gt1_refclk0 [get_pins -of_objects $switch_inst -filter {NAME =~ *gt1_refclk0_p}]
#create_clock -period 6.200 -name gt0_refclk0 [get_ports gt0_refclk0_p]
#create_clock -period 6.200 -name gt1_refclk0 [get_ports gt1_refclk0_p]

set_false_path -from [get_cells -hierarchical -filter "NAME =~ *TextaRossa*slv_reg_reg[4][0]*"]
set_false_path -from [get_cells -hierarchical -filter "NAME =~ *TextaRossa*soft_err_reg*"]
set_false_path -from [get_cells -hierarchical -filter "NAME =~ *TextaRossa*hard_err_reg*"]
set_false_path -from [get_cells -hierarchical -filter "NAME =~ *TextaRossa*channel_up_reg*"]

foreach sync_ip [get_cells -hier -filter {(ORIG_REF_NAME == data_sync_vec || REF_NAME == data_sync_vec)}] {
    puts "Setting constraints for data_sync_vec ${sync_ip}"
    set wr_path [get_cells -hierarchical -filter "PARENT == $sync_ip && NAME =~ *reg_data_wr_reg[*]"]
    set rd_path [get_cells -hierarchical -filter "PARENT == $sync_ip && NAME =~ *reg_data_rd_reg[*]"]
    puts "wr_path: ${wr_path}"
    puts "rd_path: ${rd_path}"
    if {[llength $wr_path] == 0 || [llength $rd_path] == 0} {
        puts "Missing register, skipping this instance"
        continue
    }
    set_false_path -from $wr_path -to $rd_path
}
foreach sync_ip [get_cells -hier -filter {(ORIG_REF_NAME == data_sync || REF_NAME == data_sync)}] {
    puts "Setting constraints for data_sync ${sync_ip}"
    set wr_path [get_cells -hierarchical -filter "PARENT == $sync_ip && NAME =~ *reg_data_wr_reg"]
    set rd_path [get_cells -hierarchical -filter "PARENT == $sync_ip && NAME =~ *reg_data_tmp_reg"]
    puts "wr_path: ${wr_path}"
    puts "rd_path: ${rd_path}"
    if {[llength $wr_path] == 0 || [llength $rd_path] == 0} {
        puts "Missing register, skipping this instance"
        continue
    }
    set_false_path -from $wr_path -to $rd_path
}
