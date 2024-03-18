connect -url tcp:127.0.0.1:3121
source C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.sdk/design_test_wrapper_hw_platform_0/ps7_init.tcl
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Digilent Zybo Z7 210351A77AD3A"} -index 0
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent Zybo Z7 210351A77AD3A" && level==0} -index 1
fpga -file C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.sdk/design_test_wrapper_hw_platform_0/design_test_wrapper.bit
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Digilent Zybo Z7 210351A77AD3A"} -index 0
loadhw -hw C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.sdk/design_test_wrapper_hw_platform_0/system.hdf -mem-ranges [list {0x40000000 0xbfffffff}]
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Digilent Zybo Z7 210351A77AD3A"} -index 0
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Digilent Zybo Z7 210351A77AD3A"} -index 0
dow C:/Users/matahir/Desktop/ECE-315-Lab/Lab_3/lab_3_hw/lab_3_hw.sdk/part_1/Debug/part_1.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Digilent Zybo Z7 210351A77AD3A"} -index 0
con
