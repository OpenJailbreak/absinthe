// liblaunch.dylib
struct liblaunch_dylib_syms {
	void* _bootstrap_look_up;
};
struct liblaunch_dylib_syms liblaunch_dylib = {
	(void*)0x314bffe8
};

// libsystem_c.dylib
struct libsystem_c_dylib_syms {
	void* _memcpy;
	void* _fopen;
	void* _fread;
	void* _fclose;
	void* _exit;
	void* _syslog;
	void* _sysctl;
	void* _malloc;
	void* _memmem;
	void* _sleep;
	void* _proc_pidinfo;
	void* _system;
	void* _strcpy;
	void* _sys_dcache_flush;
};
struct libsystem_c_dylib_syms libsystem_c_dylib = {
	(void*)0x32afe0e0,
	(void*)0x32aff004,
	(void*)0x32b05814,
	(void*)0x32aff65c,
	(void*)0x32b019d0,
	(void*)0x32af9ad0,
	(void*)0x32af6c64,
	(void*)0x32af6184,
	(void*)0x32b0fcf0,
	(void*)0x32b0da54,
	(void*)0x32afe4e0,
	(void*)0x32b0d814,
	(void*)0x32afbd30,
	(void*)0x32b0780c
};

// libdyld.dylib
struct libdyld_dylib_syms {
	void* _dlsym;
};
struct libdyld_dylib_syms libdyld_dylib = {
	(void*)0x333fd6ec
};

// libsystem_kernel.dylib
struct libsystem_kernel_dylib_syms {
	void* _open;
	void* _fstat;
	void* _memcpy;
	void* _munmap;
	void* _unlink;
	void* _mkdir;
	void* _ftruncate;
	void* _ioctl;
	void* _close;
	void* _ptrace;
	void* _chown;
	void* _shm_open;
	void* _mmap;
	void* _mach_task_self;
	void* _mach_port_allocate;
	void* _mach_port_insert_right;
	void* _mach_msg;
};
struct libsystem_kernel_dylib_syms libsystem_kernel_dylib = {
	(void*)0x35c72dc4,
	(void*)0x35c636f4,
	(void*)0x35c6398c,
	(void*)0x35c72d68,
	(void*)0x35c64eb4,
	(void*)0x35c65c34,
	(void*)0x35c65ca0,
	(void*)0x35c64bf8,
	(void*)0x35c6371c,
	(void*)0x35c73394,
	(void*)0x35c65518,
	(void*)0x35c654a8,
	(void*)0x35c63658,
	(void*)0x35c71d88,
	(void*)0x35c63e44,
	(void*)0x35c63e70,
	(void*)0x35c631d4
};

