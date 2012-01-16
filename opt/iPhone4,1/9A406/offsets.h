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
	(void*)0x30402dc4,
	(void*)0x303f36f4,
	(void*)0x303f398c,
	(void*)0x30402d68,
	(void*)0x303f4eb4,
	(void*)0x303f5c34,
	(void*)0x303f5ca0,
	(void*)0x303f4bf8,
	(void*)0x303f371c,
	(void*)0x30403394,
	(void*)0x303f5518,
	(void*)0x303f54a8,
	(void*)0x303f3658,
	(void*)0x30401d88,
	(void*)0x303f3e44,
	(void*)0x303f3e70,
	(void*)0x303f31d4
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
	(void*)0x319df0e0,
	(void*)0x319e0004,
	(void*)0x319e6814,
	(void*)0x319e065c,
	(void*)0x319e29d0,
	(void*)0x319daad0,
	(void*)0x319d7c64,
	(void*)0x319d7184,
	(void*)0x319f0cf0,
	(void*)0x319eea54,
	(void*)0x319df4e0,
	(void*)0x319ee814,
	(void*)0x319dcd30,
	(void*)0x319e880c
};

// liblaunch.dylib
struct liblaunch_dylib_syms {
	void* _bootstrap_look_up;
};
struct liblaunch_dylib_syms liblaunch_dylib = {
	(void*)0x36c35fe8
};

// libdyld.dylib
struct libdyld_dylib_syms {
	void* _dlsym;
};
struct libdyld_dylib_syms libdyld_dylib = {
	(void*)0x376f36ec
};

