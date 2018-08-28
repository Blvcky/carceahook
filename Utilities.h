#pragma once

// Includes
#include "CommonIncludes.h"
#include <time.h>
#include <locale>
#include <codecvt>
#include <sstream>
#include <chrono>

// Colors for the console
//Define extra colours
#define FOREGROUND_WHITE		    (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_YELLOW       	(FOREGROUND_RED | FOREGROUND_GREEN)
#define FOREGROUND_CYAN		        (FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_MAGENTA	        (FOREGROUND_RED | FOREGROUND_BLUE)
#define FOREGROUND_BLACK		    0

#define FOREGROUND_INTENSE_RED		(FOREGROUND_RED | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_GREEN	(FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_BLUE		(FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_WHITE	(FOREGROUND_WHITE | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_YELLOW	(FOREGROUND_YELLOW | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_CYAN		(FOREGROUND_CYAN | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_MAGENTA	(FOREGROUND_MAGENTA | FOREGROUND_INTENSITY)

#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))


// U Namespace
// Purpose: Contains misc functionality for memory related functionality
namespace U
{
	// Debug console controls
	void OpenConsole(std::string Title);
	void CloseConsole();
	void Log(const char *fmt, ...);
	extern void FullUpdate();
	void EnableLogFile(std::string filename);

	void SetConsoleColor(WORD color);

	// Misc Shizz
	std::string GetTimeString();

	// Memory utils
	// Purpose: Provides memeory related functionality (pattern scanning ect)

	// Waits for a module to be available, before returning it's base address
	DWORD WaitOnModuleHandle(std::string moduleName);

	uintptr_t FindSig(std::string moduleName, std::string pattern);

	uintptr_t FindPatternNew(const char * module, const char * pattern_string, const char * mask);

	// Attempts to locate the given signature and mask in a memory range
	// Returns the address at which it has been found
	DWORD FindPattern(std::string moduleName, BYTE* Mask, char* szMask);
	extern DWORD FindPatternSig(std::string moduleName, std::string pattern);
	DWORD_PTR GetPatternOffset(std::string moduleName, PBYTE pattern, std::string mask, DWORD_PTR codeBase, DWORD_PTR codeSize);
	// Attempts to locate the given text in a memory range
	// Returns the address at which it has been found
	DWORD FindTextPattern(std::string moduleName, char* string);
	uint64_t FindPatternIDA(const char * szModule, const char * szSignature);
    std::uint8_t* pattern_scan(void* module, const char* signature);
	std::wstring StringToWstring(std::string str);

	//clantag + namespammer shit
	long GetEpochTime();
	const char * PadStringRight(std::string text, size_t value);
	template <typename T>
	T* CaptureInterface(std::string strModule, std::string strInterface)
	{
		typedef T* (*CreateInterfaceFn)(const char* szName, int iReturn);
		CreateInterfaceFn CreateInterface = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA(strModule.c_str()), "CreateInterface");

		return CreateInterface(strInterface.c_str(), 0);
	}

	bool bin_match(uint8_t *code, std::vector< uint8_t > &pattern);
	template< typename T = uintptr_t > static T first_match(uintptr_t start, std::string sig, size_t len);
	template< typename T = uintptr_t > static T first_code_match(HMODULE start, std::string sig);

};
namespace detail
{
    class protect_guard
    {
    public:
        protect_guard(void* base, size_t len, std::uint32_t flags)
        {
            _base = base;
            _length = len;
            if (!VirtualProtect(base, len, flags, (PDWORD)&_old))
                throw std::runtime_error("Failed to protect region.");
        }
        ~protect_guard()
        {
            VirtualProtect(_base, _length, _old, (PDWORD)&_old);
        }

    private:
        void*         _base;
        size_t        _length;
        std::uint32_t _old;
    };
}
class vfunc_hook
{
public:
    vfunc_hook();
    vfunc_hook(void* base);
    ~vfunc_hook();

    bool setup(void* class_base = nullptr);

    template<typename T>
    void hook_index(int index, T fun)
    {
        assert(index >= 0 && index <= (int)vftbl_len);
        new_vftbl[index + 1] = reinterpret_cast<std::uintptr_t>(fun);
    }
    void unhook_index(int index)
    {
        new_vftbl[index] = old_vftbl[index];
    }
    void unhook_all()
    {
        try {
            if (old_vftbl != nullptr) {
                auto guard = detail::protect_guard{ class_base, sizeof(std::uintptr_t), PAGE_READWRITE };
                *(std::uintptr_t**)class_base = old_vftbl;
                old_vftbl = nullptr;
            }
        }
        catch (...) {
        }
    }

    template<typename T>
    T get_original(int index)
    {
        return (T)old_vftbl[index];
    }

private:
    static inline std::size_t estimate_vftbl_length(std::uintptr_t* vftbl_start);

    void*           class_base;
    std::size_t     vftbl_len;
    std::uintptr_t* new_vftbl;
    std::uintptr_t* old_vftbl;
};

inline bool Compare(const uint8_t* data, const uint8_t* pattern, const char* mask) {
	for (; *mask; ++mask, ++data, ++pattern)
		if (*mask == 'x' && *data != *pattern)
			return false;

	return (*mask) == 0;
}
