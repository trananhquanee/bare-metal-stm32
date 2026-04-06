TARGET = blink
CPU = -mcpu=cortex-m3 -mthumb

all:
	arm-none-eabi-gcc $(CPU) -nostdlib -T stm32f103.ld \
		-o $(TARGET).elf startup.c main.c
	arm-none-eabi-objcopy -O binary $(TARGET).elf $(TARGET).bin

flash:
	st-flash write $(TARGET).bin 0x08000000

clean:
	rm -f $(TARGET).elf $(TARGET).bin
