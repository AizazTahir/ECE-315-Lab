# 
# Synthesis run script generated by Vivado
# 

set TIME_start [clock seconds] 
proc create_report { reportName command } {
  set status "."
  append status $reportName ".fail"
  if { [file exists $status] } {
    eval file delete [glob $status]
  }
  send_msg_id runtcl-4 info "Executing : $command"
  set retval [eval catch { $command } msg]
  if { $retval != 0 } {
    set fp [open $status w]
    close $fp
    send_msg_id runtcl-5 warning "$msg"
  }
}
create_project -in_memory -part xc7z010clg400-1

set_param project.singleFileAddWarning.threshold 0
set_param project.compositeFile.enableAutoGeneration 0
set_param synth.vivado.isSynthRun true
set_msg_config -source 4 -id {IP_Flow 19-2162} -severity warning -new_severity info
set_property webtalk.parent_dir C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.cache/wt [current_project]
set_property parent.project_path C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.xpr [current_project]
set_property XPM_LIBRARIES {XPM_CDC XPM_FIFO XPM_MEMORY} [current_project]
set_property default_lib xil_defaultlib [current_project]
set_property target_language VHDL [current_project]
set_property board_part_repo_paths {C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.board} [current_project]
set_property board_part digilentinc.com:zybo-z7-10:part0:1.0 [current_project]
set_property ip_repo_paths c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.ipdefs/vivado-library-v2019.1-1_0_0_0_0_0_0 [current_project]
update_ip_catalog
set_property ip_output_repo c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.cache/ip [current_project]
set_property ip_cache_permissions {read write} [current_project]
read_vhdl -library xil_defaultlib C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/hdl/design_test_wrapper.vhd
add_files C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/design_test.bd
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_processing_system7_0_0/design_test_processing_system7_0_0.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_rst_ps7_0_50M_0/design_test_rst_ps7_0_50M_0_board.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_rst_ps7_0_50M_0/design_test_rst_ps7_0_50M_0.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_rst_ps7_0_50M_0/design_test_rst_ps7_0_50M_0_ooc.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_xbar_0/design_test_xbar_0_ooc.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/design_test_PmodOLED_0_0_board.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/src/PmodOLED_ooc.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/src/PmodOLED_pmod_bridge_0_0/PmodOLED_pmod_bridge_0_0_board.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/src/PmodOLED_pmod_bridge_0_0/src/pmod_concat_ooc.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/src/PmodOLED_axi_quad_spi_0_0/PmodOLED_axi_quad_spi_0_0_board.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/src/PmodOLED_axi_quad_spi_0_0/PmodOLED_axi_quad_spi_0_0.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/src/PmodOLED_axi_quad_spi_0_0/PmodOLED_axi_quad_spi_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/src/PmodOLED_axi_quad_spi_0_0/PmodOLED_axi_quad_spi_0_0_clocks.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/src/PmodOLED_axi_gpio_0_0/PmodOLED_axi_gpio_0_0_board.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/src/PmodOLED_axi_gpio_0_0/PmodOLED_axi_gpio_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_PmodOLED_0_0/src/PmodOLED_axi_gpio_0_0/PmodOLED_axi_gpio_0_0.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_axi_gpio_0_1/design_test_axi_gpio_0_1_board.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_axi_gpio_0_1/design_test_axi_gpio_0_1_ooc.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_axi_gpio_0_1/design_test_axi_gpio_0_1.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_axi_gpio_0_2/design_test_axi_gpio_0_2_board.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_axi_gpio_0_2/design_test_axi_gpio_0_2_ooc.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_axi_gpio_0_2/design_test_axi_gpio_0_2.xdc]
set_property used_in_implementation false [get_files -all c:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/ip/design_test_auto_pc_0/design_test_auto_pc_0_ooc.xdc]
set_property used_in_implementation false [get_files -all C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/sources_1/bd/design_test/design_test_ooc.xdc]

# Mark all dcp files as not used in implementation to prevent them from being
# stitched into the results of this synthesis run. Any black boxes in the
# design are intentionally left as such for best results. Dcp files will be
# stitched into the design at a later time, either when this synthesis run is
# opened, or when it is stitched into a dependent implementation run.
foreach dcp [get_files -quiet -all -filter file_type=="Design\ Checkpoint"] {
  set_property used_in_implementation false $dcp
}
read_xdc C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/constrs_1/new/lab_3_const.xdc
set_property used_in_implementation false [get_files C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.srcs/constrs_1/new/lab_3_const.xdc]

read_xdc dont_touch.xdc
set_property used_in_implementation false [get_files dont_touch.xdc]
set_param ips.enableIPCacheLiteLoad 1
close [open __synthesis_is_running__ w]

synth_design -top design_test_wrapper -part xc7z010clg400-1


# disable binary constraint mode for synth run checkpoints
set_param constraints.enableBinaryConstraints false
write_checkpoint -force -noxdef design_test_wrapper.dcp
create_report "synth_1_synth_report_utilization_0" "report_utilization -file design_test_wrapper_utilization_synth.rpt -pb design_test_wrapper_utilization_synth.pb"
file delete __synthesis_is_running__
close [open __synthesis_is_complete__ w]
