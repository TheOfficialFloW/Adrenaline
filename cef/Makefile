all: _payloadex _rebootex _binary _systemctrl _popcorn _vshctrl _satelite _recovery bin2h

_payloadex:
	make -C payloadex clean
	make -C payloadex
	cp "payloadex/payloadex.bin" "flash0/payloadex.bin"
	make -C payloadex clean
	bin2c flash0/payloadex.bin ../user/files/payloadex.h payloadex

_rebootex:
	make -C rebootex clean
	make -C rebootex
	bin2c rebootex/rebootex.bin systemctrl/rebootex.h rebootex
	make -C rebootex clean

_binary:
	make -C binary clean
	make -C binary
	bin2c binary/binary.bin systemctrl/binary.h binary
	make -C binary clean

_systemctrl:
	make -C systemctrl clean
	make -C systemctrl
	psp-packer systemctrl/systemctrl.prx
	cp "systemctrl/systemctrl.prx" "flash0/kd/systemctrl.prx"
	make -C systemctrl clean
	bin2c flash0/kd/systemctrl.prx ../user/files/systemctrl.h systemctrl

_popcorn:
	make -C popcorn clean
	make -C popcorn
	psp-packer popcorn/popcorn.prx
	cp "popcorn/popcorn.prx" "flash0/kd/popcorn.prx"
	make -C popcorn clean
	bin2c flash0/kd/popcorn.prx ../user/files/popcorn.h popcorn

_vshctrl:
	make -C vshctrl clean
	make -C vshctrl
	psp-packer vshctrl/vshctrl.prx
	cp "vshctrl/vshctrl.prx" "flash0/kd/vshctrl.prx"
	make -C vshctrl clean
	bin2c flash0/kd/vshctrl.prx ../user/files/vshctrl.h vshctrl

_satelite:
	make -C satelite clean
	make -C satelite
	psp-packer satelite/satelite.prx
	cp "satelite/satelite.prx" "flash0/vsh/module/satelite.prx"
	make -C satelite clean
	bin2c flash0/vsh/module/satelite.prx ../user/files/satelite.h satelite
_recovery:
	make -C recovery clean
	make -C recovery
	psp-packer recovery/recovery.prx
	cp "recovery/recovery.prx" "flash0/vsh/module/recovery.prx"
	make -C recovery clean
	bin2c flash0/vsh/module/recovery.prx ../user/files/recovery.h recovery

bin2h:
	bin2c flash0/kd/pspbtjnf.bin ../user/files/pspbtjnf.h pspbtjnf
	bin2c flash0/kd/pspbtknf.bin ../user/files/pspbtknf.h pspbtknf
	bin2c flash0/kd/pspbtlnf.bin ../user/files/pspbtlnf.h pspbtlnf
	bin2c flash0/kd/pspbtmnf.bin ../user/files/pspbtmnf.h pspbtmnf
	bin2c flash0/kd/pspbtrnf.bin ../user/files/pspbtrnf.h pspbtrnf