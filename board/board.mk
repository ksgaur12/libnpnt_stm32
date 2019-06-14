# List of all the board related files.
BOARDSRC = $(BOARD_DIR)/board.c

# Required include directories
BOARDINC = $(BOARD_DIR)

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)