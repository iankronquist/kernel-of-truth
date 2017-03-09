.text

.global interrupts_disable
.type interrupts_disable, @function
interrupts_disable:
	cli
	ret

.global interrupts_enable
.type interrupts_enable, @function
interrupts_enable:
	sti
	ret
