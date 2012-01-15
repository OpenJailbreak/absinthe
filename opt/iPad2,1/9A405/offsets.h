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
	(void*)0x30278dc4,
	(void*)0x302696f4,
	(void*)0x3026998c,
	(void*)0x30278d68,
	(void*)0x3026aeb4,
	(void*)0x3026bc34,
	(void*)0x3026bca0,
	(void*)0x3026abf8,
	(void*)0x3026971c,
	(void*)0x30279394,
	(void*)0x3026b518,
	(void*)0x3026b4a8,
	(void*)0x30269658,
	(void*)0x30277d88,
	(void*)0x30269e44,
	(void*)0x30269e70,
	(void*)0x302691d4
};

// libdyld.dylib
struct libdyld_dylib_syms {
	void* _dlsym;
};
struct libdyld_dylib_syms libdyld_dylib = {
	(void*)0x308ad6ec
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
	(void*)0x376dc0e0,
	(void*)0x376dd004,
	(void*)0x376e3814,
	(void*)0x376dd65c,
	(void*)0x376df9d0,
	(void*)0x376d7ad0,
	(void*)0x376d4c64,
	(void*)0x376d4184,
	(void*)0x376edcf0,
	(void*)0x376eba54,
	(void*)0x376dc4e0,
	(void*)0x376eb814,
	(void*)0x376d9d30,
	(void*)0x376e580c
};

// liblaunch.dylib
struct liblaunch_dylib_syms {
	void* _bootstrap_look_up;
};
struct liblaunch_dylib_syms liblaunch_dylib = {
	(void*)0x378b1fe8
};

