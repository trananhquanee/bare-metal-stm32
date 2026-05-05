# Tên file đầu ra
TARGET = blink

# Cấu hình Vi xử lý
CPU = -mcpu=cortex-m3 -mthumb

# Công cụ biên dịch
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# Các cờ biên dịch
CFLAGS = $(CPU) -nostdlib -Wall -g -O2

# Tự động tìm tất cả file .c
SRCS = $(wildcard *.c)

# Chuyển .c thành .o
OBJS = $(SRCS:.c=.o)

# Lệnh mặc định
all: $(TARGET).bin

# Tạo file .bin
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Tạo file .elf từ tất cả các file .o
$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) -T stm32f103.ld -o $@ $^ -lgcc

# Quy tắc biên dịch file .c sang file .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Lệnh nạp chương trình
flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

# Lệnh dọn dẹp
clean:
	rm -f *.o $(TARGET).elf $(TARGET).bin

.PHONY: all clean flash
