PSPSDK=$(shell psp-config --pspsdk-path)
OBJS = SystemCtrlForUser_0000.o SystemCtrlForUser_0001.o SystemCtrlForUser_0002.o SystemCtrlForUser_0003.o SystemCtrlForUser_0004.o SystemCtrlForUser_0005.o SystemCtrlForUser_0006.o SystemCtrlForUser_0007.o SystemCtrlForUser_0008.o SystemCtrlForUser_0009.o SystemCtrlForUser_0010.o SystemCtrlForUser_0011.o SystemCtrlForUser_0012.o SystemCtrlForUser_0013.o SystemCtrlForUser_0014.o SystemCtrlForUser_0015.o SystemCtrlForUser_0016.o SystemCtrlForUser_0017.o SystemCtrlForUser_0018.o SystemCtrlForUser_0019.o SystemCtrlForUser_0020.o SystemCtrlForUser_0021.o SystemCtrlForUser_0022.o SystemCtrlForUser_0023.o SystemCtrlForUser_0024.o SystemCtrlForUser_0025.o SystemCtrlForUser_0026.o SystemCtrlForUser_0027.o 

%.o: SystemCtrlForUser.S
	psp-gcc -g -O2 -G0 -Wall -I${PSPSDK}/include -DF_$* $< -c -o $@

all: $(OBJS)
	psp-ar cru libpspsystemctrl_user.a $(OBJS)
	psp-ranlib libpspsystemctrl_user.a

clean:
	rm -f *.o
	rm -f *.a