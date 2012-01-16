// liblaunch.dylib
struct liblaunch_dylib_syms {
	void* _bootstrap_look_up;
};
struct liblaunch_dylib_syms liblaunch_dylib = {
	(void*)0x31d27fe8
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
	(void*)0x332cfdc4,
	(void*)0x332c06f4,
	(void*)0x332c098c,
	(void*)0x332cfd68,
	(void*)0x332c1eb4,
	(void*)0x332c2c34,
	(void*)0x332c2ca0,
	(void*)0x332c1bf8,
	(void*)0x332c071c,
	(void*)0x332d0394,
	(void*)0x332c2518,
	(void*)0x332c24a8,
	(void*)0x332c0658,
	(void*)0x332ced88,
	(void*)0x332c0e44,
	(void*)0x332c0e70,
	(void*)0x332c01d4
};

// libdyld.dylib
struct libdyld_dylib_syms {
	void* _dlsym;
};
struct libdyld_dylib_syms libdyld_dylib = {
	(void*)0x358116ec
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
	(void*)0x35e5e0e0,
	(void*)0x35e5f004,
	(void*)0x35e65814,
	(void*)0x35e5f65c,
	(void*)0x35e619d0,
	(void*)0x35e59ad0,
	(void*)0x35e56c64,
	(void*)0x35e56184,
	(void*)0x35e6fcf0,
	(void*)0x35e6da54,
	(void*)0x35e5e4e0,
	(void*)0x35e6d814,
	(void*)0x35e5bd30,
	(void*)0x35e6780c
};

