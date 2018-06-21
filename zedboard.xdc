# zedboard configuration for PMOD RS485 (only)
# configure the PMOD RS485 device to work on PMOD ports JA1 (top row)
set_property PACKAGE_PIN AA11 [get_ports pmod_rs485_TXD]
set_property PACKAGE_PIN Y10 [get_ports pmod_rs485_RXD]
set_property IOSTANDARD LVCMOS33 [get_ports pmod_rs485_RXD]
set_property IOSTANDARD LVCMOS33 [get_ports pmod_rs485_TXD]

set_property SLEW FAST [get_ports pmod_rs485_TXD]




set_property PULLUP true [get_ports pmod_rs485_RXD]

set_property PULLUP true [get_ports pmod_rs485_TXD]
