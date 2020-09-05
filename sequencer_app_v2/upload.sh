avrdude -P COM3 -p m328p -c stk500 -U lfuse:w:0xFF:m -U hfuse:w:0xDE:m -U efuse:w:0x05:m
avrdude -P COM3 -p m328p -c stk500 -U flash:w:build/firmware.hex
