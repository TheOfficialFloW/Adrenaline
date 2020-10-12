all: _libpspsystemctrl_kernel _libpspsystemctrl_user _payloadex _rebootex _binary _systemctrl _popcorn _inferno _vshctrl _satelite _recovery

_payloadex:
	make -C payloadex clean
	make -C payloadex
	mkdir -p "../user/flash0" && cp "payloadex/payloadex.bin" "../user/flash0/payloadex.bin"

_rebootex:
	make -C rebootex clean
	make -C rebootex
	bin2c rebootex/rebootex.bin systemctrl/rebootex.h rebootex

_binary:
	make -C binary clean
	make -C binary
	bin2c binary/binary.bin systemctrl/binary.h binary

_systemctrl:
	make -C systemctrl clean
	make -C systemctrl release
	psp-packer systemctrl/systemctrl.prx
	mkdir -p "../user/flash0/kd" && cp "systemctrl/systemctrl.prx" "../user/flash0/kd/systemctrl.prx"

_popcorn:
	make -C popcorn clean
	make -C popcorn
	psp-packer popcorn/popcorn.prx
	mkdir -p "../user/flash0/kd" && cp "popcorn/popcorn.prx" "../user/flash0/kd/popcorn.prx"

_inferno:
	make -C inferno clean
	make -C inferno
	psp-packer inferno/inferno.prx
	mkdir -p "../user/flash0/kd" && cp "inferno/inferno.prx" "../user/flash0/kd/inferno.prx"

_vshctrl:
	make -C vshctrl clean
	make -C vshctrl
	psp-packer vshctrl/vshctrl.prx
	mkdir -p "../user/flash0/kd" && cp "vshctrl/vshctrl.prx" "../user/flash0/kd/vshctrl.prx"

_satelite:
	make -C satelite clean
	make -C satelite
	psp-packer satelite/satelite.prx
	mkdir -p "../user/flash0/vsh/module" && cp "satelite/satelite.prx" "../user/flash0/vsh/module/satelite.prx"

_recovery:
	make -C recovery clean
	make -C recovery
	psp-packer recovery/recovery.prx
	mkdir -p "../user/flash0/vsh/module" && cp "recovery/recovery.prx" "../user/flash0/vsh/module/recovery.prx"

_libpspsystemctrl_kernel:
	make -C lib/libpspsystemctrl_kernel clean
	make -C lib/libpspsystemctrl_kernel
	cp lib/libpspsystemctrl_kernel/libpspsystemctrl_kernel.a lib/

_libpspsystemctrl_user:
	make -C lib/libpspsystemctrl_user clean
	make -C lib/libpspsystemctrl_user
	cp lib/libpspsystemctrl_user/libpspsystemctrl_user.a lib/

clean:
	make -C payloadex clean
	make -C rebootex clean
	make -C binary clean
	make -C systemctrl clean
	make -C popcorn clean
	make -C vshctrl clean
	make -C satelite clean
	make -C recovery clean
	make -C inferno clean
	make -C lib/libpspsystemctrl_kernel clean
	make -C lib/libpspsystemctrl_user clean