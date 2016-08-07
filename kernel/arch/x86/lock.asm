; void acquire_spinlock(spinlock_t *s);
global acquire_spinlock
spin_wait:
	test dword [esp+4], 1
	jnz spin_wait
acquire_spinlock:
	lock bts dword [esp+4], 0
	jc spin_wait
	ret

; int release_spinlock(spinlock_t *s);
global release_spinlock
release_spinlock:
	mov eax, [esp+4]
	mov dword [esp+4], 0
	ret
