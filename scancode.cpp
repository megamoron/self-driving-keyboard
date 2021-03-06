#include <Windows.h>

#include "scancode.h"

WORD vk2scan(WORD vk, BOOL press) {
	switch (vk) {
	case 0x8: return press ? 0xe : 0x8e;
	case 0x9: return press ? 0xf : 0x8f;
	case 0xc: return press ? 0x4c : 0xcc;
	case 0xd: return press ? 0x1c : 0x9c;
	case 0x10: return press ? 0x2a : 0xaa; // left shift
//	case 0x10: return press ? 0x36 : 0xb6; // right shift
	case 0x11: return press ? 0x1d : 0x9d;
//	case 0x11: return press ? 0xe01d : 0xe09d; // right control
	case 0x12: return press ? 0x38 : 0xb8;
//	case 0x12: return press ? 0xe038 : 0xe0b8; // right alt
	case 0x13: return press ? 0x1d : 0x9d;
	case 0x14: return press ? 0x3a : 0xba;
	case 0x1b: return press ? 0x1 : 0x81;
	case 0x20: return press ? 0x39 : 0xb9;
//	case 0x21: return press ? 0x49 : 0xc9; // numpad page up
	case 0x21: return press ? 0xe049 : 0xe0c9;
//	case 0x22: return press ? 0x51 : 0xd1; // numpad page down
	case 0x22: return press ? 0xe051 : 0xe0d1;
//	case 0x23: return press ? 0x4f : 0xcf; // numpad end
	case 0x23: return press ? 0xe04f : 0xe0cf;
//	case 0x24: return press ? 0x47 : 0xc7; // numpad home
	case 0x24: return press ? 0xe047 : 0xe0c7;
//	case 0x25: return press ? 0x4b : 0xcb; // numpad left arrow
	case 0x25: return press ? 0xe04b : 0xe0cb;
//	case 0x26: return press ? 0x48 : 0xc8; // numpad up arrow
	case 0x26: return press ? 0xe048 : 0xe0c8;
//	case 0x27: return press ? 0x4d : 0xcd; // numpad right arrow
	case 0x27: return press ? 0xe04d : 0xe0cd;
//	case 0x28: return press ? 0x50 : 0xd0; // numpad down arrow
	case 0x28: return press ? 0xe050 : 0xe0d0;
	case 0x2c: return press ? 0xe037 : 0xe0b7;
//	case 0x2d: return press ? 0x52 : 0xd2; // numpad insert
	case 0x2d: return press ? 0xe052 : 0xe0d2;
//	case 0x2e: return press ? 0x53 : 0xd3; // numpad delete
	case 0x2e: return press ? 0xe053 : 0xe0d3;
	case 0x30: return press ? 0xb : 0x8b;
	case 0x31: return press ? 0x2 : 0x82;
	case 0x32: return press ? 0x3 : 0x83;
	case 0x33: return press ? 0x4 : 0x84;
	case 0x34: return press ? 0x5 : 0x85;
	case 0x35: return press ? 0x6 : 0x86;
	case 0x36: return press ? 0x7 : 0x87;
	case 0x37: return press ? 0x8 : 0x88;
	case 0x38: return press ? 0x9 : 0x89;
	case 0x39: return press ? 0xa : 0x8a;
	case 0x41: return press ? 0x1e : 0x9e;
	case 0x42: return press ? 0x30 : 0xb0;
	case 0x43: return press ? 0x2e : 0xae;
	case 0x44: return press ? 0x20 : 0xa0;
	case 0x45: return press ? 0x12 : 0x92;
	case 0x46: return press ? 0x21 : 0xa1;
	case 0x47: return press ? 0x22 : 0xa2;
	case 0x48: return press ? 0x23 : 0xa3;
	case 0x49: return press ? 0x17 : 0x97;
	case 0x4a: return press ? 0x24 : 0xa4;
	case 0x4b: return press ? 0x25 : 0xa5;
	case 0x4c: return press ? 0x26 : 0xa6;
	case 0x4d: return press ? 0x32 : 0xb2;
	case 0x4e: return press ? 0x31 : 0xb1;
	case 0x4f: return press ? 0x18 : 0x98;
	case 0x50: return press ? 0x19 : 0x99;
	case 0x51: return press ? 0x10 : 0x90;
	case 0x52: return press ? 0x13 : 0x93;
	case 0x53: return press ? 0x1f : 0x9f;
	case 0x54: return press ? 0x14 : 0x94;
	case 0x55: return press ? 0x16 : 0x96;
	case 0x56: return press ? 0x2f : 0xaf;
	case 0x57: return press ? 0x11 : 0x91;
	case 0x58: return press ? 0x2d : 0xad;
	case 0x59: return press ? 0x15 : 0x95;
	case 0x5a: return press ? 0x2c : 0xac;
	case 0x5b: return press ? 0xe05b : 0xe0db;
	case 0x5c: return press ? 0xe05c : 0xe0dc;
	case 0x5d: return press ? 0xe05d : 0xe0dd;
	case 0x60: return press ? 0x52 : 0xd2;
	case 0x61: return press ? 0x4f : 0xcf;
	case 0x62: return press ? 0x50 : 0xd0;
	case 0x63: return press ? 0x51 : 0xd1;
	case 0x64: return press ? 0x4b : 0xcb;
	case 0x65: return press ? 0x4c : 0xcc;
	case 0x66: return press ? 0x4d : 0xcd;
	case 0x67: return press ? 0x47 : 0xc7;
	case 0x68: return press ? 0x48 : 0xc8;
	case 0x69: return press ? 0x49 : 0xc9;
	case 0x6a: return press ? 0x37 : 0xb7;
	case 0x6b: return press ? 0x4e : 0xce;
	case 0x6d: return press ? 0x4a : 0xca;
	case 0x6e: return press ? 0x53 : 0xd3;
	case 0x6f: return press ? 0xe035 : 0xe0b5;
	case 0x70: return press ? 0x3b : 0xbb;
	case 0x71: return press ? 0x3c : 0xbc;
	case 0x72: return press ? 0x3d : 0xbd;
	case 0x73: return press ? 0x3e : 0xbe;
	case 0x74: return press ? 0x3f : 0xbf;
	case 0x75: return press ? 0x40 : 0xc0;
	case 0x76: return press ? 0x41 : 0xc1;
	case 0x77: return press ? 0x42 : 0xc2;
	case 0x78: return press ? 0x43 : 0xc3;
	case 0x79: return press ? 0x44 : 0xc4;
	case 0x7a: return press ? 0x57 : 0xd7;
	case 0x7b: return press ? 0x58 : 0xd8;
	case 0x90: return press ? 0x45 : 0xc5;
	case 0x91: return press ? 0x46 : 0xc6;
	case 0xba: return press ? 0x27 : 0xa7;
	case 0xbb: return press ? 0xd : 0x8d;
	case 0xbc: return press ? 0x33 : 0xb3;
	case 0xbd: return press ? 0xc : 0x8c;
	case 0xbe: return press ? 0x34 : 0xb4;
	case 0xbf: return press ? 0x35 : 0xb5;
	case 0xc0: return press ? 0x29 : 0xa9;
	case 0xdb: return press ? 0x1a : 0x9a;
	case 0xdc: return press ? 0x2b : 0xab;
	case 0xdd: return press ? 0x1b : 0x9b;
	case 0xde: return press ? 0x28 : 0xa8;
	case 0xe2: return press ? 0x56 : 0xd6;
	default: return 0;
	}
}