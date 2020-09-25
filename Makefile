BUILD_DIR = build

all: stasis stasisd

clean:
	rm -rf $(BUILD_DIR)/*.o

$(BUILD_DIR)/%.o: ./%.c
	mkdir -p $(@D)
	gcc -c $< -o $@

C_FILES = $(wildcard ./*.c)
OBJ_FILES = $(C_FILES:./%.c=$(BUILD_DIR)/%.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

stasis: $(BUILD_DIR)/stasis.o $(BUILD_DIR)/data.o
	gcc -o $(BUILD_DIR)/stasis $(BUILD_DIR)/stasis.o $(BUILD_DIR)/data.o

stasisd: $(BUILD_DIR)/stasisd.o $(BUILD_DIR)/data.o
	gcc -o $(BUILD_DIR)/stasisd $(BUILD_DIR)/stasisd.o $(BUILD_DIR)/data.o
