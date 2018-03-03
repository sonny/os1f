OOCD-PATH := /opt/openocd-git
OPENOCD := $(OOCD-PATH)/bin/openocd
TERM   := xterm

# TARGETS -- command
gdb_server:
	@killall -q openocd || true # silent killer
	@($(OPENOCD) -f board/stm32f7discovery.cfg 2>&1) > openocd.log &


experimental-debug: $(ELF) | gdb_server
	$(GDB) -q -x gdb/dash-debug-hw.gdb $<
	@killall -v openocd


debug: $(ELF) | gdb_server
	$(GDB) -q -x gdb/debug-hw.gdb $<
	@killall -v openocd

xoocd:
	@($(KILL_OOCD)) || true
#       force openocd to log, so we can redirect semihosting output to it
	$(TERM) -e $(OPENOCD) -f board/stm32f7discovery.cfg -l openocd.log &


semi: $(ELF) | xoocd
	$(GDB) -q -x gdb/debug-hw.gdb $<
	@($(KILL_OOCD)) || true

load: $(ELF) | gdb_server
	$(GDB) -q -x gdb/load-hw.gdb $<

