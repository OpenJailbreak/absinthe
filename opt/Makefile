SYMBOLS=_open _fstat _memcpy _munmap _unlink _mkdir _ftruncate _ioctl _close _ptrace _fopen _fread _fclose _exit _syslog _sysctl _malloc _memmem _sleep _bootstrap_look_up _bsdthread_terminate _chown _proc_pidinfo _system _strcpy _sys_dcache_flush _shm_open _mmap _mach_task_self _mach_port_allocate _mach_port_insert_right _mach_msg _dlsym

all:
	make -C ../tools
	@echo "// GreenPois0n Absinthe - offsets.h" >../include/offsets.h
	@echo "// Copyright (C) 2011 Chronic-Dev Team" >>../include/offsets.h
	@for i in `find . -name "dyld_shared_cache_armv7"`;do echo "Analyzing $$i...";../tools/dyldrop $$i -S $(SYMBOLS) >./`dirname $$i`/offsets.h;cd ../include;echo "#include \"`dirname ../opt/$$i`/offsets.h\"" >>../include/offsets.h;cd ../opt;done;
	
clean:
	@rm -rf ../include/offsets.h
	@for i in `find . -name "offsets.h"`;do echo "Removing $$i...";rm -rf $$i;done;

maintainer-clean:
	@rm -rf ../include/offsets.h
	@for i in `find . -name "offsets.h"`;do echo "Removing $$i...";rm -rf $$i;done;