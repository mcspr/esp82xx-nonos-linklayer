.PHONY: .mkdir

OBJ := $(patsubst %.c,$(BUILD)/%.o,$(SRC))

$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD_ROOT)/user_config.h:
	@touch $@

$(OBJ): $(BUILD)/%.o: %.c
	$(CC) \
        -c \
        $(BUILD_FLAGS) \
        $(BUILD_DEFINES) \
        $(BUILD_INCLUDES) \
        $< -o $@

DIRS := $(sort $(dir $(OBJ)))
$(DIRS): $(SRC)
	mkdir -p $@

.mkdir: $(DIRS)

$(OBJ): .mkdir
all: $(BUILD) $(OBJ)
