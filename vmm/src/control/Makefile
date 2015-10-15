obj :=	ctrl.o		\
	ctrl_evt.o	\
	dbg.o		\
	dbg_evt.o	\
	dbg_soft.o	\
	dbg_hard.o	\
	dbg_hard_brk.o	\
	dbg_hard_stp.o

ifeq ($(CONFIG_GDBSTUB),yes)
obj +=	gdbstub.o	\
	gdbstub_pkt.o	\
	gdbstub_reg.o	\
	gdbstub_cmd.o	\
	gdbstub_vmm.o	\
	gdbstub_evt.o
else
obj +=	local_evt.o
endif
