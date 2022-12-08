#include "pch.h"
#include <cstdio>
#include <cstdint>
#include <Psapi.h>

struct TrapFunc
{
	void(*Func)(intptr_t* args);
	intptr_t Flags;
};

const struct ModuleInfo
{
	const char* startAddr;
	const char* endAddr;

	ModuleInfo()
	{
		startAddr = (const char*)GetModuleHandle(NULL);
		MODULEINFO moduleInfo;
		GetModuleInformation(GetCurrentProcess(), (HMODULE)startAddr, &moduleInfo, sizeof(MODULEINFO));
		endAddr = startAddr + moduleInfo.SizeOfImage;
	}
} moduleInfo;
HANDLE _hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

template <typename T>
T Hook(const char* pattern, const char* patvalid)
{
	size_t patlen = strlen(patvalid);
	for (const char* addr = moduleInfo.startAddr; addr < moduleInfo.endAddr - 0x10; addr += 0x10)
	{

		size_t i = 0;
		for (; i < patlen; i++)
			if (patvalid[i] != '?' && pattern[i] != addr[i])
				break;
		if (i == patlen)
			return (T)addr;
	}
	return nullptr;
}

template <typename T>
class VarPtr
{
public:
	void SetPtr(T* ptr)
	{
		this->ptr = ptr;
	}

	T* operator &()
	{
		return ptr;
	}

	operator T& ()
	{
		return *ptr;
	}

private:
	T* ptr;
};

template <typename T, size_t len>
class ArrayPtr
{
public:
	void SetPtr(T* ptr)
	{
		this->ptr = ptr;
	}

	T* operator &()
	{
		return ptr;
	}

	operator T* ()
	{
		return ptr;
	}

	constexpr size_t size() const noexcept
	{
		return len;
	}

private:
	T* ptr;
};

template <typename T>
inline T GetPtr(void* offset)
{
	return (T)(*(int*)offset + ((char*)offset + 4));
}

template <typename T>
inline void GetVarPtr(VarPtr<T>& vp, void* offset)
{
	vp.SetPtr((T*)(*(int*)offset + ((char*)offset + 4)));
}

template <typename T, size_t N>
inline void GetArrPtr(ArrayPtr<T, N>& vp, void* offset)
{
	vp.SetPtr((T*)(*(int*)offset + ((char*)offset + 4)));
}

namespace YS
{
	namespace VM
	{
		VarPtr<char**> Current;
	}
}
VarPtr<int> MainCounter;
void* (* const GetAIDataPtr)(int ptr) = GetPtr<void* (*)(int)>(Hook<char*>("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x83\xEC\x40\x4C\x8B\xF1\x8B\x09\xE8", "xxxx?xxxx?xxxx?xxxx?xxxxxxxxxxxx") + 0x20);

void trap_puti(intptr_t* args)
{
	SetConsoleTextAttribute(_hConsole, FOREGROUND_BLUE);
	printf(
		"[%s:%d]%s = %d(0x%x)\n",
		(const char*)*YS::VM::Current,
		(unsigned int)MainCounter,
		reinterpret_cast<const char*>(GetAIDataPtr((int)args[1])),
		(int)args[0],
		(int)args[0]);
	SetConsoleTextAttribute(_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void trap_putf(intptr_t* args)
{
	SetConsoleTextAttribute(_hConsole, FOREGROUND_BLUE);
	printf(
		"[%s]%s = %f\n",
		(const char*)*YS::VM::Current,
		reinterpret_cast<const char*>(GetAIDataPtr((int)args[1])),
		*reinterpret_cast<float*>(&args[0]));
	SetConsoleTextAttribute(_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void trap_puts(intptr_t* args)
{
	SetConsoleTextAttribute(_hConsole, FOREGROUND_BLUE);
	printf("[%s]%s\n", (const char*)*YS::VM::Current, reinterpret_cast<const char*>(GetAIDataPtr((int)args[0])));
	SetConsoleTextAttribute(_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void trap_vector_dump(intptr_t* args)
{
	auto vec = reinterpret_cast<float*>(GetAIDataPtr((int)args[0]));
	printf("%s = (%f, %f, %f, %f)\n", reinterpret_cast<const char*>(args[1]), vec[0], vec[1], vec[2], vec[3]);
}

void trap_assert(intptr_t* args)
{
	printf("[%s:%d] assert(%d)\n", (const char*)*YS::VM::Current, (unsigned int)MainCounter, (int)args[0]);
}

void trap_obj_dump(intptr_t* args)
{
	printf("[%s] OBJ#dump : %s\n", (const char*)*YS::VM::Current, (char*)GetAIDataPtr((int)args[0]) + 4);
}

void trap_target_set_after_player(intptr_t* args)
{
	printf("[%s:%d]", "C:\\hd25\\kingdom2\\yasui\\libys\\trapfield.cpp", 806);
	SetConsoleTextAttribute(_hConsole, FOREGROUND_RED);
	printf("** WARNING! ");
	printf("obsolete TARGET.set_after_player");
	printf(" **\n");
	SetConsoleTextAttribute(_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void trap_target_clear_after_player(intptr_t* args)
{
	printf("[%s:%d]", "C:\\hd25\\kingdom2\\yasui\\libys\\trapfield.cpp", 810);
	SetConsoleTextAttribute(_hConsole, FOREGROUND_RED);
	printf("** WARNING! ");
	printf("obsolete TARGET.clear_after_player");
	printf(" **\n");
	SetConsoleTextAttribute(_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

extern "C"
{
	__declspec(dllexport) void OnInit(const char* mod_path)
	{
		auto execfunc = Hook<char*>("\x40\x56\x48\x83\xEC\x20\x8B\x41\x38\x48", "xxxxxxxxxx");
		TrapFunc** traptbl = GetPtr<TrapFunc**>(execfunc + 0x35);
		traptbl[0][0].Func = trap_puti;
		traptbl[0][1].Func = trap_putf;
		traptbl[0][2].Func = trap_puts;
		traptbl[0][8].Func = trap_vector_dump;
		traptbl[0][24].Func = trap_puti;
		traptbl[0][25].Func = trap_putf;
		traptbl[0][26].Func = trap_puts;
		traptbl[0][60].Func = trap_assert;
		//traptbl[1][122].Func = trap_obj_dump;
		traptbl[1][163].Func = trap_target_set_after_player;
		traptbl[1][246].Func = trap_target_clear_after_player;
		GetVarPtr(YS::VM::Current, execfunc + 0x2B);
		auto framefunc = Hook<char*>("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x40\x33\xFF\x48\x8B\xD9", "xxxx?xxxx?xxxxxxxxxx");
		GetVarPtr(MainCounter, framefunc + 0x14D);
		printf("test\n");
	}
}