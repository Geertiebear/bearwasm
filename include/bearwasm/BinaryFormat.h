#ifndef BEARWASM_BINARYFORMAT_H
#define BEARWASM_BINARYFORMAT_H

#include <stdint.h>
#include <bearwasm/host.hpp>
#include <frg/hash.hpp>
#include <frg/hash_map.hpp>

namespace bearwasm {

static constexpr int PAGE_SIZE = 65536;

enum Sections : uint8_t {
	SECTION_CUSTOM = 0,
	SECTION_TYPE,
	SECTION_IMPORT,
	SECTION_FUNCTION,
	SECTION_TABLE,
	SECTION_MEMORY,
	SECTION_GLOBAL,
	SECTION_EXPORT,
	SECTION_START,
	SECTION_ELEMENT,
	SECTION_CODE,
	SECTION_DATA,
};

enum BinaryType : uint8_t {
	EMPTY = 0x40,
	I_32 = 0x7F,
	I_64 = 0x7E,
	F_32 = 0x7D,
	F_64 = 0x7C,
};

enum TableType : uint8_t {
	TABLE_FUNCREF = 0x70,
};

enum Instructions : uint8_t {
	INSTR_UNREACHABLE = 0x0,
	INSTR_NOP = 0x01,
	INSTR_BLOCK = 0x02,
	INSTR_LOOP = 0x03,
	INSTR_IF = 0x04,
	INSTR_END = 0x0B,
	BR = 0xC,
	BR_IF = 0xD,
	INSTR_RETURN = 0xF,
	INSTR_CALL = 0x10,
	INSTR_DROP = 0x1A,
	INSTR_SELECT = 0x1B,
	LOCAL_GET = 0x20,
	LOCAL_SET = 0x21,
	LOCAL_TEE = 0x22,
	GLOBAL_GET = 0x23,
	GLOBAL_SET = 0x24,
	I_32_LOAD = 0x28,
	I_32_LOAD_8_S = 0x2C,
	I_32_LOAD_8_U = 0x2D,
	I_32_STORE = 0x36,
	I_32_CONST = 0x41,
	I_64_CONST = 0x42,
	F_32_CONST = 0x43,
	F_64_CONST = 0x44,
	I_32_EQZ = 0x45,
	I_32_EQ = 0x46,
	I_32_NE = 0x47,
	I_32_LT_S = 0x48,
	I_32_LT_U = 0x49,
	I_32_GT_S = 0x4A,
	I_32_GT_U = 0x4B,
	I_32_LE_S = 0x4C,
	I_32_LE_U = 0x4D,
	I_32_ADD = 0x6A,
	I_32_SUB = 0x6B,
	I_32_MUL = 0x6C,
	I_32_DIV_S = 0x6D,
	I_32_REM_S = 0x6F,
	I_32_AND = 0x71,
	I_32_OR = 0x72,
	I_32_SHL = 0x74,
	I_32_SHR_S = 0x75,
	I_64_DIV_U = 0x80,
};

enum ExportType : uint8_t {
	EXPORT_FUNC = 0,
	EXPORT_TABLE,
	EXPORT_MEM,
	EXPORT_GLOBAL,
};

enum InstructionArgSize {
	SIZE_BLOCK,
	SIZE_U8,
	SIZE_I32,
	SIZE_I64,
	SIZE_U32,
	SIZE_U64,
	SIZE_F32,
	SIZE_F64,
	SIZE_0,
	SIZE_MEMARG,
};

static const frg::hash_map<Instructions, InstructionArgSize,
	     frg::hash<int>, frg_allocator> instruction_sizes {
	frg::hash<int>{}, {
	{INSTR_BLOCK, SIZE_BLOCK},
	{INSTR_LOOP, SIZE_BLOCK},
	{INSTR_IF, SIZE_BLOCK},
	{I_32_CONST, SIZE_I32},
	{I_64_CONST, SIZE_I64},
	{F_32_CONST, SIZE_F32},
	{F_64_CONST, SIZE_F64},
	{I_64_DIV_U, SIZE_0},
	{INSTR_UNREACHABLE, SIZE_0},
	{INSTR_NOP, SIZE_0},
	{INSTR_CALL, SIZE_U32},
	{LOCAL_GET, SIZE_U32},
	{LOCAL_SET, SIZE_U32},
	{GLOBAL_GET, SIZE_U32},
	{I_32_SUB, SIZE_0},
	{I_32_STORE, SIZE_MEMARG},
	{I_32_LOAD, SIZE_MEMARG},
	{I_32_LOAD_8_U, SIZE_MEMARG},
	{I_32_SHL, SIZE_0},
	{I_32_SHR_S, SIZE_0},
	{I_32_ADD, SIZE_0},
	{INSTR_RETURN, SIZE_0},
	{GLOBAL_SET, SIZE_U32},
	{I_32_LOAD_8_S, SIZE_MEMARG},
	{LOCAL_TEE, SIZE_U32},
	{I_32_LT_S, SIZE_0},
	{BR_IF, SIZE_U32},
	{I_32_MUL, SIZE_0},
	{I_32_GT_S, SIZE_0},
	{INSTR_SELECT, SIZE_0},
	{I_32_AND, SIZE_0},
	{I_32_EQZ, SIZE_0},
	{BR, SIZE_U32},
	{I_32_NE, SIZE_0},
	{INSTR_DROP, SIZE_0},
	{I_32_REM_S, SIZE_0},
	{I_32_DIV_S, SIZE_0},
	{I_32_LE_S, SIZE_0},
	{I_32_GT_U, SIZE_0},
	{I_32_EQ, SIZE_0},
	{I_32_LE_U, SIZE_0},
	{I_32_LT_U, SIZE_0},
	{I_32_OR, SIZE_0},}
};

} /* namespace bearwasm */

#endif
