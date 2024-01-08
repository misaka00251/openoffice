# mk file for Unix Linux HPPA using GCC, please make generic modifications to unxlng.mk
PICSWITCH:=-fPIC
.INCLUDE : unxlng.mk
CDEFS+=-DRISCV64
CFLAGS+=
CFLAGSCC+=
CFLAGSCXX+=
DLLPOSTFIX=
BUILD64=1