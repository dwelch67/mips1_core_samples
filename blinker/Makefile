
MIPSGNU ?= mips-none-elf


COPS = -EL -march=mips1 -G0 -Wall -O2 -nostdlib -nostartfiles -ffreestanding

AOPS = -EL -G0 --warn --fatal-warnings

LDOPS = -EL -G0

#-relocation-model                   - Choose relocation model
#    =default                          -   Target default relocation model
#    =static                           -   Non-relocatable code
#    =pic                              -   Fully relocatable, position independent code
#    =dynamic-no-pic                   -   Relocatable external ref

#  4ke      - Select the 4ke processor.
#  allegrex - Select the allegrex processor.
#  mips1    - Select the mips1 processor.
#  mips2    - Select the mips2 processor.
#  r2000    - Select the r2000 processor.
#  r3000    - Select the r3000 processor.
#  r6000    - Select the r6000 processor.


all : blinker03.hex


clean :
	rm -f *.hex
	rm -f *.o
	rm -f *.elf
	rm -f *.list
	rm -f *.bc
	rm -f *.norm.s
	rm -f *.opt.s
	rm -f ihex

ihex : ihex.c
	gcc ihex.c -o ihex

startup.o : startup.s
	$(MIPSGNU)-as $(AOPS) startup.s -o startup.o

blinker03.o : blinker03.c
	$(MIPSGNU)-gcc $(COPS) -c blinker03.c -o blinker03.o

blinker03.hex : memmap startup.o blinker03.o
	$(MIPSGNU)-ld $(LDOPS) -T memmap startup.o blinker03.o -o blinker03.elf
	$(MIPSGNU)-objdump -D blinker03.elf > blinker03.list
	$(MIPSGNU)-objcopy blinker03.elf -O ihex blinker03.hex
	
blinker03.hex.hex : blinker03.hex ihex
	./ihex blinker03.hex


