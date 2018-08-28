#pragma once


// Includes
#include "Utilities.h"
#include <fstream>
#include <PsapI.h>
#include "Interfaces.h"



bool FileLog = false;
std::ofstream logFile;

// --------         U Core           ------------ //
// Opens a debug console
void  U::OpenConsole(std::string Title)
{
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	SetConsoleTitle(Title.c_str());
}
void U::FullUpdate()
{
	typedef void(*FullUpdate_t) (void);
	FullUpdate_t FullUpdate = (FullUpdate_t)(offsetz.FullUpdate);
	FullUpdate();
}
// Closes the debug console
void  U::CloseConsole()
{
	FreeConsole();
}

// Outputs text to the console
void  U::Log(const char *fmt, ...)
{
	if (!fmt) return; //if the passed string is null return
	if (strlen(fmt) < 2) return;

	//Set up va_list and buffer to hold the params 
	va_list va_alist;
	char logBuf[256] = { 0 };

	//Do sprintf with the parameters
	va_start(va_alist, fmt);
	_vsnprintf(logBuf + strlen(logBuf), sizeof(logBuf) - strlen(logBuf), fmt, va_alist);
	va_end(va_alist);

	//Output to console
	if (logBuf[0] != '\0')
	{
		SetConsoleColor(FOREGROUND_INTENSE_RED);
		printf("[%s]", GetTimeString().c_str());
		SetConsoleColor(FOREGROUND_WHITE);
		printf(": %s\n", logBuf);
	}

	if (FileLog)
	{
		logFile << logBuf << std::endl;
	}
}

// Gets the current time as a string
std::string  U::GetTimeString()
{
	//Time related variables
	time_t current_time;
	struct tm *time_info;
	static char timeString[10];

	//Get current time
	time(&current_time);
	time_info = localtime(&current_time);

	//Get current time as string
	strftime(timeString, sizeof(timeString), "%I:%M%p", time_info);
	return timeString;
}

// Sets the console color for upcoming text
void  U::SetConsoleColor(WORD color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Enables writing all log calls to a file
void  U::EnableLogFile(std::string filename)
{
	logFile.open(filename.c_str());
	if (logFile.is_open())
		FileLog = true;
}


// --------         U Memory           ------------ //

DWORD U::WaitOnModuleHandle(std::string moduleName)
{
	DWORD ModuleHandle = NULL;
	while (!ModuleHandle)
	{
		ModuleHandle = (DWORD)GetModuleHandle(moduleName.c_str());
		if (!ModuleHandle)
			Sleep(50);
	}
	return ModuleHandle;
}

uintptr_t U::FindSig(std::string moduleName, std::string pattern)
{
	const char* daPattern = pattern.c_str();
	uintptr_t firstMatch = 0;
	uintptr_t moduleBase = (uintptr_t)GetModuleHandleA(moduleName.c_str());
	MODULEINFO miModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)moduleBase, &miModInfo, sizeof(MODULEINFO));
	uintptr_t moduleEnd = moduleBase + miModInfo.SizeOfImage;
	for (uintptr_t pCur = moduleBase; pCur < moduleEnd; pCur++)
	{
		if (!*daPattern)
			return firstMatch;

		if (*(PBYTE)daPattern == '\?' || *(BYTE*)pCur == getByte(daPattern))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!daPattern[2])
				return firstMatch;

			if (*(PWORD)daPattern == '\?\?' || *(PBYTE)daPattern != '\?')
				daPattern += 3;

			else
				daPattern += 2;
		}
		else
		{
			daPattern = pattern.c_str();
			firstMatch = 0;
		}
	}
	return 0;
}
DWORD U::FindPatternSig(std::string moduleName, std::string pattern)
{
	const char* pat = pattern.c_str();
	DWORD firstMatch = 0;
	DWORD rangeStart = (DWORD)GetModuleHandleA(moduleName.c_str());
	MODULEINFO miModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
	DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;
	for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++)
	{
		if (!*pat)
			return firstMatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!pat[2])
				return firstMatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;

			else
				pat += 2;    //one ?
		}
		else
		{
			pat = pattern.c_str();
			firstMatch = 0;
		}
	}
	return NULL;
}
bool bCompare(const BYTE* Data, const BYTE* Mask, const char* szMask)
{
	for (; *szMask; ++szMask, ++Mask, ++Data)
	{
		if (*szMask == 'x' && *Mask != *Data)
		{
			return false;
		}
	}
	return (*szMask) == 0;
}

uintptr_t U::FindPatternNew(const char* module, const char* pattern_string, const char* mask) {
	MODULEINFO module_info = {};
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(module), &module_info, sizeof MODULEINFO);

	uintptr_t module_start = uintptr_t(module_info.lpBaseOfDll);

	const uint8_t* pattern = reinterpret_cast<const uint8_t*>(pattern_string);

	for (size_t i = 0; i < module_info.SizeOfImage; i++)
		if (Compare(reinterpret_cast<uint8_t*>(module_start + i), pattern, mask))
			return module_start + i;

	return 0;
}
DWORD U::FindPattern(std::string moduleName, BYTE* Mask, char* szMask)
{
	DWORD Address = WaitOnModuleHandle(moduleName.c_str());
	MODULEINFO ModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)Address, &ModInfo, sizeof(MODULEINFO));
	DWORD Length = ModInfo.SizeOfImage;
	for (DWORD c = 0; c < Length; c += 1)
	{
		if (bCompare((BYTE*)(Address + c), Mask, szMask))
		{
			return DWORD(Address + c);
		}
	}
	return 0;
}

DWORD U::FindTextPattern(std::string moduleName, char* string)
{
	DWORD Address = WaitOnModuleHandle(moduleName.c_str());
	MODULEINFO ModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)Address, &ModInfo, sizeof(MODULEINFO));
	DWORD Length = ModInfo.SizeOfImage;

	int len = strlen(string);
	char* szMask = new char[len + 1];
	for (int i = 0; i < len; i++)
	{
		szMask[i] = 'x';
	}
	szMask[len] = '\0';

	for (DWORD c = 0; c < Length; c += 1)
	{
		if (bCompare((BYTE*)(Address + c), (BYTE*)string, szMask))
		{
			return (DWORD)(Address + c);
		}
	}
	return 0;
}

uint64_t U::FindPatternIDA(const char* szModule, const char* szSignature)
{
	//CREDITS: learn_more
#define INRANGE(x,a,b)  (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),('A'),('F')) ? ((x&(~0x20)) - ('A') + 0xa) : (INRANGE(x,('0'),('9')) ? x - ('0') : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

	MODULEINFO modInfo;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(szModule), &modInfo, sizeof(MODULEINFO));
	DWORD startAddress = (DWORD)modInfo.lpBaseOfDll;
	DWORD endAddress = startAddress + modInfo.SizeOfImage;
	const char* pat = szSignature;
	DWORD firstMatch = 0;
	for (DWORD pCur = startAddress; pCur < endAddress; pCur++) {
		if (!*pat) return firstMatch;
		if (*(PBYTE)pat == ('\?') || *(BYTE*)pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) return firstMatch;
			if (*(PWORD)pat == ('\?\?') || *(PBYTE)pat != ('\?')) pat += 3;
			else pat += 2;    //one ?
		}
		else {
			pat = szSignature;
			firstMatch = 0;
		}
	}
	return NULL;
}

#pragma warning( disable : 4018 )  
#pragma warning( disable : 4348 )  

bool U::bin_match(uint8_t *code, std::vector< uint8_t > &pattern)
{
	for (int j = 0; j < pattern.size(); j++)
	{
		if (!pattern[j] && code[j] != pattern[j])
		{
			return false;
		}
	}

	return true;
}

template< typename T = uintptr_t > static T U::first_match(uintptr_t start, std::string sig, size_t len)
{
	std::istringstream iss(sig);
	std::vector< std::string > tokens{ std::istream_iterator< std::string >{ iss }, std::istream_iterator< std::string >{} };
	std::vector< uint8_t > sig_bytes;

	for (auto hex_byte : tokens)
	{
		sig_bytes.push_back(std::strtoul(hex_byte.c_str(), nullptr, 16));
	}

	if (sig_bytes.empty() || sig_bytes.size() < 2)
	{
		return T{};
	}

	for (size_t i{}; i < len; i++)
	{
		uint8_t *code_ptr = reinterpret_cast< uint8_t * >(start + i);

		if (code_ptr[0] != sig_bytes.at(0))
		{
			continue;
		}

		if (bin_match(code_ptr, sig_bytes))
		{
			return((T)(start + i));
		}
	}




	return T{};
}

template< typename T = uintptr_t > static T U::first_code_match(HMODULE start, std::string sig)
{
	auto ntoskrnl = reinterpret_cast< PIMAGE_DOS_HEADER >(start);

	if (ntoskrnl->e_magic != 0x5a4d)
	{
		return T{};
	}

	auto nt_hdrs = reinterpret_cast< PIMAGE_NT_HEADERS >(reinterpret_cast< uintptr_t >(ntoskrnl) + ntoskrnl->e_lfanew);

	return first_match< T >(reinterpret_cast< uintptr_t >(ntoskrnl) + nt_hdrs->OptionalHeader.BaseOfCode, sig, nt_hdrs->OptionalHeader.SizeOfCode);
}

std::uint8_t* U::pattern_scan(void* module, const char* signature)
{
    static auto pattern_to_byte = [](const char* pattern) {
        auto bytes = std::vector<int>{};
        auto start = const_cast<char*>(pattern);
        auto end = const_cast<char*>(pattern) + strlen(pattern);

        for (auto current = start; current < end; ++current) {
            if (*current == '?') {
                ++current;
                if (*current == '?')
                    ++current;
                bytes.push_back(-1);
            }
            else {
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
    };

    auto dosHeader = (PIMAGE_DOS_HEADER)module;
    auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

    auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    auto patternBytes = pattern_to_byte(signature);
    auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

    auto s = patternBytes.size();
    auto d = patternBytes.data();

    for (auto i = 0ul; i < sizeOfImage - s; ++i) {
        bool found = true;
        for (auto j = 0ul; j < s; ++j) {
            if (scanBytes[i + j] != d[j] && d[j] != -1) {
                found = false;
                break;
            }
        }
        if (found) {
            return &scanBytes[i];
        }
    }
    return nullptr;
}



vfunc_hook::vfunc_hook()
    : class_base(nullptr), vftbl_len(0), new_vftbl(nullptr), old_vftbl(nullptr)
{
}
vfunc_hook::vfunc_hook(void* base)
    : class_base(base), vftbl_len(0), new_vftbl(nullptr), old_vftbl(nullptr)
{
}
vfunc_hook::~vfunc_hook()
{
    unhook_all();

    delete[] new_vftbl;
}


bool vfunc_hook::setup(void* base /*= nullptr*/)
{
    if (base != nullptr)
        class_base = base;

    if (class_base == nullptr)
        return false;

    old_vftbl = *(std::uintptr_t**)class_base;
    vftbl_len = estimate_vftbl_length(old_vftbl);

    if (vftbl_len == 0)
        return false;

    new_vftbl = new std::uintptr_t[vftbl_len + 1]();

    std::memcpy(&new_vftbl[1], old_vftbl, vftbl_len * sizeof(std::uintptr_t));


    try {
        auto guard = detail::protect_guard{ class_base, sizeof(std::uintptr_t), PAGE_READWRITE };
        new_vftbl[0] = old_vftbl[-1];
        *(std::uintptr_t**)class_base = &new_vftbl[1];
    }
    catch (...) {
        delete[] new_vftbl;
        return false;
    }

    return true;
}



std::size_t vfunc_hook::estimate_vftbl_length(std::uintptr_t* vftbl_start)
{
    auto len = std::size_t{};

    while (vftbl_start[len] >= 0x00010000 &&
        vftbl_start[len] <  0xFFF00000 &&
        len < 512 /* Hard coded value. Can cause problems, beware.*/) {
        len++;
    }

    return len;
}


std::wstring U::StringToWstring(std::string str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

	try
	{
		return converter.from_bytes(str);
	}
	catch (std::range_error)
	{
		std::wostringstream s;
		s << str.c_str();
		return s.str();
	}
}

long U::GetEpochTime()
{
	using namespace std::chrono;
	milliseconds ms = duration_cast< milliseconds >(
		system_clock::now().time_since_epoch()
		);

	return ms.count();
}

const char* U::PadStringRight(std::string text, size_t value)
{
	text.insert(text.length(), value - text.length(), ' ');

	return text.c_str();
}






















































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// Junk Code By Troll Face & Thaisen's Gen
void neXgZRLPTXEJzsLi75194394() {     double ZzduThqJorQLAQBCI98392723 = -819876256;    double ZzduThqJorQLAQBCI33928456 = -30391694;    double ZzduThqJorQLAQBCI41998748 = -992915767;    double ZzduThqJorQLAQBCI39812647 = -350963631;    double ZzduThqJorQLAQBCI30507367 = -678704153;    double ZzduThqJorQLAQBCI47419736 = -479468300;    double ZzduThqJorQLAQBCI5764396 = -872673099;    double ZzduThqJorQLAQBCI83422483 = -668796676;    double ZzduThqJorQLAQBCI98355540 = -259695386;    double ZzduThqJorQLAQBCI80447481 = -524352329;    double ZzduThqJorQLAQBCI58436285 = -568163836;    double ZzduThqJorQLAQBCI15792617 = -489060525;    double ZzduThqJorQLAQBCI9198850 = -592078524;    double ZzduThqJorQLAQBCI71751012 = -187766046;    double ZzduThqJorQLAQBCI82646345 = 31239920;    double ZzduThqJorQLAQBCI34776387 = -751269222;    double ZzduThqJorQLAQBCI41033795 = -531625966;    double ZzduThqJorQLAQBCI58776945 = -930198147;    double ZzduThqJorQLAQBCI67062174 = -454354313;    double ZzduThqJorQLAQBCI98279784 = 5316151;    double ZzduThqJorQLAQBCI20982612 = -597619243;    double ZzduThqJorQLAQBCI15696207 = -385726843;    double ZzduThqJorQLAQBCI44495080 = 88485937;    double ZzduThqJorQLAQBCI70847997 = -59386538;    double ZzduThqJorQLAQBCI77926813 = 90993930;    double ZzduThqJorQLAQBCI25937330 = -156106337;    double ZzduThqJorQLAQBCI6367137 = -899630862;    double ZzduThqJorQLAQBCI9192682 = -91276127;    double ZzduThqJorQLAQBCI88228597 = 79087637;    double ZzduThqJorQLAQBCI45104333 = -857658127;    double ZzduThqJorQLAQBCI84519680 = 57687873;    double ZzduThqJorQLAQBCI82009917 = -731243929;    double ZzduThqJorQLAQBCI42287071 = -588584779;    double ZzduThqJorQLAQBCI82228682 = -189164356;    double ZzduThqJorQLAQBCI84468607 = -211613538;    double ZzduThqJorQLAQBCI86449227 = -750577823;    double ZzduThqJorQLAQBCI64604766 = -804236419;    double ZzduThqJorQLAQBCI46897580 = 44878450;    double ZzduThqJorQLAQBCI90367860 = -343964957;    double ZzduThqJorQLAQBCI55007949 = -380414100;    double ZzduThqJorQLAQBCI40955664 = 22562749;    double ZzduThqJorQLAQBCI10011528 = -892894040;    double ZzduThqJorQLAQBCI72257522 = -446016773;    double ZzduThqJorQLAQBCI1510749 = -67461769;    double ZzduThqJorQLAQBCI36781017 = -312343173;    double ZzduThqJorQLAQBCI89880431 = -443765103;    double ZzduThqJorQLAQBCI15116377 = -379927310;    double ZzduThqJorQLAQBCI87926426 = -877104736;    double ZzduThqJorQLAQBCI55200425 = 12536139;    double ZzduThqJorQLAQBCI99380334 = 4778067;    double ZzduThqJorQLAQBCI41457278 = -980150582;    double ZzduThqJorQLAQBCI79984257 = -906807798;    double ZzduThqJorQLAQBCI42163619 = -236576234;    double ZzduThqJorQLAQBCI24624540 = -843903722;    double ZzduThqJorQLAQBCI54211782 = -738479617;    double ZzduThqJorQLAQBCI82696517 = -334149414;    double ZzduThqJorQLAQBCI89433375 = -18877632;    double ZzduThqJorQLAQBCI71150751 = -833529230;    double ZzduThqJorQLAQBCI61885833 = -341957561;    double ZzduThqJorQLAQBCI4570037 = -422597816;    double ZzduThqJorQLAQBCI41052599 = -579837438;    double ZzduThqJorQLAQBCI96571714 = -681396972;    double ZzduThqJorQLAQBCI95193885 = -647884313;    double ZzduThqJorQLAQBCI53251208 = -402037260;    double ZzduThqJorQLAQBCI95927800 = -482040203;    double ZzduThqJorQLAQBCI76426367 = -836919908;    double ZzduThqJorQLAQBCI73505545 = -900475747;    double ZzduThqJorQLAQBCI26970168 = -302914168;    double ZzduThqJorQLAQBCI87282405 = -976152508;    double ZzduThqJorQLAQBCI96197118 = -218182257;    double ZzduThqJorQLAQBCI70171621 = -947032804;    double ZzduThqJorQLAQBCI94136214 = -476504416;    double ZzduThqJorQLAQBCI68409085 = -486233190;    double ZzduThqJorQLAQBCI12054226 = 26059787;    double ZzduThqJorQLAQBCI57324121 = 82753402;    double ZzduThqJorQLAQBCI10971085 = -704725204;    double ZzduThqJorQLAQBCI43438684 = -939710070;    double ZzduThqJorQLAQBCI42984332 = -844052294;    double ZzduThqJorQLAQBCI34066981 = -747043366;    double ZzduThqJorQLAQBCI88046382 = -465240967;    double ZzduThqJorQLAQBCI10820953 = -776179027;    double ZzduThqJorQLAQBCI18440711 = 77473873;    double ZzduThqJorQLAQBCI53992256 = -3812266;    double ZzduThqJorQLAQBCI88848262 = -925690431;    double ZzduThqJorQLAQBCI3647055 = -877507545;    double ZzduThqJorQLAQBCI4535423 = -35504330;    double ZzduThqJorQLAQBCI39846299 = -394667695;    double ZzduThqJorQLAQBCI17662531 = -744681057;    double ZzduThqJorQLAQBCI28016900 = -450684739;    double ZzduThqJorQLAQBCI1772091 = -877464125;    double ZzduThqJorQLAQBCI97015852 = -631700192;    double ZzduThqJorQLAQBCI93454015 = -970707189;    double ZzduThqJorQLAQBCI85011747 = -613163990;    double ZzduThqJorQLAQBCI85797823 = -921367141;    double ZzduThqJorQLAQBCI13955350 = -800576662;    double ZzduThqJorQLAQBCI44383950 = -296040280;    double ZzduThqJorQLAQBCI14817642 = -145009727;    double ZzduThqJorQLAQBCI19006314 = 56020486;    double ZzduThqJorQLAQBCI5582949 = -585421567;    double ZzduThqJorQLAQBCI60354649 = -819876256;     ZzduThqJorQLAQBCI98392723 = ZzduThqJorQLAQBCI33928456;     ZzduThqJorQLAQBCI33928456 = ZzduThqJorQLAQBCI41998748;     ZzduThqJorQLAQBCI41998748 = ZzduThqJorQLAQBCI39812647;     ZzduThqJorQLAQBCI39812647 = ZzduThqJorQLAQBCI30507367;     ZzduThqJorQLAQBCI30507367 = ZzduThqJorQLAQBCI47419736;     ZzduThqJorQLAQBCI47419736 = ZzduThqJorQLAQBCI5764396;     ZzduThqJorQLAQBCI5764396 = ZzduThqJorQLAQBCI83422483;     ZzduThqJorQLAQBCI83422483 = ZzduThqJorQLAQBCI98355540;     ZzduThqJorQLAQBCI98355540 = ZzduThqJorQLAQBCI80447481;     ZzduThqJorQLAQBCI80447481 = ZzduThqJorQLAQBCI58436285;     ZzduThqJorQLAQBCI58436285 = ZzduThqJorQLAQBCI15792617;     ZzduThqJorQLAQBCI15792617 = ZzduThqJorQLAQBCI9198850;     ZzduThqJorQLAQBCI9198850 = ZzduThqJorQLAQBCI71751012;     ZzduThqJorQLAQBCI71751012 = ZzduThqJorQLAQBCI82646345;     ZzduThqJorQLAQBCI82646345 = ZzduThqJorQLAQBCI34776387;     ZzduThqJorQLAQBCI34776387 = ZzduThqJorQLAQBCI41033795;     ZzduThqJorQLAQBCI41033795 = ZzduThqJorQLAQBCI58776945;     ZzduThqJorQLAQBCI58776945 = ZzduThqJorQLAQBCI67062174;     ZzduThqJorQLAQBCI67062174 = ZzduThqJorQLAQBCI98279784;     ZzduThqJorQLAQBCI98279784 = ZzduThqJorQLAQBCI20982612;     ZzduThqJorQLAQBCI20982612 = ZzduThqJorQLAQBCI15696207;     ZzduThqJorQLAQBCI15696207 = ZzduThqJorQLAQBCI44495080;     ZzduThqJorQLAQBCI44495080 = ZzduThqJorQLAQBCI70847997;     ZzduThqJorQLAQBCI70847997 = ZzduThqJorQLAQBCI77926813;     ZzduThqJorQLAQBCI77926813 = ZzduThqJorQLAQBCI25937330;     ZzduThqJorQLAQBCI25937330 = ZzduThqJorQLAQBCI6367137;     ZzduThqJorQLAQBCI6367137 = ZzduThqJorQLAQBCI9192682;     ZzduThqJorQLAQBCI9192682 = ZzduThqJorQLAQBCI88228597;     ZzduThqJorQLAQBCI88228597 = ZzduThqJorQLAQBCI45104333;     ZzduThqJorQLAQBCI45104333 = ZzduThqJorQLAQBCI84519680;     ZzduThqJorQLAQBCI84519680 = ZzduThqJorQLAQBCI82009917;     ZzduThqJorQLAQBCI82009917 = ZzduThqJorQLAQBCI42287071;     ZzduThqJorQLAQBCI42287071 = ZzduThqJorQLAQBCI82228682;     ZzduThqJorQLAQBCI82228682 = ZzduThqJorQLAQBCI84468607;     ZzduThqJorQLAQBCI84468607 = ZzduThqJorQLAQBCI86449227;     ZzduThqJorQLAQBCI86449227 = ZzduThqJorQLAQBCI64604766;     ZzduThqJorQLAQBCI64604766 = ZzduThqJorQLAQBCI46897580;     ZzduThqJorQLAQBCI46897580 = ZzduThqJorQLAQBCI90367860;     ZzduThqJorQLAQBCI90367860 = ZzduThqJorQLAQBCI55007949;     ZzduThqJorQLAQBCI55007949 = ZzduThqJorQLAQBCI40955664;     ZzduThqJorQLAQBCI40955664 = ZzduThqJorQLAQBCI10011528;     ZzduThqJorQLAQBCI10011528 = ZzduThqJorQLAQBCI72257522;     ZzduThqJorQLAQBCI72257522 = ZzduThqJorQLAQBCI1510749;     ZzduThqJorQLAQBCI1510749 = ZzduThqJorQLAQBCI36781017;     ZzduThqJorQLAQBCI36781017 = ZzduThqJorQLAQBCI89880431;     ZzduThqJorQLAQBCI89880431 = ZzduThqJorQLAQBCI15116377;     ZzduThqJorQLAQBCI15116377 = ZzduThqJorQLAQBCI87926426;     ZzduThqJorQLAQBCI87926426 = ZzduThqJorQLAQBCI55200425;     ZzduThqJorQLAQBCI55200425 = ZzduThqJorQLAQBCI99380334;     ZzduThqJorQLAQBCI99380334 = ZzduThqJorQLAQBCI41457278;     ZzduThqJorQLAQBCI41457278 = ZzduThqJorQLAQBCI79984257;     ZzduThqJorQLAQBCI79984257 = ZzduThqJorQLAQBCI42163619;     ZzduThqJorQLAQBCI42163619 = ZzduThqJorQLAQBCI24624540;     ZzduThqJorQLAQBCI24624540 = ZzduThqJorQLAQBCI54211782;     ZzduThqJorQLAQBCI54211782 = ZzduThqJorQLAQBCI82696517;     ZzduThqJorQLAQBCI82696517 = ZzduThqJorQLAQBCI89433375;     ZzduThqJorQLAQBCI89433375 = ZzduThqJorQLAQBCI71150751;     ZzduThqJorQLAQBCI71150751 = ZzduThqJorQLAQBCI61885833;     ZzduThqJorQLAQBCI61885833 = ZzduThqJorQLAQBCI4570037;     ZzduThqJorQLAQBCI4570037 = ZzduThqJorQLAQBCI41052599;     ZzduThqJorQLAQBCI41052599 = ZzduThqJorQLAQBCI96571714;     ZzduThqJorQLAQBCI96571714 = ZzduThqJorQLAQBCI95193885;     ZzduThqJorQLAQBCI95193885 = ZzduThqJorQLAQBCI53251208;     ZzduThqJorQLAQBCI53251208 = ZzduThqJorQLAQBCI95927800;     ZzduThqJorQLAQBCI95927800 = ZzduThqJorQLAQBCI76426367;     ZzduThqJorQLAQBCI76426367 = ZzduThqJorQLAQBCI73505545;     ZzduThqJorQLAQBCI73505545 = ZzduThqJorQLAQBCI26970168;     ZzduThqJorQLAQBCI26970168 = ZzduThqJorQLAQBCI87282405;     ZzduThqJorQLAQBCI87282405 = ZzduThqJorQLAQBCI96197118;     ZzduThqJorQLAQBCI96197118 = ZzduThqJorQLAQBCI70171621;     ZzduThqJorQLAQBCI70171621 = ZzduThqJorQLAQBCI94136214;     ZzduThqJorQLAQBCI94136214 = ZzduThqJorQLAQBCI68409085;     ZzduThqJorQLAQBCI68409085 = ZzduThqJorQLAQBCI12054226;     ZzduThqJorQLAQBCI12054226 = ZzduThqJorQLAQBCI57324121;     ZzduThqJorQLAQBCI57324121 = ZzduThqJorQLAQBCI10971085;     ZzduThqJorQLAQBCI10971085 = ZzduThqJorQLAQBCI43438684;     ZzduThqJorQLAQBCI43438684 = ZzduThqJorQLAQBCI42984332;     ZzduThqJorQLAQBCI42984332 = ZzduThqJorQLAQBCI34066981;     ZzduThqJorQLAQBCI34066981 = ZzduThqJorQLAQBCI88046382;     ZzduThqJorQLAQBCI88046382 = ZzduThqJorQLAQBCI10820953;     ZzduThqJorQLAQBCI10820953 = ZzduThqJorQLAQBCI18440711;     ZzduThqJorQLAQBCI18440711 = ZzduThqJorQLAQBCI53992256;     ZzduThqJorQLAQBCI53992256 = ZzduThqJorQLAQBCI88848262;     ZzduThqJorQLAQBCI88848262 = ZzduThqJorQLAQBCI3647055;     ZzduThqJorQLAQBCI3647055 = ZzduThqJorQLAQBCI4535423;     ZzduThqJorQLAQBCI4535423 = ZzduThqJorQLAQBCI39846299;     ZzduThqJorQLAQBCI39846299 = ZzduThqJorQLAQBCI17662531;     ZzduThqJorQLAQBCI17662531 = ZzduThqJorQLAQBCI28016900;     ZzduThqJorQLAQBCI28016900 = ZzduThqJorQLAQBCI1772091;     ZzduThqJorQLAQBCI1772091 = ZzduThqJorQLAQBCI97015852;     ZzduThqJorQLAQBCI97015852 = ZzduThqJorQLAQBCI93454015;     ZzduThqJorQLAQBCI93454015 = ZzduThqJorQLAQBCI85011747;     ZzduThqJorQLAQBCI85011747 = ZzduThqJorQLAQBCI85797823;     ZzduThqJorQLAQBCI85797823 = ZzduThqJorQLAQBCI13955350;     ZzduThqJorQLAQBCI13955350 = ZzduThqJorQLAQBCI44383950;     ZzduThqJorQLAQBCI44383950 = ZzduThqJorQLAQBCI14817642;     ZzduThqJorQLAQBCI14817642 = ZzduThqJorQLAQBCI19006314;     ZzduThqJorQLAQBCI19006314 = ZzduThqJorQLAQBCI5582949;     ZzduThqJorQLAQBCI5582949 = ZzduThqJorQLAQBCI60354649;     ZzduThqJorQLAQBCI60354649 = ZzduThqJorQLAQBCI98392723;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void clRhbNHUkwaWDnCY42485993() {     double etWzZXiphPmhlAyMc33851692 = -186153156;    double etWzZXiphPmhlAyMc47200534 = 89056699;    double etWzZXiphPmhlAyMc79385225 = -658582842;    double etWzZXiphPmhlAyMc12036002 = -172114702;    double etWzZXiphPmhlAyMc86792636 = -784302998;    double etWzZXiphPmhlAyMc83642608 = -741816195;    double etWzZXiphPmhlAyMc16421802 = -717398833;    double etWzZXiphPmhlAyMc45554044 = -313030817;    double etWzZXiphPmhlAyMc99873146 = -748026793;    double etWzZXiphPmhlAyMc95350193 = -189039327;    double etWzZXiphPmhlAyMc96618019 = -370171566;    double etWzZXiphPmhlAyMc16360765 = -353181297;    double etWzZXiphPmhlAyMc47433495 = -44091028;    double etWzZXiphPmhlAyMc24891876 = -164129455;    double etWzZXiphPmhlAyMc40840862 = -472185095;    double etWzZXiphPmhlAyMc18317688 = -387080829;    double etWzZXiphPmhlAyMc96869730 = 41399481;    double etWzZXiphPmhlAyMc58537137 = 96373785;    double etWzZXiphPmhlAyMc67612504 = -770917734;    double etWzZXiphPmhlAyMc92117003 = -421244915;    double etWzZXiphPmhlAyMc34069475 = -254893932;    double etWzZXiphPmhlAyMc19117731 = -472363137;    double etWzZXiphPmhlAyMc15020461 = -649146247;    double etWzZXiphPmhlAyMc64640942 = -570211142;    double etWzZXiphPmhlAyMc85944765 = 67008732;    double etWzZXiphPmhlAyMc36635502 = -944683144;    double etWzZXiphPmhlAyMc66787711 = -705915003;    double etWzZXiphPmhlAyMc49921106 = -261048302;    double etWzZXiphPmhlAyMc34896812 = -495007166;    double etWzZXiphPmhlAyMc50637744 = -892165297;    double etWzZXiphPmhlAyMc3102913 = -494292614;    double etWzZXiphPmhlAyMc19832936 = -504215343;    double etWzZXiphPmhlAyMc54505082 = -736180756;    double etWzZXiphPmhlAyMc11977109 = -253858169;    double etWzZXiphPmhlAyMc50277061 = -162713789;    double etWzZXiphPmhlAyMc38010071 = -965143129;    double etWzZXiphPmhlAyMc14016583 = -668932459;    double etWzZXiphPmhlAyMc98112983 = 62222868;    double etWzZXiphPmhlAyMc20758422 = 5158767;    double etWzZXiphPmhlAyMc19279482 = -945676646;    double etWzZXiphPmhlAyMc74407272 = -846636165;    double etWzZXiphPmhlAyMc99376654 = -487439331;    double etWzZXiphPmhlAyMc61640900 = 802441;    double etWzZXiphPmhlAyMc41642087 = -671794179;    double etWzZXiphPmhlAyMc36797475 = -138854334;    double etWzZXiphPmhlAyMc35841760 = -482688735;    double etWzZXiphPmhlAyMc34461541 = -863913671;    double etWzZXiphPmhlAyMc5652738 = 56368671;    double etWzZXiphPmhlAyMc78093091 = -751200742;    double etWzZXiphPmhlAyMc23909924 = -486248749;    double etWzZXiphPmhlAyMc22228002 = -854427140;    double etWzZXiphPmhlAyMc63469730 = -20460144;    double etWzZXiphPmhlAyMc10784768 = 7378212;    double etWzZXiphPmhlAyMc40616387 = 64587778;    double etWzZXiphPmhlAyMc8416519 = 15030237;    double etWzZXiphPmhlAyMc14733961 = -713790020;    double etWzZXiphPmhlAyMc32180074 = -261797054;    double etWzZXiphPmhlAyMc14744283 = 11628299;    double etWzZXiphPmhlAyMc26091237 = -139123434;    double etWzZXiphPmhlAyMc50157134 = -839619855;    double etWzZXiphPmhlAyMc16854898 = 64098807;    double etWzZXiphPmhlAyMc66500695 = -356350532;    double etWzZXiphPmhlAyMc10657233 = -818023651;    double etWzZXiphPmhlAyMc49235402 = -855861496;    double etWzZXiphPmhlAyMc92247280 = -694746714;    double etWzZXiphPmhlAyMc76785084 = -865956223;    double etWzZXiphPmhlAyMc61855683 = -617000541;    double etWzZXiphPmhlAyMc35456386 = -790232859;    double etWzZXiphPmhlAyMc74614814 = 98584333;    double etWzZXiphPmhlAyMc2830791 = -507041966;    double etWzZXiphPmhlAyMc4301106 = -718148371;    double etWzZXiphPmhlAyMc98756746 = 79176612;    double etWzZXiphPmhlAyMc37778715 = -908784982;    double etWzZXiphPmhlAyMc48333023 = -825241089;    double etWzZXiphPmhlAyMc17709732 = -574608750;    double etWzZXiphPmhlAyMc34692821 = -767454602;    double etWzZXiphPmhlAyMc57476830 = -373165578;    double etWzZXiphPmhlAyMc73378373 = -977352069;    double etWzZXiphPmhlAyMc27843467 = -331356808;    double etWzZXiphPmhlAyMc50103005 = -450302534;    double etWzZXiphPmhlAyMc2173961 = 19230526;    double etWzZXiphPmhlAyMc61134973 = -662283674;    double etWzZXiphPmhlAyMc71828015 = -509847561;    double etWzZXiphPmhlAyMc10986889 = 91241583;    double etWzZXiphPmhlAyMc28409742 = 62261842;    double etWzZXiphPmhlAyMc39633182 = -373832470;    double etWzZXiphPmhlAyMc9048169 = -411593555;    double etWzZXiphPmhlAyMc13888696 = -700768534;    double etWzZXiphPmhlAyMc3560591 = -168888407;    double etWzZXiphPmhlAyMc35543100 = -448923769;    double etWzZXiphPmhlAyMc5829998 = -603346076;    double etWzZXiphPmhlAyMc99272300 = -580560759;    double etWzZXiphPmhlAyMc72021747 = -798653698;    double etWzZXiphPmhlAyMc70601287 = -155221379;    double etWzZXiphPmhlAyMc2424584 = -909775454;    double etWzZXiphPmhlAyMc7906577 = -390285634;    double etWzZXiphPmhlAyMc88719422 = -669415680;    double etWzZXiphPmhlAyMc12405498 = -143336064;    double etWzZXiphPmhlAyMc49394806 = -977047466;    double etWzZXiphPmhlAyMc60012391 = -186153156;     etWzZXiphPmhlAyMc33851692 = etWzZXiphPmhlAyMc47200534;     etWzZXiphPmhlAyMc47200534 = etWzZXiphPmhlAyMc79385225;     etWzZXiphPmhlAyMc79385225 = etWzZXiphPmhlAyMc12036002;     etWzZXiphPmhlAyMc12036002 = etWzZXiphPmhlAyMc86792636;     etWzZXiphPmhlAyMc86792636 = etWzZXiphPmhlAyMc83642608;     etWzZXiphPmhlAyMc83642608 = etWzZXiphPmhlAyMc16421802;     etWzZXiphPmhlAyMc16421802 = etWzZXiphPmhlAyMc45554044;     etWzZXiphPmhlAyMc45554044 = etWzZXiphPmhlAyMc99873146;     etWzZXiphPmhlAyMc99873146 = etWzZXiphPmhlAyMc95350193;     etWzZXiphPmhlAyMc95350193 = etWzZXiphPmhlAyMc96618019;     etWzZXiphPmhlAyMc96618019 = etWzZXiphPmhlAyMc16360765;     etWzZXiphPmhlAyMc16360765 = etWzZXiphPmhlAyMc47433495;     etWzZXiphPmhlAyMc47433495 = etWzZXiphPmhlAyMc24891876;     etWzZXiphPmhlAyMc24891876 = etWzZXiphPmhlAyMc40840862;     etWzZXiphPmhlAyMc40840862 = etWzZXiphPmhlAyMc18317688;     etWzZXiphPmhlAyMc18317688 = etWzZXiphPmhlAyMc96869730;     etWzZXiphPmhlAyMc96869730 = etWzZXiphPmhlAyMc58537137;     etWzZXiphPmhlAyMc58537137 = etWzZXiphPmhlAyMc67612504;     etWzZXiphPmhlAyMc67612504 = etWzZXiphPmhlAyMc92117003;     etWzZXiphPmhlAyMc92117003 = etWzZXiphPmhlAyMc34069475;     etWzZXiphPmhlAyMc34069475 = etWzZXiphPmhlAyMc19117731;     etWzZXiphPmhlAyMc19117731 = etWzZXiphPmhlAyMc15020461;     etWzZXiphPmhlAyMc15020461 = etWzZXiphPmhlAyMc64640942;     etWzZXiphPmhlAyMc64640942 = etWzZXiphPmhlAyMc85944765;     etWzZXiphPmhlAyMc85944765 = etWzZXiphPmhlAyMc36635502;     etWzZXiphPmhlAyMc36635502 = etWzZXiphPmhlAyMc66787711;     etWzZXiphPmhlAyMc66787711 = etWzZXiphPmhlAyMc49921106;     etWzZXiphPmhlAyMc49921106 = etWzZXiphPmhlAyMc34896812;     etWzZXiphPmhlAyMc34896812 = etWzZXiphPmhlAyMc50637744;     etWzZXiphPmhlAyMc50637744 = etWzZXiphPmhlAyMc3102913;     etWzZXiphPmhlAyMc3102913 = etWzZXiphPmhlAyMc19832936;     etWzZXiphPmhlAyMc19832936 = etWzZXiphPmhlAyMc54505082;     etWzZXiphPmhlAyMc54505082 = etWzZXiphPmhlAyMc11977109;     etWzZXiphPmhlAyMc11977109 = etWzZXiphPmhlAyMc50277061;     etWzZXiphPmhlAyMc50277061 = etWzZXiphPmhlAyMc38010071;     etWzZXiphPmhlAyMc38010071 = etWzZXiphPmhlAyMc14016583;     etWzZXiphPmhlAyMc14016583 = etWzZXiphPmhlAyMc98112983;     etWzZXiphPmhlAyMc98112983 = etWzZXiphPmhlAyMc20758422;     etWzZXiphPmhlAyMc20758422 = etWzZXiphPmhlAyMc19279482;     etWzZXiphPmhlAyMc19279482 = etWzZXiphPmhlAyMc74407272;     etWzZXiphPmhlAyMc74407272 = etWzZXiphPmhlAyMc99376654;     etWzZXiphPmhlAyMc99376654 = etWzZXiphPmhlAyMc61640900;     etWzZXiphPmhlAyMc61640900 = etWzZXiphPmhlAyMc41642087;     etWzZXiphPmhlAyMc41642087 = etWzZXiphPmhlAyMc36797475;     etWzZXiphPmhlAyMc36797475 = etWzZXiphPmhlAyMc35841760;     etWzZXiphPmhlAyMc35841760 = etWzZXiphPmhlAyMc34461541;     etWzZXiphPmhlAyMc34461541 = etWzZXiphPmhlAyMc5652738;     etWzZXiphPmhlAyMc5652738 = etWzZXiphPmhlAyMc78093091;     etWzZXiphPmhlAyMc78093091 = etWzZXiphPmhlAyMc23909924;     etWzZXiphPmhlAyMc23909924 = etWzZXiphPmhlAyMc22228002;     etWzZXiphPmhlAyMc22228002 = etWzZXiphPmhlAyMc63469730;     etWzZXiphPmhlAyMc63469730 = etWzZXiphPmhlAyMc10784768;     etWzZXiphPmhlAyMc10784768 = etWzZXiphPmhlAyMc40616387;     etWzZXiphPmhlAyMc40616387 = etWzZXiphPmhlAyMc8416519;     etWzZXiphPmhlAyMc8416519 = etWzZXiphPmhlAyMc14733961;     etWzZXiphPmhlAyMc14733961 = etWzZXiphPmhlAyMc32180074;     etWzZXiphPmhlAyMc32180074 = etWzZXiphPmhlAyMc14744283;     etWzZXiphPmhlAyMc14744283 = etWzZXiphPmhlAyMc26091237;     etWzZXiphPmhlAyMc26091237 = etWzZXiphPmhlAyMc50157134;     etWzZXiphPmhlAyMc50157134 = etWzZXiphPmhlAyMc16854898;     etWzZXiphPmhlAyMc16854898 = etWzZXiphPmhlAyMc66500695;     etWzZXiphPmhlAyMc66500695 = etWzZXiphPmhlAyMc10657233;     etWzZXiphPmhlAyMc10657233 = etWzZXiphPmhlAyMc49235402;     etWzZXiphPmhlAyMc49235402 = etWzZXiphPmhlAyMc92247280;     etWzZXiphPmhlAyMc92247280 = etWzZXiphPmhlAyMc76785084;     etWzZXiphPmhlAyMc76785084 = etWzZXiphPmhlAyMc61855683;     etWzZXiphPmhlAyMc61855683 = etWzZXiphPmhlAyMc35456386;     etWzZXiphPmhlAyMc35456386 = etWzZXiphPmhlAyMc74614814;     etWzZXiphPmhlAyMc74614814 = etWzZXiphPmhlAyMc2830791;     etWzZXiphPmhlAyMc2830791 = etWzZXiphPmhlAyMc4301106;     etWzZXiphPmhlAyMc4301106 = etWzZXiphPmhlAyMc98756746;     etWzZXiphPmhlAyMc98756746 = etWzZXiphPmhlAyMc37778715;     etWzZXiphPmhlAyMc37778715 = etWzZXiphPmhlAyMc48333023;     etWzZXiphPmhlAyMc48333023 = etWzZXiphPmhlAyMc17709732;     etWzZXiphPmhlAyMc17709732 = etWzZXiphPmhlAyMc34692821;     etWzZXiphPmhlAyMc34692821 = etWzZXiphPmhlAyMc57476830;     etWzZXiphPmhlAyMc57476830 = etWzZXiphPmhlAyMc73378373;     etWzZXiphPmhlAyMc73378373 = etWzZXiphPmhlAyMc27843467;     etWzZXiphPmhlAyMc27843467 = etWzZXiphPmhlAyMc50103005;     etWzZXiphPmhlAyMc50103005 = etWzZXiphPmhlAyMc2173961;     etWzZXiphPmhlAyMc2173961 = etWzZXiphPmhlAyMc61134973;     etWzZXiphPmhlAyMc61134973 = etWzZXiphPmhlAyMc71828015;     etWzZXiphPmhlAyMc71828015 = etWzZXiphPmhlAyMc10986889;     etWzZXiphPmhlAyMc10986889 = etWzZXiphPmhlAyMc28409742;     etWzZXiphPmhlAyMc28409742 = etWzZXiphPmhlAyMc39633182;     etWzZXiphPmhlAyMc39633182 = etWzZXiphPmhlAyMc9048169;     etWzZXiphPmhlAyMc9048169 = etWzZXiphPmhlAyMc13888696;     etWzZXiphPmhlAyMc13888696 = etWzZXiphPmhlAyMc3560591;     etWzZXiphPmhlAyMc3560591 = etWzZXiphPmhlAyMc35543100;     etWzZXiphPmhlAyMc35543100 = etWzZXiphPmhlAyMc5829998;     etWzZXiphPmhlAyMc5829998 = etWzZXiphPmhlAyMc99272300;     etWzZXiphPmhlAyMc99272300 = etWzZXiphPmhlAyMc72021747;     etWzZXiphPmhlAyMc72021747 = etWzZXiphPmhlAyMc70601287;     etWzZXiphPmhlAyMc70601287 = etWzZXiphPmhlAyMc2424584;     etWzZXiphPmhlAyMc2424584 = etWzZXiphPmhlAyMc7906577;     etWzZXiphPmhlAyMc7906577 = etWzZXiphPmhlAyMc88719422;     etWzZXiphPmhlAyMc88719422 = etWzZXiphPmhlAyMc12405498;     etWzZXiphPmhlAyMc12405498 = etWzZXiphPmhlAyMc49394806;     etWzZXiphPmhlAyMc49394806 = etWzZXiphPmhlAyMc60012391;     etWzZXiphPmhlAyMc60012391 = etWzZXiphPmhlAyMc33851692;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GneqPNqDbDzlCxEr57535061() {     double LeQoXWFKEIohRVILD39968799 = -298055045;    double LeQoXWFKEIohRVILD90573813 = -720543081;    double LeQoXWFKEIohRVILD95427408 = -809415763;    double LeQoXWFKEIohRVILD39110782 = -362863332;    double LeQoXWFKEIohRVILD55688124 = -204952870;    double LeQoXWFKEIohRVILD75059810 = 93687570;    double LeQoXWFKEIohRVILD76910767 = -856363810;    double LeQoXWFKEIohRVILD95313935 = -599904730;    double LeQoXWFKEIohRVILD64515641 = -516059459;    double LeQoXWFKEIohRVILD36579852 = -518014805;    double LeQoXWFKEIohRVILD56728310 = -247609152;    double LeQoXWFKEIohRVILD48183807 = -797587499;    double LeQoXWFKEIohRVILD33642614 = -483982117;    double LeQoXWFKEIohRVILD8035553 = -696368023;    double LeQoXWFKEIohRVILD36583289 = -62647261;    double LeQoXWFKEIohRVILD59823082 = -360080373;    double LeQoXWFKEIohRVILD90705733 = -575145631;    double LeQoXWFKEIohRVILD55590290 = -669148858;    double LeQoXWFKEIohRVILD29442347 = -76686831;    double LeQoXWFKEIohRVILD72667168 = -778147834;    double LeQoXWFKEIohRVILD22622668 = -110381388;    double LeQoXWFKEIohRVILD5638755 = -380292853;    double LeQoXWFKEIohRVILD50611252 = -395663734;    double LeQoXWFKEIohRVILD48633316 = -792516130;    double LeQoXWFKEIohRVILD29644415 = -306187301;    double LeQoXWFKEIohRVILD61357177 = -823338467;    double LeQoXWFKEIohRVILD65247489 = -606577206;    double LeQoXWFKEIohRVILD63028231 = -326977255;    double LeQoXWFKEIohRVILD1029068 = -717389870;    double LeQoXWFKEIohRVILD19253381 = -709636165;    double LeQoXWFKEIohRVILD95345461 = -407613552;    double LeQoXWFKEIohRVILD17443558 = -213662159;    double LeQoXWFKEIohRVILD64216030 = -628794126;    double LeQoXWFKEIohRVILD6807673 = -5399935;    double LeQoXWFKEIohRVILD11751714 = -801446894;    double LeQoXWFKEIohRVILD6735985 = -79169237;    double LeQoXWFKEIohRVILD95278517 = -622568329;    double LeQoXWFKEIohRVILD32203196 = -809283141;    double LeQoXWFKEIohRVILD94368675 = 11235270;    double LeQoXWFKEIohRVILD6781419 = -319115869;    double LeQoXWFKEIohRVILD80076496 = -911910059;    double LeQoXWFKEIohRVILD31527111 = -757581609;    double LeQoXWFKEIohRVILD39707879 = -80002143;    double LeQoXWFKEIohRVILD29049827 = -691137117;    double LeQoXWFKEIohRVILD48749940 = -900752488;    double LeQoXWFKEIohRVILD47780955 = -467904852;    double LeQoXWFKEIohRVILD52240115 = -982689208;    double LeQoXWFKEIohRVILD13609679 = 46473139;    double LeQoXWFKEIohRVILD87768819 = -836937730;    double LeQoXWFKEIohRVILD36821576 = -573259557;    double LeQoXWFKEIohRVILD57928680 = -696720811;    double LeQoXWFKEIohRVILD74279449 = -921186465;    double LeQoXWFKEIohRVILD59033225 = -884411376;    double LeQoXWFKEIohRVILD36585267 = -255882367;    double LeQoXWFKEIohRVILD34912393 = -685501546;    double LeQoXWFKEIohRVILD34330044 = -917762192;    double LeQoXWFKEIohRVILD39962562 = -224879348;    double LeQoXWFKEIohRVILD46794092 = 83100366;    double LeQoXWFKEIohRVILD9466368 = 43323969;    double LeQoXWFKEIohRVILD94330947 = -381614403;    double LeQoXWFKEIohRVILD9812322 = -299735225;    double LeQoXWFKEIohRVILD13882536 = -429386555;    double LeQoXWFKEIohRVILD94284867 = -882514860;    double LeQoXWFKEIohRVILD45262261 = -806423294;    double LeQoXWFKEIohRVILD41234391 = -10401253;    double LeQoXWFKEIohRVILD39284752 = 66053006;    double LeQoXWFKEIohRVILD83967777 = -68793373;    double LeQoXWFKEIohRVILD26834941 = -378582183;    double LeQoXWFKEIohRVILD96283839 = -894921129;    double LeQoXWFKEIohRVILD29847305 = -983478025;    double LeQoXWFKEIohRVILD64544564 = -737512044;    double LeQoXWFKEIohRVILD58502537 = -765862491;    double LeQoXWFKEIohRVILD61221614 = -580384128;    double LeQoXWFKEIohRVILD22660928 = -757570963;    double LeQoXWFKEIohRVILD92590672 = -866237775;    double LeQoXWFKEIohRVILD91095557 = -352799780;    double LeQoXWFKEIohRVILD65930876 = -200290710;    double LeQoXWFKEIohRVILD21561425 = -704526617;    double LeQoXWFKEIohRVILD99883375 = -891763642;    double LeQoXWFKEIohRVILD81863460 = -838282450;    double LeQoXWFKEIohRVILD9117062 = -840649259;    double LeQoXWFKEIohRVILD51637810 = -553050345;    double LeQoXWFKEIohRVILD75259411 = -490039526;    double LeQoXWFKEIohRVILD64207491 = -44130314;    double LeQoXWFKEIohRVILD61324700 = 87084646;    double LeQoXWFKEIohRVILD21066012 = -486427088;    double LeQoXWFKEIohRVILD58410333 = -329250783;    double LeQoXWFKEIohRVILD27630764 = -272911760;    double LeQoXWFKEIohRVILD71895280 = -319898390;    double LeQoXWFKEIohRVILD77421669 = -883684702;    double LeQoXWFKEIohRVILD66773423 = -854289889;    double LeQoXWFKEIohRVILD48484425 = -605668695;    double LeQoXWFKEIohRVILD22736829 = -752607110;    double LeQoXWFKEIohRVILD37729 = -607150328;    double LeQoXWFKEIohRVILD96969097 = 80619356;    double LeQoXWFKEIohRVILD66193960 = -382523505;    double LeQoXWFKEIohRVILD37242244 = -875066749;    double LeQoXWFKEIohRVILD94445617 = -273578849;    double LeQoXWFKEIohRVILD87815436 = -580735865;    double LeQoXWFKEIohRVILD9465188 = -298055045;     LeQoXWFKEIohRVILD39968799 = LeQoXWFKEIohRVILD90573813;     LeQoXWFKEIohRVILD90573813 = LeQoXWFKEIohRVILD95427408;     LeQoXWFKEIohRVILD95427408 = LeQoXWFKEIohRVILD39110782;     LeQoXWFKEIohRVILD39110782 = LeQoXWFKEIohRVILD55688124;     LeQoXWFKEIohRVILD55688124 = LeQoXWFKEIohRVILD75059810;     LeQoXWFKEIohRVILD75059810 = LeQoXWFKEIohRVILD76910767;     LeQoXWFKEIohRVILD76910767 = LeQoXWFKEIohRVILD95313935;     LeQoXWFKEIohRVILD95313935 = LeQoXWFKEIohRVILD64515641;     LeQoXWFKEIohRVILD64515641 = LeQoXWFKEIohRVILD36579852;     LeQoXWFKEIohRVILD36579852 = LeQoXWFKEIohRVILD56728310;     LeQoXWFKEIohRVILD56728310 = LeQoXWFKEIohRVILD48183807;     LeQoXWFKEIohRVILD48183807 = LeQoXWFKEIohRVILD33642614;     LeQoXWFKEIohRVILD33642614 = LeQoXWFKEIohRVILD8035553;     LeQoXWFKEIohRVILD8035553 = LeQoXWFKEIohRVILD36583289;     LeQoXWFKEIohRVILD36583289 = LeQoXWFKEIohRVILD59823082;     LeQoXWFKEIohRVILD59823082 = LeQoXWFKEIohRVILD90705733;     LeQoXWFKEIohRVILD90705733 = LeQoXWFKEIohRVILD55590290;     LeQoXWFKEIohRVILD55590290 = LeQoXWFKEIohRVILD29442347;     LeQoXWFKEIohRVILD29442347 = LeQoXWFKEIohRVILD72667168;     LeQoXWFKEIohRVILD72667168 = LeQoXWFKEIohRVILD22622668;     LeQoXWFKEIohRVILD22622668 = LeQoXWFKEIohRVILD5638755;     LeQoXWFKEIohRVILD5638755 = LeQoXWFKEIohRVILD50611252;     LeQoXWFKEIohRVILD50611252 = LeQoXWFKEIohRVILD48633316;     LeQoXWFKEIohRVILD48633316 = LeQoXWFKEIohRVILD29644415;     LeQoXWFKEIohRVILD29644415 = LeQoXWFKEIohRVILD61357177;     LeQoXWFKEIohRVILD61357177 = LeQoXWFKEIohRVILD65247489;     LeQoXWFKEIohRVILD65247489 = LeQoXWFKEIohRVILD63028231;     LeQoXWFKEIohRVILD63028231 = LeQoXWFKEIohRVILD1029068;     LeQoXWFKEIohRVILD1029068 = LeQoXWFKEIohRVILD19253381;     LeQoXWFKEIohRVILD19253381 = LeQoXWFKEIohRVILD95345461;     LeQoXWFKEIohRVILD95345461 = LeQoXWFKEIohRVILD17443558;     LeQoXWFKEIohRVILD17443558 = LeQoXWFKEIohRVILD64216030;     LeQoXWFKEIohRVILD64216030 = LeQoXWFKEIohRVILD6807673;     LeQoXWFKEIohRVILD6807673 = LeQoXWFKEIohRVILD11751714;     LeQoXWFKEIohRVILD11751714 = LeQoXWFKEIohRVILD6735985;     LeQoXWFKEIohRVILD6735985 = LeQoXWFKEIohRVILD95278517;     LeQoXWFKEIohRVILD95278517 = LeQoXWFKEIohRVILD32203196;     LeQoXWFKEIohRVILD32203196 = LeQoXWFKEIohRVILD94368675;     LeQoXWFKEIohRVILD94368675 = LeQoXWFKEIohRVILD6781419;     LeQoXWFKEIohRVILD6781419 = LeQoXWFKEIohRVILD80076496;     LeQoXWFKEIohRVILD80076496 = LeQoXWFKEIohRVILD31527111;     LeQoXWFKEIohRVILD31527111 = LeQoXWFKEIohRVILD39707879;     LeQoXWFKEIohRVILD39707879 = LeQoXWFKEIohRVILD29049827;     LeQoXWFKEIohRVILD29049827 = LeQoXWFKEIohRVILD48749940;     LeQoXWFKEIohRVILD48749940 = LeQoXWFKEIohRVILD47780955;     LeQoXWFKEIohRVILD47780955 = LeQoXWFKEIohRVILD52240115;     LeQoXWFKEIohRVILD52240115 = LeQoXWFKEIohRVILD13609679;     LeQoXWFKEIohRVILD13609679 = LeQoXWFKEIohRVILD87768819;     LeQoXWFKEIohRVILD87768819 = LeQoXWFKEIohRVILD36821576;     LeQoXWFKEIohRVILD36821576 = LeQoXWFKEIohRVILD57928680;     LeQoXWFKEIohRVILD57928680 = LeQoXWFKEIohRVILD74279449;     LeQoXWFKEIohRVILD74279449 = LeQoXWFKEIohRVILD59033225;     LeQoXWFKEIohRVILD59033225 = LeQoXWFKEIohRVILD36585267;     LeQoXWFKEIohRVILD36585267 = LeQoXWFKEIohRVILD34912393;     LeQoXWFKEIohRVILD34912393 = LeQoXWFKEIohRVILD34330044;     LeQoXWFKEIohRVILD34330044 = LeQoXWFKEIohRVILD39962562;     LeQoXWFKEIohRVILD39962562 = LeQoXWFKEIohRVILD46794092;     LeQoXWFKEIohRVILD46794092 = LeQoXWFKEIohRVILD9466368;     LeQoXWFKEIohRVILD9466368 = LeQoXWFKEIohRVILD94330947;     LeQoXWFKEIohRVILD94330947 = LeQoXWFKEIohRVILD9812322;     LeQoXWFKEIohRVILD9812322 = LeQoXWFKEIohRVILD13882536;     LeQoXWFKEIohRVILD13882536 = LeQoXWFKEIohRVILD94284867;     LeQoXWFKEIohRVILD94284867 = LeQoXWFKEIohRVILD45262261;     LeQoXWFKEIohRVILD45262261 = LeQoXWFKEIohRVILD41234391;     LeQoXWFKEIohRVILD41234391 = LeQoXWFKEIohRVILD39284752;     LeQoXWFKEIohRVILD39284752 = LeQoXWFKEIohRVILD83967777;     LeQoXWFKEIohRVILD83967777 = LeQoXWFKEIohRVILD26834941;     LeQoXWFKEIohRVILD26834941 = LeQoXWFKEIohRVILD96283839;     LeQoXWFKEIohRVILD96283839 = LeQoXWFKEIohRVILD29847305;     LeQoXWFKEIohRVILD29847305 = LeQoXWFKEIohRVILD64544564;     LeQoXWFKEIohRVILD64544564 = LeQoXWFKEIohRVILD58502537;     LeQoXWFKEIohRVILD58502537 = LeQoXWFKEIohRVILD61221614;     LeQoXWFKEIohRVILD61221614 = LeQoXWFKEIohRVILD22660928;     LeQoXWFKEIohRVILD22660928 = LeQoXWFKEIohRVILD92590672;     LeQoXWFKEIohRVILD92590672 = LeQoXWFKEIohRVILD91095557;     LeQoXWFKEIohRVILD91095557 = LeQoXWFKEIohRVILD65930876;     LeQoXWFKEIohRVILD65930876 = LeQoXWFKEIohRVILD21561425;     LeQoXWFKEIohRVILD21561425 = LeQoXWFKEIohRVILD99883375;     LeQoXWFKEIohRVILD99883375 = LeQoXWFKEIohRVILD81863460;     LeQoXWFKEIohRVILD81863460 = LeQoXWFKEIohRVILD9117062;     LeQoXWFKEIohRVILD9117062 = LeQoXWFKEIohRVILD51637810;     LeQoXWFKEIohRVILD51637810 = LeQoXWFKEIohRVILD75259411;     LeQoXWFKEIohRVILD75259411 = LeQoXWFKEIohRVILD64207491;     LeQoXWFKEIohRVILD64207491 = LeQoXWFKEIohRVILD61324700;     LeQoXWFKEIohRVILD61324700 = LeQoXWFKEIohRVILD21066012;     LeQoXWFKEIohRVILD21066012 = LeQoXWFKEIohRVILD58410333;     LeQoXWFKEIohRVILD58410333 = LeQoXWFKEIohRVILD27630764;     LeQoXWFKEIohRVILD27630764 = LeQoXWFKEIohRVILD71895280;     LeQoXWFKEIohRVILD71895280 = LeQoXWFKEIohRVILD77421669;     LeQoXWFKEIohRVILD77421669 = LeQoXWFKEIohRVILD66773423;     LeQoXWFKEIohRVILD66773423 = LeQoXWFKEIohRVILD48484425;     LeQoXWFKEIohRVILD48484425 = LeQoXWFKEIohRVILD22736829;     LeQoXWFKEIohRVILD22736829 = LeQoXWFKEIohRVILD37729;     LeQoXWFKEIohRVILD37729 = LeQoXWFKEIohRVILD96969097;     LeQoXWFKEIohRVILD96969097 = LeQoXWFKEIohRVILD66193960;     LeQoXWFKEIohRVILD66193960 = LeQoXWFKEIohRVILD37242244;     LeQoXWFKEIohRVILD37242244 = LeQoXWFKEIohRVILD94445617;     LeQoXWFKEIohRVILD94445617 = LeQoXWFKEIohRVILD87815436;     LeQoXWFKEIohRVILD87815436 = LeQoXWFKEIohRVILD9465188;     LeQoXWFKEIohRVILD9465188 = LeQoXWFKEIohRVILD39968799;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void cOAfichryqHZRNPY24826661() {     double OQLsDECFlOETTmMNt75427767 = -764331944;    double OQLsDECFlOETTmMNt3845892 = -601094688;    double OQLsDECFlOETTmMNt32813885 = -475082838;    double OQLsDECFlOETTmMNt11334137 = -184014404;    double OQLsDECFlOETTmMNt11973394 = -310551715;    double OQLsDECFlOETTmMNt11282683 = -168660325;    double OQLsDECFlOETTmMNt87568173 = -701089544;    double OQLsDECFlOETTmMNt57445496 = -244138870;    double OQLsDECFlOETTmMNt66033247 = 95609134;    double OQLsDECFlOETTmMNt51482564 = -182701803;    double OQLsDECFlOETTmMNt94910044 = -49616881;    double OQLsDECFlOETTmMNt48751956 = -661708270;    double OQLsDECFlOETTmMNt71877259 = 64005378;    double OQLsDECFlOETTmMNt61176415 = -672731432;    double OQLsDECFlOETTmMNt94777805 = -566072276;    double OQLsDECFlOETTmMNt43364383 = 4108020;    double OQLsDECFlOETTmMNt46541669 = -2120184;    double OQLsDECFlOETTmMNt55350481 = -742576926;    double OQLsDECFlOETTmMNt29992676 = -393250253;    double OQLsDECFlOETTmMNt66504387 = -104708900;    double OQLsDECFlOETTmMNt35709531 = -867656076;    double OQLsDECFlOETTmMNt9060280 = -466929147;    double OQLsDECFlOETTmMNt21136632 = -33295918;    double OQLsDECFlOETTmMNt42426261 = -203340734;    double OQLsDECFlOETTmMNt37662366 = -330172500;    double OQLsDECFlOETTmMNt72055349 = -511915274;    double OQLsDECFlOETTmMNt25668063 = -412861346;    double OQLsDECFlOETTmMNt3756657 = -496749430;    double OQLsDECFlOETTmMNt47697282 = -191484673;    double OQLsDECFlOETTmMNt24786792 = -744143335;    double OQLsDECFlOETTmMNt13928694 = -959594039;    double OQLsDECFlOETTmMNt55266576 = 13366427;    double OQLsDECFlOETTmMNt76434041 = -776390104;    double OQLsDECFlOETTmMNt36556100 = -70093748;    double OQLsDECFlOETTmMNt77560167 = -752547145;    double OQLsDECFlOETTmMNt58296828 = -293734543;    double OQLsDECFlOETTmMNt44690334 = -487264369;    double OQLsDECFlOETTmMNt83418599 = -791938722;    double OQLsDECFlOETTmMNt24759237 = -739641006;    double OQLsDECFlOETTmMNt71052951 = -884378415;    double OQLsDECFlOETTmMNt13528104 = -681108974;    double OQLsDECFlOETTmMNt20892238 = -352126900;    double OQLsDECFlOETTmMNt29091257 = -733182929;    double OQLsDECFlOETTmMNt69181165 = -195469527;    double OQLsDECFlOETTmMNt48766399 = -727263650;    double OQLsDECFlOETTmMNt93742283 = -506828484;    double OQLsDECFlOETTmMNt71585278 = -366675569;    double OQLsDECFlOETTmMNt31335991 = -120053455;    double OQLsDECFlOETTmMNt10661486 = -500674610;    double OQLsDECFlOETTmMNt61351166 = 35713627;    double OQLsDECFlOETTmMNt38699404 = -570997368;    double OQLsDECFlOETTmMNt57764922 = -34838811;    double OQLsDECFlOETTmMNt27654374 = -640456930;    double OQLsDECFlOETTmMNt52577113 = -447390867;    double OQLsDECFlOETTmMNt89117129 = 68008309;    double OQLsDECFlOETTmMNt66367488 = -197402798;    double OQLsDECFlOETTmMNt82709260 = -467798770;    double OQLsDECFlOETTmMNt90387624 = -171742105;    double OQLsDECFlOETTmMNt73671771 = -853841904;    double OQLsDECFlOETTmMNt39918045 = -798636441;    double OQLsDECFlOETTmMNt85614619 = -755798979;    double OQLsDECFlOETTmMNt83811517 = -104340115;    double OQLsDECFlOETTmMNt9748214 = 47345802;    double OQLsDECFlOETTmMNt41246455 = -160247531;    double OQLsDECFlOETTmMNt37553871 = -223107764;    double OQLsDECFlOETTmMNt39643469 = 37016691;    double OQLsDECFlOETTmMNt72317914 = -885318167;    double OQLsDECFlOETTmMNt35321160 = -865900874;    double OQLsDECFlOETTmMNt83616248 = -920184288;    double OQLsDECFlOETTmMNt36480977 = -172337734;    double OQLsDECFlOETTmMNt98674048 = -508627611;    double OQLsDECFlOETTmMNt63123070 = -210181462;    double OQLsDECFlOETTmMNt30591244 = 97064079;    double OQLsDECFlOETTmMNt58939725 = -508871838;    double OQLsDECFlOETTmMNt52976283 = -423599927;    double OQLsDECFlOETTmMNt14817293 = -415529177;    double OQLsDECFlOETTmMNt79969022 = -733746218;    double OQLsDECFlOETTmMNt51955466 = -837826392;    double OQLsDECFlOETTmMNt93659862 = -476077085;    double OQLsDECFlOETTmMNt43920083 = -823344016;    double OQLsDECFlOETTmMNt470071 = -45239706;    double OQLsDECFlOETTmMNt94332072 = -192807892;    double OQLsDECFlOETTmMNt93095170 = -996074820;    double OQLsDECFlOETTmMNt86346116 = -127198300;    double OQLsDECFlOETTmMNt86087387 = -73145967;    double OQLsDECFlOETTmMNt56163771 = -824755229;    double OQLsDECFlOETTmMNt27612203 = -346176644;    double OQLsDECFlOETTmMNt23856928 = -228999237;    double OQLsDECFlOETTmMNt47438970 = -38102057;    double OQLsDECFlOETTmMNt11192679 = -455144347;    double OQLsDECFlOETTmMNt75587568 = -825935773;    double OQLsDECFlOETTmMNt54302710 = -215522264;    double OQLsDECFlOETTmMNt9746829 = -938096818;    double OQLsDECFlOETTmMNt84841192 = -941004565;    double OQLsDECFlOETTmMNt85438331 = -28579436;    double OQLsDECFlOETTmMNt29716586 = -476768859;    double OQLsDECFlOETTmMNt11144024 = -299472702;    double OQLsDECFlOETTmMNt87844801 = -472935399;    double OQLsDECFlOETTmMNt31627294 = -972361764;    double OQLsDECFlOETTmMNt9122931 = -764331944;     OQLsDECFlOETTmMNt75427767 = OQLsDECFlOETTmMNt3845892;     OQLsDECFlOETTmMNt3845892 = OQLsDECFlOETTmMNt32813885;     OQLsDECFlOETTmMNt32813885 = OQLsDECFlOETTmMNt11334137;     OQLsDECFlOETTmMNt11334137 = OQLsDECFlOETTmMNt11973394;     OQLsDECFlOETTmMNt11973394 = OQLsDECFlOETTmMNt11282683;     OQLsDECFlOETTmMNt11282683 = OQLsDECFlOETTmMNt87568173;     OQLsDECFlOETTmMNt87568173 = OQLsDECFlOETTmMNt57445496;     OQLsDECFlOETTmMNt57445496 = OQLsDECFlOETTmMNt66033247;     OQLsDECFlOETTmMNt66033247 = OQLsDECFlOETTmMNt51482564;     OQLsDECFlOETTmMNt51482564 = OQLsDECFlOETTmMNt94910044;     OQLsDECFlOETTmMNt94910044 = OQLsDECFlOETTmMNt48751956;     OQLsDECFlOETTmMNt48751956 = OQLsDECFlOETTmMNt71877259;     OQLsDECFlOETTmMNt71877259 = OQLsDECFlOETTmMNt61176415;     OQLsDECFlOETTmMNt61176415 = OQLsDECFlOETTmMNt94777805;     OQLsDECFlOETTmMNt94777805 = OQLsDECFlOETTmMNt43364383;     OQLsDECFlOETTmMNt43364383 = OQLsDECFlOETTmMNt46541669;     OQLsDECFlOETTmMNt46541669 = OQLsDECFlOETTmMNt55350481;     OQLsDECFlOETTmMNt55350481 = OQLsDECFlOETTmMNt29992676;     OQLsDECFlOETTmMNt29992676 = OQLsDECFlOETTmMNt66504387;     OQLsDECFlOETTmMNt66504387 = OQLsDECFlOETTmMNt35709531;     OQLsDECFlOETTmMNt35709531 = OQLsDECFlOETTmMNt9060280;     OQLsDECFlOETTmMNt9060280 = OQLsDECFlOETTmMNt21136632;     OQLsDECFlOETTmMNt21136632 = OQLsDECFlOETTmMNt42426261;     OQLsDECFlOETTmMNt42426261 = OQLsDECFlOETTmMNt37662366;     OQLsDECFlOETTmMNt37662366 = OQLsDECFlOETTmMNt72055349;     OQLsDECFlOETTmMNt72055349 = OQLsDECFlOETTmMNt25668063;     OQLsDECFlOETTmMNt25668063 = OQLsDECFlOETTmMNt3756657;     OQLsDECFlOETTmMNt3756657 = OQLsDECFlOETTmMNt47697282;     OQLsDECFlOETTmMNt47697282 = OQLsDECFlOETTmMNt24786792;     OQLsDECFlOETTmMNt24786792 = OQLsDECFlOETTmMNt13928694;     OQLsDECFlOETTmMNt13928694 = OQLsDECFlOETTmMNt55266576;     OQLsDECFlOETTmMNt55266576 = OQLsDECFlOETTmMNt76434041;     OQLsDECFlOETTmMNt76434041 = OQLsDECFlOETTmMNt36556100;     OQLsDECFlOETTmMNt36556100 = OQLsDECFlOETTmMNt77560167;     OQLsDECFlOETTmMNt77560167 = OQLsDECFlOETTmMNt58296828;     OQLsDECFlOETTmMNt58296828 = OQLsDECFlOETTmMNt44690334;     OQLsDECFlOETTmMNt44690334 = OQLsDECFlOETTmMNt83418599;     OQLsDECFlOETTmMNt83418599 = OQLsDECFlOETTmMNt24759237;     OQLsDECFlOETTmMNt24759237 = OQLsDECFlOETTmMNt71052951;     OQLsDECFlOETTmMNt71052951 = OQLsDECFlOETTmMNt13528104;     OQLsDECFlOETTmMNt13528104 = OQLsDECFlOETTmMNt20892238;     OQLsDECFlOETTmMNt20892238 = OQLsDECFlOETTmMNt29091257;     OQLsDECFlOETTmMNt29091257 = OQLsDECFlOETTmMNt69181165;     OQLsDECFlOETTmMNt69181165 = OQLsDECFlOETTmMNt48766399;     OQLsDECFlOETTmMNt48766399 = OQLsDECFlOETTmMNt93742283;     OQLsDECFlOETTmMNt93742283 = OQLsDECFlOETTmMNt71585278;     OQLsDECFlOETTmMNt71585278 = OQLsDECFlOETTmMNt31335991;     OQLsDECFlOETTmMNt31335991 = OQLsDECFlOETTmMNt10661486;     OQLsDECFlOETTmMNt10661486 = OQLsDECFlOETTmMNt61351166;     OQLsDECFlOETTmMNt61351166 = OQLsDECFlOETTmMNt38699404;     OQLsDECFlOETTmMNt38699404 = OQLsDECFlOETTmMNt57764922;     OQLsDECFlOETTmMNt57764922 = OQLsDECFlOETTmMNt27654374;     OQLsDECFlOETTmMNt27654374 = OQLsDECFlOETTmMNt52577113;     OQLsDECFlOETTmMNt52577113 = OQLsDECFlOETTmMNt89117129;     OQLsDECFlOETTmMNt89117129 = OQLsDECFlOETTmMNt66367488;     OQLsDECFlOETTmMNt66367488 = OQLsDECFlOETTmMNt82709260;     OQLsDECFlOETTmMNt82709260 = OQLsDECFlOETTmMNt90387624;     OQLsDECFlOETTmMNt90387624 = OQLsDECFlOETTmMNt73671771;     OQLsDECFlOETTmMNt73671771 = OQLsDECFlOETTmMNt39918045;     OQLsDECFlOETTmMNt39918045 = OQLsDECFlOETTmMNt85614619;     OQLsDECFlOETTmMNt85614619 = OQLsDECFlOETTmMNt83811517;     OQLsDECFlOETTmMNt83811517 = OQLsDECFlOETTmMNt9748214;     OQLsDECFlOETTmMNt9748214 = OQLsDECFlOETTmMNt41246455;     OQLsDECFlOETTmMNt41246455 = OQLsDECFlOETTmMNt37553871;     OQLsDECFlOETTmMNt37553871 = OQLsDECFlOETTmMNt39643469;     OQLsDECFlOETTmMNt39643469 = OQLsDECFlOETTmMNt72317914;     OQLsDECFlOETTmMNt72317914 = OQLsDECFlOETTmMNt35321160;     OQLsDECFlOETTmMNt35321160 = OQLsDECFlOETTmMNt83616248;     OQLsDECFlOETTmMNt83616248 = OQLsDECFlOETTmMNt36480977;     OQLsDECFlOETTmMNt36480977 = OQLsDECFlOETTmMNt98674048;     OQLsDECFlOETTmMNt98674048 = OQLsDECFlOETTmMNt63123070;     OQLsDECFlOETTmMNt63123070 = OQLsDECFlOETTmMNt30591244;     OQLsDECFlOETTmMNt30591244 = OQLsDECFlOETTmMNt58939725;     OQLsDECFlOETTmMNt58939725 = OQLsDECFlOETTmMNt52976283;     OQLsDECFlOETTmMNt52976283 = OQLsDECFlOETTmMNt14817293;     OQLsDECFlOETTmMNt14817293 = OQLsDECFlOETTmMNt79969022;     OQLsDECFlOETTmMNt79969022 = OQLsDECFlOETTmMNt51955466;     OQLsDECFlOETTmMNt51955466 = OQLsDECFlOETTmMNt93659862;     OQLsDECFlOETTmMNt93659862 = OQLsDECFlOETTmMNt43920083;     OQLsDECFlOETTmMNt43920083 = OQLsDECFlOETTmMNt470071;     OQLsDECFlOETTmMNt470071 = OQLsDECFlOETTmMNt94332072;     OQLsDECFlOETTmMNt94332072 = OQLsDECFlOETTmMNt93095170;     OQLsDECFlOETTmMNt93095170 = OQLsDECFlOETTmMNt86346116;     OQLsDECFlOETTmMNt86346116 = OQLsDECFlOETTmMNt86087387;     OQLsDECFlOETTmMNt86087387 = OQLsDECFlOETTmMNt56163771;     OQLsDECFlOETTmMNt56163771 = OQLsDECFlOETTmMNt27612203;     OQLsDECFlOETTmMNt27612203 = OQLsDECFlOETTmMNt23856928;     OQLsDECFlOETTmMNt23856928 = OQLsDECFlOETTmMNt47438970;     OQLsDECFlOETTmMNt47438970 = OQLsDECFlOETTmMNt11192679;     OQLsDECFlOETTmMNt11192679 = OQLsDECFlOETTmMNt75587568;     OQLsDECFlOETTmMNt75587568 = OQLsDECFlOETTmMNt54302710;     OQLsDECFlOETTmMNt54302710 = OQLsDECFlOETTmMNt9746829;     OQLsDECFlOETTmMNt9746829 = OQLsDECFlOETTmMNt84841192;     OQLsDECFlOETTmMNt84841192 = OQLsDECFlOETTmMNt85438331;     OQLsDECFlOETTmMNt85438331 = OQLsDECFlOETTmMNt29716586;     OQLsDECFlOETTmMNt29716586 = OQLsDECFlOETTmMNt11144024;     OQLsDECFlOETTmMNt11144024 = OQLsDECFlOETTmMNt87844801;     OQLsDECFlOETTmMNt87844801 = OQLsDECFlOETTmMNt31627294;     OQLsDECFlOETTmMNt31627294 = OQLsDECFlOETTmMNt9122931;     OQLsDECFlOETTmMNt9122931 = OQLsDECFlOETTmMNt75427767;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VZWKfrmguygtXQFp39875728() {     double pZAImnXWiTOHYAASW81544874 = -876233833;    double pZAImnXWiTOHYAASW47219171 = -310694469;    double pZAImnXWiTOHYAASW48856069 = -625915760;    double pZAImnXWiTOHYAASW38408918 = -374763034;    double pZAImnXWiTOHYAASW80868882 = -831201586;    double pZAImnXWiTOHYAASW2699885 = -433156560;    double pZAImnXWiTOHYAASW48057140 = -840054521;    double pZAImnXWiTOHYAASW7205387 = -531012784;    double pZAImnXWiTOHYAASW30675742 = -772423531;    double pZAImnXWiTOHYAASW92712223 = -511677280;    double pZAImnXWiTOHYAASW55020335 = 72945532;    double pZAImnXWiTOHYAASW80574998 = -6114472;    double pZAImnXWiTOHYAASW58086377 = -375885711;    double pZAImnXWiTOHYAASW44320092 = -104970000;    double pZAImnXWiTOHYAASW90520231 = -156534443;    double pZAImnXWiTOHYAASW84869776 = 31108477;    double pZAImnXWiTOHYAASW40377673 = -618665297;    double pZAImnXWiTOHYAASW52403634 = -408099569;    double pZAImnXWiTOHYAASW91822518 = -799019349;    double pZAImnXWiTOHYAASW47054552 = -461611819;    double pZAImnXWiTOHYAASW24262725 = -723143532;    double pZAImnXWiTOHYAASW95581303 = -374858863;    double pZAImnXWiTOHYAASW56727423 = -879813405;    double pZAImnXWiTOHYAASW26418635 = -425645722;    double pZAImnXWiTOHYAASW81362016 = -703368533;    double pZAImnXWiTOHYAASW96777024 = -390570597;    double pZAImnXWiTOHYAASW24127841 = -313523549;    double pZAImnXWiTOHYAASW16863782 = -562678383;    double pZAImnXWiTOHYAASW13829538 = -413867377;    double pZAImnXWiTOHYAASW93402428 = -561614203;    double pZAImnXWiTOHYAASW6171243 = -872914978;    double pZAImnXWiTOHYAASW52877198 = -796080389;    double pZAImnXWiTOHYAASW86144990 = -669003474;    double pZAImnXWiTOHYAASW31386664 = -921635513;    double pZAImnXWiTOHYAASW39034819 = -291280250;    double pZAImnXWiTOHYAASW27022741 = -507760650;    double pZAImnXWiTOHYAASW25952269 = -440900239;    double pZAImnXWiTOHYAASW17508812 = -563444732;    double pZAImnXWiTOHYAASW98369490 = -733564503;    double pZAImnXWiTOHYAASW58554889 = -257817637;    double pZAImnXWiTOHYAASW19197328 = -746382868;    double pZAImnXWiTOHYAASW53042695 = -622269178;    double pZAImnXWiTOHYAASW7158236 = -813987513;    double pZAImnXWiTOHYAASW56588906 = -214812465;    double pZAImnXWiTOHYAASW60718864 = -389161804;    double pZAImnXWiTOHYAASW5681479 = -492044600;    double pZAImnXWiTOHYAASW89363853 = -485451107;    double pZAImnXWiTOHYAASW39292932 = -129948987;    double pZAImnXWiTOHYAASW20337215 = -586411598;    double pZAImnXWiTOHYAASW74262818 = -51297181;    double pZAImnXWiTOHYAASW74400082 = -413291040;    double pZAImnXWiTOHYAASW68574641 = -935565131;    double pZAImnXWiTOHYAASW75902830 = -432246518;    double pZAImnXWiTOHYAASW48545994 = -767861012;    double pZAImnXWiTOHYAASW15613004 = -632523474;    double pZAImnXWiTOHYAASW85963570 = -401374970;    double pZAImnXWiTOHYAASW90491747 = -430881064;    double pZAImnXWiTOHYAASW22437434 = -100270039;    double pZAImnXWiTOHYAASW57046902 = -671394502;    double pZAImnXWiTOHYAASW84091858 = -340630989;    double pZAImnXWiTOHYAASW78572043 = -19633011;    double pZAImnXWiTOHYAASW31193358 = -177376139;    double pZAImnXWiTOHYAASW93375849 = -17145407;    double pZAImnXWiTOHYAASW37273314 = -110809329;    double pZAImnXWiTOHYAASW86540981 = -638762303;    double pZAImnXWiTOHYAASW2143138 = -130974080;    double pZAImnXWiTOHYAASW94430008 = -337110999;    double pZAImnXWiTOHYAASW26699714 = -454250198;    double pZAImnXWiTOHYAASW5285274 = -813689751;    double pZAImnXWiTOHYAASW63497490 = -648773793;    double pZAImnXWiTOHYAASW58917508 = -527991285;    double pZAImnXWiTOHYAASW22868861 = 44779435;    double pZAImnXWiTOHYAASW54034143 = -674535066;    double pZAImnXWiTOHYAASW33267629 = -441201713;    double pZAImnXWiTOHYAASW27857224 = -715228952;    double pZAImnXWiTOHYAASW71220029 = -874355;    double pZAImnXWiTOHYAASW88423067 = -560871351;    double pZAImnXWiTOHYAASW138518 = -565000940;    double pZAImnXWiTOHYAASW65699771 = 63516082;    double pZAImnXWiTOHYAASW75680538 = -111323933;    double pZAImnXWiTOHYAASW7413172 = -905119491;    double pZAImnXWiTOHYAASW84834909 = -83574563;    double pZAImnXWiTOHYAASW96526566 = -976266786;    double pZAImnXWiTOHYAASW39566719 = -262570197;    double pZAImnXWiTOHYAASW19002346 = -48323164;    double pZAImnXWiTOHYAASW37596601 = -937349847;    double pZAImnXWiTOHYAASW76974367 = -263833872;    double pZAImnXWiTOHYAASW37598996 = -901142462;    double pZAImnXWiTOHYAASW15773660 = -189112040;    double pZAImnXWiTOHYAASW53071248 = -889905280;    double pZAImnXWiTOHYAASW36530994 = 23120414;    double pZAImnXWiTOHYAASW3514835 = -240630201;    double pZAImnXWiTOHYAASW60461910 = -892050231;    double pZAImnXWiTOHYAASW14277633 = -292933514;    double pZAImnXWiTOHYAASW79982845 = -138184626;    double pZAImnXWiTOHYAASW88003969 = -469006730;    double pZAImnXWiTOHYAASW59666845 = -505123772;    double pZAImnXWiTOHYAASW69884921 = -603178184;    double pZAImnXWiTOHYAASW70047924 = -576050162;    double pZAImnXWiTOHYAASW58575727 = -876233833;     pZAImnXWiTOHYAASW81544874 = pZAImnXWiTOHYAASW47219171;     pZAImnXWiTOHYAASW47219171 = pZAImnXWiTOHYAASW48856069;     pZAImnXWiTOHYAASW48856069 = pZAImnXWiTOHYAASW38408918;     pZAImnXWiTOHYAASW38408918 = pZAImnXWiTOHYAASW80868882;     pZAImnXWiTOHYAASW80868882 = pZAImnXWiTOHYAASW2699885;     pZAImnXWiTOHYAASW2699885 = pZAImnXWiTOHYAASW48057140;     pZAImnXWiTOHYAASW48057140 = pZAImnXWiTOHYAASW7205387;     pZAImnXWiTOHYAASW7205387 = pZAImnXWiTOHYAASW30675742;     pZAImnXWiTOHYAASW30675742 = pZAImnXWiTOHYAASW92712223;     pZAImnXWiTOHYAASW92712223 = pZAImnXWiTOHYAASW55020335;     pZAImnXWiTOHYAASW55020335 = pZAImnXWiTOHYAASW80574998;     pZAImnXWiTOHYAASW80574998 = pZAImnXWiTOHYAASW58086377;     pZAImnXWiTOHYAASW58086377 = pZAImnXWiTOHYAASW44320092;     pZAImnXWiTOHYAASW44320092 = pZAImnXWiTOHYAASW90520231;     pZAImnXWiTOHYAASW90520231 = pZAImnXWiTOHYAASW84869776;     pZAImnXWiTOHYAASW84869776 = pZAImnXWiTOHYAASW40377673;     pZAImnXWiTOHYAASW40377673 = pZAImnXWiTOHYAASW52403634;     pZAImnXWiTOHYAASW52403634 = pZAImnXWiTOHYAASW91822518;     pZAImnXWiTOHYAASW91822518 = pZAImnXWiTOHYAASW47054552;     pZAImnXWiTOHYAASW47054552 = pZAImnXWiTOHYAASW24262725;     pZAImnXWiTOHYAASW24262725 = pZAImnXWiTOHYAASW95581303;     pZAImnXWiTOHYAASW95581303 = pZAImnXWiTOHYAASW56727423;     pZAImnXWiTOHYAASW56727423 = pZAImnXWiTOHYAASW26418635;     pZAImnXWiTOHYAASW26418635 = pZAImnXWiTOHYAASW81362016;     pZAImnXWiTOHYAASW81362016 = pZAImnXWiTOHYAASW96777024;     pZAImnXWiTOHYAASW96777024 = pZAImnXWiTOHYAASW24127841;     pZAImnXWiTOHYAASW24127841 = pZAImnXWiTOHYAASW16863782;     pZAImnXWiTOHYAASW16863782 = pZAImnXWiTOHYAASW13829538;     pZAImnXWiTOHYAASW13829538 = pZAImnXWiTOHYAASW93402428;     pZAImnXWiTOHYAASW93402428 = pZAImnXWiTOHYAASW6171243;     pZAImnXWiTOHYAASW6171243 = pZAImnXWiTOHYAASW52877198;     pZAImnXWiTOHYAASW52877198 = pZAImnXWiTOHYAASW86144990;     pZAImnXWiTOHYAASW86144990 = pZAImnXWiTOHYAASW31386664;     pZAImnXWiTOHYAASW31386664 = pZAImnXWiTOHYAASW39034819;     pZAImnXWiTOHYAASW39034819 = pZAImnXWiTOHYAASW27022741;     pZAImnXWiTOHYAASW27022741 = pZAImnXWiTOHYAASW25952269;     pZAImnXWiTOHYAASW25952269 = pZAImnXWiTOHYAASW17508812;     pZAImnXWiTOHYAASW17508812 = pZAImnXWiTOHYAASW98369490;     pZAImnXWiTOHYAASW98369490 = pZAImnXWiTOHYAASW58554889;     pZAImnXWiTOHYAASW58554889 = pZAImnXWiTOHYAASW19197328;     pZAImnXWiTOHYAASW19197328 = pZAImnXWiTOHYAASW53042695;     pZAImnXWiTOHYAASW53042695 = pZAImnXWiTOHYAASW7158236;     pZAImnXWiTOHYAASW7158236 = pZAImnXWiTOHYAASW56588906;     pZAImnXWiTOHYAASW56588906 = pZAImnXWiTOHYAASW60718864;     pZAImnXWiTOHYAASW60718864 = pZAImnXWiTOHYAASW5681479;     pZAImnXWiTOHYAASW5681479 = pZAImnXWiTOHYAASW89363853;     pZAImnXWiTOHYAASW89363853 = pZAImnXWiTOHYAASW39292932;     pZAImnXWiTOHYAASW39292932 = pZAImnXWiTOHYAASW20337215;     pZAImnXWiTOHYAASW20337215 = pZAImnXWiTOHYAASW74262818;     pZAImnXWiTOHYAASW74262818 = pZAImnXWiTOHYAASW74400082;     pZAImnXWiTOHYAASW74400082 = pZAImnXWiTOHYAASW68574641;     pZAImnXWiTOHYAASW68574641 = pZAImnXWiTOHYAASW75902830;     pZAImnXWiTOHYAASW75902830 = pZAImnXWiTOHYAASW48545994;     pZAImnXWiTOHYAASW48545994 = pZAImnXWiTOHYAASW15613004;     pZAImnXWiTOHYAASW15613004 = pZAImnXWiTOHYAASW85963570;     pZAImnXWiTOHYAASW85963570 = pZAImnXWiTOHYAASW90491747;     pZAImnXWiTOHYAASW90491747 = pZAImnXWiTOHYAASW22437434;     pZAImnXWiTOHYAASW22437434 = pZAImnXWiTOHYAASW57046902;     pZAImnXWiTOHYAASW57046902 = pZAImnXWiTOHYAASW84091858;     pZAImnXWiTOHYAASW84091858 = pZAImnXWiTOHYAASW78572043;     pZAImnXWiTOHYAASW78572043 = pZAImnXWiTOHYAASW31193358;     pZAImnXWiTOHYAASW31193358 = pZAImnXWiTOHYAASW93375849;     pZAImnXWiTOHYAASW93375849 = pZAImnXWiTOHYAASW37273314;     pZAImnXWiTOHYAASW37273314 = pZAImnXWiTOHYAASW86540981;     pZAImnXWiTOHYAASW86540981 = pZAImnXWiTOHYAASW2143138;     pZAImnXWiTOHYAASW2143138 = pZAImnXWiTOHYAASW94430008;     pZAImnXWiTOHYAASW94430008 = pZAImnXWiTOHYAASW26699714;     pZAImnXWiTOHYAASW26699714 = pZAImnXWiTOHYAASW5285274;     pZAImnXWiTOHYAASW5285274 = pZAImnXWiTOHYAASW63497490;     pZAImnXWiTOHYAASW63497490 = pZAImnXWiTOHYAASW58917508;     pZAImnXWiTOHYAASW58917508 = pZAImnXWiTOHYAASW22868861;     pZAImnXWiTOHYAASW22868861 = pZAImnXWiTOHYAASW54034143;     pZAImnXWiTOHYAASW54034143 = pZAImnXWiTOHYAASW33267629;     pZAImnXWiTOHYAASW33267629 = pZAImnXWiTOHYAASW27857224;     pZAImnXWiTOHYAASW27857224 = pZAImnXWiTOHYAASW71220029;     pZAImnXWiTOHYAASW71220029 = pZAImnXWiTOHYAASW88423067;     pZAImnXWiTOHYAASW88423067 = pZAImnXWiTOHYAASW138518;     pZAImnXWiTOHYAASW138518 = pZAImnXWiTOHYAASW65699771;     pZAImnXWiTOHYAASW65699771 = pZAImnXWiTOHYAASW75680538;     pZAImnXWiTOHYAASW75680538 = pZAImnXWiTOHYAASW7413172;     pZAImnXWiTOHYAASW7413172 = pZAImnXWiTOHYAASW84834909;     pZAImnXWiTOHYAASW84834909 = pZAImnXWiTOHYAASW96526566;     pZAImnXWiTOHYAASW96526566 = pZAImnXWiTOHYAASW39566719;     pZAImnXWiTOHYAASW39566719 = pZAImnXWiTOHYAASW19002346;     pZAImnXWiTOHYAASW19002346 = pZAImnXWiTOHYAASW37596601;     pZAImnXWiTOHYAASW37596601 = pZAImnXWiTOHYAASW76974367;     pZAImnXWiTOHYAASW76974367 = pZAImnXWiTOHYAASW37598996;     pZAImnXWiTOHYAASW37598996 = pZAImnXWiTOHYAASW15773660;     pZAImnXWiTOHYAASW15773660 = pZAImnXWiTOHYAASW53071248;     pZAImnXWiTOHYAASW53071248 = pZAImnXWiTOHYAASW36530994;     pZAImnXWiTOHYAASW36530994 = pZAImnXWiTOHYAASW3514835;     pZAImnXWiTOHYAASW3514835 = pZAImnXWiTOHYAASW60461910;     pZAImnXWiTOHYAASW60461910 = pZAImnXWiTOHYAASW14277633;     pZAImnXWiTOHYAASW14277633 = pZAImnXWiTOHYAASW79982845;     pZAImnXWiTOHYAASW79982845 = pZAImnXWiTOHYAASW88003969;     pZAImnXWiTOHYAASW88003969 = pZAImnXWiTOHYAASW59666845;     pZAImnXWiTOHYAASW59666845 = pZAImnXWiTOHYAASW69884921;     pZAImnXWiTOHYAASW69884921 = pZAImnXWiTOHYAASW70047924;     pZAImnXWiTOHYAASW70047924 = pZAImnXWiTOHYAASW58575727;     pZAImnXWiTOHYAASW58575727 = pZAImnXWiTOHYAASW81544874;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rutwOKBSMLgGBHmG7167328() {     double ZLgUGOCNnOrgcZUjZ17003843 = -242510733;    double ZLgUGOCNnOrgcZUjZ60491250 = -191246075;    double ZLgUGOCNnOrgcZUjZ86242545 = -291582835;    double ZLgUGOCNnOrgcZUjZ10632273 = -195914105;    double ZLgUGOCNnOrgcZUjZ37154152 = -936800432;    double ZLgUGOCNnOrgcZUjZ38922757 = -695504456;    double ZLgUGOCNnOrgcZUjZ58714546 = -684780256;    double ZLgUGOCNnOrgcZUjZ69336948 = -175246924;    double ZLgUGOCNnOrgcZUjZ32193347 = -160754938;    double ZLgUGOCNnOrgcZUjZ7614936 = -176364278;    double ZLgUGOCNnOrgcZUjZ93202069 = -829062197;    double ZLgUGOCNnOrgcZUjZ81143146 = -970235243;    double ZLgUGOCNnOrgcZUjZ96321022 = -927898216;    double ZLgUGOCNnOrgcZUjZ97460955 = -81333409;    double ZLgUGOCNnOrgcZUjZ48714748 = -659959458;    double ZLgUGOCNnOrgcZUjZ68411077 = -704703130;    double ZLgUGOCNnOrgcZUjZ96213608 = -45639849;    double ZLgUGOCNnOrgcZUjZ52163826 = -481527638;    double ZLgUGOCNnOrgcZUjZ92372848 = -15582771;    double ZLgUGOCNnOrgcZUjZ40891770 = -888172885;    double ZLgUGOCNnOrgcZUjZ37349587 = -380418221;    double ZLgUGOCNnOrgcZUjZ99002827 = -461495157;    double ZLgUGOCNnOrgcZUjZ27252804 = -517445589;    double ZLgUGOCNnOrgcZUjZ20211581 = -936470326;    double ZLgUGOCNnOrgcZUjZ89379967 = -727353731;    double ZLgUGOCNnOrgcZUjZ7475196 = -79147404;    double ZLgUGOCNnOrgcZUjZ84548415 = -119807690;    double ZLgUGOCNnOrgcZUjZ57592207 = -732450558;    double ZLgUGOCNnOrgcZUjZ60497752 = -987962180;    double ZLgUGOCNnOrgcZUjZ98935838 = -596121373;    double ZLgUGOCNnOrgcZUjZ24754474 = -324895465;    double ZLgUGOCNnOrgcZUjZ90700216 = -569051803;    double ZLgUGOCNnOrgcZUjZ98363000 = -816599451;    double ZLgUGOCNnOrgcZUjZ61135090 = -986329327;    double ZLgUGOCNnOrgcZUjZ4843273 = -242380501;    double ZLgUGOCNnOrgcZUjZ78583585 = -722325956;    double ZLgUGOCNnOrgcZUjZ75364085 = -305596279;    double ZLgUGOCNnOrgcZUjZ68724215 = -546100313;    double ZLgUGOCNnOrgcZUjZ28760053 = -384440780;    double ZLgUGOCNnOrgcZUjZ22826421 = -823080183;    double ZLgUGOCNnOrgcZUjZ52648935 = -515581782;    double ZLgUGOCNnOrgcZUjZ42407822 = -216814469;    double ZLgUGOCNnOrgcZUjZ96541613 = -367168299;    double ZLgUGOCNnOrgcZUjZ96720243 = -819144875;    double ZLgUGOCNnOrgcZUjZ60735323 = -215672965;    double ZLgUGOCNnOrgcZUjZ51642807 = -530968233;    double ZLgUGOCNnOrgcZUjZ8709017 = -969437467;    double ZLgUGOCNnOrgcZUjZ57019243 = -296475581;    double ZLgUGOCNnOrgcZUjZ43229881 = -250148478;    double ZLgUGOCNnOrgcZUjZ98792407 = -542323997;    double ZLgUGOCNnOrgcZUjZ55170806 = -287567597;    double ZLgUGOCNnOrgcZUjZ52060114 = -49217478;    double ZLgUGOCNnOrgcZUjZ44523979 = -188292072;    double ZLgUGOCNnOrgcZUjZ64537840 = -959369512;    double ZLgUGOCNnOrgcZUjZ69817740 = -979013620;    double ZLgUGOCNnOrgcZUjZ18001015 = -781015576;    double ZLgUGOCNnOrgcZUjZ33238447 = -673800487;    double ZLgUGOCNnOrgcZUjZ66030965 = -355112509;    double ZLgUGOCNnOrgcZUjZ21252306 = -468560375;    double ZLgUGOCNnOrgcZUjZ29678956 = -757653028;    double ZLgUGOCNnOrgcZUjZ54374342 = -475696766;    double ZLgUGOCNnOrgcZUjZ1122340 = -952329698;    double ZLgUGOCNnOrgcZUjZ8839196 = -187284745;    double ZLgUGOCNnOrgcZUjZ33257509 = -564633566;    double ZLgUGOCNnOrgcZUjZ82860461 = -851468814;    double ZLgUGOCNnOrgcZUjZ2501854 = -160010395;    double ZLgUGOCNnOrgcZUjZ82780146 = -53635793;    double ZLgUGOCNnOrgcZUjZ35185933 = -941568889;    double ZLgUGOCNnOrgcZUjZ92617683 = -838952909;    double ZLgUGOCNnOrgcZUjZ70131163 = -937633502;    double ZLgUGOCNnOrgcZUjZ93046992 = -299106851;    double ZLgUGOCNnOrgcZUjZ27489393 = -499539537;    double ZLgUGOCNnOrgcZUjZ23403774 = 2913141;    double ZLgUGOCNnOrgcZUjZ69546427 = -192502588;    double ZLgUGOCNnOrgcZUjZ88242834 = -272591104;    double ZLgUGOCNnOrgcZUjZ94941765 = -63603753;    double ZLgUGOCNnOrgcZUjZ2461215 = 5673141;    double ZLgUGOCNnOrgcZUjZ30532560 = -698300714;    double ZLgUGOCNnOrgcZUjZ59476258 = -620797361;    double ZLgUGOCNnOrgcZUjZ37737161 = -96385499;    double ZLgUGOCNnOrgcZUjZ98766179 = -109709937;    double ZLgUGOCNnOrgcZUjZ27529172 = -823332110;    double ZLgUGOCNnOrgcZUjZ14362326 = -382302080;    double ZLgUGOCNnOrgcZUjZ61705344 = -345638184;    double ZLgUGOCNnOrgcZUjZ43765033 = -208553777;    double ZLgUGOCNnOrgcZUjZ72694360 = -175677987;    double ZLgUGOCNnOrgcZUjZ46176237 = -280759732;    double ZLgUGOCNnOrgcZUjZ33825161 = -857229940;    double ZLgUGOCNnOrgcZUjZ91317349 = 92684292;    double ZLgUGOCNnOrgcZUjZ86842258 = -461364925;    double ZLgUGOCNnOrgcZUjZ45345139 = 51474530;    double ZLgUGOCNnOrgcZUjZ9333120 = -950483770;    double ZLgUGOCNnOrgcZUjZ47471910 = 22460061;    double ZLgUGOCNnOrgcZUjZ99081096 = -626787752;    double ZLgUGOCNnOrgcZUjZ68452079 = -247383418;    double ZLgUGOCNnOrgcZUjZ51526596 = -563252084;    double ZLgUGOCNnOrgcZUjZ33568626 = 70470275;    double ZLgUGOCNnOrgcZUjZ63284105 = -802534734;    double ZLgUGOCNnOrgcZUjZ13859783 = -967676062;    double ZLgUGOCNnOrgcZUjZ58233469 = -242510733;     ZLgUGOCNnOrgcZUjZ17003843 = ZLgUGOCNnOrgcZUjZ60491250;     ZLgUGOCNnOrgcZUjZ60491250 = ZLgUGOCNnOrgcZUjZ86242545;     ZLgUGOCNnOrgcZUjZ86242545 = ZLgUGOCNnOrgcZUjZ10632273;     ZLgUGOCNnOrgcZUjZ10632273 = ZLgUGOCNnOrgcZUjZ37154152;     ZLgUGOCNnOrgcZUjZ37154152 = ZLgUGOCNnOrgcZUjZ38922757;     ZLgUGOCNnOrgcZUjZ38922757 = ZLgUGOCNnOrgcZUjZ58714546;     ZLgUGOCNnOrgcZUjZ58714546 = ZLgUGOCNnOrgcZUjZ69336948;     ZLgUGOCNnOrgcZUjZ69336948 = ZLgUGOCNnOrgcZUjZ32193347;     ZLgUGOCNnOrgcZUjZ32193347 = ZLgUGOCNnOrgcZUjZ7614936;     ZLgUGOCNnOrgcZUjZ7614936 = ZLgUGOCNnOrgcZUjZ93202069;     ZLgUGOCNnOrgcZUjZ93202069 = ZLgUGOCNnOrgcZUjZ81143146;     ZLgUGOCNnOrgcZUjZ81143146 = ZLgUGOCNnOrgcZUjZ96321022;     ZLgUGOCNnOrgcZUjZ96321022 = ZLgUGOCNnOrgcZUjZ97460955;     ZLgUGOCNnOrgcZUjZ97460955 = ZLgUGOCNnOrgcZUjZ48714748;     ZLgUGOCNnOrgcZUjZ48714748 = ZLgUGOCNnOrgcZUjZ68411077;     ZLgUGOCNnOrgcZUjZ68411077 = ZLgUGOCNnOrgcZUjZ96213608;     ZLgUGOCNnOrgcZUjZ96213608 = ZLgUGOCNnOrgcZUjZ52163826;     ZLgUGOCNnOrgcZUjZ52163826 = ZLgUGOCNnOrgcZUjZ92372848;     ZLgUGOCNnOrgcZUjZ92372848 = ZLgUGOCNnOrgcZUjZ40891770;     ZLgUGOCNnOrgcZUjZ40891770 = ZLgUGOCNnOrgcZUjZ37349587;     ZLgUGOCNnOrgcZUjZ37349587 = ZLgUGOCNnOrgcZUjZ99002827;     ZLgUGOCNnOrgcZUjZ99002827 = ZLgUGOCNnOrgcZUjZ27252804;     ZLgUGOCNnOrgcZUjZ27252804 = ZLgUGOCNnOrgcZUjZ20211581;     ZLgUGOCNnOrgcZUjZ20211581 = ZLgUGOCNnOrgcZUjZ89379967;     ZLgUGOCNnOrgcZUjZ89379967 = ZLgUGOCNnOrgcZUjZ7475196;     ZLgUGOCNnOrgcZUjZ7475196 = ZLgUGOCNnOrgcZUjZ84548415;     ZLgUGOCNnOrgcZUjZ84548415 = ZLgUGOCNnOrgcZUjZ57592207;     ZLgUGOCNnOrgcZUjZ57592207 = ZLgUGOCNnOrgcZUjZ60497752;     ZLgUGOCNnOrgcZUjZ60497752 = ZLgUGOCNnOrgcZUjZ98935838;     ZLgUGOCNnOrgcZUjZ98935838 = ZLgUGOCNnOrgcZUjZ24754474;     ZLgUGOCNnOrgcZUjZ24754474 = ZLgUGOCNnOrgcZUjZ90700216;     ZLgUGOCNnOrgcZUjZ90700216 = ZLgUGOCNnOrgcZUjZ98363000;     ZLgUGOCNnOrgcZUjZ98363000 = ZLgUGOCNnOrgcZUjZ61135090;     ZLgUGOCNnOrgcZUjZ61135090 = ZLgUGOCNnOrgcZUjZ4843273;     ZLgUGOCNnOrgcZUjZ4843273 = ZLgUGOCNnOrgcZUjZ78583585;     ZLgUGOCNnOrgcZUjZ78583585 = ZLgUGOCNnOrgcZUjZ75364085;     ZLgUGOCNnOrgcZUjZ75364085 = ZLgUGOCNnOrgcZUjZ68724215;     ZLgUGOCNnOrgcZUjZ68724215 = ZLgUGOCNnOrgcZUjZ28760053;     ZLgUGOCNnOrgcZUjZ28760053 = ZLgUGOCNnOrgcZUjZ22826421;     ZLgUGOCNnOrgcZUjZ22826421 = ZLgUGOCNnOrgcZUjZ52648935;     ZLgUGOCNnOrgcZUjZ52648935 = ZLgUGOCNnOrgcZUjZ42407822;     ZLgUGOCNnOrgcZUjZ42407822 = ZLgUGOCNnOrgcZUjZ96541613;     ZLgUGOCNnOrgcZUjZ96541613 = ZLgUGOCNnOrgcZUjZ96720243;     ZLgUGOCNnOrgcZUjZ96720243 = ZLgUGOCNnOrgcZUjZ60735323;     ZLgUGOCNnOrgcZUjZ60735323 = ZLgUGOCNnOrgcZUjZ51642807;     ZLgUGOCNnOrgcZUjZ51642807 = ZLgUGOCNnOrgcZUjZ8709017;     ZLgUGOCNnOrgcZUjZ8709017 = ZLgUGOCNnOrgcZUjZ57019243;     ZLgUGOCNnOrgcZUjZ57019243 = ZLgUGOCNnOrgcZUjZ43229881;     ZLgUGOCNnOrgcZUjZ43229881 = ZLgUGOCNnOrgcZUjZ98792407;     ZLgUGOCNnOrgcZUjZ98792407 = ZLgUGOCNnOrgcZUjZ55170806;     ZLgUGOCNnOrgcZUjZ55170806 = ZLgUGOCNnOrgcZUjZ52060114;     ZLgUGOCNnOrgcZUjZ52060114 = ZLgUGOCNnOrgcZUjZ44523979;     ZLgUGOCNnOrgcZUjZ44523979 = ZLgUGOCNnOrgcZUjZ64537840;     ZLgUGOCNnOrgcZUjZ64537840 = ZLgUGOCNnOrgcZUjZ69817740;     ZLgUGOCNnOrgcZUjZ69817740 = ZLgUGOCNnOrgcZUjZ18001015;     ZLgUGOCNnOrgcZUjZ18001015 = ZLgUGOCNnOrgcZUjZ33238447;     ZLgUGOCNnOrgcZUjZ33238447 = ZLgUGOCNnOrgcZUjZ66030965;     ZLgUGOCNnOrgcZUjZ66030965 = ZLgUGOCNnOrgcZUjZ21252306;     ZLgUGOCNnOrgcZUjZ21252306 = ZLgUGOCNnOrgcZUjZ29678956;     ZLgUGOCNnOrgcZUjZ29678956 = ZLgUGOCNnOrgcZUjZ54374342;     ZLgUGOCNnOrgcZUjZ54374342 = ZLgUGOCNnOrgcZUjZ1122340;     ZLgUGOCNnOrgcZUjZ1122340 = ZLgUGOCNnOrgcZUjZ8839196;     ZLgUGOCNnOrgcZUjZ8839196 = ZLgUGOCNnOrgcZUjZ33257509;     ZLgUGOCNnOrgcZUjZ33257509 = ZLgUGOCNnOrgcZUjZ82860461;     ZLgUGOCNnOrgcZUjZ82860461 = ZLgUGOCNnOrgcZUjZ2501854;     ZLgUGOCNnOrgcZUjZ2501854 = ZLgUGOCNnOrgcZUjZ82780146;     ZLgUGOCNnOrgcZUjZ82780146 = ZLgUGOCNnOrgcZUjZ35185933;     ZLgUGOCNnOrgcZUjZ35185933 = ZLgUGOCNnOrgcZUjZ92617683;     ZLgUGOCNnOrgcZUjZ92617683 = ZLgUGOCNnOrgcZUjZ70131163;     ZLgUGOCNnOrgcZUjZ70131163 = ZLgUGOCNnOrgcZUjZ93046992;     ZLgUGOCNnOrgcZUjZ93046992 = ZLgUGOCNnOrgcZUjZ27489393;     ZLgUGOCNnOrgcZUjZ27489393 = ZLgUGOCNnOrgcZUjZ23403774;     ZLgUGOCNnOrgcZUjZ23403774 = ZLgUGOCNnOrgcZUjZ69546427;     ZLgUGOCNnOrgcZUjZ69546427 = ZLgUGOCNnOrgcZUjZ88242834;     ZLgUGOCNnOrgcZUjZ88242834 = ZLgUGOCNnOrgcZUjZ94941765;     ZLgUGOCNnOrgcZUjZ94941765 = ZLgUGOCNnOrgcZUjZ2461215;     ZLgUGOCNnOrgcZUjZ2461215 = ZLgUGOCNnOrgcZUjZ30532560;     ZLgUGOCNnOrgcZUjZ30532560 = ZLgUGOCNnOrgcZUjZ59476258;     ZLgUGOCNnOrgcZUjZ59476258 = ZLgUGOCNnOrgcZUjZ37737161;     ZLgUGOCNnOrgcZUjZ37737161 = ZLgUGOCNnOrgcZUjZ98766179;     ZLgUGOCNnOrgcZUjZ98766179 = ZLgUGOCNnOrgcZUjZ27529172;     ZLgUGOCNnOrgcZUjZ27529172 = ZLgUGOCNnOrgcZUjZ14362326;     ZLgUGOCNnOrgcZUjZ14362326 = ZLgUGOCNnOrgcZUjZ61705344;     ZLgUGOCNnOrgcZUjZ61705344 = ZLgUGOCNnOrgcZUjZ43765033;     ZLgUGOCNnOrgcZUjZ43765033 = ZLgUGOCNnOrgcZUjZ72694360;     ZLgUGOCNnOrgcZUjZ72694360 = ZLgUGOCNnOrgcZUjZ46176237;     ZLgUGOCNnOrgcZUjZ46176237 = ZLgUGOCNnOrgcZUjZ33825161;     ZLgUGOCNnOrgcZUjZ33825161 = ZLgUGOCNnOrgcZUjZ91317349;     ZLgUGOCNnOrgcZUjZ91317349 = ZLgUGOCNnOrgcZUjZ86842258;     ZLgUGOCNnOrgcZUjZ86842258 = ZLgUGOCNnOrgcZUjZ45345139;     ZLgUGOCNnOrgcZUjZ45345139 = ZLgUGOCNnOrgcZUjZ9333120;     ZLgUGOCNnOrgcZUjZ9333120 = ZLgUGOCNnOrgcZUjZ47471910;     ZLgUGOCNnOrgcZUjZ47471910 = ZLgUGOCNnOrgcZUjZ99081096;     ZLgUGOCNnOrgcZUjZ99081096 = ZLgUGOCNnOrgcZUjZ68452079;     ZLgUGOCNnOrgcZUjZ68452079 = ZLgUGOCNnOrgcZUjZ51526596;     ZLgUGOCNnOrgcZUjZ51526596 = ZLgUGOCNnOrgcZUjZ33568626;     ZLgUGOCNnOrgcZUjZ33568626 = ZLgUGOCNnOrgcZUjZ63284105;     ZLgUGOCNnOrgcZUjZ63284105 = ZLgUGOCNnOrgcZUjZ13859783;     ZLgUGOCNnOrgcZUjZ13859783 = ZLgUGOCNnOrgcZUjZ58233469;     ZLgUGOCNnOrgcZUjZ58233469 = ZLgUGOCNnOrgcZUjZ17003843;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rEKeaZbcFKHnUwHW22216396() {     double TStvbVgqKYjgqPQTf23120949 = -354412622;    double TStvbVgqKYjgqPQTf3864530 = 99154144;    double TStvbVgqKYjgqPQTf2284730 = -442415756;    double TStvbVgqKYjgqPQTf37707054 = -386662735;    double TStvbVgqKYjgqPQTf6049640 = -357450303;    double TStvbVgqKYjgqPQTf30339959 = -960000691;    double TStvbVgqKYjgqPQTf19203512 = -823745233;    double TStvbVgqKYjgqPQTf19096839 = -462120837;    double TStvbVgqKYjgqPQTf96835842 = 71212396;    double TStvbVgqKYjgqPQTf48844595 = -505339756;    double TStvbVgqKYjgqPQTf53312360 = -706499784;    double TStvbVgqKYjgqPQTf12966190 = -314641445;    double TStvbVgqKYjgqPQTf82530141 = -267789305;    double TStvbVgqKYjgqPQTf80604632 = -613571977;    double TStvbVgqKYjgqPQTf44457175 = -250421624;    double TStvbVgqKYjgqPQTf9916472 = -677702673;    double TStvbVgqKYjgqPQTf90049611 = -662184962;    double TStvbVgqKYjgqPQTf49216979 = -147050280;    double TStvbVgqKYjgqPQTf54202690 = -421351868;    double TStvbVgqKYjgqPQTf21441935 = -145075805;    double TStvbVgqKYjgqPQTf25902781 = -235905676;    double TStvbVgqKYjgqPQTf85523851 = -369424874;    double TStvbVgqKYjgqPQTf62843595 = -263963076;    double TStvbVgqKYjgqPQTf4203955 = -58775314;    double TStvbVgqKYjgqPQTf33079618 = -549764;    double TStvbVgqKYjgqPQTf32196871 = 42197273;    double TStvbVgqKYjgqPQTf83008193 = -20469893;    double TStvbVgqKYjgqPQTf70699332 = -798379511;    double TStvbVgqKYjgqPQTf26630008 = -110344884;    double TStvbVgqKYjgqPQTf67551475 = -413592241;    double TStvbVgqKYjgqPQTf16997024 = -238216403;    double TStvbVgqKYjgqPQTf88310838 = -278498619;    double TStvbVgqKYjgqPQTf8073950 = -709212822;    double TStvbVgqKYjgqPQTf55965654 = -737871092;    double TStvbVgqKYjgqPQTf66317924 = -881113606;    double TStvbVgqKYjgqPQTf47309498 = -936352064;    double TStvbVgqKYjgqPQTf56626020 = -259232149;    double TStvbVgqKYjgqPQTf2814428 = -317606323;    double TStvbVgqKYjgqPQTf2370307 = -378364276;    double TStvbVgqKYjgqPQTf10328359 = -196519406;    double TStvbVgqKYjgqPQTf58318159 = -580855676;    double TStvbVgqKYjgqPQTf74558278 = -486956747;    double TStvbVgqKYjgqPQTf74608592 = -447972883;    double TStvbVgqKYjgqPQTf84127984 = -838487813;    double TStvbVgqKYjgqPQTf72687787 = -977571120;    double TStvbVgqKYjgqPQTf63582002 = -516184349;    double TStvbVgqKYjgqPQTf26487591 = 11786995;    double TStvbVgqKYjgqPQTf64976184 = -306371113;    double TStvbVgqKYjgqPQTf52905610 = -335885466;    double TStvbVgqKYjgqPQTf11704061 = -629334805;    double TStvbVgqKYjgqPQTf90871484 = -129861268;    double TStvbVgqKYjgqPQTf62869833 = -949943798;    double TStvbVgqKYjgqPQTf92772436 = 19918341;    double TStvbVgqKYjgqPQTf60506721 = -179839657;    double TStvbVgqKYjgqPQTf96313615 = -579545402;    double TStvbVgqKYjgqPQTf37597098 = -984987748;    double TStvbVgqKYjgqPQTf41020934 = -636882781;    double TStvbVgqKYjgqPQTf98080775 = -283640443;    double TStvbVgqKYjgqPQTf4627436 = -286112972;    double TStvbVgqKYjgqPQTf73852769 = -299647576;    double TStvbVgqKYjgqPQTf47331766 = -839530798;    double TStvbVgqKYjgqPQTf48504180 = 74634278;    double TStvbVgqKYjgqPQTf92466831 = -251775953;    double TStvbVgqKYjgqPQTf29284367 = -515195364;    double TStvbVgqKYjgqPQTf31847572 = -167123354;    double TStvbVgqKYjgqPQTf65001522 = -328001166;    double TStvbVgqKYjgqPQTf4892241 = -605428624;    double TStvbVgqKYjgqPQTf26564487 = -529918213;    double TStvbVgqKYjgqPQTf14286708 = -732458372;    double TStvbVgqKYjgqPQTf97147676 = -314069561;    double TStvbVgqKYjgqPQTf53290451 = -318470525;    double TStvbVgqKYjgqPQTf87235183 = -244578640;    double TStvbVgqKYjgqPQTf46846673 = -768686005;    double TStvbVgqKYjgqPQTf43874331 = -124832462;    double TStvbVgqKYjgqPQTf63123775 = -564220129;    double TStvbVgqKYjgqPQTf51344502 = -748948930;    double TStvbVgqKYjgqPQTf10915260 = -921451992;    double TStvbVgqKYjgqPQTf78715610 = -425475263;    double TStvbVgqKYjgqPQTf31516167 = -81204194;    double TStvbVgqKYjgqPQTf69497616 = -484365415;    double TStvbVgqKYjgqPQTf5709281 = -969589723;    double TStvbVgqKYjgqPQTf18032009 = -714098781;    double TStvbVgqKYjgqPQTf17793722 = -362494046;    double TStvbVgqKYjgqPQTf14925948 = -481010080;    double TStvbVgqKYjgqPQTf76679991 = -183730973;    double TStvbVgqKYjgqPQTf54127190 = -288272605;    double TStvbVgqKYjgqPQTf95538401 = -198416960;    double TStvbVgqKYjgqPQTf47567229 = -429373165;    double TStvbVgqKYjgqPQTf59652039 = -58325691;    double TStvbVgqKYjgqPQTf28720827 = -896125858;    double TStvbVgqKYjgqPQTf6288565 = -199469284;    double TStvbVgqKYjgqPQTf58545245 = -975591706;    double TStvbVgqKYjgqPQTf98186991 = 68506649;    double TStvbVgqKYjgqPQTf28517538 = 21283299;    double TStvbVgqKYjgqPQTf62996593 = -356988608;    double TStvbVgqKYjgqPQTf9813980 = -555489955;    double TStvbVgqKYjgqPQTf82091447 = -135180794;    double TStvbVgqKYjgqPQTf45324225 = -932777519;    double TStvbVgqKYjgqPQTf52280413 = -571364460;    double TStvbVgqKYjgqPQTf7686266 = -354412622;     TStvbVgqKYjgqPQTf23120949 = TStvbVgqKYjgqPQTf3864530;     TStvbVgqKYjgqPQTf3864530 = TStvbVgqKYjgqPQTf2284730;     TStvbVgqKYjgqPQTf2284730 = TStvbVgqKYjgqPQTf37707054;     TStvbVgqKYjgqPQTf37707054 = TStvbVgqKYjgqPQTf6049640;     TStvbVgqKYjgqPQTf6049640 = TStvbVgqKYjgqPQTf30339959;     TStvbVgqKYjgqPQTf30339959 = TStvbVgqKYjgqPQTf19203512;     TStvbVgqKYjgqPQTf19203512 = TStvbVgqKYjgqPQTf19096839;     TStvbVgqKYjgqPQTf19096839 = TStvbVgqKYjgqPQTf96835842;     TStvbVgqKYjgqPQTf96835842 = TStvbVgqKYjgqPQTf48844595;     TStvbVgqKYjgqPQTf48844595 = TStvbVgqKYjgqPQTf53312360;     TStvbVgqKYjgqPQTf53312360 = TStvbVgqKYjgqPQTf12966190;     TStvbVgqKYjgqPQTf12966190 = TStvbVgqKYjgqPQTf82530141;     TStvbVgqKYjgqPQTf82530141 = TStvbVgqKYjgqPQTf80604632;     TStvbVgqKYjgqPQTf80604632 = TStvbVgqKYjgqPQTf44457175;     TStvbVgqKYjgqPQTf44457175 = TStvbVgqKYjgqPQTf9916472;     TStvbVgqKYjgqPQTf9916472 = TStvbVgqKYjgqPQTf90049611;     TStvbVgqKYjgqPQTf90049611 = TStvbVgqKYjgqPQTf49216979;     TStvbVgqKYjgqPQTf49216979 = TStvbVgqKYjgqPQTf54202690;     TStvbVgqKYjgqPQTf54202690 = TStvbVgqKYjgqPQTf21441935;     TStvbVgqKYjgqPQTf21441935 = TStvbVgqKYjgqPQTf25902781;     TStvbVgqKYjgqPQTf25902781 = TStvbVgqKYjgqPQTf85523851;     TStvbVgqKYjgqPQTf85523851 = TStvbVgqKYjgqPQTf62843595;     TStvbVgqKYjgqPQTf62843595 = TStvbVgqKYjgqPQTf4203955;     TStvbVgqKYjgqPQTf4203955 = TStvbVgqKYjgqPQTf33079618;     TStvbVgqKYjgqPQTf33079618 = TStvbVgqKYjgqPQTf32196871;     TStvbVgqKYjgqPQTf32196871 = TStvbVgqKYjgqPQTf83008193;     TStvbVgqKYjgqPQTf83008193 = TStvbVgqKYjgqPQTf70699332;     TStvbVgqKYjgqPQTf70699332 = TStvbVgqKYjgqPQTf26630008;     TStvbVgqKYjgqPQTf26630008 = TStvbVgqKYjgqPQTf67551475;     TStvbVgqKYjgqPQTf67551475 = TStvbVgqKYjgqPQTf16997024;     TStvbVgqKYjgqPQTf16997024 = TStvbVgqKYjgqPQTf88310838;     TStvbVgqKYjgqPQTf88310838 = TStvbVgqKYjgqPQTf8073950;     TStvbVgqKYjgqPQTf8073950 = TStvbVgqKYjgqPQTf55965654;     TStvbVgqKYjgqPQTf55965654 = TStvbVgqKYjgqPQTf66317924;     TStvbVgqKYjgqPQTf66317924 = TStvbVgqKYjgqPQTf47309498;     TStvbVgqKYjgqPQTf47309498 = TStvbVgqKYjgqPQTf56626020;     TStvbVgqKYjgqPQTf56626020 = TStvbVgqKYjgqPQTf2814428;     TStvbVgqKYjgqPQTf2814428 = TStvbVgqKYjgqPQTf2370307;     TStvbVgqKYjgqPQTf2370307 = TStvbVgqKYjgqPQTf10328359;     TStvbVgqKYjgqPQTf10328359 = TStvbVgqKYjgqPQTf58318159;     TStvbVgqKYjgqPQTf58318159 = TStvbVgqKYjgqPQTf74558278;     TStvbVgqKYjgqPQTf74558278 = TStvbVgqKYjgqPQTf74608592;     TStvbVgqKYjgqPQTf74608592 = TStvbVgqKYjgqPQTf84127984;     TStvbVgqKYjgqPQTf84127984 = TStvbVgqKYjgqPQTf72687787;     TStvbVgqKYjgqPQTf72687787 = TStvbVgqKYjgqPQTf63582002;     TStvbVgqKYjgqPQTf63582002 = TStvbVgqKYjgqPQTf26487591;     TStvbVgqKYjgqPQTf26487591 = TStvbVgqKYjgqPQTf64976184;     TStvbVgqKYjgqPQTf64976184 = TStvbVgqKYjgqPQTf52905610;     TStvbVgqKYjgqPQTf52905610 = TStvbVgqKYjgqPQTf11704061;     TStvbVgqKYjgqPQTf11704061 = TStvbVgqKYjgqPQTf90871484;     TStvbVgqKYjgqPQTf90871484 = TStvbVgqKYjgqPQTf62869833;     TStvbVgqKYjgqPQTf62869833 = TStvbVgqKYjgqPQTf92772436;     TStvbVgqKYjgqPQTf92772436 = TStvbVgqKYjgqPQTf60506721;     TStvbVgqKYjgqPQTf60506721 = TStvbVgqKYjgqPQTf96313615;     TStvbVgqKYjgqPQTf96313615 = TStvbVgqKYjgqPQTf37597098;     TStvbVgqKYjgqPQTf37597098 = TStvbVgqKYjgqPQTf41020934;     TStvbVgqKYjgqPQTf41020934 = TStvbVgqKYjgqPQTf98080775;     TStvbVgqKYjgqPQTf98080775 = TStvbVgqKYjgqPQTf4627436;     TStvbVgqKYjgqPQTf4627436 = TStvbVgqKYjgqPQTf73852769;     TStvbVgqKYjgqPQTf73852769 = TStvbVgqKYjgqPQTf47331766;     TStvbVgqKYjgqPQTf47331766 = TStvbVgqKYjgqPQTf48504180;     TStvbVgqKYjgqPQTf48504180 = TStvbVgqKYjgqPQTf92466831;     TStvbVgqKYjgqPQTf92466831 = TStvbVgqKYjgqPQTf29284367;     TStvbVgqKYjgqPQTf29284367 = TStvbVgqKYjgqPQTf31847572;     TStvbVgqKYjgqPQTf31847572 = TStvbVgqKYjgqPQTf65001522;     TStvbVgqKYjgqPQTf65001522 = TStvbVgqKYjgqPQTf4892241;     TStvbVgqKYjgqPQTf4892241 = TStvbVgqKYjgqPQTf26564487;     TStvbVgqKYjgqPQTf26564487 = TStvbVgqKYjgqPQTf14286708;     TStvbVgqKYjgqPQTf14286708 = TStvbVgqKYjgqPQTf97147676;     TStvbVgqKYjgqPQTf97147676 = TStvbVgqKYjgqPQTf53290451;     TStvbVgqKYjgqPQTf53290451 = TStvbVgqKYjgqPQTf87235183;     TStvbVgqKYjgqPQTf87235183 = TStvbVgqKYjgqPQTf46846673;     TStvbVgqKYjgqPQTf46846673 = TStvbVgqKYjgqPQTf43874331;     TStvbVgqKYjgqPQTf43874331 = TStvbVgqKYjgqPQTf63123775;     TStvbVgqKYjgqPQTf63123775 = TStvbVgqKYjgqPQTf51344502;     TStvbVgqKYjgqPQTf51344502 = TStvbVgqKYjgqPQTf10915260;     TStvbVgqKYjgqPQTf10915260 = TStvbVgqKYjgqPQTf78715610;     TStvbVgqKYjgqPQTf78715610 = TStvbVgqKYjgqPQTf31516167;     TStvbVgqKYjgqPQTf31516167 = TStvbVgqKYjgqPQTf69497616;     TStvbVgqKYjgqPQTf69497616 = TStvbVgqKYjgqPQTf5709281;     TStvbVgqKYjgqPQTf5709281 = TStvbVgqKYjgqPQTf18032009;     TStvbVgqKYjgqPQTf18032009 = TStvbVgqKYjgqPQTf17793722;     TStvbVgqKYjgqPQTf17793722 = TStvbVgqKYjgqPQTf14925948;     TStvbVgqKYjgqPQTf14925948 = TStvbVgqKYjgqPQTf76679991;     TStvbVgqKYjgqPQTf76679991 = TStvbVgqKYjgqPQTf54127190;     TStvbVgqKYjgqPQTf54127190 = TStvbVgqKYjgqPQTf95538401;     TStvbVgqKYjgqPQTf95538401 = TStvbVgqKYjgqPQTf47567229;     TStvbVgqKYjgqPQTf47567229 = TStvbVgqKYjgqPQTf59652039;     TStvbVgqKYjgqPQTf59652039 = TStvbVgqKYjgqPQTf28720827;     TStvbVgqKYjgqPQTf28720827 = TStvbVgqKYjgqPQTf6288565;     TStvbVgqKYjgqPQTf6288565 = TStvbVgqKYjgqPQTf58545245;     TStvbVgqKYjgqPQTf58545245 = TStvbVgqKYjgqPQTf98186991;     TStvbVgqKYjgqPQTf98186991 = TStvbVgqKYjgqPQTf28517538;     TStvbVgqKYjgqPQTf28517538 = TStvbVgqKYjgqPQTf62996593;     TStvbVgqKYjgqPQTf62996593 = TStvbVgqKYjgqPQTf9813980;     TStvbVgqKYjgqPQTf9813980 = TStvbVgqKYjgqPQTf82091447;     TStvbVgqKYjgqPQTf82091447 = TStvbVgqKYjgqPQTf45324225;     TStvbVgqKYjgqPQTf45324225 = TStvbVgqKYjgqPQTf52280413;     TStvbVgqKYjgqPQTf52280413 = TStvbVgqKYjgqPQTf7686266;     TStvbVgqKYjgqPQTf7686266 = TStvbVgqKYjgqPQTf23120949;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void aKhKuCjnYuRBqioQ19824422() {     double IaWXvEjQUGMQCOiCy26312638 = -17565901;    double IaWXvEjQUGMQCOiCy54486488 = -250375703;    double IaWXvEjQUGMQCOiCy7707839 = 37647939;    double IaWXvEjQUGMQCOiCy53502155 = -208118927;    double IaWXvEjQUGMQCOiCy86057492 = -281670910;    double IaWXvEjQUGMQCOiCy67271551 = -953806128;    double IaWXvEjQUGMQCOiCy915954 = -188565601;    double IaWXvEjQUGMQCOiCy27687155 = -76383389;    double IaWXvEjQUGMQCOiCy7742169 = -254461679;    double IaWXvEjQUGMQCOiCy72878906 = -733966817;    double IaWXvEjQUGMQCOiCy50424660 = -979775342;    double IaWXvEjQUGMQCOiCy6672574 = -243083421;    double IaWXvEjQUGMQCOiCy23955653 = -534978825;    double IaWXvEjQUGMQCOiCy83393817 = -405540565;    double IaWXvEjQUGMQCOiCy9162895 = -474202721;    double IaWXvEjQUGMQCOiCy47946149 = 34977742;    double IaWXvEjQUGMQCOiCy95877135 = -992839506;    double IaWXvEjQUGMQCOiCy87357000 = -608656572;    double IaWXvEjQUGMQCOiCy94814049 = -587205867;    double IaWXvEjQUGMQCOiCy76160881 = -478905178;    double IaWXvEjQUGMQCOiCy5698363 = -416584523;    double IaWXvEjQUGMQCOiCy1508006 = -794383373;    double IaWXvEjQUGMQCOiCy192467 = -590932431;    double IaWXvEjQUGMQCOiCy23068318 = -334551959;    double IaWXvEjQUGMQCOiCy80885199 = -373180635;    double IaWXvEjQUGMQCOiCy89957090 = -86564973;    double IaWXvEjQUGMQCOiCy88528263 = -129496248;    double IaWXvEjQUGMQCOiCy64090207 = -748554279;    double IaWXvEjQUGMQCOiCy40293106 = -648451931;    double IaWXvEjQUGMQCOiCy87806657 = -77637310;    double IaWXvEjQUGMQCOiCy28165532 = -322640516;    double IaWXvEjQUGMQCOiCy52683437 = -292044860;    double IaWXvEjQUGMQCOiCy33674754 = -829634680;    double IaWXvEjQUGMQCOiCy50446875 = -346570946;    double IaWXvEjQUGMQCOiCy73851586 = -678107020;    double IaWXvEjQUGMQCOiCy53236669 = -118317149;    double IaWXvEjQUGMQCOiCy24773061 = -6449520;    double IaWXvEjQUGMQCOiCy94678693 = -293958355;    double IaWXvEjQUGMQCOiCy30299350 = -48337983;    double IaWXvEjQUGMQCOiCy70799211 = -139697382;    double IaWXvEjQUGMQCOiCy64567737 = -740682098;    double IaWXvEjQUGMQCOiCy15757139 = -331878642;    double IaWXvEjQUGMQCOiCy78541979 = -48178935;    double IaWXvEjQUGMQCOiCy14709043 = -866504207;    double IaWXvEjQUGMQCOiCy80703449 = -480708161;    double IaWXvEjQUGMQCOiCy62310010 = -668547462;    double IaWXvEjQUGMQCOiCy41656440 = 76447765;    double IaWXvEjQUGMQCOiCy3873862 = -251780325;    double IaWXvEjQUGMQCOiCy27915414 = -49608856;    double IaWXvEjQUGMQCOiCy88475732 = 77637415;    double IaWXvEjQUGMQCOiCy5397886 = -701998601;    double IaWXvEjQUGMQCOiCy30824413 = 20650556;    double IaWXvEjQUGMQCOiCy7979986 = -768122986;    double IaWXvEjQUGMQCOiCy79369355 = -751142481;    double IaWXvEjQUGMQCOiCy14126060 = -670830982;    double IaWXvEjQUGMQCOiCy24804632 = -223182528;    double IaWXvEjQUGMQCOiCy54294022 = -659443273;    double IaWXvEjQUGMQCOiCy84639520 = -627800103;    double IaWXvEjQUGMQCOiCy72616956 = -834938293;    double IaWXvEjQUGMQCOiCy96100402 = -95105937;    double IaWXvEjQUGMQCOiCy78743287 = -724309880;    double IaWXvEjQUGMQCOiCy36825747 = -440011322;    double IaWXvEjQUGMQCOiCy87394048 = -427931459;    double IaWXvEjQUGMQCOiCy19935512 = -76824370;    double IaWXvEjQUGMQCOiCy44713375 = -311326302;    double IaWXvEjQUGMQCOiCy97741222 = -587730483;    double IaWXvEjQUGMQCOiCy72997819 = -413448742;    double IaWXvEjQUGMQCOiCy73508777 = -88407879;    double IaWXvEjQUGMQCOiCy9542231 = -727433546;    double IaWXvEjQUGMQCOiCy55926226 = -255885572;    double IaWXvEjQUGMQCOiCy23173088 = -958572739;    double IaWXvEjQUGMQCOiCy1198443 = -598881152;    double IaWXvEjQUGMQCOiCy57057650 = -460318590;    double IaWXvEjQUGMQCOiCy24014839 = -347508485;    double IaWXvEjQUGMQCOiCy11593144 = -738223080;    double IaWXvEjQUGMQCOiCy89941224 = 15294119;    double IaWXvEjQUGMQCOiCy22966027 = -646204439;    double IaWXvEjQUGMQCOiCy85483424 = -724428225;    double IaWXvEjQUGMQCOiCy42364868 = -853843798;    double IaWXvEjQUGMQCOiCy18575189 = -704633173;    double IaWXvEjQUGMQCOiCy48300650 = -63012739;    double IaWXvEjQUGMQCOiCy84654401 = -877715923;    double IaWXvEjQUGMQCOiCy36174793 = -598945423;    double IaWXvEjQUGMQCOiCy51817373 = -626089346;    double IaWXvEjQUGMQCOiCy82408772 = -375638709;    double IaWXvEjQUGMQCOiCy97341118 = -243291073;    double IaWXvEjQUGMQCOiCy44703452 = -523921874;    double IaWXvEjQUGMQCOiCy54305399 = 21507801;    double IaWXvEjQUGMQCOiCy36320816 = -675739965;    double IaWXvEjQUGMQCOiCy49046954 = -354924492;    double IaWXvEjQUGMQCOiCy98942647 = -458873877;    double IaWXvEjQUGMQCOiCy40133540 = -378649417;    double IaWXvEjQUGMQCOiCy22061738 = -459020062;    double IaWXvEjQUGMQCOiCy34198948 = -953232046;    double IaWXvEjQUGMQCOiCy92055923 = -415387502;    double IaWXvEjQUGMQCOiCy27741991 = -200670777;    double IaWXvEjQUGMQCOiCy28363090 = -903947184;    double IaWXvEjQUGMQCOiCy58606468 = -971354565;    double IaWXvEjQUGMQCOiCy69995667 = -455177905;    double IaWXvEjQUGMQCOiCy82962227 = -17565901;     IaWXvEjQUGMQCOiCy26312638 = IaWXvEjQUGMQCOiCy54486488;     IaWXvEjQUGMQCOiCy54486488 = IaWXvEjQUGMQCOiCy7707839;     IaWXvEjQUGMQCOiCy7707839 = IaWXvEjQUGMQCOiCy53502155;     IaWXvEjQUGMQCOiCy53502155 = IaWXvEjQUGMQCOiCy86057492;     IaWXvEjQUGMQCOiCy86057492 = IaWXvEjQUGMQCOiCy67271551;     IaWXvEjQUGMQCOiCy67271551 = IaWXvEjQUGMQCOiCy915954;     IaWXvEjQUGMQCOiCy915954 = IaWXvEjQUGMQCOiCy27687155;     IaWXvEjQUGMQCOiCy27687155 = IaWXvEjQUGMQCOiCy7742169;     IaWXvEjQUGMQCOiCy7742169 = IaWXvEjQUGMQCOiCy72878906;     IaWXvEjQUGMQCOiCy72878906 = IaWXvEjQUGMQCOiCy50424660;     IaWXvEjQUGMQCOiCy50424660 = IaWXvEjQUGMQCOiCy6672574;     IaWXvEjQUGMQCOiCy6672574 = IaWXvEjQUGMQCOiCy23955653;     IaWXvEjQUGMQCOiCy23955653 = IaWXvEjQUGMQCOiCy83393817;     IaWXvEjQUGMQCOiCy83393817 = IaWXvEjQUGMQCOiCy9162895;     IaWXvEjQUGMQCOiCy9162895 = IaWXvEjQUGMQCOiCy47946149;     IaWXvEjQUGMQCOiCy47946149 = IaWXvEjQUGMQCOiCy95877135;     IaWXvEjQUGMQCOiCy95877135 = IaWXvEjQUGMQCOiCy87357000;     IaWXvEjQUGMQCOiCy87357000 = IaWXvEjQUGMQCOiCy94814049;     IaWXvEjQUGMQCOiCy94814049 = IaWXvEjQUGMQCOiCy76160881;     IaWXvEjQUGMQCOiCy76160881 = IaWXvEjQUGMQCOiCy5698363;     IaWXvEjQUGMQCOiCy5698363 = IaWXvEjQUGMQCOiCy1508006;     IaWXvEjQUGMQCOiCy1508006 = IaWXvEjQUGMQCOiCy192467;     IaWXvEjQUGMQCOiCy192467 = IaWXvEjQUGMQCOiCy23068318;     IaWXvEjQUGMQCOiCy23068318 = IaWXvEjQUGMQCOiCy80885199;     IaWXvEjQUGMQCOiCy80885199 = IaWXvEjQUGMQCOiCy89957090;     IaWXvEjQUGMQCOiCy89957090 = IaWXvEjQUGMQCOiCy88528263;     IaWXvEjQUGMQCOiCy88528263 = IaWXvEjQUGMQCOiCy64090207;     IaWXvEjQUGMQCOiCy64090207 = IaWXvEjQUGMQCOiCy40293106;     IaWXvEjQUGMQCOiCy40293106 = IaWXvEjQUGMQCOiCy87806657;     IaWXvEjQUGMQCOiCy87806657 = IaWXvEjQUGMQCOiCy28165532;     IaWXvEjQUGMQCOiCy28165532 = IaWXvEjQUGMQCOiCy52683437;     IaWXvEjQUGMQCOiCy52683437 = IaWXvEjQUGMQCOiCy33674754;     IaWXvEjQUGMQCOiCy33674754 = IaWXvEjQUGMQCOiCy50446875;     IaWXvEjQUGMQCOiCy50446875 = IaWXvEjQUGMQCOiCy73851586;     IaWXvEjQUGMQCOiCy73851586 = IaWXvEjQUGMQCOiCy53236669;     IaWXvEjQUGMQCOiCy53236669 = IaWXvEjQUGMQCOiCy24773061;     IaWXvEjQUGMQCOiCy24773061 = IaWXvEjQUGMQCOiCy94678693;     IaWXvEjQUGMQCOiCy94678693 = IaWXvEjQUGMQCOiCy30299350;     IaWXvEjQUGMQCOiCy30299350 = IaWXvEjQUGMQCOiCy70799211;     IaWXvEjQUGMQCOiCy70799211 = IaWXvEjQUGMQCOiCy64567737;     IaWXvEjQUGMQCOiCy64567737 = IaWXvEjQUGMQCOiCy15757139;     IaWXvEjQUGMQCOiCy15757139 = IaWXvEjQUGMQCOiCy78541979;     IaWXvEjQUGMQCOiCy78541979 = IaWXvEjQUGMQCOiCy14709043;     IaWXvEjQUGMQCOiCy14709043 = IaWXvEjQUGMQCOiCy80703449;     IaWXvEjQUGMQCOiCy80703449 = IaWXvEjQUGMQCOiCy62310010;     IaWXvEjQUGMQCOiCy62310010 = IaWXvEjQUGMQCOiCy41656440;     IaWXvEjQUGMQCOiCy41656440 = IaWXvEjQUGMQCOiCy3873862;     IaWXvEjQUGMQCOiCy3873862 = IaWXvEjQUGMQCOiCy27915414;     IaWXvEjQUGMQCOiCy27915414 = IaWXvEjQUGMQCOiCy88475732;     IaWXvEjQUGMQCOiCy88475732 = IaWXvEjQUGMQCOiCy5397886;     IaWXvEjQUGMQCOiCy5397886 = IaWXvEjQUGMQCOiCy30824413;     IaWXvEjQUGMQCOiCy30824413 = IaWXvEjQUGMQCOiCy7979986;     IaWXvEjQUGMQCOiCy7979986 = IaWXvEjQUGMQCOiCy79369355;     IaWXvEjQUGMQCOiCy79369355 = IaWXvEjQUGMQCOiCy14126060;     IaWXvEjQUGMQCOiCy14126060 = IaWXvEjQUGMQCOiCy24804632;     IaWXvEjQUGMQCOiCy24804632 = IaWXvEjQUGMQCOiCy54294022;     IaWXvEjQUGMQCOiCy54294022 = IaWXvEjQUGMQCOiCy84639520;     IaWXvEjQUGMQCOiCy84639520 = IaWXvEjQUGMQCOiCy72616956;     IaWXvEjQUGMQCOiCy72616956 = IaWXvEjQUGMQCOiCy96100402;     IaWXvEjQUGMQCOiCy96100402 = IaWXvEjQUGMQCOiCy78743287;     IaWXvEjQUGMQCOiCy78743287 = IaWXvEjQUGMQCOiCy36825747;     IaWXvEjQUGMQCOiCy36825747 = IaWXvEjQUGMQCOiCy87394048;     IaWXvEjQUGMQCOiCy87394048 = IaWXvEjQUGMQCOiCy19935512;     IaWXvEjQUGMQCOiCy19935512 = IaWXvEjQUGMQCOiCy44713375;     IaWXvEjQUGMQCOiCy44713375 = IaWXvEjQUGMQCOiCy97741222;     IaWXvEjQUGMQCOiCy97741222 = IaWXvEjQUGMQCOiCy72997819;     IaWXvEjQUGMQCOiCy72997819 = IaWXvEjQUGMQCOiCy73508777;     IaWXvEjQUGMQCOiCy73508777 = IaWXvEjQUGMQCOiCy9542231;     IaWXvEjQUGMQCOiCy9542231 = IaWXvEjQUGMQCOiCy55926226;     IaWXvEjQUGMQCOiCy55926226 = IaWXvEjQUGMQCOiCy23173088;     IaWXvEjQUGMQCOiCy23173088 = IaWXvEjQUGMQCOiCy1198443;     IaWXvEjQUGMQCOiCy1198443 = IaWXvEjQUGMQCOiCy57057650;     IaWXvEjQUGMQCOiCy57057650 = IaWXvEjQUGMQCOiCy24014839;     IaWXvEjQUGMQCOiCy24014839 = IaWXvEjQUGMQCOiCy11593144;     IaWXvEjQUGMQCOiCy11593144 = IaWXvEjQUGMQCOiCy89941224;     IaWXvEjQUGMQCOiCy89941224 = IaWXvEjQUGMQCOiCy22966027;     IaWXvEjQUGMQCOiCy22966027 = IaWXvEjQUGMQCOiCy85483424;     IaWXvEjQUGMQCOiCy85483424 = IaWXvEjQUGMQCOiCy42364868;     IaWXvEjQUGMQCOiCy42364868 = IaWXvEjQUGMQCOiCy18575189;     IaWXvEjQUGMQCOiCy18575189 = IaWXvEjQUGMQCOiCy48300650;     IaWXvEjQUGMQCOiCy48300650 = IaWXvEjQUGMQCOiCy84654401;     IaWXvEjQUGMQCOiCy84654401 = IaWXvEjQUGMQCOiCy36174793;     IaWXvEjQUGMQCOiCy36174793 = IaWXvEjQUGMQCOiCy51817373;     IaWXvEjQUGMQCOiCy51817373 = IaWXvEjQUGMQCOiCy82408772;     IaWXvEjQUGMQCOiCy82408772 = IaWXvEjQUGMQCOiCy97341118;     IaWXvEjQUGMQCOiCy97341118 = IaWXvEjQUGMQCOiCy44703452;     IaWXvEjQUGMQCOiCy44703452 = IaWXvEjQUGMQCOiCy54305399;     IaWXvEjQUGMQCOiCy54305399 = IaWXvEjQUGMQCOiCy36320816;     IaWXvEjQUGMQCOiCy36320816 = IaWXvEjQUGMQCOiCy49046954;     IaWXvEjQUGMQCOiCy49046954 = IaWXvEjQUGMQCOiCy98942647;     IaWXvEjQUGMQCOiCy98942647 = IaWXvEjQUGMQCOiCy40133540;     IaWXvEjQUGMQCOiCy40133540 = IaWXvEjQUGMQCOiCy22061738;     IaWXvEjQUGMQCOiCy22061738 = IaWXvEjQUGMQCOiCy34198948;     IaWXvEjQUGMQCOiCy34198948 = IaWXvEjQUGMQCOiCy92055923;     IaWXvEjQUGMQCOiCy92055923 = IaWXvEjQUGMQCOiCy27741991;     IaWXvEjQUGMQCOiCy27741991 = IaWXvEjQUGMQCOiCy28363090;     IaWXvEjQUGMQCOiCy28363090 = IaWXvEjQUGMQCOiCy58606468;     IaWXvEjQUGMQCOiCy58606468 = IaWXvEjQUGMQCOiCy69995667;     IaWXvEjQUGMQCOiCy69995667 = IaWXvEjQUGMQCOiCy82962227;     IaWXvEjQUGMQCOiCy82962227 = IaWXvEjQUGMQCOiCy26312638;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NyCleLnldzjtkqJQ34873490() {     double UFPUTbgiWJKzygkOo32429744 = -129467790;    double UFPUTbgiWJKzygkOo97859767 = 40024516;    double UFPUTbgiWJKzygkOo23750022 = -113184983;    double UFPUTbgiWJKzygkOo80576936 = -398867558;    double UFPUTbgiWJKzygkOo54952981 = -802320782;    double UFPUTbgiWJKzygkOo58688753 = -118302363;    double UFPUTbgiWJKzygkOo61404919 = -327530578;    double UFPUTbgiWJKzygkOo77447045 = -363257302;    double UFPUTbgiWJKzygkOo72384664 = -22494345;    double UFPUTbgiWJKzygkOo14108566 = 37057705;    double UFPUTbgiWJKzygkOo10534950 = -857212928;    double UFPUTbgiWJKzygkOo38495616 = -687489623;    double UFPUTbgiWJKzygkOo10164771 = -974869913;    double UFPUTbgiWJKzygkOo66537493 = -937779133;    double UFPUTbgiWJKzygkOo4905322 = -64664887;    double UFPUTbgiWJKzygkOo89451542 = 61978198;    double UFPUTbgiWJKzygkOo89713138 = -509384619;    double UFPUTbgiWJKzygkOo84410153 = -274179215;    double UFPUTbgiWJKzygkOo56643891 = -992974963;    double UFPUTbgiWJKzygkOo56711046 = -835808097;    double UFPUTbgiWJKzygkOo94251556 = -272071978;    double UFPUTbgiWJKzygkOo88029029 = -702313090;    double UFPUTbgiWJKzygkOo35783258 = -337449918;    double UFPUTbgiWJKzygkOo7060692 = -556856946;    double UFPUTbgiWJKzygkOo24584850 = -746376668;    double UFPUTbgiWJKzygkOo14678766 = 34779703;    double UFPUTbgiWJKzygkOo86988041 = -30158451;    double UFPUTbgiWJKzygkOo77197332 = -814483232;    double UFPUTbgiWJKzygkOo6425362 = -870834635;    double UFPUTbgiWJKzygkOo56422294 = -995108177;    double UFPUTbgiWJKzygkOo20408081 = -235961454;    double UFPUTbgiWJKzygkOo50294059 = -1491675;    double UFPUTbgiWJKzygkOo43385702 = -722248050;    double UFPUTbgiWJKzygkOo45277440 = -98112711;    double UFPUTbgiWJKzygkOo35326238 = -216840125;    double UFPUTbgiWJKzygkOo21962583 = -332343257;    double UFPUTbgiWJKzygkOo6034996 = 39914611;    double UFPUTbgiWJKzygkOo28768906 = -65464364;    double UFPUTbgiWJKzygkOo3909604 = -42261480;    double UFPUTbgiWJKzygkOo58301148 = -613136604;    double UFPUTbgiWJKzygkOo70236961 = -805955992;    double UFPUTbgiWJKzygkOo47907595 = -602020920;    double UFPUTbgiWJKzygkOo56608958 = -128983518;    double UFPUTbgiWJKzygkOo2116783 = -885847145;    double UFPUTbgiWJKzygkOo92655914 = -142606315;    double UFPUTbgiWJKzygkOo74249205 = -653763578;    double UFPUTbgiWJKzygkOo59435014 = -42327772;    double UFPUTbgiWJKzygkOo11830803 = -261675858;    double UFPUTbgiWJKzygkOo37591143 = -135345844;    double UFPUTbgiWJKzygkOo1387386 = -9373393;    double UFPUTbgiWJKzygkOo41098563 = -544292272;    double UFPUTbgiWJKzygkOo41634133 = -880075764;    double UFPUTbgiWJKzygkOo56228443 = -559912574;    double UFPUTbgiWJKzygkOo75338235 = 28387374;    double UFPUTbgiWJKzygkOo40621934 = -271362764;    double UFPUTbgiWJKzygkOo44400715 = -427154701;    double UFPUTbgiWJKzygkOo62076510 = -622525567;    double UFPUTbgiWJKzygkOo16689331 = -556328037;    double UFPUTbgiWJKzygkOo55992087 = -652490890;    double UFPUTbgiWJKzygkOo40274216 = -737100485;    double UFPUTbgiWJKzygkOo71700711 = 11856088;    double UFPUTbgiWJKzygkOo84207587 = -513047346;    double UFPUTbgiWJKzygkOo71021684 = -492422668;    double UFPUTbgiWJKzygkOo15962370 = -27386169;    double UFPUTbgiWJKzygkOo93700484 = -726980841;    double UFPUTbgiWJKzygkOo60240891 = -755721254;    double UFPUTbgiWJKzygkOo95109913 = -965241574;    double UFPUTbgiWJKzygkOo64887331 = -776757203;    double UFPUTbgiWJKzygkOo31211256 = -620939009;    double UFPUTbgiWJKzygkOo82942739 = -732321631;    double UFPUTbgiWJKzygkOo83416547 = -977936413;    double UFPUTbgiWJKzygkOo60944233 = -343920255;    double UFPUTbgiWJKzygkOo80500549 = -131917736;    double UFPUTbgiWJKzygkOo98342742 = -279838359;    double UFPUTbgiWJKzygkOo86474084 = 70147895;    double UFPUTbgiWJKzygkOo46343961 = -670051058;    double UFPUTbgiWJKzygkOo31420072 = -473329572;    double UFPUTbgiWJKzygkOo33666475 = -451602774;    double UFPUTbgiWJKzygkOo14404777 = -314250632;    double UFPUTbgiWJKzygkOo50335644 = 7386910;    double UFPUTbgiWJKzygkOo55243751 = -922892525;    double UFPUTbgiWJKzygkOo75157239 = -768482594;    double UFPUTbgiWJKzygkOo39606189 = -579137389;    double UFPUTbgiWJKzygkOo5037977 = -761461242;    double UFPUTbgiWJKzygkOo15323731 = -350815905;    double UFPUTbgiWJKzygkOo78773948 = -355885691;    double UFPUTbgiWJKzygkOo94065616 = -441579102;    double UFPUTbgiWJKzygkOo68047467 = -650635424;    double UFPUTbgiWJKzygkOo4655506 = -826749947;    double UFPUTbgiWJKzygkOo90925523 = -789685425;    double UFPUTbgiWJKzygkOo59886073 = -709817691;    double UFPUTbgiWJKzygkOo89345665 = -403757353;    double UFPUTbgiWJKzygkOo72776819 = -412973474;    double UFPUTbgiWJKzygkOo63635388 = -305160995;    double UFPUTbgiWJKzygkOo86600437 = -524992693;    double UFPUTbgiWJKzygkOo86029373 = -192908647;    double UFPUTbgiWJKzygkOo76885911 = -9598253;    double UFPUTbgiWJKzygkOo40646588 = -1597350;    double UFPUTbgiWJKzygkOo8416298 = -58866304;    double UFPUTbgiWJKzygkOo32415024 = -129467790;     UFPUTbgiWJKzygkOo32429744 = UFPUTbgiWJKzygkOo97859767;     UFPUTbgiWJKzygkOo97859767 = UFPUTbgiWJKzygkOo23750022;     UFPUTbgiWJKzygkOo23750022 = UFPUTbgiWJKzygkOo80576936;     UFPUTbgiWJKzygkOo80576936 = UFPUTbgiWJKzygkOo54952981;     UFPUTbgiWJKzygkOo54952981 = UFPUTbgiWJKzygkOo58688753;     UFPUTbgiWJKzygkOo58688753 = UFPUTbgiWJKzygkOo61404919;     UFPUTbgiWJKzygkOo61404919 = UFPUTbgiWJKzygkOo77447045;     UFPUTbgiWJKzygkOo77447045 = UFPUTbgiWJKzygkOo72384664;     UFPUTbgiWJKzygkOo72384664 = UFPUTbgiWJKzygkOo14108566;     UFPUTbgiWJKzygkOo14108566 = UFPUTbgiWJKzygkOo10534950;     UFPUTbgiWJKzygkOo10534950 = UFPUTbgiWJKzygkOo38495616;     UFPUTbgiWJKzygkOo38495616 = UFPUTbgiWJKzygkOo10164771;     UFPUTbgiWJKzygkOo10164771 = UFPUTbgiWJKzygkOo66537493;     UFPUTbgiWJKzygkOo66537493 = UFPUTbgiWJKzygkOo4905322;     UFPUTbgiWJKzygkOo4905322 = UFPUTbgiWJKzygkOo89451542;     UFPUTbgiWJKzygkOo89451542 = UFPUTbgiWJKzygkOo89713138;     UFPUTbgiWJKzygkOo89713138 = UFPUTbgiWJKzygkOo84410153;     UFPUTbgiWJKzygkOo84410153 = UFPUTbgiWJKzygkOo56643891;     UFPUTbgiWJKzygkOo56643891 = UFPUTbgiWJKzygkOo56711046;     UFPUTbgiWJKzygkOo56711046 = UFPUTbgiWJKzygkOo94251556;     UFPUTbgiWJKzygkOo94251556 = UFPUTbgiWJKzygkOo88029029;     UFPUTbgiWJKzygkOo88029029 = UFPUTbgiWJKzygkOo35783258;     UFPUTbgiWJKzygkOo35783258 = UFPUTbgiWJKzygkOo7060692;     UFPUTbgiWJKzygkOo7060692 = UFPUTbgiWJKzygkOo24584850;     UFPUTbgiWJKzygkOo24584850 = UFPUTbgiWJKzygkOo14678766;     UFPUTbgiWJKzygkOo14678766 = UFPUTbgiWJKzygkOo86988041;     UFPUTbgiWJKzygkOo86988041 = UFPUTbgiWJKzygkOo77197332;     UFPUTbgiWJKzygkOo77197332 = UFPUTbgiWJKzygkOo6425362;     UFPUTbgiWJKzygkOo6425362 = UFPUTbgiWJKzygkOo56422294;     UFPUTbgiWJKzygkOo56422294 = UFPUTbgiWJKzygkOo20408081;     UFPUTbgiWJKzygkOo20408081 = UFPUTbgiWJKzygkOo50294059;     UFPUTbgiWJKzygkOo50294059 = UFPUTbgiWJKzygkOo43385702;     UFPUTbgiWJKzygkOo43385702 = UFPUTbgiWJKzygkOo45277440;     UFPUTbgiWJKzygkOo45277440 = UFPUTbgiWJKzygkOo35326238;     UFPUTbgiWJKzygkOo35326238 = UFPUTbgiWJKzygkOo21962583;     UFPUTbgiWJKzygkOo21962583 = UFPUTbgiWJKzygkOo6034996;     UFPUTbgiWJKzygkOo6034996 = UFPUTbgiWJKzygkOo28768906;     UFPUTbgiWJKzygkOo28768906 = UFPUTbgiWJKzygkOo3909604;     UFPUTbgiWJKzygkOo3909604 = UFPUTbgiWJKzygkOo58301148;     UFPUTbgiWJKzygkOo58301148 = UFPUTbgiWJKzygkOo70236961;     UFPUTbgiWJKzygkOo70236961 = UFPUTbgiWJKzygkOo47907595;     UFPUTbgiWJKzygkOo47907595 = UFPUTbgiWJKzygkOo56608958;     UFPUTbgiWJKzygkOo56608958 = UFPUTbgiWJKzygkOo2116783;     UFPUTbgiWJKzygkOo2116783 = UFPUTbgiWJKzygkOo92655914;     UFPUTbgiWJKzygkOo92655914 = UFPUTbgiWJKzygkOo74249205;     UFPUTbgiWJKzygkOo74249205 = UFPUTbgiWJKzygkOo59435014;     UFPUTbgiWJKzygkOo59435014 = UFPUTbgiWJKzygkOo11830803;     UFPUTbgiWJKzygkOo11830803 = UFPUTbgiWJKzygkOo37591143;     UFPUTbgiWJKzygkOo37591143 = UFPUTbgiWJKzygkOo1387386;     UFPUTbgiWJKzygkOo1387386 = UFPUTbgiWJKzygkOo41098563;     UFPUTbgiWJKzygkOo41098563 = UFPUTbgiWJKzygkOo41634133;     UFPUTbgiWJKzygkOo41634133 = UFPUTbgiWJKzygkOo56228443;     UFPUTbgiWJKzygkOo56228443 = UFPUTbgiWJKzygkOo75338235;     UFPUTbgiWJKzygkOo75338235 = UFPUTbgiWJKzygkOo40621934;     UFPUTbgiWJKzygkOo40621934 = UFPUTbgiWJKzygkOo44400715;     UFPUTbgiWJKzygkOo44400715 = UFPUTbgiWJKzygkOo62076510;     UFPUTbgiWJKzygkOo62076510 = UFPUTbgiWJKzygkOo16689331;     UFPUTbgiWJKzygkOo16689331 = UFPUTbgiWJKzygkOo55992087;     UFPUTbgiWJKzygkOo55992087 = UFPUTbgiWJKzygkOo40274216;     UFPUTbgiWJKzygkOo40274216 = UFPUTbgiWJKzygkOo71700711;     UFPUTbgiWJKzygkOo71700711 = UFPUTbgiWJKzygkOo84207587;     UFPUTbgiWJKzygkOo84207587 = UFPUTbgiWJKzygkOo71021684;     UFPUTbgiWJKzygkOo71021684 = UFPUTbgiWJKzygkOo15962370;     UFPUTbgiWJKzygkOo15962370 = UFPUTbgiWJKzygkOo93700484;     UFPUTbgiWJKzygkOo93700484 = UFPUTbgiWJKzygkOo60240891;     UFPUTbgiWJKzygkOo60240891 = UFPUTbgiWJKzygkOo95109913;     UFPUTbgiWJKzygkOo95109913 = UFPUTbgiWJKzygkOo64887331;     UFPUTbgiWJKzygkOo64887331 = UFPUTbgiWJKzygkOo31211256;     UFPUTbgiWJKzygkOo31211256 = UFPUTbgiWJKzygkOo82942739;     UFPUTbgiWJKzygkOo82942739 = UFPUTbgiWJKzygkOo83416547;     UFPUTbgiWJKzygkOo83416547 = UFPUTbgiWJKzygkOo60944233;     UFPUTbgiWJKzygkOo60944233 = UFPUTbgiWJKzygkOo80500549;     UFPUTbgiWJKzygkOo80500549 = UFPUTbgiWJKzygkOo98342742;     UFPUTbgiWJKzygkOo98342742 = UFPUTbgiWJKzygkOo86474084;     UFPUTbgiWJKzygkOo86474084 = UFPUTbgiWJKzygkOo46343961;     UFPUTbgiWJKzygkOo46343961 = UFPUTbgiWJKzygkOo31420072;     UFPUTbgiWJKzygkOo31420072 = UFPUTbgiWJKzygkOo33666475;     UFPUTbgiWJKzygkOo33666475 = UFPUTbgiWJKzygkOo14404777;     UFPUTbgiWJKzygkOo14404777 = UFPUTbgiWJKzygkOo50335644;     UFPUTbgiWJKzygkOo50335644 = UFPUTbgiWJKzygkOo55243751;     UFPUTbgiWJKzygkOo55243751 = UFPUTbgiWJKzygkOo75157239;     UFPUTbgiWJKzygkOo75157239 = UFPUTbgiWJKzygkOo39606189;     UFPUTbgiWJKzygkOo39606189 = UFPUTbgiWJKzygkOo5037977;     UFPUTbgiWJKzygkOo5037977 = UFPUTbgiWJKzygkOo15323731;     UFPUTbgiWJKzygkOo15323731 = UFPUTbgiWJKzygkOo78773948;     UFPUTbgiWJKzygkOo78773948 = UFPUTbgiWJKzygkOo94065616;     UFPUTbgiWJKzygkOo94065616 = UFPUTbgiWJKzygkOo68047467;     UFPUTbgiWJKzygkOo68047467 = UFPUTbgiWJKzygkOo4655506;     UFPUTbgiWJKzygkOo4655506 = UFPUTbgiWJKzygkOo90925523;     UFPUTbgiWJKzygkOo90925523 = UFPUTbgiWJKzygkOo59886073;     UFPUTbgiWJKzygkOo59886073 = UFPUTbgiWJKzygkOo89345665;     UFPUTbgiWJKzygkOo89345665 = UFPUTbgiWJKzygkOo72776819;     UFPUTbgiWJKzygkOo72776819 = UFPUTbgiWJKzygkOo63635388;     UFPUTbgiWJKzygkOo63635388 = UFPUTbgiWJKzygkOo86600437;     UFPUTbgiWJKzygkOo86600437 = UFPUTbgiWJKzygkOo86029373;     UFPUTbgiWJKzygkOo86029373 = UFPUTbgiWJKzygkOo76885911;     UFPUTbgiWJKzygkOo76885911 = UFPUTbgiWJKzygkOo40646588;     UFPUTbgiWJKzygkOo40646588 = UFPUTbgiWJKzygkOo8416298;     UFPUTbgiWJKzygkOo8416298 = UFPUTbgiWJKzygkOo32415024;     UFPUTbgiWJKzygkOo32415024 = UFPUTbgiWJKzygkOo32429744;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BwvzhBVOROzNaIYa2165090() {     double wJKBTCKUVJWdCeYOI67888712 = -595744689;    double wJKBTCKUVJWdCeYOI11131847 = -940527091;    double wJKBTCKUVJWdCeYOI61136499 = -878852058;    double wJKBTCKUVJWdCeYOI52800291 = -220018629;    double wJKBTCKUVJWdCeYOI11238251 = -907919627;    double wJKBTCKUVJWdCeYOI94911625 = -380650258;    double wJKBTCKUVJWdCeYOI72062326 = -172256312;    double wJKBTCKUVJWdCeYOI39578607 = -7491443;    double wJKBTCKUVJWdCeYOI73902269 = -510825752;    double wJKBTCKUVJWdCeYOI29011278 = -727629293;    double wJKBTCKUVJWdCeYOI48716685 = -659220658;    double wJKBTCKUVJWdCeYOI39063764 = -551610395;    double wJKBTCKUVJWdCeYOI48399416 = -426882418;    double wJKBTCKUVJWdCeYOI19678357 = -914142543;    double wJKBTCKUVJWdCeYOI63099838 = -568089902;    double wJKBTCKUVJWdCeYOI72992843 = -673833409;    double wJKBTCKUVJWdCeYOI45549074 = 63640828;    double wJKBTCKUVJWdCeYOI84170344 = -347607284;    double wJKBTCKUVJWdCeYOI57194221 = -209538385;    double wJKBTCKUVJWdCeYOI50548264 = -162369163;    double wJKBTCKUVJWdCeYOI7338419 = 70653333;    double wJKBTCKUVJWdCeYOI91450554 = -788949384;    double wJKBTCKUVJWdCeYOI6308638 = 24917898;    double wJKBTCKUVJWdCeYOI853637 = 32318450;    double wJKBTCKUVJWdCeYOI32602801 = -770361866;    double wJKBTCKUVJWdCeYOI25376937 = -753797104;    double wJKBTCKUVJWdCeYOI47408616 = -936442592;    double wJKBTCKUVJWdCeYOI17925757 = -984255407;    double wJKBTCKUVJWdCeYOI53093576 = -344929438;    double wJKBTCKUVJWdCeYOI61955704 = 70384652;    double wJKBTCKUVJWdCeYOI38991313 = -787941941;    double wJKBTCKUVJWdCeYOI88117077 = -874463090;    double wJKBTCKUVJWdCeYOI55603713 = -869844027;    double wJKBTCKUVJWdCeYOI75025866 = -162806525;    double wJKBTCKUVJWdCeYOI1134692 = -167940376;    double wJKBTCKUVJWdCeYOI73523426 = -546908563;    double wJKBTCKUVJWdCeYOI55446812 = -924781430;    double wJKBTCKUVJWdCeYOI79984309 = -48119946;    double wJKBTCKUVJWdCeYOI34300166 = -793137756;    double wJKBTCKUVJWdCeYOI22572681 = -78399151;    double wJKBTCKUVJWdCeYOI3688569 = -575154907;    double wJKBTCKUVJWdCeYOI37272722 = -196566211;    double wJKBTCKUVJWdCeYOI45992336 = -782164304;    double wJKBTCKUVJWdCeYOI42248121 = -390179555;    double wJKBTCKUVJWdCeYOI92672373 = 30882523;    double wJKBTCKUVJWdCeYOI20210534 = -692687211;    double wJKBTCKUVJWdCeYOI78780178 = -526314133;    double wJKBTCKUVJWdCeYOI29557115 = -428202451;    double wJKBTCKUVJWdCeYOI60483809 = -899082724;    double wJKBTCKUVJWdCeYOI25916975 = -500400209;    double wJKBTCKUVJWdCeYOI21869287 = -418568830;    double wJKBTCKUVJWdCeYOI25119605 = 6271890;    double wJKBTCKUVJWdCeYOI24849592 = -315958128;    double wJKBTCKUVJWdCeYOI91330082 = -163121126;    double wJKBTCKUVJWdCeYOI94826670 = -617852910;    double wJKBTCKUVJWdCeYOI76438158 = -806795306;    double wJKBTCKUVJWdCeYOI4823209 = -865444989;    double wJKBTCKUVJWdCeYOI60282862 = -811170508;    double wJKBTCKUVJWdCeYOI20197491 = -449656763;    double wJKBTCKUVJWdCeYOI85861313 = -54122524;    double wJKBTCKUVJWdCeYOI47503010 = -444207667;    double wJKBTCKUVJWdCeYOI54136569 = -188000906;    double wJKBTCKUVJWdCeYOI86485030 = -662562006;    double wJKBTCKUVJWdCeYOI11946565 = -481210405;    double wJKBTCKUVJWdCeYOI90019965 = -939687352;    double wJKBTCKUVJWdCeYOI60599607 = -784757568;    double wJKBTCKUVJWdCeYOI83460051 = -681766368;    double wJKBTCKUVJWdCeYOI73373550 = -164075894;    double wJKBTCKUVJWdCeYOI18543665 = -646202167;    double wJKBTCKUVJWdCeYOI89576411 = 78818660;    double wJKBTCKUVJWdCeYOI17546032 = -749051979;    double wJKBTCKUVJWdCeYOI65564765 = -888239227;    double wJKBTCKUVJWdCeYOI49870179 = -554469528;    double wJKBTCKUVJWdCeYOI34621541 = -31139235;    double wJKBTCKUVJWdCeYOI46859696 = -587214257;    double wJKBTCKUVJWdCeYOI70065697 = -732780456;    double wJKBTCKUVJWdCeYOI45458218 = 93214920;    double wJKBTCKUVJWdCeYOI64060517 = -584902548;    double wJKBTCKUVJWdCeYOI8181264 = -998564074;    double wJKBTCKUVJWdCeYOI12392267 = 22325344;    double wJKBTCKUVJWdCeYOI46596759 = -127482971;    double wJKBTCKUVJWdCeYOI17851502 = -408240141;    double wJKBTCKUVJWdCeYOI57441948 = 14827317;    double wJKBTCKUVJWdCeYOI27176602 = -844529229;    double wJKBTCKUVJWdCeYOI40086418 = -511046518;    double wJKBTCKUVJWdCeYOI13871708 = -694213832;    double wJKBTCKUVJWdCeYOI63267486 = -458504962;    double wJKBTCKUVJWdCeYOI64273631 = -606722902;    double wJKBTCKUVJWdCeYOI80199195 = -544953615;    double wJKBTCKUVJWdCeYOI24696534 = -361145070;    double wJKBTCKUVJWdCeYOI68700218 = -681463574;    double wJKBTCKUVJWdCeYOI95163950 = -13610923;    double wJKBTCKUVJWdCeYOI59786819 = -598463183;    double wJKBTCKUVJWdCeYOI48438852 = -639015232;    double wJKBTCKUVJWdCeYOI75069671 = -634191484;    double wJKBTCKUVJWdCeYOI49552000 = -287154002;    double wJKBTCKUVJWdCeYOI50787692 = -534004206;    double wJKBTCKUVJWdCeYOI34045772 = -200953900;    double wJKBTCKUVJWdCeYOI52228156 = -450492203;    double wJKBTCKUVJWdCeYOI32072766 = -595744689;     wJKBTCKUVJWdCeYOI67888712 = wJKBTCKUVJWdCeYOI11131847;     wJKBTCKUVJWdCeYOI11131847 = wJKBTCKUVJWdCeYOI61136499;     wJKBTCKUVJWdCeYOI61136499 = wJKBTCKUVJWdCeYOI52800291;     wJKBTCKUVJWdCeYOI52800291 = wJKBTCKUVJWdCeYOI11238251;     wJKBTCKUVJWdCeYOI11238251 = wJKBTCKUVJWdCeYOI94911625;     wJKBTCKUVJWdCeYOI94911625 = wJKBTCKUVJWdCeYOI72062326;     wJKBTCKUVJWdCeYOI72062326 = wJKBTCKUVJWdCeYOI39578607;     wJKBTCKUVJWdCeYOI39578607 = wJKBTCKUVJWdCeYOI73902269;     wJKBTCKUVJWdCeYOI73902269 = wJKBTCKUVJWdCeYOI29011278;     wJKBTCKUVJWdCeYOI29011278 = wJKBTCKUVJWdCeYOI48716685;     wJKBTCKUVJWdCeYOI48716685 = wJKBTCKUVJWdCeYOI39063764;     wJKBTCKUVJWdCeYOI39063764 = wJKBTCKUVJWdCeYOI48399416;     wJKBTCKUVJWdCeYOI48399416 = wJKBTCKUVJWdCeYOI19678357;     wJKBTCKUVJWdCeYOI19678357 = wJKBTCKUVJWdCeYOI63099838;     wJKBTCKUVJWdCeYOI63099838 = wJKBTCKUVJWdCeYOI72992843;     wJKBTCKUVJWdCeYOI72992843 = wJKBTCKUVJWdCeYOI45549074;     wJKBTCKUVJWdCeYOI45549074 = wJKBTCKUVJWdCeYOI84170344;     wJKBTCKUVJWdCeYOI84170344 = wJKBTCKUVJWdCeYOI57194221;     wJKBTCKUVJWdCeYOI57194221 = wJKBTCKUVJWdCeYOI50548264;     wJKBTCKUVJWdCeYOI50548264 = wJKBTCKUVJWdCeYOI7338419;     wJKBTCKUVJWdCeYOI7338419 = wJKBTCKUVJWdCeYOI91450554;     wJKBTCKUVJWdCeYOI91450554 = wJKBTCKUVJWdCeYOI6308638;     wJKBTCKUVJWdCeYOI6308638 = wJKBTCKUVJWdCeYOI853637;     wJKBTCKUVJWdCeYOI853637 = wJKBTCKUVJWdCeYOI32602801;     wJKBTCKUVJWdCeYOI32602801 = wJKBTCKUVJWdCeYOI25376937;     wJKBTCKUVJWdCeYOI25376937 = wJKBTCKUVJWdCeYOI47408616;     wJKBTCKUVJWdCeYOI47408616 = wJKBTCKUVJWdCeYOI17925757;     wJKBTCKUVJWdCeYOI17925757 = wJKBTCKUVJWdCeYOI53093576;     wJKBTCKUVJWdCeYOI53093576 = wJKBTCKUVJWdCeYOI61955704;     wJKBTCKUVJWdCeYOI61955704 = wJKBTCKUVJWdCeYOI38991313;     wJKBTCKUVJWdCeYOI38991313 = wJKBTCKUVJWdCeYOI88117077;     wJKBTCKUVJWdCeYOI88117077 = wJKBTCKUVJWdCeYOI55603713;     wJKBTCKUVJWdCeYOI55603713 = wJKBTCKUVJWdCeYOI75025866;     wJKBTCKUVJWdCeYOI75025866 = wJKBTCKUVJWdCeYOI1134692;     wJKBTCKUVJWdCeYOI1134692 = wJKBTCKUVJWdCeYOI73523426;     wJKBTCKUVJWdCeYOI73523426 = wJKBTCKUVJWdCeYOI55446812;     wJKBTCKUVJWdCeYOI55446812 = wJKBTCKUVJWdCeYOI79984309;     wJKBTCKUVJWdCeYOI79984309 = wJKBTCKUVJWdCeYOI34300166;     wJKBTCKUVJWdCeYOI34300166 = wJKBTCKUVJWdCeYOI22572681;     wJKBTCKUVJWdCeYOI22572681 = wJKBTCKUVJWdCeYOI3688569;     wJKBTCKUVJWdCeYOI3688569 = wJKBTCKUVJWdCeYOI37272722;     wJKBTCKUVJWdCeYOI37272722 = wJKBTCKUVJWdCeYOI45992336;     wJKBTCKUVJWdCeYOI45992336 = wJKBTCKUVJWdCeYOI42248121;     wJKBTCKUVJWdCeYOI42248121 = wJKBTCKUVJWdCeYOI92672373;     wJKBTCKUVJWdCeYOI92672373 = wJKBTCKUVJWdCeYOI20210534;     wJKBTCKUVJWdCeYOI20210534 = wJKBTCKUVJWdCeYOI78780178;     wJKBTCKUVJWdCeYOI78780178 = wJKBTCKUVJWdCeYOI29557115;     wJKBTCKUVJWdCeYOI29557115 = wJKBTCKUVJWdCeYOI60483809;     wJKBTCKUVJWdCeYOI60483809 = wJKBTCKUVJWdCeYOI25916975;     wJKBTCKUVJWdCeYOI25916975 = wJKBTCKUVJWdCeYOI21869287;     wJKBTCKUVJWdCeYOI21869287 = wJKBTCKUVJWdCeYOI25119605;     wJKBTCKUVJWdCeYOI25119605 = wJKBTCKUVJWdCeYOI24849592;     wJKBTCKUVJWdCeYOI24849592 = wJKBTCKUVJWdCeYOI91330082;     wJKBTCKUVJWdCeYOI91330082 = wJKBTCKUVJWdCeYOI94826670;     wJKBTCKUVJWdCeYOI94826670 = wJKBTCKUVJWdCeYOI76438158;     wJKBTCKUVJWdCeYOI76438158 = wJKBTCKUVJWdCeYOI4823209;     wJKBTCKUVJWdCeYOI4823209 = wJKBTCKUVJWdCeYOI60282862;     wJKBTCKUVJWdCeYOI60282862 = wJKBTCKUVJWdCeYOI20197491;     wJKBTCKUVJWdCeYOI20197491 = wJKBTCKUVJWdCeYOI85861313;     wJKBTCKUVJWdCeYOI85861313 = wJKBTCKUVJWdCeYOI47503010;     wJKBTCKUVJWdCeYOI47503010 = wJKBTCKUVJWdCeYOI54136569;     wJKBTCKUVJWdCeYOI54136569 = wJKBTCKUVJWdCeYOI86485030;     wJKBTCKUVJWdCeYOI86485030 = wJKBTCKUVJWdCeYOI11946565;     wJKBTCKUVJWdCeYOI11946565 = wJKBTCKUVJWdCeYOI90019965;     wJKBTCKUVJWdCeYOI90019965 = wJKBTCKUVJWdCeYOI60599607;     wJKBTCKUVJWdCeYOI60599607 = wJKBTCKUVJWdCeYOI83460051;     wJKBTCKUVJWdCeYOI83460051 = wJKBTCKUVJWdCeYOI73373550;     wJKBTCKUVJWdCeYOI73373550 = wJKBTCKUVJWdCeYOI18543665;     wJKBTCKUVJWdCeYOI18543665 = wJKBTCKUVJWdCeYOI89576411;     wJKBTCKUVJWdCeYOI89576411 = wJKBTCKUVJWdCeYOI17546032;     wJKBTCKUVJWdCeYOI17546032 = wJKBTCKUVJWdCeYOI65564765;     wJKBTCKUVJWdCeYOI65564765 = wJKBTCKUVJWdCeYOI49870179;     wJKBTCKUVJWdCeYOI49870179 = wJKBTCKUVJWdCeYOI34621541;     wJKBTCKUVJWdCeYOI34621541 = wJKBTCKUVJWdCeYOI46859696;     wJKBTCKUVJWdCeYOI46859696 = wJKBTCKUVJWdCeYOI70065697;     wJKBTCKUVJWdCeYOI70065697 = wJKBTCKUVJWdCeYOI45458218;     wJKBTCKUVJWdCeYOI45458218 = wJKBTCKUVJWdCeYOI64060517;     wJKBTCKUVJWdCeYOI64060517 = wJKBTCKUVJWdCeYOI8181264;     wJKBTCKUVJWdCeYOI8181264 = wJKBTCKUVJWdCeYOI12392267;     wJKBTCKUVJWdCeYOI12392267 = wJKBTCKUVJWdCeYOI46596759;     wJKBTCKUVJWdCeYOI46596759 = wJKBTCKUVJWdCeYOI17851502;     wJKBTCKUVJWdCeYOI17851502 = wJKBTCKUVJWdCeYOI57441948;     wJKBTCKUVJWdCeYOI57441948 = wJKBTCKUVJWdCeYOI27176602;     wJKBTCKUVJWdCeYOI27176602 = wJKBTCKUVJWdCeYOI40086418;     wJKBTCKUVJWdCeYOI40086418 = wJKBTCKUVJWdCeYOI13871708;     wJKBTCKUVJWdCeYOI13871708 = wJKBTCKUVJWdCeYOI63267486;     wJKBTCKUVJWdCeYOI63267486 = wJKBTCKUVJWdCeYOI64273631;     wJKBTCKUVJWdCeYOI64273631 = wJKBTCKUVJWdCeYOI80199195;     wJKBTCKUVJWdCeYOI80199195 = wJKBTCKUVJWdCeYOI24696534;     wJKBTCKUVJWdCeYOI24696534 = wJKBTCKUVJWdCeYOI68700218;     wJKBTCKUVJWdCeYOI68700218 = wJKBTCKUVJWdCeYOI95163950;     wJKBTCKUVJWdCeYOI95163950 = wJKBTCKUVJWdCeYOI59786819;     wJKBTCKUVJWdCeYOI59786819 = wJKBTCKUVJWdCeYOI48438852;     wJKBTCKUVJWdCeYOI48438852 = wJKBTCKUVJWdCeYOI75069671;     wJKBTCKUVJWdCeYOI75069671 = wJKBTCKUVJWdCeYOI49552000;     wJKBTCKUVJWdCeYOI49552000 = wJKBTCKUVJWdCeYOI50787692;     wJKBTCKUVJWdCeYOI50787692 = wJKBTCKUVJWdCeYOI34045772;     wJKBTCKUVJWdCeYOI34045772 = wJKBTCKUVJWdCeYOI52228156;     wJKBTCKUVJWdCeYOI52228156 = wJKBTCKUVJWdCeYOI32072766;     wJKBTCKUVJWdCeYOI32072766 = wJKBTCKUVJWdCeYOI67888712;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void puuuRlelIcmZPDEy17214158() {     double lSaXLuUavdEFeyNrK74005819 = -707646578;    double lSaXLuUavdEFeyNrK54505126 = -650126871;    double lSaXLuUavdEFeyNrK77178682 = 70315021;    double lSaXLuUavdEFeyNrK79875072 = -410767259;    double lSaXLuUavdEFeyNrK80133739 = -328569498;    double lSaXLuUavdEFeyNrK86328827 = -645146493;    double lSaXLuUavdEFeyNrK32551292 = -311221289;    double lSaXLuUavdEFeyNrK89338497 = -294365356;    double lSaXLuUavdEFeyNrK38544765 = -278858418;    double lSaXLuUavdEFeyNrK70240937 = 43395230;    double lSaXLuUavdEFeyNrK8826975 = -536658244;    double lSaXLuUavdEFeyNrK70886807 = -996016596;    double lSaXLuUavdEFeyNrK34608535 = -866773507;    double lSaXLuUavdEFeyNrK2822034 = -346381111;    double lSaXLuUavdEFeyNrK58842265 = -158552068;    double lSaXLuUavdEFeyNrK14498238 = -646832952;    double lSaXLuUavdEFeyNrK39385078 = -552904285;    double lSaXLuUavdEFeyNrK81223497 = -13129926;    double lSaXLuUavdEFeyNrK19024063 = -615307482;    double lSaXLuUavdEFeyNrK31098429 = -519272082;    double lSaXLuUavdEFeyNrK95891612 = -884834122;    double lSaXLuUavdEFeyNrK77971578 = -696879100;    double lSaXLuUavdEFeyNrK41899430 = -821599589;    double lSaXLuUavdEFeyNrK84846010 = -189986538;    double lSaXLuUavdEFeyNrK76302451 = -43557899;    double lSaXLuUavdEFeyNrK50098613 = -632452427;    double lSaXLuUavdEFeyNrK45868394 = -837104795;    double lSaXLuUavdEFeyNrK31032883 = 49815640;    double lSaXLuUavdEFeyNrK19225832 = -567312142;    double lSaXLuUavdEFeyNrK30571342 = -847086215;    double lSaXLuUavdEFeyNrK31233862 = -701262879;    double lSaXLuUavdEFeyNrK85727699 = -583909905;    double lSaXLuUavdEFeyNrK65314662 = -762457398;    double lSaXLuUavdEFeyNrK69856430 = 85651710;    double lSaXLuUavdEFeyNrK62609344 = -806673481;    double lSaXLuUavdEFeyNrK42249339 = -760934670;    double lSaXLuUavdEFeyNrK36708747 = -878417299;    double lSaXLuUavdEFeyNrK14074522 = -919625955;    double lSaXLuUavdEFeyNrK7910420 = -787061253;    double lSaXLuUavdEFeyNrK10074619 = -551838373;    double lSaXLuUavdEFeyNrK9357793 = -640428801;    double lSaXLuUavdEFeyNrK69423179 = -466708489;    double lSaXLuUavdEFeyNrK24059315 = -862968888;    double lSaXLuUavdEFeyNrK29655862 = -409522493;    double lSaXLuUavdEFeyNrK4624839 = -731015631;    double lSaXLuUavdEFeyNrK32149729 = -677903327;    double lSaXLuUavdEFeyNrK96558752 = -645089670;    double lSaXLuUavdEFeyNrK37514056 = -438097983;    double lSaXLuUavdEFeyNrK70159538 = -984819712;    double lSaXLuUavdEFeyNrK38828627 = -587411017;    double lSaXLuUavdEFeyNrK57569965 = -260862501;    double lSaXLuUavdEFeyNrK35929325 = -894454430;    double lSaXLuUavdEFeyNrK73098048 = -107747716;    double lSaXLuUavdEFeyNrK87298962 = -483591271;    double lSaXLuUavdEFeyNrK21322545 = -218384693;    double lSaXLuUavdEFeyNrK96034241 = 89232521;    double lSaXLuUavdEFeyNrK12605696 = -828527283;    double lSaXLuUavdEFeyNrK92332671 = -739698441;    double lSaXLuUavdEFeyNrK3572622 = -267209361;    double lSaXLuUavdEFeyNrK30035127 = -696117072;    double lSaXLuUavdEFeyNrK40460434 = -808041699;    double lSaXLuUavdEFeyNrK1518410 = -261036929;    double lSaXLuUavdEFeyNrK70112666 = -727053214;    double lSaXLuUavdEFeyNrK7973423 = -431772203;    double lSaXLuUavdEFeyNrK39007075 = -255341891;    double lSaXLuUavdEFeyNrK23099276 = -952748339;    double lSaXLuUavdEFeyNrK5572146 = -133559199;    double lSaXLuUavdEFeyNrK64752104 = -852425218;    double lSaXLuUavdEFeyNrK40212690 = -539707630;    double lSaXLuUavdEFeyNrK16592926 = -397617399;    double lSaXLuUavdEFeyNrK77789490 = -768415653;    double lSaXLuUavdEFeyNrK25310556 = -633278330;    double lSaXLuUavdEFeyNrK73313078 = -226068674;    double lSaXLuUavdEFeyNrK8949445 = 36530891;    double lSaXLuUavdEFeyNrK21740637 = -878843282;    double lSaXLuUavdEFeyNrK26468433 = -318125634;    double lSaXLuUavdEFeyNrK53912263 = -833910212;    double lSaXLuUavdEFeyNrK12243569 = -312077097;    double lSaXLuUavdEFeyNrK80221172 = -458970908;    double lSaXLuUavdEFeyNrK44152722 = -365654573;    double lSaXLuUavdEFeyNrK53539860 = -987362757;    double lSaXLuUavdEFeyNrK8354339 = -299006812;    double lSaXLuUavdEFeyNrK60873344 = 34635351;    double lSaXLuUavdEFeyNrK80397204 = -979901125;    double lSaXLuUavdEFeyNrK73001376 = -486223715;    double lSaXLuUavdEFeyNrK95304537 = -806808450;    double lSaXLuUavdEFeyNrK12629651 = -376162190;    double lSaXLuUavdEFeyNrK78015699 = -178866127;    double lSaXLuUavdEFeyNrK48533885 = -695963598;    double lSaXLuUavdEFeyNrK66575102 = -795906003;    double lSaXLuUavdEFeyNrK29643644 = -932407388;    double lSaXLuUavdEFeyNrK44376075 = -38718859;    double lSaXLuUavdEFeyNrK10501901 = -552416595;    double lSaXLuUavdEFeyNrK77875292 = 9055819;    double lSaXLuUavdEFeyNrK69614185 = -743796675;    double lSaXLuUavdEFeyNrK7839384 = -279391872;    double lSaXLuUavdEFeyNrK99310513 = -739655275;    double lSaXLuUavdEFeyNrK16085892 = -331196686;    double lSaXLuUavdEFeyNrK90648786 = -54180602;    double lSaXLuUavdEFeyNrK81525562 = -707646578;     lSaXLuUavdEFeyNrK74005819 = lSaXLuUavdEFeyNrK54505126;     lSaXLuUavdEFeyNrK54505126 = lSaXLuUavdEFeyNrK77178682;     lSaXLuUavdEFeyNrK77178682 = lSaXLuUavdEFeyNrK79875072;     lSaXLuUavdEFeyNrK79875072 = lSaXLuUavdEFeyNrK80133739;     lSaXLuUavdEFeyNrK80133739 = lSaXLuUavdEFeyNrK86328827;     lSaXLuUavdEFeyNrK86328827 = lSaXLuUavdEFeyNrK32551292;     lSaXLuUavdEFeyNrK32551292 = lSaXLuUavdEFeyNrK89338497;     lSaXLuUavdEFeyNrK89338497 = lSaXLuUavdEFeyNrK38544765;     lSaXLuUavdEFeyNrK38544765 = lSaXLuUavdEFeyNrK70240937;     lSaXLuUavdEFeyNrK70240937 = lSaXLuUavdEFeyNrK8826975;     lSaXLuUavdEFeyNrK8826975 = lSaXLuUavdEFeyNrK70886807;     lSaXLuUavdEFeyNrK70886807 = lSaXLuUavdEFeyNrK34608535;     lSaXLuUavdEFeyNrK34608535 = lSaXLuUavdEFeyNrK2822034;     lSaXLuUavdEFeyNrK2822034 = lSaXLuUavdEFeyNrK58842265;     lSaXLuUavdEFeyNrK58842265 = lSaXLuUavdEFeyNrK14498238;     lSaXLuUavdEFeyNrK14498238 = lSaXLuUavdEFeyNrK39385078;     lSaXLuUavdEFeyNrK39385078 = lSaXLuUavdEFeyNrK81223497;     lSaXLuUavdEFeyNrK81223497 = lSaXLuUavdEFeyNrK19024063;     lSaXLuUavdEFeyNrK19024063 = lSaXLuUavdEFeyNrK31098429;     lSaXLuUavdEFeyNrK31098429 = lSaXLuUavdEFeyNrK95891612;     lSaXLuUavdEFeyNrK95891612 = lSaXLuUavdEFeyNrK77971578;     lSaXLuUavdEFeyNrK77971578 = lSaXLuUavdEFeyNrK41899430;     lSaXLuUavdEFeyNrK41899430 = lSaXLuUavdEFeyNrK84846010;     lSaXLuUavdEFeyNrK84846010 = lSaXLuUavdEFeyNrK76302451;     lSaXLuUavdEFeyNrK76302451 = lSaXLuUavdEFeyNrK50098613;     lSaXLuUavdEFeyNrK50098613 = lSaXLuUavdEFeyNrK45868394;     lSaXLuUavdEFeyNrK45868394 = lSaXLuUavdEFeyNrK31032883;     lSaXLuUavdEFeyNrK31032883 = lSaXLuUavdEFeyNrK19225832;     lSaXLuUavdEFeyNrK19225832 = lSaXLuUavdEFeyNrK30571342;     lSaXLuUavdEFeyNrK30571342 = lSaXLuUavdEFeyNrK31233862;     lSaXLuUavdEFeyNrK31233862 = lSaXLuUavdEFeyNrK85727699;     lSaXLuUavdEFeyNrK85727699 = lSaXLuUavdEFeyNrK65314662;     lSaXLuUavdEFeyNrK65314662 = lSaXLuUavdEFeyNrK69856430;     lSaXLuUavdEFeyNrK69856430 = lSaXLuUavdEFeyNrK62609344;     lSaXLuUavdEFeyNrK62609344 = lSaXLuUavdEFeyNrK42249339;     lSaXLuUavdEFeyNrK42249339 = lSaXLuUavdEFeyNrK36708747;     lSaXLuUavdEFeyNrK36708747 = lSaXLuUavdEFeyNrK14074522;     lSaXLuUavdEFeyNrK14074522 = lSaXLuUavdEFeyNrK7910420;     lSaXLuUavdEFeyNrK7910420 = lSaXLuUavdEFeyNrK10074619;     lSaXLuUavdEFeyNrK10074619 = lSaXLuUavdEFeyNrK9357793;     lSaXLuUavdEFeyNrK9357793 = lSaXLuUavdEFeyNrK69423179;     lSaXLuUavdEFeyNrK69423179 = lSaXLuUavdEFeyNrK24059315;     lSaXLuUavdEFeyNrK24059315 = lSaXLuUavdEFeyNrK29655862;     lSaXLuUavdEFeyNrK29655862 = lSaXLuUavdEFeyNrK4624839;     lSaXLuUavdEFeyNrK4624839 = lSaXLuUavdEFeyNrK32149729;     lSaXLuUavdEFeyNrK32149729 = lSaXLuUavdEFeyNrK96558752;     lSaXLuUavdEFeyNrK96558752 = lSaXLuUavdEFeyNrK37514056;     lSaXLuUavdEFeyNrK37514056 = lSaXLuUavdEFeyNrK70159538;     lSaXLuUavdEFeyNrK70159538 = lSaXLuUavdEFeyNrK38828627;     lSaXLuUavdEFeyNrK38828627 = lSaXLuUavdEFeyNrK57569965;     lSaXLuUavdEFeyNrK57569965 = lSaXLuUavdEFeyNrK35929325;     lSaXLuUavdEFeyNrK35929325 = lSaXLuUavdEFeyNrK73098048;     lSaXLuUavdEFeyNrK73098048 = lSaXLuUavdEFeyNrK87298962;     lSaXLuUavdEFeyNrK87298962 = lSaXLuUavdEFeyNrK21322545;     lSaXLuUavdEFeyNrK21322545 = lSaXLuUavdEFeyNrK96034241;     lSaXLuUavdEFeyNrK96034241 = lSaXLuUavdEFeyNrK12605696;     lSaXLuUavdEFeyNrK12605696 = lSaXLuUavdEFeyNrK92332671;     lSaXLuUavdEFeyNrK92332671 = lSaXLuUavdEFeyNrK3572622;     lSaXLuUavdEFeyNrK3572622 = lSaXLuUavdEFeyNrK30035127;     lSaXLuUavdEFeyNrK30035127 = lSaXLuUavdEFeyNrK40460434;     lSaXLuUavdEFeyNrK40460434 = lSaXLuUavdEFeyNrK1518410;     lSaXLuUavdEFeyNrK1518410 = lSaXLuUavdEFeyNrK70112666;     lSaXLuUavdEFeyNrK70112666 = lSaXLuUavdEFeyNrK7973423;     lSaXLuUavdEFeyNrK7973423 = lSaXLuUavdEFeyNrK39007075;     lSaXLuUavdEFeyNrK39007075 = lSaXLuUavdEFeyNrK23099276;     lSaXLuUavdEFeyNrK23099276 = lSaXLuUavdEFeyNrK5572146;     lSaXLuUavdEFeyNrK5572146 = lSaXLuUavdEFeyNrK64752104;     lSaXLuUavdEFeyNrK64752104 = lSaXLuUavdEFeyNrK40212690;     lSaXLuUavdEFeyNrK40212690 = lSaXLuUavdEFeyNrK16592926;     lSaXLuUavdEFeyNrK16592926 = lSaXLuUavdEFeyNrK77789490;     lSaXLuUavdEFeyNrK77789490 = lSaXLuUavdEFeyNrK25310556;     lSaXLuUavdEFeyNrK25310556 = lSaXLuUavdEFeyNrK73313078;     lSaXLuUavdEFeyNrK73313078 = lSaXLuUavdEFeyNrK8949445;     lSaXLuUavdEFeyNrK8949445 = lSaXLuUavdEFeyNrK21740637;     lSaXLuUavdEFeyNrK21740637 = lSaXLuUavdEFeyNrK26468433;     lSaXLuUavdEFeyNrK26468433 = lSaXLuUavdEFeyNrK53912263;     lSaXLuUavdEFeyNrK53912263 = lSaXLuUavdEFeyNrK12243569;     lSaXLuUavdEFeyNrK12243569 = lSaXLuUavdEFeyNrK80221172;     lSaXLuUavdEFeyNrK80221172 = lSaXLuUavdEFeyNrK44152722;     lSaXLuUavdEFeyNrK44152722 = lSaXLuUavdEFeyNrK53539860;     lSaXLuUavdEFeyNrK53539860 = lSaXLuUavdEFeyNrK8354339;     lSaXLuUavdEFeyNrK8354339 = lSaXLuUavdEFeyNrK60873344;     lSaXLuUavdEFeyNrK60873344 = lSaXLuUavdEFeyNrK80397204;     lSaXLuUavdEFeyNrK80397204 = lSaXLuUavdEFeyNrK73001376;     lSaXLuUavdEFeyNrK73001376 = lSaXLuUavdEFeyNrK95304537;     lSaXLuUavdEFeyNrK95304537 = lSaXLuUavdEFeyNrK12629651;     lSaXLuUavdEFeyNrK12629651 = lSaXLuUavdEFeyNrK78015699;     lSaXLuUavdEFeyNrK78015699 = lSaXLuUavdEFeyNrK48533885;     lSaXLuUavdEFeyNrK48533885 = lSaXLuUavdEFeyNrK66575102;     lSaXLuUavdEFeyNrK66575102 = lSaXLuUavdEFeyNrK29643644;     lSaXLuUavdEFeyNrK29643644 = lSaXLuUavdEFeyNrK44376075;     lSaXLuUavdEFeyNrK44376075 = lSaXLuUavdEFeyNrK10501901;     lSaXLuUavdEFeyNrK10501901 = lSaXLuUavdEFeyNrK77875292;     lSaXLuUavdEFeyNrK77875292 = lSaXLuUavdEFeyNrK69614185;     lSaXLuUavdEFeyNrK69614185 = lSaXLuUavdEFeyNrK7839384;     lSaXLuUavdEFeyNrK7839384 = lSaXLuUavdEFeyNrK99310513;     lSaXLuUavdEFeyNrK99310513 = lSaXLuUavdEFeyNrK16085892;     lSaXLuUavdEFeyNrK16085892 = lSaXLuUavdEFeyNrK90648786;     lSaXLuUavdEFeyNrK90648786 = lSaXLuUavdEFeyNrK81525562;     lSaXLuUavdEFeyNrK81525562 = lSaXLuUavdEFeyNrK74005819;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gkAKfjFCozLrTako84505756() {     double lQpkRAqTAyNpZUSAB9464788 = -73923478;    double lQpkRAqTAyNpZUSAB67777204 = -530678478;    double lQpkRAqTAyNpZUSAB14565160 = -695352054;    double lQpkRAqTAyNpZUSAB52098427 = -231918331;    double lQpkRAqTAyNpZUSAB36419008 = -434168344;    double lQpkRAqTAyNpZUSAB22551700 = -907494389;    double lQpkRAqTAyNpZUSAB43208698 = -155947023;    double lQpkRAqTAyNpZUSAB51470059 = 61400504;    double lQpkRAqTAyNpZUSAB40062370 = -767189825;    double lQpkRAqTAyNpZUSAB85143649 = -721291768;    double lQpkRAqTAyNpZUSAB47008710 = -338665973;    double lQpkRAqTAyNpZUSAB71454955 = -860137368;    double lQpkRAqTAyNpZUSAB72843180 = -318786012;    double lQpkRAqTAyNpZUSAB55962897 = -322744520;    double lQpkRAqTAyNpZUSAB17036781 = -661977083;    double lQpkRAqTAyNpZUSAB98039538 = -282644559;    double lQpkRAqTAyNpZUSAB95221013 = 20121163;    double lQpkRAqTAyNpZUSAB80983689 = -86557995;    double lQpkRAqTAyNpZUSAB19574393 = -931870903;    double lQpkRAqTAyNpZUSAB24935648 = -945833148;    double lQpkRAqTAyNpZUSAB8978476 = -542108811;    double lQpkRAqTAyNpZUSAB81393102 = -783515394;    double lQpkRAqTAyNpZUSAB12424810 = -459231773;    double lQpkRAqTAyNpZUSAB78638956 = -700811142;    double lQpkRAqTAyNpZUSAB84320402 = -67543097;    double lQpkRAqTAyNpZUSAB60796784 = -321029234;    double lQpkRAqTAyNpZUSAB6288968 = -643388935;    double lQpkRAqTAyNpZUSAB71761307 = -119956535;    double lQpkRAqTAyNpZUSAB65894046 = -41406944;    double lQpkRAqTAyNpZUSAB36104752 = -881593385;    double lQpkRAqTAyNpZUSAB49817094 = -153243367;    double lQpkRAqTAyNpZUSAB23550718 = -356881320;    double lQpkRAqTAyNpZUSAB77532672 = -910053375;    double lQpkRAqTAyNpZUSAB99604856 = 20957896;    double lQpkRAqTAyNpZUSAB28417798 = -757773732;    double lQpkRAqTAyNpZUSAB93810183 = -975499976;    double lQpkRAqTAyNpZUSAB86120563 = -743113340;    double lQpkRAqTAyNpZUSAB65289925 = -902281536;    double lQpkRAqTAyNpZUSAB38300981 = -437937529;    double lQpkRAqTAyNpZUSAB74346151 = -17100919;    double lQpkRAqTAyNpZUSAB42809400 = -409627715;    double lQpkRAqTAyNpZUSAB58788306 = -61253780;    double lQpkRAqTAyNpZUSAB13442693 = -416149674;    double lQpkRAqTAyNpZUSAB69787200 = 86145097;    double lQpkRAqTAyNpZUSAB4641297 = -557526792;    double lQpkRAqTAyNpZUSAB78111057 = -716826959;    double lQpkRAqTAyNpZUSAB15903917 = -29076031;    double lQpkRAqTAyNpZUSAB55240367 = -604624577;    double lQpkRAqTAyNpZUSAB93052204 = -648556592;    double lQpkRAqTAyNpZUSAB63358216 = 21562167;    double lQpkRAqTAyNpZUSAB38340689 = -135139058;    double lQpkRAqTAyNpZUSAB19414797 = -8106777;    double lQpkRAqTAyNpZUSAB41719197 = -963793270;    double lQpkRAqTAyNpZUSAB3290809 = -675099771;    double lQpkRAqTAyNpZUSAB75527281 = -564874838;    double lQpkRAqTAyNpZUSAB28071685 = -290408085;    double lQpkRAqTAyNpZUSAB55352395 = 28553295;    double lQpkRAqTAyNpZUSAB35926203 = -994540912;    double lQpkRAqTAyNpZUSAB67778025 = -64375234;    double lQpkRAqTAyNpZUSAB75622224 = -13139111;    double lQpkRAqTAyNpZUSAB16262732 = -164105454;    double lQpkRAqTAyNpZUSAB71447390 = 64009511;    double lQpkRAqTAyNpZUSAB85576012 = -897192552;    double lQpkRAqTAyNpZUSAB3957618 = -885596440;    double lQpkRAqTAyNpZUSAB35326556 = -468048402;    double lQpkRAqTAyNpZUSAB23457992 = -981784654;    double lQpkRAqTAyNpZUSAB93922282 = -950083994;    double lQpkRAqTAyNpZUSAB73238323 = -239743909;    double lQpkRAqTAyNpZUSAB27545100 = -564970788;    double lQpkRAqTAyNpZUSAB23226598 = -686477108;    double lQpkRAqTAyNpZUSAB11918975 = -539531220;    double lQpkRAqTAyNpZUSAB29931088 = -77597302;    double lQpkRAqTAyNpZUSAB42682708 = -648620466;    double lQpkRAqTAyNpZUSAB45228242 = -814769984;    double lQpkRAqTAyNpZUSAB82126247 = -436205434;    double lQpkRAqTAyNpZUSAB50190169 = -380855031;    double lQpkRAqTAyNpZUSAB67950410 = -267365720;    double lQpkRAqTAyNpZUSAB42637610 = -445376871;    double lQpkRAqTAyNpZUSAB73997659 = -43284351;    double lQpkRAqTAyNpZUSAB6209345 = -350716139;    double lQpkRAqTAyNpZUSAB44892868 = -191953203;    double lQpkRAqTAyNpZUSAB51048601 = 61235641;    double lQpkRAqTAyNpZUSAB78709103 = -471399943;    double lQpkRAqTAyNpZUSAB2535830 = 37030888;    double lQpkRAqTAyNpZUSAB97764063 = -646454328;    double lQpkRAqTAyNpZUSAB30402297 = -45136590;    double lQpkRAqTAyNpZUSAB81831520 = -393088051;    double lQpkRAqTAyNpZUSAB74241864 = -134953605;    double lQpkRAqTAyNpZUSAB24077576 = -414167266;    double lQpkRAqTAyNpZUSAB346113 = -367365648;    double lQpkRAqTAyNpZUSAB38457789 = -904053271;    double lQpkRAqTAyNpZUSAB50194360 = -748572428;    double lQpkRAqTAyNpZUSAB97511900 = -737906303;    double lQpkRAqTAyNpZUSAB62678757 = -324798419;    double lQpkRAqTAyNpZUSAB58083419 = -852995466;    double lQpkRAqTAyNpZUSAB71362009 = -373637227;    double lQpkRAqTAyNpZUSAB73212293 = -164061228;    double lQpkRAqTAyNpZUSAB9485076 = -530553235;    double lQpkRAqTAyNpZUSAB34460644 = -445806501;    double lQpkRAqTAyNpZUSAB81183304 = -73923478;     lQpkRAqTAyNpZUSAB9464788 = lQpkRAqTAyNpZUSAB67777204;     lQpkRAqTAyNpZUSAB67777204 = lQpkRAqTAyNpZUSAB14565160;     lQpkRAqTAyNpZUSAB14565160 = lQpkRAqTAyNpZUSAB52098427;     lQpkRAqTAyNpZUSAB52098427 = lQpkRAqTAyNpZUSAB36419008;     lQpkRAqTAyNpZUSAB36419008 = lQpkRAqTAyNpZUSAB22551700;     lQpkRAqTAyNpZUSAB22551700 = lQpkRAqTAyNpZUSAB43208698;     lQpkRAqTAyNpZUSAB43208698 = lQpkRAqTAyNpZUSAB51470059;     lQpkRAqTAyNpZUSAB51470059 = lQpkRAqTAyNpZUSAB40062370;     lQpkRAqTAyNpZUSAB40062370 = lQpkRAqTAyNpZUSAB85143649;     lQpkRAqTAyNpZUSAB85143649 = lQpkRAqTAyNpZUSAB47008710;     lQpkRAqTAyNpZUSAB47008710 = lQpkRAqTAyNpZUSAB71454955;     lQpkRAqTAyNpZUSAB71454955 = lQpkRAqTAyNpZUSAB72843180;     lQpkRAqTAyNpZUSAB72843180 = lQpkRAqTAyNpZUSAB55962897;     lQpkRAqTAyNpZUSAB55962897 = lQpkRAqTAyNpZUSAB17036781;     lQpkRAqTAyNpZUSAB17036781 = lQpkRAqTAyNpZUSAB98039538;     lQpkRAqTAyNpZUSAB98039538 = lQpkRAqTAyNpZUSAB95221013;     lQpkRAqTAyNpZUSAB95221013 = lQpkRAqTAyNpZUSAB80983689;     lQpkRAqTAyNpZUSAB80983689 = lQpkRAqTAyNpZUSAB19574393;     lQpkRAqTAyNpZUSAB19574393 = lQpkRAqTAyNpZUSAB24935648;     lQpkRAqTAyNpZUSAB24935648 = lQpkRAqTAyNpZUSAB8978476;     lQpkRAqTAyNpZUSAB8978476 = lQpkRAqTAyNpZUSAB81393102;     lQpkRAqTAyNpZUSAB81393102 = lQpkRAqTAyNpZUSAB12424810;     lQpkRAqTAyNpZUSAB12424810 = lQpkRAqTAyNpZUSAB78638956;     lQpkRAqTAyNpZUSAB78638956 = lQpkRAqTAyNpZUSAB84320402;     lQpkRAqTAyNpZUSAB84320402 = lQpkRAqTAyNpZUSAB60796784;     lQpkRAqTAyNpZUSAB60796784 = lQpkRAqTAyNpZUSAB6288968;     lQpkRAqTAyNpZUSAB6288968 = lQpkRAqTAyNpZUSAB71761307;     lQpkRAqTAyNpZUSAB71761307 = lQpkRAqTAyNpZUSAB65894046;     lQpkRAqTAyNpZUSAB65894046 = lQpkRAqTAyNpZUSAB36104752;     lQpkRAqTAyNpZUSAB36104752 = lQpkRAqTAyNpZUSAB49817094;     lQpkRAqTAyNpZUSAB49817094 = lQpkRAqTAyNpZUSAB23550718;     lQpkRAqTAyNpZUSAB23550718 = lQpkRAqTAyNpZUSAB77532672;     lQpkRAqTAyNpZUSAB77532672 = lQpkRAqTAyNpZUSAB99604856;     lQpkRAqTAyNpZUSAB99604856 = lQpkRAqTAyNpZUSAB28417798;     lQpkRAqTAyNpZUSAB28417798 = lQpkRAqTAyNpZUSAB93810183;     lQpkRAqTAyNpZUSAB93810183 = lQpkRAqTAyNpZUSAB86120563;     lQpkRAqTAyNpZUSAB86120563 = lQpkRAqTAyNpZUSAB65289925;     lQpkRAqTAyNpZUSAB65289925 = lQpkRAqTAyNpZUSAB38300981;     lQpkRAqTAyNpZUSAB38300981 = lQpkRAqTAyNpZUSAB74346151;     lQpkRAqTAyNpZUSAB74346151 = lQpkRAqTAyNpZUSAB42809400;     lQpkRAqTAyNpZUSAB42809400 = lQpkRAqTAyNpZUSAB58788306;     lQpkRAqTAyNpZUSAB58788306 = lQpkRAqTAyNpZUSAB13442693;     lQpkRAqTAyNpZUSAB13442693 = lQpkRAqTAyNpZUSAB69787200;     lQpkRAqTAyNpZUSAB69787200 = lQpkRAqTAyNpZUSAB4641297;     lQpkRAqTAyNpZUSAB4641297 = lQpkRAqTAyNpZUSAB78111057;     lQpkRAqTAyNpZUSAB78111057 = lQpkRAqTAyNpZUSAB15903917;     lQpkRAqTAyNpZUSAB15903917 = lQpkRAqTAyNpZUSAB55240367;     lQpkRAqTAyNpZUSAB55240367 = lQpkRAqTAyNpZUSAB93052204;     lQpkRAqTAyNpZUSAB93052204 = lQpkRAqTAyNpZUSAB63358216;     lQpkRAqTAyNpZUSAB63358216 = lQpkRAqTAyNpZUSAB38340689;     lQpkRAqTAyNpZUSAB38340689 = lQpkRAqTAyNpZUSAB19414797;     lQpkRAqTAyNpZUSAB19414797 = lQpkRAqTAyNpZUSAB41719197;     lQpkRAqTAyNpZUSAB41719197 = lQpkRAqTAyNpZUSAB3290809;     lQpkRAqTAyNpZUSAB3290809 = lQpkRAqTAyNpZUSAB75527281;     lQpkRAqTAyNpZUSAB75527281 = lQpkRAqTAyNpZUSAB28071685;     lQpkRAqTAyNpZUSAB28071685 = lQpkRAqTAyNpZUSAB55352395;     lQpkRAqTAyNpZUSAB55352395 = lQpkRAqTAyNpZUSAB35926203;     lQpkRAqTAyNpZUSAB35926203 = lQpkRAqTAyNpZUSAB67778025;     lQpkRAqTAyNpZUSAB67778025 = lQpkRAqTAyNpZUSAB75622224;     lQpkRAqTAyNpZUSAB75622224 = lQpkRAqTAyNpZUSAB16262732;     lQpkRAqTAyNpZUSAB16262732 = lQpkRAqTAyNpZUSAB71447390;     lQpkRAqTAyNpZUSAB71447390 = lQpkRAqTAyNpZUSAB85576012;     lQpkRAqTAyNpZUSAB85576012 = lQpkRAqTAyNpZUSAB3957618;     lQpkRAqTAyNpZUSAB3957618 = lQpkRAqTAyNpZUSAB35326556;     lQpkRAqTAyNpZUSAB35326556 = lQpkRAqTAyNpZUSAB23457992;     lQpkRAqTAyNpZUSAB23457992 = lQpkRAqTAyNpZUSAB93922282;     lQpkRAqTAyNpZUSAB93922282 = lQpkRAqTAyNpZUSAB73238323;     lQpkRAqTAyNpZUSAB73238323 = lQpkRAqTAyNpZUSAB27545100;     lQpkRAqTAyNpZUSAB27545100 = lQpkRAqTAyNpZUSAB23226598;     lQpkRAqTAyNpZUSAB23226598 = lQpkRAqTAyNpZUSAB11918975;     lQpkRAqTAyNpZUSAB11918975 = lQpkRAqTAyNpZUSAB29931088;     lQpkRAqTAyNpZUSAB29931088 = lQpkRAqTAyNpZUSAB42682708;     lQpkRAqTAyNpZUSAB42682708 = lQpkRAqTAyNpZUSAB45228242;     lQpkRAqTAyNpZUSAB45228242 = lQpkRAqTAyNpZUSAB82126247;     lQpkRAqTAyNpZUSAB82126247 = lQpkRAqTAyNpZUSAB50190169;     lQpkRAqTAyNpZUSAB50190169 = lQpkRAqTAyNpZUSAB67950410;     lQpkRAqTAyNpZUSAB67950410 = lQpkRAqTAyNpZUSAB42637610;     lQpkRAqTAyNpZUSAB42637610 = lQpkRAqTAyNpZUSAB73997659;     lQpkRAqTAyNpZUSAB73997659 = lQpkRAqTAyNpZUSAB6209345;     lQpkRAqTAyNpZUSAB6209345 = lQpkRAqTAyNpZUSAB44892868;     lQpkRAqTAyNpZUSAB44892868 = lQpkRAqTAyNpZUSAB51048601;     lQpkRAqTAyNpZUSAB51048601 = lQpkRAqTAyNpZUSAB78709103;     lQpkRAqTAyNpZUSAB78709103 = lQpkRAqTAyNpZUSAB2535830;     lQpkRAqTAyNpZUSAB2535830 = lQpkRAqTAyNpZUSAB97764063;     lQpkRAqTAyNpZUSAB97764063 = lQpkRAqTAyNpZUSAB30402297;     lQpkRAqTAyNpZUSAB30402297 = lQpkRAqTAyNpZUSAB81831520;     lQpkRAqTAyNpZUSAB81831520 = lQpkRAqTAyNpZUSAB74241864;     lQpkRAqTAyNpZUSAB74241864 = lQpkRAqTAyNpZUSAB24077576;     lQpkRAqTAyNpZUSAB24077576 = lQpkRAqTAyNpZUSAB346113;     lQpkRAqTAyNpZUSAB346113 = lQpkRAqTAyNpZUSAB38457789;     lQpkRAqTAyNpZUSAB38457789 = lQpkRAqTAyNpZUSAB50194360;     lQpkRAqTAyNpZUSAB50194360 = lQpkRAqTAyNpZUSAB97511900;     lQpkRAqTAyNpZUSAB97511900 = lQpkRAqTAyNpZUSAB62678757;     lQpkRAqTAyNpZUSAB62678757 = lQpkRAqTAyNpZUSAB58083419;     lQpkRAqTAyNpZUSAB58083419 = lQpkRAqTAyNpZUSAB71362009;     lQpkRAqTAyNpZUSAB71362009 = lQpkRAqTAyNpZUSAB73212293;     lQpkRAqTAyNpZUSAB73212293 = lQpkRAqTAyNpZUSAB9485076;     lQpkRAqTAyNpZUSAB9485076 = lQpkRAqTAyNpZUSAB34460644;     lQpkRAqTAyNpZUSAB34460644 = lQpkRAqTAyNpZUSAB81183304;     lQpkRAqTAyNpZUSAB81183304 = lQpkRAqTAyNpZUSAB9464788;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RGfRoUSSIbZzWsQC99554824() {     double XBVbGMqRvUyvUwOqr15581895 = -185825367;    double XBVbGMqRvUyvUwOqr11150484 = -240278258;    double XBVbGMqRvUyvUwOqr30607343 = -846184975;    double XBVbGMqRvUyvUwOqr79173207 = -422666961;    double XBVbGMqRvUyvUwOqr5314497 = -954818215;    double XBVbGMqRvUyvUwOqr13968902 = -71990624;    double XBVbGMqRvUyvUwOqr3697665 = -294912000;    double XBVbGMqRvUyvUwOqr1229950 = -225473409;    double XBVbGMqRvUyvUwOqr4704865 = -535222490;    double XBVbGMqRvUyvUwOqr26373309 = 49732754;    double XBVbGMqRvUyvUwOqr7119001 = -216103560;    double XBVbGMqRvUyvUwOqr3277998 = -204543570;    double XBVbGMqRvUyvUwOqr59052299 = -758677101;    double XBVbGMqRvUyvUwOqr39106574 = -854983088;    double XBVbGMqRvUyvUwOqr12779208 = -252439250;    double XBVbGMqRvUyvUwOqr39544932 = -255644102;    double XBVbGMqRvUyvUwOqr89057016 = -596423950;    double XBVbGMqRvUyvUwOqr78036842 = -852080638;    double XBVbGMqRvUyvUwOqr81404234 = -237640000;    double XBVbGMqRvUyvUwOqr5485813 = -202736068;    double XBVbGMqRvUyvUwOqr97531668 = -397596267;    double XBVbGMqRvUyvUwOqr67914126 = -691445110;    double XBVbGMqRvUyvUwOqr48015601 = -205749260;    double XBVbGMqRvUyvUwOqr62631330 = -923116130;    double XBVbGMqRvUyvUwOqr28020052 = -440739130;    double XBVbGMqRvUyvUwOqr85518459 = -199684557;    double XBVbGMqRvUyvUwOqr4748747 = -544051139;    double XBVbGMqRvUyvUwOqr84868432 = -185885488;    double XBVbGMqRvUyvUwOqr32026302 = -263789649;    double XBVbGMqRvUyvUwOqr4720389 = -699064253;    double XBVbGMqRvUyvUwOqr42059643 = -66564305;    double XBVbGMqRvUyvUwOqr21161340 = -66328135;    double XBVbGMqRvUyvUwOqr87243621 = -802666745;    double XBVbGMqRvUyvUwOqr94435421 = -830583869;    double XBVbGMqRvUyvUwOqr89892449 = -296506837;    double XBVbGMqRvUyvUwOqr62536096 = -89526084;    double XBVbGMqRvUyvUwOqr67382498 = -696749209;    double XBVbGMqRvUyvUwOqr99380137 = -673787546;    double XBVbGMqRvUyvUwOqr11911235 = -431861026;    double XBVbGMqRvUyvUwOqr61848088 = -490540142;    double XBVbGMqRvUyvUwOqr48478624 = -474901609;    double XBVbGMqRvUyvUwOqr90938762 = -331396058;    double XBVbGMqRvUyvUwOqr91509670 = -496954258;    double XBVbGMqRvUyvUwOqr57194940 = 66802159;    double XBVbGMqRvUyvUwOqr16593762 = -219424947;    double XBVbGMqRvUyvUwOqr90050252 = -702043076;    double XBVbGMqRvUyvUwOqr33682491 = -147851569;    double XBVbGMqRvUyvUwOqr63197308 = -614520109;    double XBVbGMqRvUyvUwOqr2727934 = -734293581;    double XBVbGMqRvUyvUwOqr76269869 = -65448641;    double XBVbGMqRvUyvUwOqr74041367 = 22567270;    double XBVbGMqRvUyvUwOqr30224517 = -908833097;    double XBVbGMqRvUyvUwOqr89967654 = -755582857;    double XBVbGMqRvUyvUwOqr99259689 = -995569916;    double XBVbGMqRvUyvUwOqr2023157 = -165406621;    double XBVbGMqRvUyvUwOqr47667768 = -494380257;    double XBVbGMqRvUyvUwOqr63134882 = 65471001;    double XBVbGMqRvUyvUwOqr67976013 = -923068845;    double XBVbGMqRvUyvUwOqr51153155 = -981927831;    double XBVbGMqRvUyvUwOqr19796038 = -655133659;    double XBVbGMqRvUyvUwOqr9220156 = -527939486;    double XBVbGMqRvUyvUwOqr18829232 = -9026512;    double XBVbGMqRvUyvUwOqr69203647 = -961683761;    double XBVbGMqRvUyvUwOqr99984476 = -836158238;    double XBVbGMqRvUyvUwOqr84313665 = -883702942;    double XBVbGMqRvUyvUwOqr85957660 = -49775425;    double XBVbGMqRvUyvUwOqr16034377 = -401876825;    double XBVbGMqRvUyvUwOqr64616878 = -928093233;    double XBVbGMqRvUyvUwOqr49214124 = -458476251;    double XBVbGMqRvUyvUwOqr50243111 = -62913167;    double XBVbGMqRvUyvUwOqr72162434 = -558894893;    double XBVbGMqRvUyvUwOqr89676878 = -922636405;    double XBVbGMqRvUyvUwOqr66125607 = -320219612;    double XBVbGMqRvUyvUwOqr19556147 = -747099859;    double XBVbGMqRvUyvUwOqr57007188 = -727834459;    double XBVbGMqRvUyvUwOqr6592906 = 33799791;    double XBVbGMqRvUyvUwOqr76404455 = -94490853;    double XBVbGMqRvUyvUwOqr90820661 = -172551419;    double XBVbGMqRvUyvUwOqr46037568 = -603691184;    double XBVbGMqRvUyvUwOqr37969800 = -738696055;    double XBVbGMqRvUyvUwOqr51835969 = 48167011;    double XBVbGMqRvUyvUwOqr41551438 = -929531030;    double XBVbGMqRvUyvUwOqr82140499 = -451591908;    double XBVbGMqRvUyvUwOqr55756433 = -98341008;    double XBVbGMqRvUyvUwOqr30679022 = -621631524;    double XBVbGMqRvUyvUwOqr11835127 = -157731208;    double XBVbGMqRvUyvUwOqr31193685 = -310745279;    double XBVbGMqRvUyvUwOqr87983932 = -807096830;    double XBVbGMqRvUyvUwOqr92412264 = -565177248;    double XBVbGMqRvUyvUwOqr42224681 = -802126581;    double XBVbGMqRvUyvUwOqr99401214 = -54997085;    double XBVbGMqRvUyvUwOqr99406484 = -773680365;    double XBVbGMqRvUyvUwOqr48226982 = -691859715;    double XBVbGMqRvUyvUwOqr92115197 = -776727368;    double XBVbGMqRvUyvUwOqr52627933 = -962600657;    double XBVbGMqRvUyvUwOqr29649393 = -365875097;    double XBVbGMqRvUyvUwOqr21735115 = -369712298;    double XBVbGMqRvUyvUwOqr91525194 = -660796021;    double XBVbGMqRvUyvUwOqr72881274 = -49494900;    double XBVbGMqRvUyvUwOqr30636101 = -185825367;     XBVbGMqRvUyvUwOqr15581895 = XBVbGMqRvUyvUwOqr11150484;     XBVbGMqRvUyvUwOqr11150484 = XBVbGMqRvUyvUwOqr30607343;     XBVbGMqRvUyvUwOqr30607343 = XBVbGMqRvUyvUwOqr79173207;     XBVbGMqRvUyvUwOqr79173207 = XBVbGMqRvUyvUwOqr5314497;     XBVbGMqRvUyvUwOqr5314497 = XBVbGMqRvUyvUwOqr13968902;     XBVbGMqRvUyvUwOqr13968902 = XBVbGMqRvUyvUwOqr3697665;     XBVbGMqRvUyvUwOqr3697665 = XBVbGMqRvUyvUwOqr1229950;     XBVbGMqRvUyvUwOqr1229950 = XBVbGMqRvUyvUwOqr4704865;     XBVbGMqRvUyvUwOqr4704865 = XBVbGMqRvUyvUwOqr26373309;     XBVbGMqRvUyvUwOqr26373309 = XBVbGMqRvUyvUwOqr7119001;     XBVbGMqRvUyvUwOqr7119001 = XBVbGMqRvUyvUwOqr3277998;     XBVbGMqRvUyvUwOqr3277998 = XBVbGMqRvUyvUwOqr59052299;     XBVbGMqRvUyvUwOqr59052299 = XBVbGMqRvUyvUwOqr39106574;     XBVbGMqRvUyvUwOqr39106574 = XBVbGMqRvUyvUwOqr12779208;     XBVbGMqRvUyvUwOqr12779208 = XBVbGMqRvUyvUwOqr39544932;     XBVbGMqRvUyvUwOqr39544932 = XBVbGMqRvUyvUwOqr89057016;     XBVbGMqRvUyvUwOqr89057016 = XBVbGMqRvUyvUwOqr78036842;     XBVbGMqRvUyvUwOqr78036842 = XBVbGMqRvUyvUwOqr81404234;     XBVbGMqRvUyvUwOqr81404234 = XBVbGMqRvUyvUwOqr5485813;     XBVbGMqRvUyvUwOqr5485813 = XBVbGMqRvUyvUwOqr97531668;     XBVbGMqRvUyvUwOqr97531668 = XBVbGMqRvUyvUwOqr67914126;     XBVbGMqRvUyvUwOqr67914126 = XBVbGMqRvUyvUwOqr48015601;     XBVbGMqRvUyvUwOqr48015601 = XBVbGMqRvUyvUwOqr62631330;     XBVbGMqRvUyvUwOqr62631330 = XBVbGMqRvUyvUwOqr28020052;     XBVbGMqRvUyvUwOqr28020052 = XBVbGMqRvUyvUwOqr85518459;     XBVbGMqRvUyvUwOqr85518459 = XBVbGMqRvUyvUwOqr4748747;     XBVbGMqRvUyvUwOqr4748747 = XBVbGMqRvUyvUwOqr84868432;     XBVbGMqRvUyvUwOqr84868432 = XBVbGMqRvUyvUwOqr32026302;     XBVbGMqRvUyvUwOqr32026302 = XBVbGMqRvUyvUwOqr4720389;     XBVbGMqRvUyvUwOqr4720389 = XBVbGMqRvUyvUwOqr42059643;     XBVbGMqRvUyvUwOqr42059643 = XBVbGMqRvUyvUwOqr21161340;     XBVbGMqRvUyvUwOqr21161340 = XBVbGMqRvUyvUwOqr87243621;     XBVbGMqRvUyvUwOqr87243621 = XBVbGMqRvUyvUwOqr94435421;     XBVbGMqRvUyvUwOqr94435421 = XBVbGMqRvUyvUwOqr89892449;     XBVbGMqRvUyvUwOqr89892449 = XBVbGMqRvUyvUwOqr62536096;     XBVbGMqRvUyvUwOqr62536096 = XBVbGMqRvUyvUwOqr67382498;     XBVbGMqRvUyvUwOqr67382498 = XBVbGMqRvUyvUwOqr99380137;     XBVbGMqRvUyvUwOqr99380137 = XBVbGMqRvUyvUwOqr11911235;     XBVbGMqRvUyvUwOqr11911235 = XBVbGMqRvUyvUwOqr61848088;     XBVbGMqRvUyvUwOqr61848088 = XBVbGMqRvUyvUwOqr48478624;     XBVbGMqRvUyvUwOqr48478624 = XBVbGMqRvUyvUwOqr90938762;     XBVbGMqRvUyvUwOqr90938762 = XBVbGMqRvUyvUwOqr91509670;     XBVbGMqRvUyvUwOqr91509670 = XBVbGMqRvUyvUwOqr57194940;     XBVbGMqRvUyvUwOqr57194940 = XBVbGMqRvUyvUwOqr16593762;     XBVbGMqRvUyvUwOqr16593762 = XBVbGMqRvUyvUwOqr90050252;     XBVbGMqRvUyvUwOqr90050252 = XBVbGMqRvUyvUwOqr33682491;     XBVbGMqRvUyvUwOqr33682491 = XBVbGMqRvUyvUwOqr63197308;     XBVbGMqRvUyvUwOqr63197308 = XBVbGMqRvUyvUwOqr2727934;     XBVbGMqRvUyvUwOqr2727934 = XBVbGMqRvUyvUwOqr76269869;     XBVbGMqRvUyvUwOqr76269869 = XBVbGMqRvUyvUwOqr74041367;     XBVbGMqRvUyvUwOqr74041367 = XBVbGMqRvUyvUwOqr30224517;     XBVbGMqRvUyvUwOqr30224517 = XBVbGMqRvUyvUwOqr89967654;     XBVbGMqRvUyvUwOqr89967654 = XBVbGMqRvUyvUwOqr99259689;     XBVbGMqRvUyvUwOqr99259689 = XBVbGMqRvUyvUwOqr2023157;     XBVbGMqRvUyvUwOqr2023157 = XBVbGMqRvUyvUwOqr47667768;     XBVbGMqRvUyvUwOqr47667768 = XBVbGMqRvUyvUwOqr63134882;     XBVbGMqRvUyvUwOqr63134882 = XBVbGMqRvUyvUwOqr67976013;     XBVbGMqRvUyvUwOqr67976013 = XBVbGMqRvUyvUwOqr51153155;     XBVbGMqRvUyvUwOqr51153155 = XBVbGMqRvUyvUwOqr19796038;     XBVbGMqRvUyvUwOqr19796038 = XBVbGMqRvUyvUwOqr9220156;     XBVbGMqRvUyvUwOqr9220156 = XBVbGMqRvUyvUwOqr18829232;     XBVbGMqRvUyvUwOqr18829232 = XBVbGMqRvUyvUwOqr69203647;     XBVbGMqRvUyvUwOqr69203647 = XBVbGMqRvUyvUwOqr99984476;     XBVbGMqRvUyvUwOqr99984476 = XBVbGMqRvUyvUwOqr84313665;     XBVbGMqRvUyvUwOqr84313665 = XBVbGMqRvUyvUwOqr85957660;     XBVbGMqRvUyvUwOqr85957660 = XBVbGMqRvUyvUwOqr16034377;     XBVbGMqRvUyvUwOqr16034377 = XBVbGMqRvUyvUwOqr64616878;     XBVbGMqRvUyvUwOqr64616878 = XBVbGMqRvUyvUwOqr49214124;     XBVbGMqRvUyvUwOqr49214124 = XBVbGMqRvUyvUwOqr50243111;     XBVbGMqRvUyvUwOqr50243111 = XBVbGMqRvUyvUwOqr72162434;     XBVbGMqRvUyvUwOqr72162434 = XBVbGMqRvUyvUwOqr89676878;     XBVbGMqRvUyvUwOqr89676878 = XBVbGMqRvUyvUwOqr66125607;     XBVbGMqRvUyvUwOqr66125607 = XBVbGMqRvUyvUwOqr19556147;     XBVbGMqRvUyvUwOqr19556147 = XBVbGMqRvUyvUwOqr57007188;     XBVbGMqRvUyvUwOqr57007188 = XBVbGMqRvUyvUwOqr6592906;     XBVbGMqRvUyvUwOqr6592906 = XBVbGMqRvUyvUwOqr76404455;     XBVbGMqRvUyvUwOqr76404455 = XBVbGMqRvUyvUwOqr90820661;     XBVbGMqRvUyvUwOqr90820661 = XBVbGMqRvUyvUwOqr46037568;     XBVbGMqRvUyvUwOqr46037568 = XBVbGMqRvUyvUwOqr37969800;     XBVbGMqRvUyvUwOqr37969800 = XBVbGMqRvUyvUwOqr51835969;     XBVbGMqRvUyvUwOqr51835969 = XBVbGMqRvUyvUwOqr41551438;     XBVbGMqRvUyvUwOqr41551438 = XBVbGMqRvUyvUwOqr82140499;     XBVbGMqRvUyvUwOqr82140499 = XBVbGMqRvUyvUwOqr55756433;     XBVbGMqRvUyvUwOqr55756433 = XBVbGMqRvUyvUwOqr30679022;     XBVbGMqRvUyvUwOqr30679022 = XBVbGMqRvUyvUwOqr11835127;     XBVbGMqRvUyvUwOqr11835127 = XBVbGMqRvUyvUwOqr31193685;     XBVbGMqRvUyvUwOqr31193685 = XBVbGMqRvUyvUwOqr87983932;     XBVbGMqRvUyvUwOqr87983932 = XBVbGMqRvUyvUwOqr92412264;     XBVbGMqRvUyvUwOqr92412264 = XBVbGMqRvUyvUwOqr42224681;     XBVbGMqRvUyvUwOqr42224681 = XBVbGMqRvUyvUwOqr99401214;     XBVbGMqRvUyvUwOqr99401214 = XBVbGMqRvUyvUwOqr99406484;     XBVbGMqRvUyvUwOqr99406484 = XBVbGMqRvUyvUwOqr48226982;     XBVbGMqRvUyvUwOqr48226982 = XBVbGMqRvUyvUwOqr92115197;     XBVbGMqRvUyvUwOqr92115197 = XBVbGMqRvUyvUwOqr52627933;     XBVbGMqRvUyvUwOqr52627933 = XBVbGMqRvUyvUwOqr29649393;     XBVbGMqRvUyvUwOqr29649393 = XBVbGMqRvUyvUwOqr21735115;     XBVbGMqRvUyvUwOqr21735115 = XBVbGMqRvUyvUwOqr91525194;     XBVbGMqRvUyvUwOqr91525194 = XBVbGMqRvUyvUwOqr72881274;     XBVbGMqRvUyvUwOqr72881274 = XBVbGMqRvUyvUwOqr30636101;     XBVbGMqRvUyvUwOqr30636101 = XBVbGMqRvUyvUwOqr15581895;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RosAaWVKmNvoLvYj86364897() {     double LdOMNMFIvcrrYjlso60638577 = -637727609;    double LdOMNMFIvcrrYjlso91285332 = 79663531;    double LdOMNMFIvcrrYjlso29666905 = -871710819;    double LdOMNMFIvcrrYjlso51002356 = -886293702;    double LdOMNMFIvcrrYjlso70682967 = -293835257;    double LdOMNMFIvcrrYjlso39167537 = -267739989;    double LdOMNMFIvcrrYjlso69576719 = -918221204;    double LdOMNMFIvcrrYjlso46883282 = -847874596;    double LdOMNMFIvcrrYjlso49350873 = -252814879;    double LdOMNMFIvcrrYjlso25140658 = 77988620;    double LdOMNMFIvcrrYjlso65515515 = -979785711;    double LdOMNMFIvcrrYjlso34944429 = -779816000;    double LdOMNMFIvcrrYjlso10078279 = -386453505;    double LdOMNMFIvcrrYjlso62160220 = -99216581;    double LdOMNMFIvcrrYjlso6988503 = -457712573;    double LdOMNMFIvcrrYjlso26199351 = -438629688;    double LdOMNMFIvcrrYjlso66890866 = -546711141;    double LdOMNMFIvcrrYjlso61715231 = -370576906;    double LdOMNMFIvcrrYjlso82289700 = 11172515;    double LdOMNMFIvcrrYjlso11221700 = -588699674;    double LdOMNMFIvcrrYjlso71162293 = -510275186;    double LdOMNMFIvcrrYjlso95251183 = -817985311;    double LdOMNMFIvcrrYjlso1513207 = -867937062;    double LdOMNMFIvcrrYjlso60241987 = -478048841;    double LdOMNMFIvcrrYjlso29905336 = -639188456;    double LdOMNMFIvcrrYjlso61488019 = -595827353;    double LdOMNMFIvcrrYjlso85953995 = -724615348;    double LdOMNMFIvcrrYjlso81857145 = -274797506;    double LdOMNMFIvcrrYjlso75309247 = -740959488;    double LdOMNMFIvcrrYjlso54704799 = -861696822;    double LdOMNMFIvcrrYjlso77204023 = -616990811;    double LdOMNMFIvcrrYjlso5568654 = -708562107;    double LdOMNMFIvcrrYjlso77180565 = -54723541;    double LdOMNMFIvcrrYjlso6637581 = -702991180;    double LdOMNMFIvcrrYjlso90048754 = -930988592;    double LdOMNMFIvcrrYjlso81979789 = -320806535;    double LdOMNMFIvcrrYjlso29440650 = -4310540;    double LdOMNMFIvcrrYjlso81476458 = -602079847;    double LdOMNMFIvcrrYjlso2491340 = -448273465;    double LdOMNMFIvcrrYjlso5977052 = -602325714;    double LdOMNMFIvcrrYjlso43184129 = -965776145;    double LdOMNMFIvcrrYjlso53958655 = -982434747;    double LdOMNMFIvcrrYjlso31701452 = -193797310;    double LdOMNMFIvcrrYjlso31757809 = 27796121;    double LdOMNMFIvcrrYjlso46594023 = -748939326;    double LdOMNMFIvcrrYjlso90697717 = -205899699;    double LdOMNMFIvcrrYjlso1591929 = -953373441;    double LdOMNMFIvcrrYjlso83774118 = -136609592;    double LdOMNMFIvcrrYjlso35345650 = -516027115;    double LdOMNMFIvcrrYjlso60955495 = -980971696;    double LdOMNMFIvcrrYjlso51983499 = -843178759;    double LdOMNMFIvcrrYjlso66658327 = -723192262;    double LdOMNMFIvcrrYjlso90142230 = -503563363;    double LdOMNMFIvcrrYjlso35963491 = -489267408;    double LdOMNMFIvcrrYjlso15202604 = -536606034;    double LdOMNMFIvcrrYjlso65387394 = -819742298;    double LdOMNMFIvcrrYjlso89772125 = -52399407;    double LdOMNMFIvcrrYjlso69424918 = -293661979;    double LdOMNMFIvcrrYjlso21097020 = -147105247;    double LdOMNMFIvcrrYjlso9194948 = -698007905;    double LdOMNMFIvcrrYjlso53213541 = -543124642;    double LdOMNMFIvcrrYjlso87719573 = -543423698;    double LdOMNMFIvcrrYjlso71574034 = -6915108;    double LdOMNMFIvcrrYjlso94646073 = -391118057;    double LdOMNMFIvcrrYjlso47936634 = -305020569;    double LdOMNMFIvcrrYjlso59946861 = -171223605;    double LdOMNMFIvcrrYjlso57763864 = -625092460;    double LdOMNMFIvcrrYjlso3440699 = -683462326;    double LdOMNMFIvcrrYjlso72111466 = -168227989;    double LdOMNMFIvcrrYjlso25008714 = -36906039;    double LdOMNMFIvcrrYjlso96758700 = -334319149;    double LdOMNMFIvcrrYjlso85414407 = -944631294;    double LdOMNMFIvcrrYjlso59223891 = -922303441;    double LdOMNMFIvcrrYjlso76312648 = -386501771;    double LdOMNMFIvcrrYjlso68037571 = -622923529;    double LdOMNMFIvcrrYjlso17203639 = -527840440;    double LdOMNMFIvcrrYjlso63549731 = -524188002;    double LdOMNMFIvcrrYjlso69755398 = -795733184;    double LdOMNMFIvcrrYjlso13647964 = -729109515;    double LdOMNMFIvcrrYjlso39207618 = -333288757;    double LdOMNMFIvcrrYjlso59896091 = -642453912;    double LdOMNMFIvcrrYjlso2179877 = -488005757;    double LdOMNMFIvcrrYjlso46511496 = -758770391;    double LdOMNMFIvcrrYjlso14353753 = -759987792;    double LdOMNMFIvcrrYjlso2721300 = 81481937;    double LdOMNMFIvcrrYjlso10545696 = -893798550;    double LdOMNMFIvcrrYjlso15426424 = -104998744;    double LdOMNMFIvcrrYjlso41217074 = -565456134;    double LdOMNMFIvcrrYjlso91434976 = -66385146;    double LdOMNMFIvcrrYjlso24661361 = -11246295;    double LdOMNMFIvcrrYjlso92207663 = -168407129;    double LdOMNMFIvcrrYjlso60015732 = -710648561;    double LdOMNMFIvcrrYjlso60379439 = -354974600;    double LdOMNMFIvcrrYjlso93296392 = -750265560;    double LdOMNMFIvcrrYjlso52763511 = 40798927;    double LdOMNMFIvcrrYjlso55464555 = -322352448;    double LdOMNMFIvcrrYjlso82384621 = -875519639;    double LdOMNMFIvcrrYjlso37055378 = -802679253;    double LdOMNMFIvcrrYjlso83821174 = -667183310;    double LdOMNMFIvcrrYjlso86647161 = -637727609;     LdOMNMFIvcrrYjlso60638577 = LdOMNMFIvcrrYjlso91285332;     LdOMNMFIvcrrYjlso91285332 = LdOMNMFIvcrrYjlso29666905;     LdOMNMFIvcrrYjlso29666905 = LdOMNMFIvcrrYjlso51002356;     LdOMNMFIvcrrYjlso51002356 = LdOMNMFIvcrrYjlso70682967;     LdOMNMFIvcrrYjlso70682967 = LdOMNMFIvcrrYjlso39167537;     LdOMNMFIvcrrYjlso39167537 = LdOMNMFIvcrrYjlso69576719;     LdOMNMFIvcrrYjlso69576719 = LdOMNMFIvcrrYjlso46883282;     LdOMNMFIvcrrYjlso46883282 = LdOMNMFIvcrrYjlso49350873;     LdOMNMFIvcrrYjlso49350873 = LdOMNMFIvcrrYjlso25140658;     LdOMNMFIvcrrYjlso25140658 = LdOMNMFIvcrrYjlso65515515;     LdOMNMFIvcrrYjlso65515515 = LdOMNMFIvcrrYjlso34944429;     LdOMNMFIvcrrYjlso34944429 = LdOMNMFIvcrrYjlso10078279;     LdOMNMFIvcrrYjlso10078279 = LdOMNMFIvcrrYjlso62160220;     LdOMNMFIvcrrYjlso62160220 = LdOMNMFIvcrrYjlso6988503;     LdOMNMFIvcrrYjlso6988503 = LdOMNMFIvcrrYjlso26199351;     LdOMNMFIvcrrYjlso26199351 = LdOMNMFIvcrrYjlso66890866;     LdOMNMFIvcrrYjlso66890866 = LdOMNMFIvcrrYjlso61715231;     LdOMNMFIvcrrYjlso61715231 = LdOMNMFIvcrrYjlso82289700;     LdOMNMFIvcrrYjlso82289700 = LdOMNMFIvcrrYjlso11221700;     LdOMNMFIvcrrYjlso11221700 = LdOMNMFIvcrrYjlso71162293;     LdOMNMFIvcrrYjlso71162293 = LdOMNMFIvcrrYjlso95251183;     LdOMNMFIvcrrYjlso95251183 = LdOMNMFIvcrrYjlso1513207;     LdOMNMFIvcrrYjlso1513207 = LdOMNMFIvcrrYjlso60241987;     LdOMNMFIvcrrYjlso60241987 = LdOMNMFIvcrrYjlso29905336;     LdOMNMFIvcrrYjlso29905336 = LdOMNMFIvcrrYjlso61488019;     LdOMNMFIvcrrYjlso61488019 = LdOMNMFIvcrrYjlso85953995;     LdOMNMFIvcrrYjlso85953995 = LdOMNMFIvcrrYjlso81857145;     LdOMNMFIvcrrYjlso81857145 = LdOMNMFIvcrrYjlso75309247;     LdOMNMFIvcrrYjlso75309247 = LdOMNMFIvcrrYjlso54704799;     LdOMNMFIvcrrYjlso54704799 = LdOMNMFIvcrrYjlso77204023;     LdOMNMFIvcrrYjlso77204023 = LdOMNMFIvcrrYjlso5568654;     LdOMNMFIvcrrYjlso5568654 = LdOMNMFIvcrrYjlso77180565;     LdOMNMFIvcrrYjlso77180565 = LdOMNMFIvcrrYjlso6637581;     LdOMNMFIvcrrYjlso6637581 = LdOMNMFIvcrrYjlso90048754;     LdOMNMFIvcrrYjlso90048754 = LdOMNMFIvcrrYjlso81979789;     LdOMNMFIvcrrYjlso81979789 = LdOMNMFIvcrrYjlso29440650;     LdOMNMFIvcrrYjlso29440650 = LdOMNMFIvcrrYjlso81476458;     LdOMNMFIvcrrYjlso81476458 = LdOMNMFIvcrrYjlso2491340;     LdOMNMFIvcrrYjlso2491340 = LdOMNMFIvcrrYjlso5977052;     LdOMNMFIvcrrYjlso5977052 = LdOMNMFIvcrrYjlso43184129;     LdOMNMFIvcrrYjlso43184129 = LdOMNMFIvcrrYjlso53958655;     LdOMNMFIvcrrYjlso53958655 = LdOMNMFIvcrrYjlso31701452;     LdOMNMFIvcrrYjlso31701452 = LdOMNMFIvcrrYjlso31757809;     LdOMNMFIvcrrYjlso31757809 = LdOMNMFIvcrrYjlso46594023;     LdOMNMFIvcrrYjlso46594023 = LdOMNMFIvcrrYjlso90697717;     LdOMNMFIvcrrYjlso90697717 = LdOMNMFIvcrrYjlso1591929;     LdOMNMFIvcrrYjlso1591929 = LdOMNMFIvcrrYjlso83774118;     LdOMNMFIvcrrYjlso83774118 = LdOMNMFIvcrrYjlso35345650;     LdOMNMFIvcrrYjlso35345650 = LdOMNMFIvcrrYjlso60955495;     LdOMNMFIvcrrYjlso60955495 = LdOMNMFIvcrrYjlso51983499;     LdOMNMFIvcrrYjlso51983499 = LdOMNMFIvcrrYjlso66658327;     LdOMNMFIvcrrYjlso66658327 = LdOMNMFIvcrrYjlso90142230;     LdOMNMFIvcrrYjlso90142230 = LdOMNMFIvcrrYjlso35963491;     LdOMNMFIvcrrYjlso35963491 = LdOMNMFIvcrrYjlso15202604;     LdOMNMFIvcrrYjlso15202604 = LdOMNMFIvcrrYjlso65387394;     LdOMNMFIvcrrYjlso65387394 = LdOMNMFIvcrrYjlso89772125;     LdOMNMFIvcrrYjlso89772125 = LdOMNMFIvcrrYjlso69424918;     LdOMNMFIvcrrYjlso69424918 = LdOMNMFIvcrrYjlso21097020;     LdOMNMFIvcrrYjlso21097020 = LdOMNMFIvcrrYjlso9194948;     LdOMNMFIvcrrYjlso9194948 = LdOMNMFIvcrrYjlso53213541;     LdOMNMFIvcrrYjlso53213541 = LdOMNMFIvcrrYjlso87719573;     LdOMNMFIvcrrYjlso87719573 = LdOMNMFIvcrrYjlso71574034;     LdOMNMFIvcrrYjlso71574034 = LdOMNMFIvcrrYjlso94646073;     LdOMNMFIvcrrYjlso94646073 = LdOMNMFIvcrrYjlso47936634;     LdOMNMFIvcrrYjlso47936634 = LdOMNMFIvcrrYjlso59946861;     LdOMNMFIvcrrYjlso59946861 = LdOMNMFIvcrrYjlso57763864;     LdOMNMFIvcrrYjlso57763864 = LdOMNMFIvcrrYjlso3440699;     LdOMNMFIvcrrYjlso3440699 = LdOMNMFIvcrrYjlso72111466;     LdOMNMFIvcrrYjlso72111466 = LdOMNMFIvcrrYjlso25008714;     LdOMNMFIvcrrYjlso25008714 = LdOMNMFIvcrrYjlso96758700;     LdOMNMFIvcrrYjlso96758700 = LdOMNMFIvcrrYjlso85414407;     LdOMNMFIvcrrYjlso85414407 = LdOMNMFIvcrrYjlso59223891;     LdOMNMFIvcrrYjlso59223891 = LdOMNMFIvcrrYjlso76312648;     LdOMNMFIvcrrYjlso76312648 = LdOMNMFIvcrrYjlso68037571;     LdOMNMFIvcrrYjlso68037571 = LdOMNMFIvcrrYjlso17203639;     LdOMNMFIvcrrYjlso17203639 = LdOMNMFIvcrrYjlso63549731;     LdOMNMFIvcrrYjlso63549731 = LdOMNMFIvcrrYjlso69755398;     LdOMNMFIvcrrYjlso69755398 = LdOMNMFIvcrrYjlso13647964;     LdOMNMFIvcrrYjlso13647964 = LdOMNMFIvcrrYjlso39207618;     LdOMNMFIvcrrYjlso39207618 = LdOMNMFIvcrrYjlso59896091;     LdOMNMFIvcrrYjlso59896091 = LdOMNMFIvcrrYjlso2179877;     LdOMNMFIvcrrYjlso2179877 = LdOMNMFIvcrrYjlso46511496;     LdOMNMFIvcrrYjlso46511496 = LdOMNMFIvcrrYjlso14353753;     LdOMNMFIvcrrYjlso14353753 = LdOMNMFIvcrrYjlso2721300;     LdOMNMFIvcrrYjlso2721300 = LdOMNMFIvcrrYjlso10545696;     LdOMNMFIvcrrYjlso10545696 = LdOMNMFIvcrrYjlso15426424;     LdOMNMFIvcrrYjlso15426424 = LdOMNMFIvcrrYjlso41217074;     LdOMNMFIvcrrYjlso41217074 = LdOMNMFIvcrrYjlso91434976;     LdOMNMFIvcrrYjlso91434976 = LdOMNMFIvcrrYjlso24661361;     LdOMNMFIvcrrYjlso24661361 = LdOMNMFIvcrrYjlso92207663;     LdOMNMFIvcrrYjlso92207663 = LdOMNMFIvcrrYjlso60015732;     LdOMNMFIvcrrYjlso60015732 = LdOMNMFIvcrrYjlso60379439;     LdOMNMFIvcrrYjlso60379439 = LdOMNMFIvcrrYjlso93296392;     LdOMNMFIvcrrYjlso93296392 = LdOMNMFIvcrrYjlso52763511;     LdOMNMFIvcrrYjlso52763511 = LdOMNMFIvcrrYjlso55464555;     LdOMNMFIvcrrYjlso55464555 = LdOMNMFIvcrrYjlso82384621;     LdOMNMFIvcrrYjlso82384621 = LdOMNMFIvcrrYjlso37055378;     LdOMNMFIvcrrYjlso37055378 = LdOMNMFIvcrrYjlso83821174;     LdOMNMFIvcrrYjlso83821174 = LdOMNMFIvcrrYjlso86647161;     LdOMNMFIvcrrYjlso86647161 = LdOMNMFIvcrrYjlso60638577;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HEkFrURstdDcDiiE81895491() {     double AiBQmQNhqTAjiWtuh57157969 = -764004155;    double AiBQmQNhqTAjiWtuh67795841 = -930429646;    double AiBQmQNhqTAjiWtuh84036003 = -662684971;    double AiBQmQNhqTAjiWtuh78471343 = -434566662;    double AiBQmQNhqTAjiWtuh30495255 = -481066932;    double AiBQmQNhqTAjiWtuh41608976 = -598834754;    double AiBQmQNhqTAjiWtuh74844036 = -278602712;    double AiBQmQNhqTAjiWtuh13121402 = -156581463;    double AiBQmQNhqTAjiWtuh70864965 = -791586563;    double AiBQmQNhqTAjiWtuh82505680 = 56070279;    double AiBQmQNhqTAjiWtuh5411026 = -995548876;    double AiBQmQNhqTAjiWtuh35669189 = -513070543;    double AiBQmQNhqTAjiWtuh83496062 = -650580694;    double AiBQmQNhqTAjiWtuh75391113 = -263585065;    double AiBQmQNhqTAjiWtuh66716151 = -346326431;    double AiBQmQNhqTAjiWtuh64591627 = -964455252;    double AiBQmQNhqTAjiWtuh38728955 = -639943615;    double AiBQmQNhqTAjiWtuh74850186 = -591031349;    double AiBQmQNhqTAjiWtuh43784407 = -959972518;    double AiBQmQNhqTAjiWtuh79873195 = -986200053;    double AiBQmQNhqTAjiWtuh99171724 = 89641589;    double AiBQmQNhqTAjiWtuh57856675 = -686011121;    double AiBQmQNhqTAjiWtuh54131773 = -689898931;    double AiBQmQNhqTAjiWtuh40416649 = -556245722;    double AiBQmQNhqTAjiWtuh79737653 = -837920362;    double AiBQmQNhqTAjiWtuh20938307 = -866916687;    double AiBQmQNhqTAjiWtuh63629098 = -250997482;    double AiBQmQNhqTAjiWtuh38703983 = -421586616;    double AiBQmQNhqTAjiWtuh44826772 = 39732844;    double AiBQmQNhqTAjiWtuh78869436 = -551042291;    double AiBQmQNhqTAjiWtuh52885424 = -531865730;    double AiBQmQNhqTAjiWtuh56594980 = -648746366;    double AiBQmQNhqTAjiWtuh9172581 = -842876093;    double AiBQmQNhqTAjiWtuh19014412 = -646819447;    double AiBQmQNhqTAjiWtuh17175555 = -886340193;    double AiBQmQNhqTAjiWtuh82822853 = -518117497;    double AiBQmQNhqTAjiWtuh98056249 = -515081119;    double AiBQmQNhqTAjiWtuh84685753 = -427949137;    double AiBQmQNhqTAjiWtuh15912050 = -76660799;    double AiBQmQNhqTAjiWtuh13621559 = -429241910;    double AiBQmQNhqTAjiWtuh87599455 = -309374418;    double AiBQmQNhqTAjiWtuh12454347 = -196083627;    double AiBQmQNhqTAjiWtuh58960027 = -130939628;    double AiBQmQNhqTAjiWtuh84734018 = -556873189;    double AiBQmQNhqTAjiWtuh28562686 = -807834262;    double AiBQmQNhqTAjiWtuh47950776 = -726182824;    double AiBQmQNhqTAjiWtuh70806228 = -750613467;    double AiBQmQNhqTAjiWtuh88880561 = -790942235;    double AiBQmQNhqTAjiWtuh35296328 = -483767449;    double AiBQmQNhqTAjiWtuh13711111 = -643486265;    double AiBQmQNhqTAjiWtuh90512769 = -794002958;    double AiBQmQNhqTAjiWtuh24519709 = -923211764;    double AiBQmQNhqTAjiWtuh6837261 = -303417999;    double AiBQmQNhqTAjiWtuh11220416 = -407548560;    double AiBQmQNhqTAjiWtuh82723767 = -112428549;    double AiBQmQNhqTAjiWtuh99301294 = 22006965;    double AiBQmQNhqTAjiWtuh13664069 = -140530715;    double AiBQmQNhqTAjiWtuh43619355 = -6439249;    double AiBQmQNhqTAjiWtuh98733689 = -596646301;    double AiBQmQNhqTAjiWtuh9556948 = -614150245;    double AiBQmQNhqTAjiWtuh77979878 = -247837272;    double AiBQmQNhqTAjiWtuh36140054 = -857016096;    double AiBQmQNhqTAjiWtuh68294629 = -96314307;    double AiBQmQNhqTAjiWtuh91995529 = -140544273;    double AiBQmQNhqTAjiWtuh29620256 = -412063992;    double AiBQmQNhqTAjiWtuh48816046 = -246802511;    double AiBQmQNhqTAjiWtuh26496608 = -670194451;    double AiBQmQNhqTAjiWtuh64481651 = 96238753;    double AiBQmQNhqTAjiWtuh58215558 = -377244872;    double AiBQmQNhqTAjiWtuh83893297 = -828208935;    double AiBQmQNhqTAjiWtuh66535377 = -349374134;    double AiBQmQNhqTAjiWtuh54043202 = -111994479;    double AiBQmQNhqTAjiWtuh58938137 = -414370550;    double AiBQmQNhqTAjiWtuh30162848 = -430730608;    double AiBQmQNhqTAjiWtuh92273739 = -576825636;    double AiBQmQNhqTAjiWtuh86717378 = -714274784;    double AiBQmQNhqTAjiWtuh98896647 = -455071493;    double AiBQmQNhqTAjiWtuh69397754 = -33025742;    double AiBQmQNhqTAjiWtuh11853963 = -748411460;    double AiBQmQNhqTAjiWtuh31786878 = -11737538;    double AiBQmQNhqTAjiWtuh50132078 = -16303221;    double AiBQmQNhqTAjiWtuh74748537 = -460055248;    double AiBQmQNhqTAjiWtuh3407655 = -937819168;    double AiBQmQNhqTAjiWtuh31115661 = -316780892;    double AiBQmQNhqTAjiWtuh88356667 = -757039333;    double AiBQmQNhqTAjiWtuh28365716 = -608653967;    double AiBQmQNhqTAjiWtuh49757719 = -245328367;    double AiBQmQNhqTAjiWtuh97952164 = -335327533;    double AiBQmQNhqTAjiWtuh36290645 = -434390899;    double AiBQmQNhqTAjiWtuh17874261 = -808347159;    double AiBQmQNhqTAjiWtuh69158785 = -277586782;    double AiBQmQNhqTAjiWtuh54436894 = -408641870;    double AiBQmQNhqTAjiWtuh85952063 = -831302836;    double AiBQmQNhqTAjiWtuh6355102 = -462510555;    double AiBQmQNhqTAjiWtuh35641680 = -81404639;    double AiBQmQNhqTAjiWtuh51459402 = -452358322;    double AiBQmQNhqTAjiWtuh44159717 = 230680;    double AiBQmQNhqTAjiWtuh66964498 = -990395356;    double AiBQmQNhqTAjiWtuh55113762 = -44809197;    double AiBQmQNhqTAjiWtuh79746640 = -764004155;     AiBQmQNhqTAjiWtuh57157969 = AiBQmQNhqTAjiWtuh67795841;     AiBQmQNhqTAjiWtuh67795841 = AiBQmQNhqTAjiWtuh84036003;     AiBQmQNhqTAjiWtuh84036003 = AiBQmQNhqTAjiWtuh78471343;     AiBQmQNhqTAjiWtuh78471343 = AiBQmQNhqTAjiWtuh30495255;     AiBQmQNhqTAjiWtuh30495255 = AiBQmQNhqTAjiWtuh41608976;     AiBQmQNhqTAjiWtuh41608976 = AiBQmQNhqTAjiWtuh74844036;     AiBQmQNhqTAjiWtuh74844036 = AiBQmQNhqTAjiWtuh13121402;     AiBQmQNhqTAjiWtuh13121402 = AiBQmQNhqTAjiWtuh70864965;     AiBQmQNhqTAjiWtuh70864965 = AiBQmQNhqTAjiWtuh82505680;     AiBQmQNhqTAjiWtuh82505680 = AiBQmQNhqTAjiWtuh5411026;     AiBQmQNhqTAjiWtuh5411026 = AiBQmQNhqTAjiWtuh35669189;     AiBQmQNhqTAjiWtuh35669189 = AiBQmQNhqTAjiWtuh83496062;     AiBQmQNhqTAjiWtuh83496062 = AiBQmQNhqTAjiWtuh75391113;     AiBQmQNhqTAjiWtuh75391113 = AiBQmQNhqTAjiWtuh66716151;     AiBQmQNhqTAjiWtuh66716151 = AiBQmQNhqTAjiWtuh64591627;     AiBQmQNhqTAjiWtuh64591627 = AiBQmQNhqTAjiWtuh38728955;     AiBQmQNhqTAjiWtuh38728955 = AiBQmQNhqTAjiWtuh74850186;     AiBQmQNhqTAjiWtuh74850186 = AiBQmQNhqTAjiWtuh43784407;     AiBQmQNhqTAjiWtuh43784407 = AiBQmQNhqTAjiWtuh79873195;     AiBQmQNhqTAjiWtuh79873195 = AiBQmQNhqTAjiWtuh99171724;     AiBQmQNhqTAjiWtuh99171724 = AiBQmQNhqTAjiWtuh57856675;     AiBQmQNhqTAjiWtuh57856675 = AiBQmQNhqTAjiWtuh54131773;     AiBQmQNhqTAjiWtuh54131773 = AiBQmQNhqTAjiWtuh40416649;     AiBQmQNhqTAjiWtuh40416649 = AiBQmQNhqTAjiWtuh79737653;     AiBQmQNhqTAjiWtuh79737653 = AiBQmQNhqTAjiWtuh20938307;     AiBQmQNhqTAjiWtuh20938307 = AiBQmQNhqTAjiWtuh63629098;     AiBQmQNhqTAjiWtuh63629098 = AiBQmQNhqTAjiWtuh38703983;     AiBQmQNhqTAjiWtuh38703983 = AiBQmQNhqTAjiWtuh44826772;     AiBQmQNhqTAjiWtuh44826772 = AiBQmQNhqTAjiWtuh78869436;     AiBQmQNhqTAjiWtuh78869436 = AiBQmQNhqTAjiWtuh52885424;     AiBQmQNhqTAjiWtuh52885424 = AiBQmQNhqTAjiWtuh56594980;     AiBQmQNhqTAjiWtuh56594980 = AiBQmQNhqTAjiWtuh9172581;     AiBQmQNhqTAjiWtuh9172581 = AiBQmQNhqTAjiWtuh19014412;     AiBQmQNhqTAjiWtuh19014412 = AiBQmQNhqTAjiWtuh17175555;     AiBQmQNhqTAjiWtuh17175555 = AiBQmQNhqTAjiWtuh82822853;     AiBQmQNhqTAjiWtuh82822853 = AiBQmQNhqTAjiWtuh98056249;     AiBQmQNhqTAjiWtuh98056249 = AiBQmQNhqTAjiWtuh84685753;     AiBQmQNhqTAjiWtuh84685753 = AiBQmQNhqTAjiWtuh15912050;     AiBQmQNhqTAjiWtuh15912050 = AiBQmQNhqTAjiWtuh13621559;     AiBQmQNhqTAjiWtuh13621559 = AiBQmQNhqTAjiWtuh87599455;     AiBQmQNhqTAjiWtuh87599455 = AiBQmQNhqTAjiWtuh12454347;     AiBQmQNhqTAjiWtuh12454347 = AiBQmQNhqTAjiWtuh58960027;     AiBQmQNhqTAjiWtuh58960027 = AiBQmQNhqTAjiWtuh84734018;     AiBQmQNhqTAjiWtuh84734018 = AiBQmQNhqTAjiWtuh28562686;     AiBQmQNhqTAjiWtuh28562686 = AiBQmQNhqTAjiWtuh47950776;     AiBQmQNhqTAjiWtuh47950776 = AiBQmQNhqTAjiWtuh70806228;     AiBQmQNhqTAjiWtuh70806228 = AiBQmQNhqTAjiWtuh88880561;     AiBQmQNhqTAjiWtuh88880561 = AiBQmQNhqTAjiWtuh35296328;     AiBQmQNhqTAjiWtuh35296328 = AiBQmQNhqTAjiWtuh13711111;     AiBQmQNhqTAjiWtuh13711111 = AiBQmQNhqTAjiWtuh90512769;     AiBQmQNhqTAjiWtuh90512769 = AiBQmQNhqTAjiWtuh24519709;     AiBQmQNhqTAjiWtuh24519709 = AiBQmQNhqTAjiWtuh6837261;     AiBQmQNhqTAjiWtuh6837261 = AiBQmQNhqTAjiWtuh11220416;     AiBQmQNhqTAjiWtuh11220416 = AiBQmQNhqTAjiWtuh82723767;     AiBQmQNhqTAjiWtuh82723767 = AiBQmQNhqTAjiWtuh99301294;     AiBQmQNhqTAjiWtuh99301294 = AiBQmQNhqTAjiWtuh13664069;     AiBQmQNhqTAjiWtuh13664069 = AiBQmQNhqTAjiWtuh43619355;     AiBQmQNhqTAjiWtuh43619355 = AiBQmQNhqTAjiWtuh98733689;     AiBQmQNhqTAjiWtuh98733689 = AiBQmQNhqTAjiWtuh9556948;     AiBQmQNhqTAjiWtuh9556948 = AiBQmQNhqTAjiWtuh77979878;     AiBQmQNhqTAjiWtuh77979878 = AiBQmQNhqTAjiWtuh36140054;     AiBQmQNhqTAjiWtuh36140054 = AiBQmQNhqTAjiWtuh68294629;     AiBQmQNhqTAjiWtuh68294629 = AiBQmQNhqTAjiWtuh91995529;     AiBQmQNhqTAjiWtuh91995529 = AiBQmQNhqTAjiWtuh29620256;     AiBQmQNhqTAjiWtuh29620256 = AiBQmQNhqTAjiWtuh48816046;     AiBQmQNhqTAjiWtuh48816046 = AiBQmQNhqTAjiWtuh26496608;     AiBQmQNhqTAjiWtuh26496608 = AiBQmQNhqTAjiWtuh64481651;     AiBQmQNhqTAjiWtuh64481651 = AiBQmQNhqTAjiWtuh58215558;     AiBQmQNhqTAjiWtuh58215558 = AiBQmQNhqTAjiWtuh83893297;     AiBQmQNhqTAjiWtuh83893297 = AiBQmQNhqTAjiWtuh66535377;     AiBQmQNhqTAjiWtuh66535377 = AiBQmQNhqTAjiWtuh54043202;     AiBQmQNhqTAjiWtuh54043202 = AiBQmQNhqTAjiWtuh58938137;     AiBQmQNhqTAjiWtuh58938137 = AiBQmQNhqTAjiWtuh30162848;     AiBQmQNhqTAjiWtuh30162848 = AiBQmQNhqTAjiWtuh92273739;     AiBQmQNhqTAjiWtuh92273739 = AiBQmQNhqTAjiWtuh86717378;     AiBQmQNhqTAjiWtuh86717378 = AiBQmQNhqTAjiWtuh98896647;     AiBQmQNhqTAjiWtuh98896647 = AiBQmQNhqTAjiWtuh69397754;     AiBQmQNhqTAjiWtuh69397754 = AiBQmQNhqTAjiWtuh11853963;     AiBQmQNhqTAjiWtuh11853963 = AiBQmQNhqTAjiWtuh31786878;     AiBQmQNhqTAjiWtuh31786878 = AiBQmQNhqTAjiWtuh50132078;     AiBQmQNhqTAjiWtuh50132078 = AiBQmQNhqTAjiWtuh74748537;     AiBQmQNhqTAjiWtuh74748537 = AiBQmQNhqTAjiWtuh3407655;     AiBQmQNhqTAjiWtuh3407655 = AiBQmQNhqTAjiWtuh31115661;     AiBQmQNhqTAjiWtuh31115661 = AiBQmQNhqTAjiWtuh88356667;     AiBQmQNhqTAjiWtuh88356667 = AiBQmQNhqTAjiWtuh28365716;     AiBQmQNhqTAjiWtuh28365716 = AiBQmQNhqTAjiWtuh49757719;     AiBQmQNhqTAjiWtuh49757719 = AiBQmQNhqTAjiWtuh97952164;     AiBQmQNhqTAjiWtuh97952164 = AiBQmQNhqTAjiWtuh36290645;     AiBQmQNhqTAjiWtuh36290645 = AiBQmQNhqTAjiWtuh17874261;     AiBQmQNhqTAjiWtuh17874261 = AiBQmQNhqTAjiWtuh69158785;     AiBQmQNhqTAjiWtuh69158785 = AiBQmQNhqTAjiWtuh54436894;     AiBQmQNhqTAjiWtuh54436894 = AiBQmQNhqTAjiWtuh85952063;     AiBQmQNhqTAjiWtuh85952063 = AiBQmQNhqTAjiWtuh6355102;     AiBQmQNhqTAjiWtuh6355102 = AiBQmQNhqTAjiWtuh35641680;     AiBQmQNhqTAjiWtuh35641680 = AiBQmQNhqTAjiWtuh51459402;     AiBQmQNhqTAjiWtuh51459402 = AiBQmQNhqTAjiWtuh44159717;     AiBQmQNhqTAjiWtuh44159717 = AiBQmQNhqTAjiWtuh66964498;     AiBQmQNhqTAjiWtuh66964498 = AiBQmQNhqTAjiWtuh55113762;     AiBQmQNhqTAjiWtuh55113762 = AiBQmQNhqTAjiWtuh79746640;     AiBQmQNhqTAjiWtuh79746640 = AiBQmQNhqTAjiWtuh57157969;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TFwMwQbUNKXhzirB49187091() {     double tFVzMnteLMDUPGsnl92616938 = -130281055;    double tFVzMnteLMDUPGsnl81067919 = -810981252;    double tFVzMnteLMDUPGsnl21422480 = -328352046;    double tFVzMnteLMDUPGsnl50694698 = -255717734;    double tFVzMnteLMDUPGsnl86780523 = -586665777;    double tFVzMnteLMDUPGsnl77831848 = -861182649;    double tFVzMnteLMDUPGsnl85501442 = -123328446;    double tFVzMnteLMDUPGsnl75252962 = -900815603;    double tFVzMnteLMDUPGsnl72382571 = -179917970;    double tFVzMnteLMDUPGsnl97408392 = -708616719;    double tFVzMnteLMDUPGsnl43592760 = -797556605;    double tFVzMnteLMDUPGsnl36237337 = -377191315;    double tFVzMnteLMDUPGsnl21730708 = -102593199;    double tFVzMnteLMDUPGsnl28531977 = -239948474;    double tFVzMnteLMDUPGsnl24910667 = -849751446;    double tFVzMnteLMDUPGsnl48132928 = -600266859;    double tFVzMnteLMDUPGsnl94564891 = -66918168;    double tFVzMnteLMDUPGsnl74610378 = -664459417;    double tFVzMnteLMDUPGsnl44334736 = -176535940;    double tFVzMnteLMDUPGsnl73710414 = -312761119;    double tFVzMnteLMDUPGsnl12258588 = -667633100;    double tFVzMnteLMDUPGsnl61278199 = -772647415;    double tFVzMnteLMDUPGsnl24657153 = -327531115;    double tFVzMnteLMDUPGsnl34209594 = 32929674;    double tFVzMnteLMDUPGsnl87755604 = -861905560;    double tFVzMnteLMDUPGsnl31636479 = -555493494;    double tFVzMnteLMDUPGsnl24049673 = -57281623;    double tFVzMnteLMDUPGsnl79432408 = -591358791;    double tFVzMnteLMDUPGsnl91494986 = -534361958;    double tFVzMnteLMDUPGsnl84402847 = -585549461;    double tFVzMnteLMDUPGsnl71468656 = 16153783;    double tFVzMnteLMDUPGsnl94417998 = -421717780;    double tFVzMnteLMDUPGsnl21390592 = -990472070;    double tFVzMnteLMDUPGsnl48762838 = -711513261;    double tFVzMnteLMDUPGsnl82984008 = -837440444;    double tFVzMnteLMDUPGsnl34383697 = -732682803;    double tFVzMnteLMDUPGsnl47468066 = -379777159;    double tFVzMnteLMDUPGsnl35901157 = -410604718;    double tFVzMnteLMDUPGsnl46302611 = -827537076;    double tFVzMnteLMDUPGsnl77893090 = -994504456;    double tFVzMnteLMDUPGsnl21051064 = -78573332;    double tFVzMnteLMDUPGsnl1819474 = -890628918;    double tFVzMnteLMDUPGsnl48343406 = -784120414;    double tFVzMnteLMDUPGsnl24865357 = -61205599;    double tFVzMnteLMDUPGsnl28579144 = -634345424;    double tFVzMnteLMDUPGsnl93912104 = -765106456;    double tFVzMnteLMDUPGsnl90151392 = -134599827;    double tFVzMnteLMDUPGsnl6606873 = -957468828;    double tFVzMnteLMDUPGsnl58188994 = -147504329;    double tFVzMnteLMDUPGsnl38240701 = -34513081;    double tFVzMnteLMDUPGsnl71283493 = -668279516;    double tFVzMnteLMDUPGsnl8005181 = -36864110;    double tFVzMnteLMDUPGsnl75458409 = -59463553;    double tFVzMnteLMDUPGsnl27212263 = -599057060;    double tFVzMnteLMDUPGsnl36928504 = -458918695;    double tFVzMnteLMDUPGsnl31338739 = -357633641;    double tFVzMnteLMDUPGsnl56410767 = -383450138;    double tFVzMnteLMDUPGsnl87212886 = -261281720;    double tFVzMnteLMDUPGsnl62939093 = -393812174;    double tFVzMnteLMDUPGsnl55144045 = 68827716;    double tFVzMnteLMDUPGsnl53782176 = -703901027;    double tFVzMnteLMDUPGsnl6069035 = -531969656;    double tFVzMnteLMDUPGsnl83757975 = -266453645;    double tFVzMnteLMDUPGsnl87979723 = -594368509;    double tFVzMnteLMDUPGsnl25939737 = -624770503;    double tFVzMnteLMDUPGsnl49174762 = -275838826;    double tFVzMnteLMDUPGsnl14846746 = -386719245;    double tFVzMnteLMDUPGsnl72967870 = -391079939;    double tFVzMnteLMDUPGsnl45547968 = -402508031;    double tFVzMnteLMDUPGsnl90526970 = -17068644;    double tFVzMnteLMDUPGsnl664862 = -120489700;    double tFVzMnteLMDUPGsnl58663734 = -656313451;    double tFVzMnteLMDUPGsnl28307767 = -836922342;    double tFVzMnteLMDUPGsnl66441646 = -182031484;    double tFVzMnteLMDUPGsnl52659351 = -134187787;    double tFVzMnteLMDUPGsnl10439115 = -777004182;    double tFVzMnteLMDUPGsnl12934794 = -988527001;    double tFVzMnteLMDUPGsnl99791795 = -166325516;    double tFVzMnteLMDUPGsnl5630450 = -332724903;    double tFVzMnteLMDUPGsnl93843500 = 3200896;    double tFVzMnteLMDUPGsnl41485086 = -320893667;    double tFVzMnteLMDUPGsnl17442800 = -99812795;    double tFVzMnteLMDUPGsnl21243414 = -343854462;    double tFVzMnteLMDUPGsnl53254286 = -399848878;    double tFVzMnteLMDUPGsnl13119354 = -917269946;    double tFVzMnteLMDUPGsnl63463475 = -946982107;    double tFVzMnteLMDUPGsnl18959589 = -262254227;    double tFVzMnteLMDUPGsnl94178328 = -291415010;    double tFVzMnteLMDUPGsnl11834335 = -152594566;    double tFVzMnteLMDUPGsnl51645270 = -379806804;    double tFVzMnteLMDUPGsnl77972930 = -249232666;    double tFVzMnteLMDUPGsnl60255179 = -18495440;    double tFVzMnteLMDUPGsnl72962063 = 83207456;    double tFVzMnteLMDUPGsnl91158566 = -796364792;    double tFVzMnteLMDUPGsnl24110915 = -190603430;    double tFVzMnteLMDUPGsnl14982029 = -546603677;    double tFVzMnteLMDUPGsnl18061498 = -524175273;    double tFVzMnteLMDUPGsnl60363682 = -89751905;    double tFVzMnteLMDUPGsnl98925620 = -436435096;    double tFVzMnteLMDUPGsnl79404382 = -130281055;     tFVzMnteLMDUPGsnl92616938 = tFVzMnteLMDUPGsnl81067919;     tFVzMnteLMDUPGsnl81067919 = tFVzMnteLMDUPGsnl21422480;     tFVzMnteLMDUPGsnl21422480 = tFVzMnteLMDUPGsnl50694698;     tFVzMnteLMDUPGsnl50694698 = tFVzMnteLMDUPGsnl86780523;     tFVzMnteLMDUPGsnl86780523 = tFVzMnteLMDUPGsnl77831848;     tFVzMnteLMDUPGsnl77831848 = tFVzMnteLMDUPGsnl85501442;     tFVzMnteLMDUPGsnl85501442 = tFVzMnteLMDUPGsnl75252962;     tFVzMnteLMDUPGsnl75252962 = tFVzMnteLMDUPGsnl72382571;     tFVzMnteLMDUPGsnl72382571 = tFVzMnteLMDUPGsnl97408392;     tFVzMnteLMDUPGsnl97408392 = tFVzMnteLMDUPGsnl43592760;     tFVzMnteLMDUPGsnl43592760 = tFVzMnteLMDUPGsnl36237337;     tFVzMnteLMDUPGsnl36237337 = tFVzMnteLMDUPGsnl21730708;     tFVzMnteLMDUPGsnl21730708 = tFVzMnteLMDUPGsnl28531977;     tFVzMnteLMDUPGsnl28531977 = tFVzMnteLMDUPGsnl24910667;     tFVzMnteLMDUPGsnl24910667 = tFVzMnteLMDUPGsnl48132928;     tFVzMnteLMDUPGsnl48132928 = tFVzMnteLMDUPGsnl94564891;     tFVzMnteLMDUPGsnl94564891 = tFVzMnteLMDUPGsnl74610378;     tFVzMnteLMDUPGsnl74610378 = tFVzMnteLMDUPGsnl44334736;     tFVzMnteLMDUPGsnl44334736 = tFVzMnteLMDUPGsnl73710414;     tFVzMnteLMDUPGsnl73710414 = tFVzMnteLMDUPGsnl12258588;     tFVzMnteLMDUPGsnl12258588 = tFVzMnteLMDUPGsnl61278199;     tFVzMnteLMDUPGsnl61278199 = tFVzMnteLMDUPGsnl24657153;     tFVzMnteLMDUPGsnl24657153 = tFVzMnteLMDUPGsnl34209594;     tFVzMnteLMDUPGsnl34209594 = tFVzMnteLMDUPGsnl87755604;     tFVzMnteLMDUPGsnl87755604 = tFVzMnteLMDUPGsnl31636479;     tFVzMnteLMDUPGsnl31636479 = tFVzMnteLMDUPGsnl24049673;     tFVzMnteLMDUPGsnl24049673 = tFVzMnteLMDUPGsnl79432408;     tFVzMnteLMDUPGsnl79432408 = tFVzMnteLMDUPGsnl91494986;     tFVzMnteLMDUPGsnl91494986 = tFVzMnteLMDUPGsnl84402847;     tFVzMnteLMDUPGsnl84402847 = tFVzMnteLMDUPGsnl71468656;     tFVzMnteLMDUPGsnl71468656 = tFVzMnteLMDUPGsnl94417998;     tFVzMnteLMDUPGsnl94417998 = tFVzMnteLMDUPGsnl21390592;     tFVzMnteLMDUPGsnl21390592 = tFVzMnteLMDUPGsnl48762838;     tFVzMnteLMDUPGsnl48762838 = tFVzMnteLMDUPGsnl82984008;     tFVzMnteLMDUPGsnl82984008 = tFVzMnteLMDUPGsnl34383697;     tFVzMnteLMDUPGsnl34383697 = tFVzMnteLMDUPGsnl47468066;     tFVzMnteLMDUPGsnl47468066 = tFVzMnteLMDUPGsnl35901157;     tFVzMnteLMDUPGsnl35901157 = tFVzMnteLMDUPGsnl46302611;     tFVzMnteLMDUPGsnl46302611 = tFVzMnteLMDUPGsnl77893090;     tFVzMnteLMDUPGsnl77893090 = tFVzMnteLMDUPGsnl21051064;     tFVzMnteLMDUPGsnl21051064 = tFVzMnteLMDUPGsnl1819474;     tFVzMnteLMDUPGsnl1819474 = tFVzMnteLMDUPGsnl48343406;     tFVzMnteLMDUPGsnl48343406 = tFVzMnteLMDUPGsnl24865357;     tFVzMnteLMDUPGsnl24865357 = tFVzMnteLMDUPGsnl28579144;     tFVzMnteLMDUPGsnl28579144 = tFVzMnteLMDUPGsnl93912104;     tFVzMnteLMDUPGsnl93912104 = tFVzMnteLMDUPGsnl90151392;     tFVzMnteLMDUPGsnl90151392 = tFVzMnteLMDUPGsnl6606873;     tFVzMnteLMDUPGsnl6606873 = tFVzMnteLMDUPGsnl58188994;     tFVzMnteLMDUPGsnl58188994 = tFVzMnteLMDUPGsnl38240701;     tFVzMnteLMDUPGsnl38240701 = tFVzMnteLMDUPGsnl71283493;     tFVzMnteLMDUPGsnl71283493 = tFVzMnteLMDUPGsnl8005181;     tFVzMnteLMDUPGsnl8005181 = tFVzMnteLMDUPGsnl75458409;     tFVzMnteLMDUPGsnl75458409 = tFVzMnteLMDUPGsnl27212263;     tFVzMnteLMDUPGsnl27212263 = tFVzMnteLMDUPGsnl36928504;     tFVzMnteLMDUPGsnl36928504 = tFVzMnteLMDUPGsnl31338739;     tFVzMnteLMDUPGsnl31338739 = tFVzMnteLMDUPGsnl56410767;     tFVzMnteLMDUPGsnl56410767 = tFVzMnteLMDUPGsnl87212886;     tFVzMnteLMDUPGsnl87212886 = tFVzMnteLMDUPGsnl62939093;     tFVzMnteLMDUPGsnl62939093 = tFVzMnteLMDUPGsnl55144045;     tFVzMnteLMDUPGsnl55144045 = tFVzMnteLMDUPGsnl53782176;     tFVzMnteLMDUPGsnl53782176 = tFVzMnteLMDUPGsnl6069035;     tFVzMnteLMDUPGsnl6069035 = tFVzMnteLMDUPGsnl83757975;     tFVzMnteLMDUPGsnl83757975 = tFVzMnteLMDUPGsnl87979723;     tFVzMnteLMDUPGsnl87979723 = tFVzMnteLMDUPGsnl25939737;     tFVzMnteLMDUPGsnl25939737 = tFVzMnteLMDUPGsnl49174762;     tFVzMnteLMDUPGsnl49174762 = tFVzMnteLMDUPGsnl14846746;     tFVzMnteLMDUPGsnl14846746 = tFVzMnteLMDUPGsnl72967870;     tFVzMnteLMDUPGsnl72967870 = tFVzMnteLMDUPGsnl45547968;     tFVzMnteLMDUPGsnl45547968 = tFVzMnteLMDUPGsnl90526970;     tFVzMnteLMDUPGsnl90526970 = tFVzMnteLMDUPGsnl664862;     tFVzMnteLMDUPGsnl664862 = tFVzMnteLMDUPGsnl58663734;     tFVzMnteLMDUPGsnl58663734 = tFVzMnteLMDUPGsnl28307767;     tFVzMnteLMDUPGsnl28307767 = tFVzMnteLMDUPGsnl66441646;     tFVzMnteLMDUPGsnl66441646 = tFVzMnteLMDUPGsnl52659351;     tFVzMnteLMDUPGsnl52659351 = tFVzMnteLMDUPGsnl10439115;     tFVzMnteLMDUPGsnl10439115 = tFVzMnteLMDUPGsnl12934794;     tFVzMnteLMDUPGsnl12934794 = tFVzMnteLMDUPGsnl99791795;     tFVzMnteLMDUPGsnl99791795 = tFVzMnteLMDUPGsnl5630450;     tFVzMnteLMDUPGsnl5630450 = tFVzMnteLMDUPGsnl93843500;     tFVzMnteLMDUPGsnl93843500 = tFVzMnteLMDUPGsnl41485086;     tFVzMnteLMDUPGsnl41485086 = tFVzMnteLMDUPGsnl17442800;     tFVzMnteLMDUPGsnl17442800 = tFVzMnteLMDUPGsnl21243414;     tFVzMnteLMDUPGsnl21243414 = tFVzMnteLMDUPGsnl53254286;     tFVzMnteLMDUPGsnl53254286 = tFVzMnteLMDUPGsnl13119354;     tFVzMnteLMDUPGsnl13119354 = tFVzMnteLMDUPGsnl63463475;     tFVzMnteLMDUPGsnl63463475 = tFVzMnteLMDUPGsnl18959589;     tFVzMnteLMDUPGsnl18959589 = tFVzMnteLMDUPGsnl94178328;     tFVzMnteLMDUPGsnl94178328 = tFVzMnteLMDUPGsnl11834335;     tFVzMnteLMDUPGsnl11834335 = tFVzMnteLMDUPGsnl51645270;     tFVzMnteLMDUPGsnl51645270 = tFVzMnteLMDUPGsnl77972930;     tFVzMnteLMDUPGsnl77972930 = tFVzMnteLMDUPGsnl60255179;     tFVzMnteLMDUPGsnl60255179 = tFVzMnteLMDUPGsnl72962063;     tFVzMnteLMDUPGsnl72962063 = tFVzMnteLMDUPGsnl91158566;     tFVzMnteLMDUPGsnl91158566 = tFVzMnteLMDUPGsnl24110915;     tFVzMnteLMDUPGsnl24110915 = tFVzMnteLMDUPGsnl14982029;     tFVzMnteLMDUPGsnl14982029 = tFVzMnteLMDUPGsnl18061498;     tFVzMnteLMDUPGsnl18061498 = tFVzMnteLMDUPGsnl60363682;     tFVzMnteLMDUPGsnl60363682 = tFVzMnteLMDUPGsnl98925620;     tFVzMnteLMDUPGsnl98925620 = tFVzMnteLMDUPGsnl79404382;     tFVzMnteLMDUPGsnl79404382 = tFVzMnteLMDUPGsnl92616938;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FdSWVWDLUTbYIIUi64236159() {     double zIKHMucDvPCUAvjon98734044 = -242182944;    double zIKHMucDvPCUAvjon24441199 = -520581033;    double zIKHMucDvPCUAvjon37464664 = -479184967;    double zIKHMucDvPCUAvjon77769479 = -446466364;    double zIKHMucDvPCUAvjon55676012 = -7315648;    double zIKHMucDvPCUAvjon69249051 = -25678884;    double zIKHMucDvPCUAvjon45990409 = -262293423;    double zIKHMucDvPCUAvjon25012853 = -87689516;    double zIKHMucDvPCUAvjon37025066 = 52049365;    double zIKHMucDvPCUAvjon38638052 = 62407803;    double zIKHMucDvPCUAvjon3703051 = -674994192;    double zIKHMucDvPCUAvjon68060379 = -821597516;    double zIKHMucDvPCUAvjon7939827 = -542484288;    double zIKHMucDvPCUAvjon11675654 = -772187042;    double zIKHMucDvPCUAvjon20653094 = -440213613;    double zIKHMucDvPCUAvjon89638321 = -573266403;    double zIKHMucDvPCUAvjon88400894 = -683463281;    double zIKHMucDvPCUAvjon71663531 = -329982060;    double zIKHMucDvPCUAvjon6164579 = -582305036;    double zIKHMucDvPCUAvjon54260579 = -669664038;    double zIKHMucDvPCUAvjon811781 = -523120555;    double zIKHMucDvPCUAvjon47799223 = -680577131;    double zIKHMucDvPCUAvjon60247944 = -74048602;    double zIKHMucDvPCUAvjon18201968 = -189375314;    double zIKHMucDvPCUAvjon31455255 = -135101593;    double zIKHMucDvPCUAvjon56358154 = -434148817;    double zIKHMucDvPCUAvjon22509451 = 42056174;    double zIKHMucDvPCUAvjon92539533 = -657287744;    double zIKHMucDvPCUAvjon57627242 = -756744663;    double zIKHMucDvPCUAvjon53018484 = -403020329;    double zIKHMucDvPCUAvjon63711205 = -997167155;    double zIKHMucDvPCUAvjon92028620 = -131164596;    double zIKHMucDvPCUAvjon31101540 = -883085440;    double zIKHMucDvPCUAvjon43593402 = -463055026;    double zIKHMucDvPCUAvjon44458661 = -376173549;    double zIKHMucDvPCUAvjon3109611 = -946708911;    double zIKHMucDvPCUAvjon28730001 = -333413029;    double zIKHMucDvPCUAvjon69991369 = -182110727;    double zIKHMucDvPCUAvjon19912865 = -821460572;    double zIKHMucDvPCUAvjon65395028 = -367943679;    double zIKHMucDvPCUAvjon26720288 = -143847226;    double zIKHMucDvPCUAvjon33969930 = -60771196;    double zIKHMucDvPCUAvjon26410384 = -864924998;    double zIKHMucDvPCUAvjon12273098 = -80548537;    double zIKHMucDvPCUAvjon40531609 = -296243578;    double zIKHMucDvPCUAvjon5851300 = -750322573;    double zIKHMucDvPCUAvjon7929967 = -253375365;    double zIKHMucDvPCUAvjon14563814 = -967364361;    double zIKHMucDvPCUAvjon67864723 = -233241317;    double zIKHMucDvPCUAvjon51152353 = -121523889;    double zIKHMucDvPCUAvjon6984172 = -510573187;    double zIKHMucDvPCUAvjon18814901 = -937590430;    double zIKHMucDvPCUAvjon23706867 = -951253141;    double zIKHMucDvPCUAvjon23181143 = -919527205;    double zIKHMucDvPCUAvjon63424378 = -59450477;    double zIKHMucDvPCUAvjon50934822 = -561605813;    double zIKHMucDvPCUAvjon64193255 = -346532432;    double zIKHMucDvPCUAvjon19262696 = -189809653;    double zIKHMucDvPCUAvjon46314224 = -211364772;    double zIKHMucDvPCUAvjon99317858 = -573166832;    double zIKHMucDvPCUAvjon46739600 = 32264941;    double zIKHMucDvPCUAvjon53450875 = -605005679;    double zIKHMucDvPCUAvjon67385611 = -330944854;    double zIKHMucDvPCUAvjon84006582 = -544930307;    double zIKHMucDvPCUAvjon74926846 = 59574958;    double zIKHMucDvPCUAvjon11674431 = -443829597;    double zIKHMucDvPCUAvjon36958840 = -938512076;    double zIKHMucDvPCUAvjon64346424 = 20570738;    double zIKHMucDvPCUAvjon67216992 = -296013493;    double zIKHMucDvPCUAvjon17543484 = -493504703;    double zIKHMucDvPCUAvjon60908321 = -139853374;    double zIKHMucDvPCUAvjon18409525 = -401352554;    double zIKHMucDvPCUAvjon51750666 = -508521488;    double zIKHMucDvPCUAvjon40769550 = -114361358;    double zIKHMucDvPCUAvjon27540292 = -425816813;    double zIKHMucDvPCUAvjon66841850 = -362349359;    double zIKHMucDvPCUAvjon21388839 = -815652134;    double zIKHMucDvPCUAvjon47974847 = -993500065;    double zIKHMucDvPCUAvjon77670358 = -893131737;    double zIKHMucDvPCUAvjon25603955 = -384779021;    double zIKHMucDvPCUAvjon48428187 = -80773453;    double zIKHMucDvPCUAvjon7945637 = 9420534;    double zIKHMucDvPCUAvjon24674810 = -324046428;    double zIKHMucDvPCUAvjon6474890 = -535220775;    double zIKHMucDvPCUAvjon46034313 = -892447142;    double zIKHMucDvPCUAvjon44896305 = 40423275;    double zIKHMucDvPCUAvjon68321753 = -179911455;    double zIKHMucDvPCUAvjon7920397 = -963558236;    double zIKHMucDvPCUAvjon80169024 = -303604549;    double zIKHMucDvPCUAvjon93523839 = -814567737;    double zIKHMucDvPCUAvjon38916356 = -500176479;    double zIKHMucDvPCUAvjon9467305 = -43603376;    double zIKHMucDvPCUAvjon23677146 = -970745956;    double zIKHMucDvPCUAvjon20595007 = -148293741;    double zIKHMucDvPCUAvjon18655428 = -300208621;    double zIKHMucDvPCUAvjon73269412 = -538841547;    double zIKHMucDvPCUAvjon66584319 = -729826343;    double zIKHMucDvPCUAvjon42403802 = -219994691;    double zIKHMucDvPCUAvjon37346251 = -40123495;    double zIKHMucDvPCUAvjon28857179 = -242182944;     zIKHMucDvPCUAvjon98734044 = zIKHMucDvPCUAvjon24441199;     zIKHMucDvPCUAvjon24441199 = zIKHMucDvPCUAvjon37464664;     zIKHMucDvPCUAvjon37464664 = zIKHMucDvPCUAvjon77769479;     zIKHMucDvPCUAvjon77769479 = zIKHMucDvPCUAvjon55676012;     zIKHMucDvPCUAvjon55676012 = zIKHMucDvPCUAvjon69249051;     zIKHMucDvPCUAvjon69249051 = zIKHMucDvPCUAvjon45990409;     zIKHMucDvPCUAvjon45990409 = zIKHMucDvPCUAvjon25012853;     zIKHMucDvPCUAvjon25012853 = zIKHMucDvPCUAvjon37025066;     zIKHMucDvPCUAvjon37025066 = zIKHMucDvPCUAvjon38638052;     zIKHMucDvPCUAvjon38638052 = zIKHMucDvPCUAvjon3703051;     zIKHMucDvPCUAvjon3703051 = zIKHMucDvPCUAvjon68060379;     zIKHMucDvPCUAvjon68060379 = zIKHMucDvPCUAvjon7939827;     zIKHMucDvPCUAvjon7939827 = zIKHMucDvPCUAvjon11675654;     zIKHMucDvPCUAvjon11675654 = zIKHMucDvPCUAvjon20653094;     zIKHMucDvPCUAvjon20653094 = zIKHMucDvPCUAvjon89638321;     zIKHMucDvPCUAvjon89638321 = zIKHMucDvPCUAvjon88400894;     zIKHMucDvPCUAvjon88400894 = zIKHMucDvPCUAvjon71663531;     zIKHMucDvPCUAvjon71663531 = zIKHMucDvPCUAvjon6164579;     zIKHMucDvPCUAvjon6164579 = zIKHMucDvPCUAvjon54260579;     zIKHMucDvPCUAvjon54260579 = zIKHMucDvPCUAvjon811781;     zIKHMucDvPCUAvjon811781 = zIKHMucDvPCUAvjon47799223;     zIKHMucDvPCUAvjon47799223 = zIKHMucDvPCUAvjon60247944;     zIKHMucDvPCUAvjon60247944 = zIKHMucDvPCUAvjon18201968;     zIKHMucDvPCUAvjon18201968 = zIKHMucDvPCUAvjon31455255;     zIKHMucDvPCUAvjon31455255 = zIKHMucDvPCUAvjon56358154;     zIKHMucDvPCUAvjon56358154 = zIKHMucDvPCUAvjon22509451;     zIKHMucDvPCUAvjon22509451 = zIKHMucDvPCUAvjon92539533;     zIKHMucDvPCUAvjon92539533 = zIKHMucDvPCUAvjon57627242;     zIKHMucDvPCUAvjon57627242 = zIKHMucDvPCUAvjon53018484;     zIKHMucDvPCUAvjon53018484 = zIKHMucDvPCUAvjon63711205;     zIKHMucDvPCUAvjon63711205 = zIKHMucDvPCUAvjon92028620;     zIKHMucDvPCUAvjon92028620 = zIKHMucDvPCUAvjon31101540;     zIKHMucDvPCUAvjon31101540 = zIKHMucDvPCUAvjon43593402;     zIKHMucDvPCUAvjon43593402 = zIKHMucDvPCUAvjon44458661;     zIKHMucDvPCUAvjon44458661 = zIKHMucDvPCUAvjon3109611;     zIKHMucDvPCUAvjon3109611 = zIKHMucDvPCUAvjon28730001;     zIKHMucDvPCUAvjon28730001 = zIKHMucDvPCUAvjon69991369;     zIKHMucDvPCUAvjon69991369 = zIKHMucDvPCUAvjon19912865;     zIKHMucDvPCUAvjon19912865 = zIKHMucDvPCUAvjon65395028;     zIKHMucDvPCUAvjon65395028 = zIKHMucDvPCUAvjon26720288;     zIKHMucDvPCUAvjon26720288 = zIKHMucDvPCUAvjon33969930;     zIKHMucDvPCUAvjon33969930 = zIKHMucDvPCUAvjon26410384;     zIKHMucDvPCUAvjon26410384 = zIKHMucDvPCUAvjon12273098;     zIKHMucDvPCUAvjon12273098 = zIKHMucDvPCUAvjon40531609;     zIKHMucDvPCUAvjon40531609 = zIKHMucDvPCUAvjon5851300;     zIKHMucDvPCUAvjon5851300 = zIKHMucDvPCUAvjon7929967;     zIKHMucDvPCUAvjon7929967 = zIKHMucDvPCUAvjon14563814;     zIKHMucDvPCUAvjon14563814 = zIKHMucDvPCUAvjon67864723;     zIKHMucDvPCUAvjon67864723 = zIKHMucDvPCUAvjon51152353;     zIKHMucDvPCUAvjon51152353 = zIKHMucDvPCUAvjon6984172;     zIKHMucDvPCUAvjon6984172 = zIKHMucDvPCUAvjon18814901;     zIKHMucDvPCUAvjon18814901 = zIKHMucDvPCUAvjon23706867;     zIKHMucDvPCUAvjon23706867 = zIKHMucDvPCUAvjon23181143;     zIKHMucDvPCUAvjon23181143 = zIKHMucDvPCUAvjon63424378;     zIKHMucDvPCUAvjon63424378 = zIKHMucDvPCUAvjon50934822;     zIKHMucDvPCUAvjon50934822 = zIKHMucDvPCUAvjon64193255;     zIKHMucDvPCUAvjon64193255 = zIKHMucDvPCUAvjon19262696;     zIKHMucDvPCUAvjon19262696 = zIKHMucDvPCUAvjon46314224;     zIKHMucDvPCUAvjon46314224 = zIKHMucDvPCUAvjon99317858;     zIKHMucDvPCUAvjon99317858 = zIKHMucDvPCUAvjon46739600;     zIKHMucDvPCUAvjon46739600 = zIKHMucDvPCUAvjon53450875;     zIKHMucDvPCUAvjon53450875 = zIKHMucDvPCUAvjon67385611;     zIKHMucDvPCUAvjon67385611 = zIKHMucDvPCUAvjon84006582;     zIKHMucDvPCUAvjon84006582 = zIKHMucDvPCUAvjon74926846;     zIKHMucDvPCUAvjon74926846 = zIKHMucDvPCUAvjon11674431;     zIKHMucDvPCUAvjon11674431 = zIKHMucDvPCUAvjon36958840;     zIKHMucDvPCUAvjon36958840 = zIKHMucDvPCUAvjon64346424;     zIKHMucDvPCUAvjon64346424 = zIKHMucDvPCUAvjon67216992;     zIKHMucDvPCUAvjon67216992 = zIKHMucDvPCUAvjon17543484;     zIKHMucDvPCUAvjon17543484 = zIKHMucDvPCUAvjon60908321;     zIKHMucDvPCUAvjon60908321 = zIKHMucDvPCUAvjon18409525;     zIKHMucDvPCUAvjon18409525 = zIKHMucDvPCUAvjon51750666;     zIKHMucDvPCUAvjon51750666 = zIKHMucDvPCUAvjon40769550;     zIKHMucDvPCUAvjon40769550 = zIKHMucDvPCUAvjon27540292;     zIKHMucDvPCUAvjon27540292 = zIKHMucDvPCUAvjon66841850;     zIKHMucDvPCUAvjon66841850 = zIKHMucDvPCUAvjon21388839;     zIKHMucDvPCUAvjon21388839 = zIKHMucDvPCUAvjon47974847;     zIKHMucDvPCUAvjon47974847 = zIKHMucDvPCUAvjon77670358;     zIKHMucDvPCUAvjon77670358 = zIKHMucDvPCUAvjon25603955;     zIKHMucDvPCUAvjon25603955 = zIKHMucDvPCUAvjon48428187;     zIKHMucDvPCUAvjon48428187 = zIKHMucDvPCUAvjon7945637;     zIKHMucDvPCUAvjon7945637 = zIKHMucDvPCUAvjon24674810;     zIKHMucDvPCUAvjon24674810 = zIKHMucDvPCUAvjon6474890;     zIKHMucDvPCUAvjon6474890 = zIKHMucDvPCUAvjon46034313;     zIKHMucDvPCUAvjon46034313 = zIKHMucDvPCUAvjon44896305;     zIKHMucDvPCUAvjon44896305 = zIKHMucDvPCUAvjon68321753;     zIKHMucDvPCUAvjon68321753 = zIKHMucDvPCUAvjon7920397;     zIKHMucDvPCUAvjon7920397 = zIKHMucDvPCUAvjon80169024;     zIKHMucDvPCUAvjon80169024 = zIKHMucDvPCUAvjon93523839;     zIKHMucDvPCUAvjon93523839 = zIKHMucDvPCUAvjon38916356;     zIKHMucDvPCUAvjon38916356 = zIKHMucDvPCUAvjon9467305;     zIKHMucDvPCUAvjon9467305 = zIKHMucDvPCUAvjon23677146;     zIKHMucDvPCUAvjon23677146 = zIKHMucDvPCUAvjon20595007;     zIKHMucDvPCUAvjon20595007 = zIKHMucDvPCUAvjon18655428;     zIKHMucDvPCUAvjon18655428 = zIKHMucDvPCUAvjon73269412;     zIKHMucDvPCUAvjon73269412 = zIKHMucDvPCUAvjon66584319;     zIKHMucDvPCUAvjon66584319 = zIKHMucDvPCUAvjon42403802;     zIKHMucDvPCUAvjon42403802 = zIKHMucDvPCUAvjon37346251;     zIKHMucDvPCUAvjon37346251 = zIKHMucDvPCUAvjon28857179;     zIKHMucDvPCUAvjon28857179 = zIKHMucDvPCUAvjon98734044;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mpLkovtLvKDfuhXH31527758() {     double pzLejPBFRKIiPIZwc34193013 = -708459844;    double pzLejPBFRKIiPIZwc37713278 = -401132640;    double pzLejPBFRKIiPIZwc74851140 = -144852042;    double pzLejPBFRKIiPIZwc49992834 = -267617435;    double pzLejPBFRKIiPIZwc11961282 = -112914494;    double pzLejPBFRKIiPIZwc5471923 = -288026780;    double pzLejPBFRKIiPIZwc56647815 = -107019157;    double pzLejPBFRKIiPIZwc87144414 = -831923657;    double pzLejPBFRKIiPIZwc38542671 = -436282042;    double pzLejPBFRKIiPIZwc53540764 = -702279195;    double pzLejPBFRKIiPIZwc41884785 = -477001921;    double pzLejPBFRKIiPIZwc68628528 = -685718288;    double pzLejPBFRKIiPIZwc46174472 = 5503207;    double pzLejPBFRKIiPIZwc64816516 = -748550451;    double pzLejPBFRKIiPIZwc78847610 = -943638628;    double pzLejPBFRKIiPIZwc73179622 = -209078010;    double pzLejPBFRKIiPIZwc44236830 = -110437834;    double pzLejPBFRKIiPIZwc71423723 = -403410129;    double pzLejPBFRKIiPIZwc6714909 = -898868458;    double pzLejPBFRKIiPIZwc48097797 = 3774896;    double pzLejPBFRKIiPIZwc13898644 = -180395244;    double pzLejPBFRKIiPIZwc51220748 = -767213425;    double pzLejPBFRKIiPIZwc30773324 = -811680786;    double pzLejPBFRKIiPIZwc11994913 = -700199918;    double pzLejPBFRKIiPIZwc39473206 = -159086791;    double pzLejPBFRKIiPIZwc67056325 = -122725624;    double pzLejPBFRKIiPIZwc82930024 = -864227967;    double pzLejPBFRKIiPIZwc33267958 = -827059919;    double pzLejPBFRKIiPIZwc4295457 = -230839465;    double pzLejPBFRKIiPIZwc58551895 = -437527499;    double pzLejPBFRKIiPIZwc82294436 = -449147642;    double pzLejPBFRKIiPIZwc29851639 = 95863990;    double pzLejPBFRKIiPIZwc43319551 = 69318582;    double pzLejPBFRKIiPIZwc73341829 = -527748840;    double pzLejPBFRKIiPIZwc10267115 = -327273800;    double pzLejPBFRKIiPIZwc54670454 = -61274217;    double pzLejPBFRKIiPIZwc78141817 = -198109069;    double pzLejPBFRKIiPIZwc21206773 = -164766308;    double pzLejPBFRKIiPIZwc50303427 = -472336849;    double pzLejPBFRKIiPIZwc29666561 = -933206225;    double pzLejPBFRKIiPIZwc60171895 = 86953860;    double pzLejPBFRKIiPIZwc23335057 = -755316487;    double pzLejPBFRKIiPIZwc15793763 = -418105784;    double pzLejPBFRKIiPIZwc52404436 = -684880947;    double pzLejPBFRKIiPIZwc40548068 = -122754739;    double pzLejPBFRKIiPIZwc51812628 = -789246205;    double pzLejPBFRKIiPIZwc27275131 = -737361725;    double pzLejPBFRKIiPIZwc32290126 = -33890954;    double pzLejPBFRKIiPIZwc90757389 = -996978197;    double pzLejPBFRKIiPIZwc75681942 = -612550704;    double pzLejPBFRKIiPIZwc87754895 = -384849744;    double pzLejPBFRKIiPIZwc2300373 = -51242777;    double pzLejPBFRKIiPIZwc92328015 = -707298695;    double pzLejPBFRKIiPIZwc39172989 = -11035705;    double pzLejPBFRKIiPIZwc17629115 = -405940623;    double pzLejPBFRKIiPIZwc82972265 = -941246419;    double pzLejPBFRKIiPIZwc6939954 = -589451854;    double pzLejPBFRKIiPIZwc62856227 = -444652124;    double pzLejPBFRKIiPIZwc10519628 = -8530645;    double pzLejPBFRKIiPIZwc44904956 = -990188870;    double pzLejPBFRKIiPIZwc22541898 = -423798813;    double pzLejPBFRKIiPIZwc23379857 = -279959239;    double pzLejPBFRKIiPIZwc82848957 = -501084192;    double pzLejPBFRKIiPIZwc79990776 = -998754544;    double pzLejPBFRKIiPIZwc71246327 = -153131553;    double pzLejPBFRKIiPIZwc12033147 = -472865912;    double pzLejPBFRKIiPIZwc25308978 = -655036871;    double pzLejPBFRKIiPIZwc72832643 = -466747954;    double pzLejPBFRKIiPIZwc54549402 = -321276652;    double pzLejPBFRKIiPIZwc24177156 = -782364412;    double pzLejPBFRKIiPIZwc95037805 = 89031059;    double pzLejPBFRKIiPIZwc23030057 = -945671526;    double pzLejPBFRKIiPIZwc21120296 = -931073280;    double pzLejPBFRKIiPIZwc77048347 = -965662233;    double pzLejPBFRKIiPIZwc87925902 = 16821036;    double pzLejPBFRKIiPIZwc90563586 = -425078757;    double pzLejPBFRKIiPIZwc35426985 = -249107642;    double pzLejPBFRKIiPIZwc78368888 = -26799839;    double pzLejPBFRKIiPIZwc71446845 = -477445180;    double pzLejPBFRKIiPIZwc87660578 = -369840587;    double pzLejPBFRKIiPIZwc39781195 = -385363899;    double pzLejPBFRKIiPIZwc50639899 = -730337013;    double pzLejPBFRKIiPIZwc42510569 = -830081722;    double pzLejPBFRKIiPIZwc28613515 = -618288761;    double pzLejPBFRKIiPIZwc70796999 = 47322245;    double pzLejPBFRKIiPIZwc79994064 = -297904866;    double pzLejPBFRKIiPIZwc37523623 = -196837316;    double pzLejPBFRKIiPIZwc4146562 = -919645713;    double pzLejPBFRKIiPIZwc55712714 = -21808217;    double pzLejPBFRKIiPIZwc27294849 = -386027381;    double pzLejPBFRKIiPIZwc47730501 = -471822363;    double pzLejPBFRKIiPIZwc15285590 = -753456946;    double pzLejPBFRKIiPIZwc10687145 = -56235664;    double pzLejPBFRKIiPIZwc5398471 = -482147979;    double pzLejPBFRKIiPIZwc7124663 = -409407412;    double pzLejPBFRKIiPIZwc36792039 = -633086902;    double pzLejPBFRKIiPIZwc40486100 = -154232296;    double pzLejPBFRKIiPIZwc35802986 = -419351241;    double pzLejPBFRKIiPIZwc81158108 = -431749394;    double pzLejPBFRKIiPIZwc28514921 = -708459844;     pzLejPBFRKIiPIZwc34193013 = pzLejPBFRKIiPIZwc37713278;     pzLejPBFRKIiPIZwc37713278 = pzLejPBFRKIiPIZwc74851140;     pzLejPBFRKIiPIZwc74851140 = pzLejPBFRKIiPIZwc49992834;     pzLejPBFRKIiPIZwc49992834 = pzLejPBFRKIiPIZwc11961282;     pzLejPBFRKIiPIZwc11961282 = pzLejPBFRKIiPIZwc5471923;     pzLejPBFRKIiPIZwc5471923 = pzLejPBFRKIiPIZwc56647815;     pzLejPBFRKIiPIZwc56647815 = pzLejPBFRKIiPIZwc87144414;     pzLejPBFRKIiPIZwc87144414 = pzLejPBFRKIiPIZwc38542671;     pzLejPBFRKIiPIZwc38542671 = pzLejPBFRKIiPIZwc53540764;     pzLejPBFRKIiPIZwc53540764 = pzLejPBFRKIiPIZwc41884785;     pzLejPBFRKIiPIZwc41884785 = pzLejPBFRKIiPIZwc68628528;     pzLejPBFRKIiPIZwc68628528 = pzLejPBFRKIiPIZwc46174472;     pzLejPBFRKIiPIZwc46174472 = pzLejPBFRKIiPIZwc64816516;     pzLejPBFRKIiPIZwc64816516 = pzLejPBFRKIiPIZwc78847610;     pzLejPBFRKIiPIZwc78847610 = pzLejPBFRKIiPIZwc73179622;     pzLejPBFRKIiPIZwc73179622 = pzLejPBFRKIiPIZwc44236830;     pzLejPBFRKIiPIZwc44236830 = pzLejPBFRKIiPIZwc71423723;     pzLejPBFRKIiPIZwc71423723 = pzLejPBFRKIiPIZwc6714909;     pzLejPBFRKIiPIZwc6714909 = pzLejPBFRKIiPIZwc48097797;     pzLejPBFRKIiPIZwc48097797 = pzLejPBFRKIiPIZwc13898644;     pzLejPBFRKIiPIZwc13898644 = pzLejPBFRKIiPIZwc51220748;     pzLejPBFRKIiPIZwc51220748 = pzLejPBFRKIiPIZwc30773324;     pzLejPBFRKIiPIZwc30773324 = pzLejPBFRKIiPIZwc11994913;     pzLejPBFRKIiPIZwc11994913 = pzLejPBFRKIiPIZwc39473206;     pzLejPBFRKIiPIZwc39473206 = pzLejPBFRKIiPIZwc67056325;     pzLejPBFRKIiPIZwc67056325 = pzLejPBFRKIiPIZwc82930024;     pzLejPBFRKIiPIZwc82930024 = pzLejPBFRKIiPIZwc33267958;     pzLejPBFRKIiPIZwc33267958 = pzLejPBFRKIiPIZwc4295457;     pzLejPBFRKIiPIZwc4295457 = pzLejPBFRKIiPIZwc58551895;     pzLejPBFRKIiPIZwc58551895 = pzLejPBFRKIiPIZwc82294436;     pzLejPBFRKIiPIZwc82294436 = pzLejPBFRKIiPIZwc29851639;     pzLejPBFRKIiPIZwc29851639 = pzLejPBFRKIiPIZwc43319551;     pzLejPBFRKIiPIZwc43319551 = pzLejPBFRKIiPIZwc73341829;     pzLejPBFRKIiPIZwc73341829 = pzLejPBFRKIiPIZwc10267115;     pzLejPBFRKIiPIZwc10267115 = pzLejPBFRKIiPIZwc54670454;     pzLejPBFRKIiPIZwc54670454 = pzLejPBFRKIiPIZwc78141817;     pzLejPBFRKIiPIZwc78141817 = pzLejPBFRKIiPIZwc21206773;     pzLejPBFRKIiPIZwc21206773 = pzLejPBFRKIiPIZwc50303427;     pzLejPBFRKIiPIZwc50303427 = pzLejPBFRKIiPIZwc29666561;     pzLejPBFRKIiPIZwc29666561 = pzLejPBFRKIiPIZwc60171895;     pzLejPBFRKIiPIZwc60171895 = pzLejPBFRKIiPIZwc23335057;     pzLejPBFRKIiPIZwc23335057 = pzLejPBFRKIiPIZwc15793763;     pzLejPBFRKIiPIZwc15793763 = pzLejPBFRKIiPIZwc52404436;     pzLejPBFRKIiPIZwc52404436 = pzLejPBFRKIiPIZwc40548068;     pzLejPBFRKIiPIZwc40548068 = pzLejPBFRKIiPIZwc51812628;     pzLejPBFRKIiPIZwc51812628 = pzLejPBFRKIiPIZwc27275131;     pzLejPBFRKIiPIZwc27275131 = pzLejPBFRKIiPIZwc32290126;     pzLejPBFRKIiPIZwc32290126 = pzLejPBFRKIiPIZwc90757389;     pzLejPBFRKIiPIZwc90757389 = pzLejPBFRKIiPIZwc75681942;     pzLejPBFRKIiPIZwc75681942 = pzLejPBFRKIiPIZwc87754895;     pzLejPBFRKIiPIZwc87754895 = pzLejPBFRKIiPIZwc2300373;     pzLejPBFRKIiPIZwc2300373 = pzLejPBFRKIiPIZwc92328015;     pzLejPBFRKIiPIZwc92328015 = pzLejPBFRKIiPIZwc39172989;     pzLejPBFRKIiPIZwc39172989 = pzLejPBFRKIiPIZwc17629115;     pzLejPBFRKIiPIZwc17629115 = pzLejPBFRKIiPIZwc82972265;     pzLejPBFRKIiPIZwc82972265 = pzLejPBFRKIiPIZwc6939954;     pzLejPBFRKIiPIZwc6939954 = pzLejPBFRKIiPIZwc62856227;     pzLejPBFRKIiPIZwc62856227 = pzLejPBFRKIiPIZwc10519628;     pzLejPBFRKIiPIZwc10519628 = pzLejPBFRKIiPIZwc44904956;     pzLejPBFRKIiPIZwc44904956 = pzLejPBFRKIiPIZwc22541898;     pzLejPBFRKIiPIZwc22541898 = pzLejPBFRKIiPIZwc23379857;     pzLejPBFRKIiPIZwc23379857 = pzLejPBFRKIiPIZwc82848957;     pzLejPBFRKIiPIZwc82848957 = pzLejPBFRKIiPIZwc79990776;     pzLejPBFRKIiPIZwc79990776 = pzLejPBFRKIiPIZwc71246327;     pzLejPBFRKIiPIZwc71246327 = pzLejPBFRKIiPIZwc12033147;     pzLejPBFRKIiPIZwc12033147 = pzLejPBFRKIiPIZwc25308978;     pzLejPBFRKIiPIZwc25308978 = pzLejPBFRKIiPIZwc72832643;     pzLejPBFRKIiPIZwc72832643 = pzLejPBFRKIiPIZwc54549402;     pzLejPBFRKIiPIZwc54549402 = pzLejPBFRKIiPIZwc24177156;     pzLejPBFRKIiPIZwc24177156 = pzLejPBFRKIiPIZwc95037805;     pzLejPBFRKIiPIZwc95037805 = pzLejPBFRKIiPIZwc23030057;     pzLejPBFRKIiPIZwc23030057 = pzLejPBFRKIiPIZwc21120296;     pzLejPBFRKIiPIZwc21120296 = pzLejPBFRKIiPIZwc77048347;     pzLejPBFRKIiPIZwc77048347 = pzLejPBFRKIiPIZwc87925902;     pzLejPBFRKIiPIZwc87925902 = pzLejPBFRKIiPIZwc90563586;     pzLejPBFRKIiPIZwc90563586 = pzLejPBFRKIiPIZwc35426985;     pzLejPBFRKIiPIZwc35426985 = pzLejPBFRKIiPIZwc78368888;     pzLejPBFRKIiPIZwc78368888 = pzLejPBFRKIiPIZwc71446845;     pzLejPBFRKIiPIZwc71446845 = pzLejPBFRKIiPIZwc87660578;     pzLejPBFRKIiPIZwc87660578 = pzLejPBFRKIiPIZwc39781195;     pzLejPBFRKIiPIZwc39781195 = pzLejPBFRKIiPIZwc50639899;     pzLejPBFRKIiPIZwc50639899 = pzLejPBFRKIiPIZwc42510569;     pzLejPBFRKIiPIZwc42510569 = pzLejPBFRKIiPIZwc28613515;     pzLejPBFRKIiPIZwc28613515 = pzLejPBFRKIiPIZwc70796999;     pzLejPBFRKIiPIZwc70796999 = pzLejPBFRKIiPIZwc79994064;     pzLejPBFRKIiPIZwc79994064 = pzLejPBFRKIiPIZwc37523623;     pzLejPBFRKIiPIZwc37523623 = pzLejPBFRKIiPIZwc4146562;     pzLejPBFRKIiPIZwc4146562 = pzLejPBFRKIiPIZwc55712714;     pzLejPBFRKIiPIZwc55712714 = pzLejPBFRKIiPIZwc27294849;     pzLejPBFRKIiPIZwc27294849 = pzLejPBFRKIiPIZwc47730501;     pzLejPBFRKIiPIZwc47730501 = pzLejPBFRKIiPIZwc15285590;     pzLejPBFRKIiPIZwc15285590 = pzLejPBFRKIiPIZwc10687145;     pzLejPBFRKIiPIZwc10687145 = pzLejPBFRKIiPIZwc5398471;     pzLejPBFRKIiPIZwc5398471 = pzLejPBFRKIiPIZwc7124663;     pzLejPBFRKIiPIZwc7124663 = pzLejPBFRKIiPIZwc36792039;     pzLejPBFRKIiPIZwc36792039 = pzLejPBFRKIiPIZwc40486100;     pzLejPBFRKIiPIZwc40486100 = pzLejPBFRKIiPIZwc35802986;     pzLejPBFRKIiPIZwc35802986 = pzLejPBFRKIiPIZwc81158108;     pzLejPBFRKIiPIZwc81158108 = pzLejPBFRKIiPIZwc28514921;     pzLejPBFRKIiPIZwc28514921 = pzLejPBFRKIiPIZwc34193013;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YzQhaRRIDhFQvzSU46576826() {     double hygkYovrrDtGKtdmQ40310120 = -820361732;    double hygkYovrrDtGKtdmQ81086557 = -110732420;    double hygkYovrrDtGKtdmQ90893324 = -295684963;    double hygkYovrrDtGKtdmQ77067614 = -458366066;    double hygkYovrrDtGKtdmQ80856770 = -633564365;    double hygkYovrrDtGKtdmQ96889125 = -552523015;    double hygkYovrrDtGKtdmQ17136781 = -245984134;    double hygkYovrrDtGKtdmQ36904305 = -18797570;    double hygkYovrrDtGKtdmQ3185167 = -204314708;    double hygkYovrrDtGKtdmQ94770423 = 68745328;    double hygkYovrrDtGKtdmQ1995076 = -354439508;    double hygkYovrrDtGKtdmQ451571 = -30124490;    double hygkYovrrDtGKtdmQ32383591 = -434387882;    double hygkYovrrDtGKtdmQ47960193 = -180789019;    double hygkYovrrDtGKtdmQ74590037 = -534100794;    double hygkYovrrDtGKtdmQ14685016 = -182077553;    double hygkYovrrDtGKtdmQ38072833 = -726982946;    double hygkYovrrDtGKtdmQ68476876 = -68932771;    double hygkYovrrDtGKtdmQ68544750 = -204637555;    double hygkYovrrDtGKtdmQ28647962 = -353128024;    double hygkYovrrDtGKtdmQ2451837 = -35882699;    double hygkYovrrDtGKtdmQ37741772 = -675143141;    double hygkYovrrDtGKtdmQ66364116 = -558198273;    double hygkYovrrDtGKtdmQ95987286 = -922504906;    double hygkYovrrDtGKtdmQ83172856 = -532282824;    double hygkYovrrDtGKtdmQ91778000 = -1380947;    double hygkYovrrDtGKtdmQ81389802 = -764890170;    double hygkYovrrDtGKtdmQ46375084 = -892988872;    double hygkYovrrDtGKtdmQ70427712 = -453222170;    double hygkYovrrDtGKtdmQ27167532 = -254998367;    double hygkYovrrDtGKtdmQ74536986 = -362468580;    double hygkYovrrDtGKtdmQ27462260 = -713582826;    double hygkYovrrDtGKtdmQ53030499 = -923294788;    double hygkYovrrDtGKtdmQ68172393 = -279290605;    double hygkYovrrDtGKtdmQ71741766 = -966006905;    double hygkYovrrDtGKtdmQ23396368 = -275300324;    double hygkYovrrDtGKtdmQ59403751 = -151744939;    double hygkYovrrDtGKtdmQ55296985 = 63727682;    double hygkYovrrDtGKtdmQ23913681 = -466260346;    double hygkYovrrDtGKtdmQ17168499 = -306645448;    double hygkYovrrDtGKtdmQ65841119 = 21679966;    double hygkYovrrDtGKtdmQ55485514 = 74541235;    double hygkYovrrDtGKtdmQ93860740 = -498910368;    double hygkYovrrDtGKtdmQ39812176 = -704223885;    double hygkYovrrDtGKtdmQ52500533 = -884652894;    double hygkYovrrDtGKtdmQ63751823 = -774462321;    double hygkYovrrDtGKtdmQ45053705 = -856137263;    double hygkYovrrDtGKtdmQ40247067 = -43786487;    double hygkYovrrDtGKtdmQ433119 = 17284815;    double hygkYovrrDtGKtdmQ88593595 = -699561512;    double hygkYovrrDtGKtdmQ23455574 = -227143416;    double hygkYovrrDtGKtdmQ13110093 = -951969097;    double hygkYovrrDtGKtdmQ40576473 = -499088283;    double hygkYovrrDtGKtdmQ35141870 = -331505850;    double hygkYovrrDtGKtdmQ44124989 = -6472406;    double hygkYovrrDtGKtdmQ2568349 = -45218592;    double hygkYovrrDtGKtdmQ14722441 = -552534148;    double hygkYovrrDtGKtdmQ94906037 = -373180058;    double hygkYovrrDtGKtdmQ93894758 = -926083242;    double hygkYovrrDtGKtdmQ89078769 = -532183418;    double hygkYovrrDtGKtdmQ15499323 = -787632845;    double hygkYovrrDtGKtdmQ70761697 = -352995262;    double hygkYovrrDtGKtdmQ66476593 = -565575401;    double hygkYovrrDtGKtdmQ76017635 = -949316342;    double hygkYovrrDtGKtdmQ20233437 = -568786093;    double hygkYovrrDtGKtdmQ74532815 = -640856682;    double hygkYovrrDtGKtdmQ47421071 = -106829702;    double hygkYovrrDtGKtdmQ64211197 = -55097277;    double hygkYovrrDtGKtdmQ76218426 = -214782114;    double hygkYovrrDtGKtdmQ51193670 = -158800471;    double hygkYovrrDtGKtdmQ55281264 = 69667386;    double hygkYovrrDtGKtdmQ82775847 = -690710629;    double hygkYovrrDtGKtdmQ44563195 = -602672426;    double hygkYovrrDtGKtdmQ51376251 = -897992107;    double hygkYovrrDtGKtdmQ62806843 = -274807990;    double hygkYovrrDtGKtdmQ46966323 = -10423934;    double hygkYovrrDtGKtdmQ43881031 = -76232774;    double hygkYovrrDtGKtdmQ26551940 = -853974388;    double hygkYovrrDtGKtdmQ43486754 = 62147987;    double hygkYovrrDtGKtdmQ19421033 = -757820503;    double hygkYovrrDtGKtdmQ46724296 = -145243685;    double hygkYovrrDtGKtdmQ41142736 = -621103684;    double hygkYovrrDtGKtdmQ45941965 = -810273687;    double hygkYovrrDtGKtdmQ81834117 = -753660658;    double hygkYovrrDtGKtdmQ3711959 = 72145049;    double hygkYovrrDtGKtdmQ61426894 = -410499484;    double hygkYovrrDtGKtdmQ86885787 = -114494544;    double hygkYovrrDtGKtdmQ17888630 = -491788938;    double hygkYovrrDtGKtdmQ24047404 = -172818200;    double hygkYovrrDtGKtdmQ69173418 = -820788314;    double hygkYovrrDtGKtdmQ8673927 = -722766177;    double hygkYovrrDtGKtdmQ64497714 = -778564882;    double hygkYovrrDtGKtdmQ61402227 = -10189077;    double hygkYovrrDtGKtdmQ34834911 = -934076928;    double hygkYovrrDtGKtdmQ1669176 = -519012603;    double hygkYovrrDtGKtdmQ95079421 = -625324773;    double hygkYovrrDtGKtdmQ89008921 = -359883365;    double hygkYovrrDtGKtdmQ17843106 = -549594026;    double hygkYovrrDtGKtdmQ19578739 = -35437793;    double hygkYovrrDtGKtdmQ77967718 = -820361732;     hygkYovrrDtGKtdmQ40310120 = hygkYovrrDtGKtdmQ81086557;     hygkYovrrDtGKtdmQ81086557 = hygkYovrrDtGKtdmQ90893324;     hygkYovrrDtGKtdmQ90893324 = hygkYovrrDtGKtdmQ77067614;     hygkYovrrDtGKtdmQ77067614 = hygkYovrrDtGKtdmQ80856770;     hygkYovrrDtGKtdmQ80856770 = hygkYovrrDtGKtdmQ96889125;     hygkYovrrDtGKtdmQ96889125 = hygkYovrrDtGKtdmQ17136781;     hygkYovrrDtGKtdmQ17136781 = hygkYovrrDtGKtdmQ36904305;     hygkYovrrDtGKtdmQ36904305 = hygkYovrrDtGKtdmQ3185167;     hygkYovrrDtGKtdmQ3185167 = hygkYovrrDtGKtdmQ94770423;     hygkYovrrDtGKtdmQ94770423 = hygkYovrrDtGKtdmQ1995076;     hygkYovrrDtGKtdmQ1995076 = hygkYovrrDtGKtdmQ451571;     hygkYovrrDtGKtdmQ451571 = hygkYovrrDtGKtdmQ32383591;     hygkYovrrDtGKtdmQ32383591 = hygkYovrrDtGKtdmQ47960193;     hygkYovrrDtGKtdmQ47960193 = hygkYovrrDtGKtdmQ74590037;     hygkYovrrDtGKtdmQ74590037 = hygkYovrrDtGKtdmQ14685016;     hygkYovrrDtGKtdmQ14685016 = hygkYovrrDtGKtdmQ38072833;     hygkYovrrDtGKtdmQ38072833 = hygkYovrrDtGKtdmQ68476876;     hygkYovrrDtGKtdmQ68476876 = hygkYovrrDtGKtdmQ68544750;     hygkYovrrDtGKtdmQ68544750 = hygkYovrrDtGKtdmQ28647962;     hygkYovrrDtGKtdmQ28647962 = hygkYovrrDtGKtdmQ2451837;     hygkYovrrDtGKtdmQ2451837 = hygkYovrrDtGKtdmQ37741772;     hygkYovrrDtGKtdmQ37741772 = hygkYovrrDtGKtdmQ66364116;     hygkYovrrDtGKtdmQ66364116 = hygkYovrrDtGKtdmQ95987286;     hygkYovrrDtGKtdmQ95987286 = hygkYovrrDtGKtdmQ83172856;     hygkYovrrDtGKtdmQ83172856 = hygkYovrrDtGKtdmQ91778000;     hygkYovrrDtGKtdmQ91778000 = hygkYovrrDtGKtdmQ81389802;     hygkYovrrDtGKtdmQ81389802 = hygkYovrrDtGKtdmQ46375084;     hygkYovrrDtGKtdmQ46375084 = hygkYovrrDtGKtdmQ70427712;     hygkYovrrDtGKtdmQ70427712 = hygkYovrrDtGKtdmQ27167532;     hygkYovrrDtGKtdmQ27167532 = hygkYovrrDtGKtdmQ74536986;     hygkYovrrDtGKtdmQ74536986 = hygkYovrrDtGKtdmQ27462260;     hygkYovrrDtGKtdmQ27462260 = hygkYovrrDtGKtdmQ53030499;     hygkYovrrDtGKtdmQ53030499 = hygkYovrrDtGKtdmQ68172393;     hygkYovrrDtGKtdmQ68172393 = hygkYovrrDtGKtdmQ71741766;     hygkYovrrDtGKtdmQ71741766 = hygkYovrrDtGKtdmQ23396368;     hygkYovrrDtGKtdmQ23396368 = hygkYovrrDtGKtdmQ59403751;     hygkYovrrDtGKtdmQ59403751 = hygkYovrrDtGKtdmQ55296985;     hygkYovrrDtGKtdmQ55296985 = hygkYovrrDtGKtdmQ23913681;     hygkYovrrDtGKtdmQ23913681 = hygkYovrrDtGKtdmQ17168499;     hygkYovrrDtGKtdmQ17168499 = hygkYovrrDtGKtdmQ65841119;     hygkYovrrDtGKtdmQ65841119 = hygkYovrrDtGKtdmQ55485514;     hygkYovrrDtGKtdmQ55485514 = hygkYovrrDtGKtdmQ93860740;     hygkYovrrDtGKtdmQ93860740 = hygkYovrrDtGKtdmQ39812176;     hygkYovrrDtGKtdmQ39812176 = hygkYovrrDtGKtdmQ52500533;     hygkYovrrDtGKtdmQ52500533 = hygkYovrrDtGKtdmQ63751823;     hygkYovrrDtGKtdmQ63751823 = hygkYovrrDtGKtdmQ45053705;     hygkYovrrDtGKtdmQ45053705 = hygkYovrrDtGKtdmQ40247067;     hygkYovrrDtGKtdmQ40247067 = hygkYovrrDtGKtdmQ433119;     hygkYovrrDtGKtdmQ433119 = hygkYovrrDtGKtdmQ88593595;     hygkYovrrDtGKtdmQ88593595 = hygkYovrrDtGKtdmQ23455574;     hygkYovrrDtGKtdmQ23455574 = hygkYovrrDtGKtdmQ13110093;     hygkYovrrDtGKtdmQ13110093 = hygkYovrrDtGKtdmQ40576473;     hygkYovrrDtGKtdmQ40576473 = hygkYovrrDtGKtdmQ35141870;     hygkYovrrDtGKtdmQ35141870 = hygkYovrrDtGKtdmQ44124989;     hygkYovrrDtGKtdmQ44124989 = hygkYovrrDtGKtdmQ2568349;     hygkYovrrDtGKtdmQ2568349 = hygkYovrrDtGKtdmQ14722441;     hygkYovrrDtGKtdmQ14722441 = hygkYovrrDtGKtdmQ94906037;     hygkYovrrDtGKtdmQ94906037 = hygkYovrrDtGKtdmQ93894758;     hygkYovrrDtGKtdmQ93894758 = hygkYovrrDtGKtdmQ89078769;     hygkYovrrDtGKtdmQ89078769 = hygkYovrrDtGKtdmQ15499323;     hygkYovrrDtGKtdmQ15499323 = hygkYovrrDtGKtdmQ70761697;     hygkYovrrDtGKtdmQ70761697 = hygkYovrrDtGKtdmQ66476593;     hygkYovrrDtGKtdmQ66476593 = hygkYovrrDtGKtdmQ76017635;     hygkYovrrDtGKtdmQ76017635 = hygkYovrrDtGKtdmQ20233437;     hygkYovrrDtGKtdmQ20233437 = hygkYovrrDtGKtdmQ74532815;     hygkYovrrDtGKtdmQ74532815 = hygkYovrrDtGKtdmQ47421071;     hygkYovrrDtGKtdmQ47421071 = hygkYovrrDtGKtdmQ64211197;     hygkYovrrDtGKtdmQ64211197 = hygkYovrrDtGKtdmQ76218426;     hygkYovrrDtGKtdmQ76218426 = hygkYovrrDtGKtdmQ51193670;     hygkYovrrDtGKtdmQ51193670 = hygkYovrrDtGKtdmQ55281264;     hygkYovrrDtGKtdmQ55281264 = hygkYovrrDtGKtdmQ82775847;     hygkYovrrDtGKtdmQ82775847 = hygkYovrrDtGKtdmQ44563195;     hygkYovrrDtGKtdmQ44563195 = hygkYovrrDtGKtdmQ51376251;     hygkYovrrDtGKtdmQ51376251 = hygkYovrrDtGKtdmQ62806843;     hygkYovrrDtGKtdmQ62806843 = hygkYovrrDtGKtdmQ46966323;     hygkYovrrDtGKtdmQ46966323 = hygkYovrrDtGKtdmQ43881031;     hygkYovrrDtGKtdmQ43881031 = hygkYovrrDtGKtdmQ26551940;     hygkYovrrDtGKtdmQ26551940 = hygkYovrrDtGKtdmQ43486754;     hygkYovrrDtGKtdmQ43486754 = hygkYovrrDtGKtdmQ19421033;     hygkYovrrDtGKtdmQ19421033 = hygkYovrrDtGKtdmQ46724296;     hygkYovrrDtGKtdmQ46724296 = hygkYovrrDtGKtdmQ41142736;     hygkYovrrDtGKtdmQ41142736 = hygkYovrrDtGKtdmQ45941965;     hygkYovrrDtGKtdmQ45941965 = hygkYovrrDtGKtdmQ81834117;     hygkYovrrDtGKtdmQ81834117 = hygkYovrrDtGKtdmQ3711959;     hygkYovrrDtGKtdmQ3711959 = hygkYovrrDtGKtdmQ61426894;     hygkYovrrDtGKtdmQ61426894 = hygkYovrrDtGKtdmQ86885787;     hygkYovrrDtGKtdmQ86885787 = hygkYovrrDtGKtdmQ17888630;     hygkYovrrDtGKtdmQ17888630 = hygkYovrrDtGKtdmQ24047404;     hygkYovrrDtGKtdmQ24047404 = hygkYovrrDtGKtdmQ69173418;     hygkYovrrDtGKtdmQ69173418 = hygkYovrrDtGKtdmQ8673927;     hygkYovrrDtGKtdmQ8673927 = hygkYovrrDtGKtdmQ64497714;     hygkYovrrDtGKtdmQ64497714 = hygkYovrrDtGKtdmQ61402227;     hygkYovrrDtGKtdmQ61402227 = hygkYovrrDtGKtdmQ34834911;     hygkYovrrDtGKtdmQ34834911 = hygkYovrrDtGKtdmQ1669176;     hygkYovrrDtGKtdmQ1669176 = hygkYovrrDtGKtdmQ95079421;     hygkYovrrDtGKtdmQ95079421 = hygkYovrrDtGKtdmQ89008921;     hygkYovrrDtGKtdmQ89008921 = hygkYovrrDtGKtdmQ17843106;     hygkYovrrDtGKtdmQ17843106 = hygkYovrrDtGKtdmQ19578739;     hygkYovrrDtGKtdmQ19578739 = hygkYovrrDtGKtdmQ77967718;     hygkYovrrDtGKtdmQ77967718 = hygkYovrrDtGKtdmQ40310120;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void eQnCrWqSGVVjkejP13868426() {     double nNFmyYVtTCykGFvoH75769088 = -186638632;    double nNFmyYVtTCykGFvoH94358635 = 8715973;    double nNFmyYVtTCykGFvoH28279801 = 38647962;    double nNFmyYVtTCykGFvoH49290969 = -279517137;    double nNFmyYVtTCykGFvoH37142039 = -739163210;    double nNFmyYVtTCykGFvoH33111997 = -814870910;    double nNFmyYVtTCykGFvoH27794187 = -90709869;    double nNFmyYVtTCykGFvoH99035865 = -763031710;    double nNFmyYVtTCykGFvoH4702772 = -692646115;    double nNFmyYVtTCykGFvoH9673136 = -695941670;    double nNFmyYVtTCykGFvoH40176810 = -156447237;    double nNFmyYVtTCykGFvoH1019719 = -994245261;    double nNFmyYVtTCykGFvoH70618236 = -986400387;    double nNFmyYVtTCykGFvoH1101057 = -157152428;    double nNFmyYVtTCykGFvoH32784553 = 62474191;    double nNFmyYVtTCykGFvoH98226316 = -917889160;    double nNFmyYVtTCykGFvoH93908769 = -153957499;    double nNFmyYVtTCykGFvoH68237067 = -142360840;    double nNFmyYVtTCykGFvoH69095080 = -521200976;    double nNFmyYVtTCykGFvoH22485181 = -779689089;    double nNFmyYVtTCykGFvoH15538700 = -793157388;    double nNFmyYVtTCykGFvoH41163296 = -761779435;    double nNFmyYVtTCykGFvoH36889496 = -195830457;    double nNFmyYVtTCykGFvoH89780232 = -333329510;    double nNFmyYVtTCykGFvoH91190807 = -556268023;    double nNFmyYVtTCykGFvoH2476173 = -789957754;    double nNFmyYVtTCykGFvoH41810377 = -571174311;    double nNFmyYVtTCykGFvoH87103508 = 37238953;    double nNFmyYVtTCykGFvoH17095927 = 72683028;    double nNFmyYVtTCykGFvoH32700943 = -289505537;    double nNFmyYVtTCykGFvoH93120217 = -914449067;    double nNFmyYVtTCykGFvoH65285279 = -486554240;    double nNFmyYVtTCykGFvoH65248510 = 29109235;    double nNFmyYVtTCykGFvoH97920819 = -343984418;    double nNFmyYVtTCykGFvoH37550220 = -917107156;    double nNFmyYVtTCykGFvoH74957211 = -489865630;    double nNFmyYVtTCykGFvoH8815568 = -16440979;    double nNFmyYVtTCykGFvoH6512389 = 81072101;    double nNFmyYVtTCykGFvoH54304242 = -117136622;    double nNFmyYVtTCykGFvoH81440030 = -871907994;    double nNFmyYVtTCykGFvoH99292726 = -847518949;    double nNFmyYVtTCykGFvoH44850641 = -620004056;    double nNFmyYVtTCykGFvoH83244119 = -52091153;    double nNFmyYVtTCykGFvoH79943514 = -208556295;    double nNFmyYVtTCykGFvoH52516992 = -711164055;    double nNFmyYVtTCykGFvoH9713152 = -813385954;    double nNFmyYVtTCykGFvoH64398868 = -240123624;    double nNFmyYVtTCykGFvoH57973378 = -210313080;    double nNFmyYVtTCykGFvoH23325785 = -746452066;    double nNFmyYVtTCykGFvoH13123185 = -90588328;    double nNFmyYVtTCykGFvoH4226298 = -101419973;    double nNFmyYVtTCykGFvoH96595564 = -65621443;    double nNFmyYVtTCykGFvoH9197621 = -255133836;    double nNFmyYVtTCykGFvoH51133716 = -523014350;    double nNFmyYVtTCykGFvoH98329725 = -352962552;    double nNFmyYVtTCykGFvoH34605792 = -424859197;    double nNFmyYVtTCykGFvoH57469140 = -795453570;    double nNFmyYVtTCykGFvoH38499569 = -628022528;    double nNFmyYVtTCykGFvoH58100162 = -723249115;    double nNFmyYVtTCykGFvoH34665867 = -949205457;    double nNFmyYVtTCykGFvoH91301620 = -143696600;    double nNFmyYVtTCykGFvoH40690679 = -27948822;    double nNFmyYVtTCykGFvoH81939939 = -735714739;    double nNFmyYVtTCykGFvoH72001829 = -303140578;    double nNFmyYVtTCykGFvoH16552918 = -781492604;    double nNFmyYVtTCykGFvoH74891531 = -669892997;    double nNFmyYVtTCykGFvoH35771209 = -923354496;    double nNFmyYVtTCykGFvoH72697416 = -542415969;    double nNFmyYVtTCykGFvoH63550836 = -240045273;    double nNFmyYVtTCykGFvoH57827342 = -447660180;    double nNFmyYVtTCykGFvoH89410748 = -801448181;    double nNFmyYVtTCykGFvoH87396380 = -135029600;    double nNFmyYVtTCykGFvoH13932826 = 74775781;    double nNFmyYVtTCykGFvoH87655049 = -649292983;    double nNFmyYVtTCykGFvoH23192454 = -932170141;    double nNFmyYVtTCykGFvoH70688059 = -73153332;    double nNFmyYVtTCykGFvoH57919177 = -609688282;    double nNFmyYVtTCykGFvoH56945981 = -987274162;    double nNFmyYVtTCykGFvoH37263240 = -622165456;    double nNFmyYVtTCykGFvoH81477655 = -742882069;    double nNFmyYVtTCykGFvoH38077304 = -449834131;    double nNFmyYVtTCykGFvoH83836998 = -260861231;    double nNFmyYVtTCykGFvoH63777724 = -216308982;    double nNFmyYVtTCykGFvoH3972743 = -836728645;    double nNFmyYVtTCykGFvoH28474645 = -88085565;    double nNFmyYVtTCykGFvoH96524653 = -748827624;    double nNFmyYVtTCykGFvoH56087658 = -131420404;    double nNFmyYVtTCykGFvoH14114794 = -447876416;    double nNFmyYVtTCykGFvoH99591094 = -991021867;    double nNFmyYVtTCykGFvoH2944429 = -392247959;    double nNFmyYVtTCykGFvoH17488072 = -694412060;    double nNFmyYVtTCykGFvoH70315999 = -388418451;    double nNFmyYVtTCykGFvoH48412227 = -195678785;    double nNFmyYVtTCykGFvoH19638375 = -167931165;    double nNFmyYVtTCykGFvoH90138410 = -628211394;    double nNFmyYVtTCykGFvoH58602048 = -719570127;    double nNFmyYVtTCykGFvoH62910701 = -884289318;    double nNFmyYVtTCykGFvoH11242290 = -748950576;    double nNFmyYVtTCykGFvoH63390596 = -427063692;    double nNFmyYVtTCykGFvoH77625460 = -186638632;     nNFmyYVtTCykGFvoH75769088 = nNFmyYVtTCykGFvoH94358635;     nNFmyYVtTCykGFvoH94358635 = nNFmyYVtTCykGFvoH28279801;     nNFmyYVtTCykGFvoH28279801 = nNFmyYVtTCykGFvoH49290969;     nNFmyYVtTCykGFvoH49290969 = nNFmyYVtTCykGFvoH37142039;     nNFmyYVtTCykGFvoH37142039 = nNFmyYVtTCykGFvoH33111997;     nNFmyYVtTCykGFvoH33111997 = nNFmyYVtTCykGFvoH27794187;     nNFmyYVtTCykGFvoH27794187 = nNFmyYVtTCykGFvoH99035865;     nNFmyYVtTCykGFvoH99035865 = nNFmyYVtTCykGFvoH4702772;     nNFmyYVtTCykGFvoH4702772 = nNFmyYVtTCykGFvoH9673136;     nNFmyYVtTCykGFvoH9673136 = nNFmyYVtTCykGFvoH40176810;     nNFmyYVtTCykGFvoH40176810 = nNFmyYVtTCykGFvoH1019719;     nNFmyYVtTCykGFvoH1019719 = nNFmyYVtTCykGFvoH70618236;     nNFmyYVtTCykGFvoH70618236 = nNFmyYVtTCykGFvoH1101057;     nNFmyYVtTCykGFvoH1101057 = nNFmyYVtTCykGFvoH32784553;     nNFmyYVtTCykGFvoH32784553 = nNFmyYVtTCykGFvoH98226316;     nNFmyYVtTCykGFvoH98226316 = nNFmyYVtTCykGFvoH93908769;     nNFmyYVtTCykGFvoH93908769 = nNFmyYVtTCykGFvoH68237067;     nNFmyYVtTCykGFvoH68237067 = nNFmyYVtTCykGFvoH69095080;     nNFmyYVtTCykGFvoH69095080 = nNFmyYVtTCykGFvoH22485181;     nNFmyYVtTCykGFvoH22485181 = nNFmyYVtTCykGFvoH15538700;     nNFmyYVtTCykGFvoH15538700 = nNFmyYVtTCykGFvoH41163296;     nNFmyYVtTCykGFvoH41163296 = nNFmyYVtTCykGFvoH36889496;     nNFmyYVtTCykGFvoH36889496 = nNFmyYVtTCykGFvoH89780232;     nNFmyYVtTCykGFvoH89780232 = nNFmyYVtTCykGFvoH91190807;     nNFmyYVtTCykGFvoH91190807 = nNFmyYVtTCykGFvoH2476173;     nNFmyYVtTCykGFvoH2476173 = nNFmyYVtTCykGFvoH41810377;     nNFmyYVtTCykGFvoH41810377 = nNFmyYVtTCykGFvoH87103508;     nNFmyYVtTCykGFvoH87103508 = nNFmyYVtTCykGFvoH17095927;     nNFmyYVtTCykGFvoH17095927 = nNFmyYVtTCykGFvoH32700943;     nNFmyYVtTCykGFvoH32700943 = nNFmyYVtTCykGFvoH93120217;     nNFmyYVtTCykGFvoH93120217 = nNFmyYVtTCykGFvoH65285279;     nNFmyYVtTCykGFvoH65285279 = nNFmyYVtTCykGFvoH65248510;     nNFmyYVtTCykGFvoH65248510 = nNFmyYVtTCykGFvoH97920819;     nNFmyYVtTCykGFvoH97920819 = nNFmyYVtTCykGFvoH37550220;     nNFmyYVtTCykGFvoH37550220 = nNFmyYVtTCykGFvoH74957211;     nNFmyYVtTCykGFvoH74957211 = nNFmyYVtTCykGFvoH8815568;     nNFmyYVtTCykGFvoH8815568 = nNFmyYVtTCykGFvoH6512389;     nNFmyYVtTCykGFvoH6512389 = nNFmyYVtTCykGFvoH54304242;     nNFmyYVtTCykGFvoH54304242 = nNFmyYVtTCykGFvoH81440030;     nNFmyYVtTCykGFvoH81440030 = nNFmyYVtTCykGFvoH99292726;     nNFmyYVtTCykGFvoH99292726 = nNFmyYVtTCykGFvoH44850641;     nNFmyYVtTCykGFvoH44850641 = nNFmyYVtTCykGFvoH83244119;     nNFmyYVtTCykGFvoH83244119 = nNFmyYVtTCykGFvoH79943514;     nNFmyYVtTCykGFvoH79943514 = nNFmyYVtTCykGFvoH52516992;     nNFmyYVtTCykGFvoH52516992 = nNFmyYVtTCykGFvoH9713152;     nNFmyYVtTCykGFvoH9713152 = nNFmyYVtTCykGFvoH64398868;     nNFmyYVtTCykGFvoH64398868 = nNFmyYVtTCykGFvoH57973378;     nNFmyYVtTCykGFvoH57973378 = nNFmyYVtTCykGFvoH23325785;     nNFmyYVtTCykGFvoH23325785 = nNFmyYVtTCykGFvoH13123185;     nNFmyYVtTCykGFvoH13123185 = nNFmyYVtTCykGFvoH4226298;     nNFmyYVtTCykGFvoH4226298 = nNFmyYVtTCykGFvoH96595564;     nNFmyYVtTCykGFvoH96595564 = nNFmyYVtTCykGFvoH9197621;     nNFmyYVtTCykGFvoH9197621 = nNFmyYVtTCykGFvoH51133716;     nNFmyYVtTCykGFvoH51133716 = nNFmyYVtTCykGFvoH98329725;     nNFmyYVtTCykGFvoH98329725 = nNFmyYVtTCykGFvoH34605792;     nNFmyYVtTCykGFvoH34605792 = nNFmyYVtTCykGFvoH57469140;     nNFmyYVtTCykGFvoH57469140 = nNFmyYVtTCykGFvoH38499569;     nNFmyYVtTCykGFvoH38499569 = nNFmyYVtTCykGFvoH58100162;     nNFmyYVtTCykGFvoH58100162 = nNFmyYVtTCykGFvoH34665867;     nNFmyYVtTCykGFvoH34665867 = nNFmyYVtTCykGFvoH91301620;     nNFmyYVtTCykGFvoH91301620 = nNFmyYVtTCykGFvoH40690679;     nNFmyYVtTCykGFvoH40690679 = nNFmyYVtTCykGFvoH81939939;     nNFmyYVtTCykGFvoH81939939 = nNFmyYVtTCykGFvoH72001829;     nNFmyYVtTCykGFvoH72001829 = nNFmyYVtTCykGFvoH16552918;     nNFmyYVtTCykGFvoH16552918 = nNFmyYVtTCykGFvoH74891531;     nNFmyYVtTCykGFvoH74891531 = nNFmyYVtTCykGFvoH35771209;     nNFmyYVtTCykGFvoH35771209 = nNFmyYVtTCykGFvoH72697416;     nNFmyYVtTCykGFvoH72697416 = nNFmyYVtTCykGFvoH63550836;     nNFmyYVtTCykGFvoH63550836 = nNFmyYVtTCykGFvoH57827342;     nNFmyYVtTCykGFvoH57827342 = nNFmyYVtTCykGFvoH89410748;     nNFmyYVtTCykGFvoH89410748 = nNFmyYVtTCykGFvoH87396380;     nNFmyYVtTCykGFvoH87396380 = nNFmyYVtTCykGFvoH13932826;     nNFmyYVtTCykGFvoH13932826 = nNFmyYVtTCykGFvoH87655049;     nNFmyYVtTCykGFvoH87655049 = nNFmyYVtTCykGFvoH23192454;     nNFmyYVtTCykGFvoH23192454 = nNFmyYVtTCykGFvoH70688059;     nNFmyYVtTCykGFvoH70688059 = nNFmyYVtTCykGFvoH57919177;     nNFmyYVtTCykGFvoH57919177 = nNFmyYVtTCykGFvoH56945981;     nNFmyYVtTCykGFvoH56945981 = nNFmyYVtTCykGFvoH37263240;     nNFmyYVtTCykGFvoH37263240 = nNFmyYVtTCykGFvoH81477655;     nNFmyYVtTCykGFvoH81477655 = nNFmyYVtTCykGFvoH38077304;     nNFmyYVtTCykGFvoH38077304 = nNFmyYVtTCykGFvoH83836998;     nNFmyYVtTCykGFvoH83836998 = nNFmyYVtTCykGFvoH63777724;     nNFmyYVtTCykGFvoH63777724 = nNFmyYVtTCykGFvoH3972743;     nNFmyYVtTCykGFvoH3972743 = nNFmyYVtTCykGFvoH28474645;     nNFmyYVtTCykGFvoH28474645 = nNFmyYVtTCykGFvoH96524653;     nNFmyYVtTCykGFvoH96524653 = nNFmyYVtTCykGFvoH56087658;     nNFmyYVtTCykGFvoH56087658 = nNFmyYVtTCykGFvoH14114794;     nNFmyYVtTCykGFvoH14114794 = nNFmyYVtTCykGFvoH99591094;     nNFmyYVtTCykGFvoH99591094 = nNFmyYVtTCykGFvoH2944429;     nNFmyYVtTCykGFvoH2944429 = nNFmyYVtTCykGFvoH17488072;     nNFmyYVtTCykGFvoH17488072 = nNFmyYVtTCykGFvoH70315999;     nNFmyYVtTCykGFvoH70315999 = nNFmyYVtTCykGFvoH48412227;     nNFmyYVtTCykGFvoH48412227 = nNFmyYVtTCykGFvoH19638375;     nNFmyYVtTCykGFvoH19638375 = nNFmyYVtTCykGFvoH90138410;     nNFmyYVtTCykGFvoH90138410 = nNFmyYVtTCykGFvoH58602048;     nNFmyYVtTCykGFvoH58602048 = nNFmyYVtTCykGFvoH62910701;     nNFmyYVtTCykGFvoH62910701 = nNFmyYVtTCykGFvoH11242290;     nNFmyYVtTCykGFvoH11242290 = nNFmyYVtTCykGFvoH63390596;     nNFmyYVtTCykGFvoH63390596 = nNFmyYVtTCykGFvoH77625460;     nNFmyYVtTCykGFvoH77625460 = nNFmyYVtTCykGFvoH75769088;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pJrMAEAGkKGqHccc28917493() {     double LRPfEBBxcmXZuVKYM81886195 = -298540521;    double LRPfEBBxcmXZuVKYM37731915 = -800883808;    double LRPfEBBxcmXZuVKYM44321985 = -112184959;    double LRPfEBBxcmXZuVKYM76365750 = -470265767;    double LRPfEBBxcmXZuVKYM6037528 = -159813082;    double LRPfEBBxcmXZuVKYM24529200 = 20632855;    double LRPfEBBxcmXZuVKYM88283153 = -229674846;    double LRPfEBBxcmXZuVKYM48795757 = 50094376;    double LRPfEBBxcmXZuVKYM69345267 = -460678781;    double LRPfEBBxcmXZuVKYM50902795 = 75082852;    double LRPfEBBxcmXZuVKYM287101 = -33884823;    double LRPfEBBxcmXZuVKYM32842762 = -338651463;    double LRPfEBBxcmXZuVKYM56827354 = -326291475;    double LRPfEBBxcmXZuVKYM84244733 = -689390996;    double LRPfEBBxcmXZuVKYM28526980 = -627987976;    double LRPfEBBxcmXZuVKYM39731711 = -890888703;    double LRPfEBBxcmXZuVKYM87744772 = -770502612;    double LRPfEBBxcmXZuVKYM65290220 = -907883483;    double LRPfEBBxcmXZuVKYM30924922 = -926970073;    double LRPfEBBxcmXZuVKYM3035346 = -36592009;    double LRPfEBBxcmXZuVKYM4091893 = -648644844;    double LRPfEBBxcmXZuVKYM27684320 = -669709152;    double LRPfEBBxcmXZuVKYM72480287 = 57652056;    double LRPfEBBxcmXZuVKYM73772606 = -555634498;    double LRPfEBBxcmXZuVKYM34890457 = -929464056;    double LRPfEBBxcmXZuVKYM27197848 = -668613077;    double LRPfEBBxcmXZuVKYM40270155 = -471836514;    double LRPfEBBxcmXZuVKYM210634 = -28690000;    double LRPfEBBxcmXZuVKYM83228182 = -149699677;    double LRPfEBBxcmXZuVKYM1316580 = -106976405;    double LRPfEBBxcmXZuVKYM85362767 = -827770005;    double LRPfEBBxcmXZuVKYM62895900 = -196001056;    double LRPfEBBxcmXZuVKYM74959458 = -963504136;    double LRPfEBBxcmXZuVKYM92751383 = -95526184;    double LRPfEBBxcmXZuVKYM99024872 = -455840262;    double LRPfEBBxcmXZuVKYM43683124 = -703891738;    double LRPfEBBxcmXZuVKYM90077502 = 29923151;    double LRPfEBBxcmXZuVKYM40602601 = -790433909;    double LRPfEBBxcmXZuVKYM27914496 = -111060119;    double LRPfEBBxcmXZuVKYM68941968 = -245347216;    double LRPfEBBxcmXZuVKYM4961951 = -912792843;    double LRPfEBBxcmXZuVKYM77001097 = -890146334;    double LRPfEBBxcmXZuVKYM61311097 = -132895737;    double LRPfEBBxcmXZuVKYM67351254 = -227899233;    double LRPfEBBxcmXZuVKYM64469457 = -373062209;    double LRPfEBBxcmXZuVKYM21652347 = -798602070;    double LRPfEBBxcmXZuVKYM82177442 = -358899161;    double LRPfEBBxcmXZuVKYM65930319 = -220208613;    double LRPfEBBxcmXZuVKYM33001513 = -832189054;    double LRPfEBBxcmXZuVKYM26034837 = -177599136;    double LRPfEBBxcmXZuVKYM39926976 = 56286356;    double LRPfEBBxcmXZuVKYM7405285 = -966347764;    double LRPfEBBxcmXZuVKYM57446078 = -46923424;    double LRPfEBBxcmXZuVKYM47102596 = -843484495;    double LRPfEBBxcmXZuVKYM24825600 = 46505666;    double LRPfEBBxcmXZuVKYM54201875 = -628831370;    double LRPfEBBxcmXZuVKYM65251627 = -758535864;    double LRPfEBBxcmXZuVKYM70549378 = -556550462;    double LRPfEBBxcmXZuVKYM41475293 = -540801712;    double LRPfEBBxcmXZuVKYM78839680 = -491200005;    double LRPfEBBxcmXZuVKYM84259044 = -507530632;    double LRPfEBBxcmXZuVKYM88072519 = -100984846;    double LRPfEBBxcmXZuVKYM65567574 = -800205947;    double LRPfEBBxcmXZuVKYM68028688 = -253702377;    double LRPfEBBxcmXZuVKYM65540027 = -97147143;    double LRPfEBBxcmXZuVKYM37391200 = -837883768;    double LRPfEBBxcmXZuVKYM57883303 = -375147328;    double LRPfEBBxcmXZuVKYM64075970 = -130765292;    double LRPfEBBxcmXZuVKYM85219861 = -133550735;    double LRPfEBBxcmXZuVKYM84843855 = -924096239;    double LRPfEBBxcmXZuVKYM49654208 = -820811855;    double LRPfEBBxcmXZuVKYM47142171 = -980068704;    double LRPfEBBxcmXZuVKYM37375725 = -696823364;    double LRPfEBBxcmXZuVKYM61982953 = -581622857;    double LRPfEBBxcmXZuVKYM98073394 = -123799166;    double LRPfEBBxcmXZuVKYM27090796 = -758498510;    double LRPfEBBxcmXZuVKYM66373222 = -436813415;    double LRPfEBBxcmXZuVKYM5129033 = -714448711;    double LRPfEBBxcmXZuVKYM9303150 = -82572289;    double LRPfEBBxcmXZuVKYM13238111 = -30861986;    double LRPfEBBxcmXZuVKYM45020405 = -209713916;    double LRPfEBBxcmXZuVKYM74339835 = -151627902;    double LRPfEBBxcmXZuVKYM67209120 = -196500947;    double LRPfEBBxcmXZuVKYM57193345 = -972100541;    double LRPfEBBxcmXZuVKYM61389603 = -63262761;    double LRPfEBBxcmXZuVKYM77957483 = -861422242;    double LRPfEBBxcmXZuVKYM5449823 = -49077632;    double LRPfEBBxcmXZuVKYM27856862 = -20019641;    double LRPfEBBxcmXZuVKYM67925783 = -42031850;    double LRPfEBBxcmXZuVKYM44822997 = -827008892;    double LRPfEBBxcmXZuVKYM78431497 = -945355874;    double LRPfEBBxcmXZuVKYM19528124 = -413526388;    double LRPfEBBxcmXZuVKYM99127308 = -149632197;    double LRPfEBBxcmXZuVKYM49074816 = -619860114;    double LRPfEBBxcmXZuVKYM84682923 = -737816585;    double LRPfEBBxcmXZuVKYM16889432 = -711807998;    double LRPfEBBxcmXZuVKYM11433524 = 10059612;    double LRPfEBBxcmXZuVKYM93282409 = -879193361;    double LRPfEBBxcmXZuVKYM1811227 = -30752091;    double LRPfEBBxcmXZuVKYM27078257 = -298540521;     LRPfEBBxcmXZuVKYM81886195 = LRPfEBBxcmXZuVKYM37731915;     LRPfEBBxcmXZuVKYM37731915 = LRPfEBBxcmXZuVKYM44321985;     LRPfEBBxcmXZuVKYM44321985 = LRPfEBBxcmXZuVKYM76365750;     LRPfEBBxcmXZuVKYM76365750 = LRPfEBBxcmXZuVKYM6037528;     LRPfEBBxcmXZuVKYM6037528 = LRPfEBBxcmXZuVKYM24529200;     LRPfEBBxcmXZuVKYM24529200 = LRPfEBBxcmXZuVKYM88283153;     LRPfEBBxcmXZuVKYM88283153 = LRPfEBBxcmXZuVKYM48795757;     LRPfEBBxcmXZuVKYM48795757 = LRPfEBBxcmXZuVKYM69345267;     LRPfEBBxcmXZuVKYM69345267 = LRPfEBBxcmXZuVKYM50902795;     LRPfEBBxcmXZuVKYM50902795 = LRPfEBBxcmXZuVKYM287101;     LRPfEBBxcmXZuVKYM287101 = LRPfEBBxcmXZuVKYM32842762;     LRPfEBBxcmXZuVKYM32842762 = LRPfEBBxcmXZuVKYM56827354;     LRPfEBBxcmXZuVKYM56827354 = LRPfEBBxcmXZuVKYM84244733;     LRPfEBBxcmXZuVKYM84244733 = LRPfEBBxcmXZuVKYM28526980;     LRPfEBBxcmXZuVKYM28526980 = LRPfEBBxcmXZuVKYM39731711;     LRPfEBBxcmXZuVKYM39731711 = LRPfEBBxcmXZuVKYM87744772;     LRPfEBBxcmXZuVKYM87744772 = LRPfEBBxcmXZuVKYM65290220;     LRPfEBBxcmXZuVKYM65290220 = LRPfEBBxcmXZuVKYM30924922;     LRPfEBBxcmXZuVKYM30924922 = LRPfEBBxcmXZuVKYM3035346;     LRPfEBBxcmXZuVKYM3035346 = LRPfEBBxcmXZuVKYM4091893;     LRPfEBBxcmXZuVKYM4091893 = LRPfEBBxcmXZuVKYM27684320;     LRPfEBBxcmXZuVKYM27684320 = LRPfEBBxcmXZuVKYM72480287;     LRPfEBBxcmXZuVKYM72480287 = LRPfEBBxcmXZuVKYM73772606;     LRPfEBBxcmXZuVKYM73772606 = LRPfEBBxcmXZuVKYM34890457;     LRPfEBBxcmXZuVKYM34890457 = LRPfEBBxcmXZuVKYM27197848;     LRPfEBBxcmXZuVKYM27197848 = LRPfEBBxcmXZuVKYM40270155;     LRPfEBBxcmXZuVKYM40270155 = LRPfEBBxcmXZuVKYM210634;     LRPfEBBxcmXZuVKYM210634 = LRPfEBBxcmXZuVKYM83228182;     LRPfEBBxcmXZuVKYM83228182 = LRPfEBBxcmXZuVKYM1316580;     LRPfEBBxcmXZuVKYM1316580 = LRPfEBBxcmXZuVKYM85362767;     LRPfEBBxcmXZuVKYM85362767 = LRPfEBBxcmXZuVKYM62895900;     LRPfEBBxcmXZuVKYM62895900 = LRPfEBBxcmXZuVKYM74959458;     LRPfEBBxcmXZuVKYM74959458 = LRPfEBBxcmXZuVKYM92751383;     LRPfEBBxcmXZuVKYM92751383 = LRPfEBBxcmXZuVKYM99024872;     LRPfEBBxcmXZuVKYM99024872 = LRPfEBBxcmXZuVKYM43683124;     LRPfEBBxcmXZuVKYM43683124 = LRPfEBBxcmXZuVKYM90077502;     LRPfEBBxcmXZuVKYM90077502 = LRPfEBBxcmXZuVKYM40602601;     LRPfEBBxcmXZuVKYM40602601 = LRPfEBBxcmXZuVKYM27914496;     LRPfEBBxcmXZuVKYM27914496 = LRPfEBBxcmXZuVKYM68941968;     LRPfEBBxcmXZuVKYM68941968 = LRPfEBBxcmXZuVKYM4961951;     LRPfEBBxcmXZuVKYM4961951 = LRPfEBBxcmXZuVKYM77001097;     LRPfEBBxcmXZuVKYM77001097 = LRPfEBBxcmXZuVKYM61311097;     LRPfEBBxcmXZuVKYM61311097 = LRPfEBBxcmXZuVKYM67351254;     LRPfEBBxcmXZuVKYM67351254 = LRPfEBBxcmXZuVKYM64469457;     LRPfEBBxcmXZuVKYM64469457 = LRPfEBBxcmXZuVKYM21652347;     LRPfEBBxcmXZuVKYM21652347 = LRPfEBBxcmXZuVKYM82177442;     LRPfEBBxcmXZuVKYM82177442 = LRPfEBBxcmXZuVKYM65930319;     LRPfEBBxcmXZuVKYM65930319 = LRPfEBBxcmXZuVKYM33001513;     LRPfEBBxcmXZuVKYM33001513 = LRPfEBBxcmXZuVKYM26034837;     LRPfEBBxcmXZuVKYM26034837 = LRPfEBBxcmXZuVKYM39926976;     LRPfEBBxcmXZuVKYM39926976 = LRPfEBBxcmXZuVKYM7405285;     LRPfEBBxcmXZuVKYM7405285 = LRPfEBBxcmXZuVKYM57446078;     LRPfEBBxcmXZuVKYM57446078 = LRPfEBBxcmXZuVKYM47102596;     LRPfEBBxcmXZuVKYM47102596 = LRPfEBBxcmXZuVKYM24825600;     LRPfEBBxcmXZuVKYM24825600 = LRPfEBBxcmXZuVKYM54201875;     LRPfEBBxcmXZuVKYM54201875 = LRPfEBBxcmXZuVKYM65251627;     LRPfEBBxcmXZuVKYM65251627 = LRPfEBBxcmXZuVKYM70549378;     LRPfEBBxcmXZuVKYM70549378 = LRPfEBBxcmXZuVKYM41475293;     LRPfEBBxcmXZuVKYM41475293 = LRPfEBBxcmXZuVKYM78839680;     LRPfEBBxcmXZuVKYM78839680 = LRPfEBBxcmXZuVKYM84259044;     LRPfEBBxcmXZuVKYM84259044 = LRPfEBBxcmXZuVKYM88072519;     LRPfEBBxcmXZuVKYM88072519 = LRPfEBBxcmXZuVKYM65567574;     LRPfEBBxcmXZuVKYM65567574 = LRPfEBBxcmXZuVKYM68028688;     LRPfEBBxcmXZuVKYM68028688 = LRPfEBBxcmXZuVKYM65540027;     LRPfEBBxcmXZuVKYM65540027 = LRPfEBBxcmXZuVKYM37391200;     LRPfEBBxcmXZuVKYM37391200 = LRPfEBBxcmXZuVKYM57883303;     LRPfEBBxcmXZuVKYM57883303 = LRPfEBBxcmXZuVKYM64075970;     LRPfEBBxcmXZuVKYM64075970 = LRPfEBBxcmXZuVKYM85219861;     LRPfEBBxcmXZuVKYM85219861 = LRPfEBBxcmXZuVKYM84843855;     LRPfEBBxcmXZuVKYM84843855 = LRPfEBBxcmXZuVKYM49654208;     LRPfEBBxcmXZuVKYM49654208 = LRPfEBBxcmXZuVKYM47142171;     LRPfEBBxcmXZuVKYM47142171 = LRPfEBBxcmXZuVKYM37375725;     LRPfEBBxcmXZuVKYM37375725 = LRPfEBBxcmXZuVKYM61982953;     LRPfEBBxcmXZuVKYM61982953 = LRPfEBBxcmXZuVKYM98073394;     LRPfEBBxcmXZuVKYM98073394 = LRPfEBBxcmXZuVKYM27090796;     LRPfEBBxcmXZuVKYM27090796 = LRPfEBBxcmXZuVKYM66373222;     LRPfEBBxcmXZuVKYM66373222 = LRPfEBBxcmXZuVKYM5129033;     LRPfEBBxcmXZuVKYM5129033 = LRPfEBBxcmXZuVKYM9303150;     LRPfEBBxcmXZuVKYM9303150 = LRPfEBBxcmXZuVKYM13238111;     LRPfEBBxcmXZuVKYM13238111 = LRPfEBBxcmXZuVKYM45020405;     LRPfEBBxcmXZuVKYM45020405 = LRPfEBBxcmXZuVKYM74339835;     LRPfEBBxcmXZuVKYM74339835 = LRPfEBBxcmXZuVKYM67209120;     LRPfEBBxcmXZuVKYM67209120 = LRPfEBBxcmXZuVKYM57193345;     LRPfEBBxcmXZuVKYM57193345 = LRPfEBBxcmXZuVKYM61389603;     LRPfEBBxcmXZuVKYM61389603 = LRPfEBBxcmXZuVKYM77957483;     LRPfEBBxcmXZuVKYM77957483 = LRPfEBBxcmXZuVKYM5449823;     LRPfEBBxcmXZuVKYM5449823 = LRPfEBBxcmXZuVKYM27856862;     LRPfEBBxcmXZuVKYM27856862 = LRPfEBBxcmXZuVKYM67925783;     LRPfEBBxcmXZuVKYM67925783 = LRPfEBBxcmXZuVKYM44822997;     LRPfEBBxcmXZuVKYM44822997 = LRPfEBBxcmXZuVKYM78431497;     LRPfEBBxcmXZuVKYM78431497 = LRPfEBBxcmXZuVKYM19528124;     LRPfEBBxcmXZuVKYM19528124 = LRPfEBBxcmXZuVKYM99127308;     LRPfEBBxcmXZuVKYM99127308 = LRPfEBBxcmXZuVKYM49074816;     LRPfEBBxcmXZuVKYM49074816 = LRPfEBBxcmXZuVKYM84682923;     LRPfEBBxcmXZuVKYM84682923 = LRPfEBBxcmXZuVKYM16889432;     LRPfEBBxcmXZuVKYM16889432 = LRPfEBBxcmXZuVKYM11433524;     LRPfEBBxcmXZuVKYM11433524 = LRPfEBBxcmXZuVKYM93282409;     LRPfEBBxcmXZuVKYM93282409 = LRPfEBBxcmXZuVKYM1811227;     LRPfEBBxcmXZuVKYM1811227 = LRPfEBBxcmXZuVKYM27078257;     LRPfEBBxcmXZuVKYM27078257 = LRPfEBBxcmXZuVKYM81886195;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void duqJTbuQlLCaxbwL96209092() {     double SzjoUqfAcSXdJiPJc17345164 = -764817421;    double SzjoUqfAcSXdJiPJc51003993 = -681435414;    double SzjoUqfAcSXdJiPJc81708461 = -877852034;    double SzjoUqfAcSXdJiPJc48589105 = -291416839;    double SzjoUqfAcSXdJiPJc62322797 = -265411927;    double SzjoUqfAcSXdJiPJc60752071 = -241715040;    double SzjoUqfAcSXdJiPJc98940559 = -74400580;    double SzjoUqfAcSXdJiPJc10927318 = -694139764;    double SzjoUqfAcSXdJiPJc70862872 = -949010188;    double SzjoUqfAcSXdJiPJc65805507 = -689604146;    double SzjoUqfAcSXdJiPJc38468835 = -935892553;    double SzjoUqfAcSXdJiPJc33410910 = -202772235;    double SzjoUqfAcSXdJiPJc95061999 = -878303980;    double SzjoUqfAcSXdJiPJc37385596 = -665754406;    double SzjoUqfAcSXdJiPJc86721496 = -31412991;    double SzjoUqfAcSXdJiPJc23273012 = -526700310;    double SzjoUqfAcSXdJiPJc43580708 = -197477165;    double SzjoUqfAcSXdJiPJc65050412 = -981311551;    double SzjoUqfAcSXdJiPJc31475252 = -143533494;    double SzjoUqfAcSXdJiPJc96872563 = -463153075;    double SzjoUqfAcSXdJiPJc17178756 = -305919532;    double SzjoUqfAcSXdJiPJc31105845 = -756345446;    double SzjoUqfAcSXdJiPJc43005667 = -679980128;    double SzjoUqfAcSXdJiPJc67565551 = 33540898;    double SzjoUqfAcSXdJiPJc42908408 = -953449254;    double SzjoUqfAcSXdJiPJc37896020 = -357189884;    double SzjoUqfAcSXdJiPJc690730 = -278120654;    double SzjoUqfAcSXdJiPJc40939059 = -198462175;    double SzjoUqfAcSXdJiPJc29896397 = -723794479;    double SzjoUqfAcSXdJiPJc6849991 = -141483575;    double SzjoUqfAcSXdJiPJc3945999 = -279750492;    double SzjoUqfAcSXdJiPJc718919 = 31027530;    double SzjoUqfAcSXdJiPJc87177469 = -11100113;    double SzjoUqfAcSXdJiPJc22499811 = -160219997;    double SzjoUqfAcSXdJiPJc64833326 = -406940512;    double SzjoUqfAcSXdJiPJc95243968 = -918457043;    double SzjoUqfAcSXdJiPJc39489319 = -934772889;    double SzjoUqfAcSXdJiPJc91818004 = -773089490;    double SzjoUqfAcSXdJiPJc58305057 = -861936395;    double SzjoUqfAcSXdJiPJc33213501 = -810609762;    double SzjoUqfAcSXdJiPJc38413559 = -681991757;    double SzjoUqfAcSXdJiPJc66366224 = -484691625;    double SzjoUqfAcSXdJiPJc50694476 = -786076523;    double SzjoUqfAcSXdJiPJc7482593 = -832231643;    double SzjoUqfAcSXdJiPJc64485915 = -199573371;    double SzjoUqfAcSXdJiPJc67613675 = -837525702;    double SzjoUqfAcSXdJiPJc1522607 = -842885522;    double SzjoUqfAcSXdJiPJc83656631 = -386735206;    double SzjoUqfAcSXdJiPJc55894179 = -495925934;    double SzjoUqfAcSXdJiPJc50564426 = -668625952;    double SzjoUqfAcSXdJiPJc20697700 = -917990202;    double SzjoUqfAcSXdJiPJc90890756 = -80000110;    double SzjoUqfAcSXdJiPJc26067227 = -902968978;    double SzjoUqfAcSXdJiPJc63094443 = 65007005;    double SzjoUqfAcSXdJiPJc79030336 = -299984480;    double SzjoUqfAcSXdJiPJc86239318 = 91528024;    double SzjoUqfAcSXdJiPJc7998326 = 98544713;    double SzjoUqfAcSXdJiPJc14142911 = -811392933;    double SzjoUqfAcSXdJiPJc5680697 = -337967585;    double SzjoUqfAcSXdJiPJc24426778 = -908222044;    double SzjoUqfAcSXdJiPJc60061342 = -963594387;    double SzjoUqfAcSXdJiPJc58001501 = -875938406;    double SzjoUqfAcSXdJiPJc81030920 = -970345285;    double SzjoUqfAcSXdJiPJc64012882 = -707526613;    double SzjoUqfAcSXdJiPJc61859508 = -309853654;    double SzjoUqfAcSXdJiPJc37749916 = -866920083;    double SzjoUqfAcSXdJiPJc46233440 = -91672122;    double SzjoUqfAcSXdJiPJc72562189 = -618083984;    double SzjoUqfAcSXdJiPJc72552270 = -158813894;    double SzjoUqfAcSXdJiPJc91477528 = -112955948;    double SzjoUqfAcSXdJiPJc83783692 = -591927422;    double SzjoUqfAcSXdJiPJc51762703 = -424387675;    double SzjoUqfAcSXdJiPJc6745355 = -19375157;    double SzjoUqfAcSXdJiPJc98261750 = -332923733;    double SzjoUqfAcSXdJiPJc58459005 = -781161318;    double SzjoUqfAcSXdJiPJc50812532 = -821227907;    double SzjoUqfAcSXdJiPJc80411369 = -970268923;    double SzjoUqfAcSXdJiPJc35523075 = -847748485;    double SzjoUqfAcSXdJiPJc3079636 = -766885732;    double SzjoUqfAcSXdJiPJc75294733 = -15923552;    double SzjoUqfAcSXdJiPJc36373413 = -514304363;    double SzjoUqfAcSXdJiPJc17034098 = -891385449;    double SzjoUqfAcSXdJiPJc85044879 = -702536241;    double SzjoUqfAcSXdJiPJc79331971 = 44831472;    double SzjoUqfAcSXdJiPJc86152290 = -223493374;    double SzjoUqfAcSXdJiPJc13055243 = -99750383;    double SzjoUqfAcSXdJiPJc74651692 = -66003492;    double SzjoUqfAcSXdJiPJc24083027 = 23892881;    double SzjoUqfAcSXdJiPJc43469474 = -860235518;    double SzjoUqfAcSXdJiPJc78594007 = -398468537;    double SzjoUqfAcSXdJiPJc87245642 = -917001757;    double SzjoUqfAcSXdJiPJc25346409 = -23379957;    double SzjoUqfAcSXdJiPJc86137308 = -335121905;    double SzjoUqfAcSXdJiPJc33878280 = -953714352;    double SzjoUqfAcSXdJiPJc73152158 = -847015376;    double SzjoUqfAcSXdJiPJc80412057 = -806053352;    double SzjoUqfAcSXdJiPJc85335303 = -514346341;    double SzjoUqfAcSXdJiPJc86681593 = 21450089;    double SzjoUqfAcSXdJiPJc45623085 = -422377990;    double SzjoUqfAcSXdJiPJc26735999 = -764817421;     SzjoUqfAcSXdJiPJc17345164 = SzjoUqfAcSXdJiPJc51003993;     SzjoUqfAcSXdJiPJc51003993 = SzjoUqfAcSXdJiPJc81708461;     SzjoUqfAcSXdJiPJc81708461 = SzjoUqfAcSXdJiPJc48589105;     SzjoUqfAcSXdJiPJc48589105 = SzjoUqfAcSXdJiPJc62322797;     SzjoUqfAcSXdJiPJc62322797 = SzjoUqfAcSXdJiPJc60752071;     SzjoUqfAcSXdJiPJc60752071 = SzjoUqfAcSXdJiPJc98940559;     SzjoUqfAcSXdJiPJc98940559 = SzjoUqfAcSXdJiPJc10927318;     SzjoUqfAcSXdJiPJc10927318 = SzjoUqfAcSXdJiPJc70862872;     SzjoUqfAcSXdJiPJc70862872 = SzjoUqfAcSXdJiPJc65805507;     SzjoUqfAcSXdJiPJc65805507 = SzjoUqfAcSXdJiPJc38468835;     SzjoUqfAcSXdJiPJc38468835 = SzjoUqfAcSXdJiPJc33410910;     SzjoUqfAcSXdJiPJc33410910 = SzjoUqfAcSXdJiPJc95061999;     SzjoUqfAcSXdJiPJc95061999 = SzjoUqfAcSXdJiPJc37385596;     SzjoUqfAcSXdJiPJc37385596 = SzjoUqfAcSXdJiPJc86721496;     SzjoUqfAcSXdJiPJc86721496 = SzjoUqfAcSXdJiPJc23273012;     SzjoUqfAcSXdJiPJc23273012 = SzjoUqfAcSXdJiPJc43580708;     SzjoUqfAcSXdJiPJc43580708 = SzjoUqfAcSXdJiPJc65050412;     SzjoUqfAcSXdJiPJc65050412 = SzjoUqfAcSXdJiPJc31475252;     SzjoUqfAcSXdJiPJc31475252 = SzjoUqfAcSXdJiPJc96872563;     SzjoUqfAcSXdJiPJc96872563 = SzjoUqfAcSXdJiPJc17178756;     SzjoUqfAcSXdJiPJc17178756 = SzjoUqfAcSXdJiPJc31105845;     SzjoUqfAcSXdJiPJc31105845 = SzjoUqfAcSXdJiPJc43005667;     SzjoUqfAcSXdJiPJc43005667 = SzjoUqfAcSXdJiPJc67565551;     SzjoUqfAcSXdJiPJc67565551 = SzjoUqfAcSXdJiPJc42908408;     SzjoUqfAcSXdJiPJc42908408 = SzjoUqfAcSXdJiPJc37896020;     SzjoUqfAcSXdJiPJc37896020 = SzjoUqfAcSXdJiPJc690730;     SzjoUqfAcSXdJiPJc690730 = SzjoUqfAcSXdJiPJc40939059;     SzjoUqfAcSXdJiPJc40939059 = SzjoUqfAcSXdJiPJc29896397;     SzjoUqfAcSXdJiPJc29896397 = SzjoUqfAcSXdJiPJc6849991;     SzjoUqfAcSXdJiPJc6849991 = SzjoUqfAcSXdJiPJc3945999;     SzjoUqfAcSXdJiPJc3945999 = SzjoUqfAcSXdJiPJc718919;     SzjoUqfAcSXdJiPJc718919 = SzjoUqfAcSXdJiPJc87177469;     SzjoUqfAcSXdJiPJc87177469 = SzjoUqfAcSXdJiPJc22499811;     SzjoUqfAcSXdJiPJc22499811 = SzjoUqfAcSXdJiPJc64833326;     SzjoUqfAcSXdJiPJc64833326 = SzjoUqfAcSXdJiPJc95243968;     SzjoUqfAcSXdJiPJc95243968 = SzjoUqfAcSXdJiPJc39489319;     SzjoUqfAcSXdJiPJc39489319 = SzjoUqfAcSXdJiPJc91818004;     SzjoUqfAcSXdJiPJc91818004 = SzjoUqfAcSXdJiPJc58305057;     SzjoUqfAcSXdJiPJc58305057 = SzjoUqfAcSXdJiPJc33213501;     SzjoUqfAcSXdJiPJc33213501 = SzjoUqfAcSXdJiPJc38413559;     SzjoUqfAcSXdJiPJc38413559 = SzjoUqfAcSXdJiPJc66366224;     SzjoUqfAcSXdJiPJc66366224 = SzjoUqfAcSXdJiPJc50694476;     SzjoUqfAcSXdJiPJc50694476 = SzjoUqfAcSXdJiPJc7482593;     SzjoUqfAcSXdJiPJc7482593 = SzjoUqfAcSXdJiPJc64485915;     SzjoUqfAcSXdJiPJc64485915 = SzjoUqfAcSXdJiPJc67613675;     SzjoUqfAcSXdJiPJc67613675 = SzjoUqfAcSXdJiPJc1522607;     SzjoUqfAcSXdJiPJc1522607 = SzjoUqfAcSXdJiPJc83656631;     SzjoUqfAcSXdJiPJc83656631 = SzjoUqfAcSXdJiPJc55894179;     SzjoUqfAcSXdJiPJc55894179 = SzjoUqfAcSXdJiPJc50564426;     SzjoUqfAcSXdJiPJc50564426 = SzjoUqfAcSXdJiPJc20697700;     SzjoUqfAcSXdJiPJc20697700 = SzjoUqfAcSXdJiPJc90890756;     SzjoUqfAcSXdJiPJc90890756 = SzjoUqfAcSXdJiPJc26067227;     SzjoUqfAcSXdJiPJc26067227 = SzjoUqfAcSXdJiPJc63094443;     SzjoUqfAcSXdJiPJc63094443 = SzjoUqfAcSXdJiPJc79030336;     SzjoUqfAcSXdJiPJc79030336 = SzjoUqfAcSXdJiPJc86239318;     SzjoUqfAcSXdJiPJc86239318 = SzjoUqfAcSXdJiPJc7998326;     SzjoUqfAcSXdJiPJc7998326 = SzjoUqfAcSXdJiPJc14142911;     SzjoUqfAcSXdJiPJc14142911 = SzjoUqfAcSXdJiPJc5680697;     SzjoUqfAcSXdJiPJc5680697 = SzjoUqfAcSXdJiPJc24426778;     SzjoUqfAcSXdJiPJc24426778 = SzjoUqfAcSXdJiPJc60061342;     SzjoUqfAcSXdJiPJc60061342 = SzjoUqfAcSXdJiPJc58001501;     SzjoUqfAcSXdJiPJc58001501 = SzjoUqfAcSXdJiPJc81030920;     SzjoUqfAcSXdJiPJc81030920 = SzjoUqfAcSXdJiPJc64012882;     SzjoUqfAcSXdJiPJc64012882 = SzjoUqfAcSXdJiPJc61859508;     SzjoUqfAcSXdJiPJc61859508 = SzjoUqfAcSXdJiPJc37749916;     SzjoUqfAcSXdJiPJc37749916 = SzjoUqfAcSXdJiPJc46233440;     SzjoUqfAcSXdJiPJc46233440 = SzjoUqfAcSXdJiPJc72562189;     SzjoUqfAcSXdJiPJc72562189 = SzjoUqfAcSXdJiPJc72552270;     SzjoUqfAcSXdJiPJc72552270 = SzjoUqfAcSXdJiPJc91477528;     SzjoUqfAcSXdJiPJc91477528 = SzjoUqfAcSXdJiPJc83783692;     SzjoUqfAcSXdJiPJc83783692 = SzjoUqfAcSXdJiPJc51762703;     SzjoUqfAcSXdJiPJc51762703 = SzjoUqfAcSXdJiPJc6745355;     SzjoUqfAcSXdJiPJc6745355 = SzjoUqfAcSXdJiPJc98261750;     SzjoUqfAcSXdJiPJc98261750 = SzjoUqfAcSXdJiPJc58459005;     SzjoUqfAcSXdJiPJc58459005 = SzjoUqfAcSXdJiPJc50812532;     SzjoUqfAcSXdJiPJc50812532 = SzjoUqfAcSXdJiPJc80411369;     SzjoUqfAcSXdJiPJc80411369 = SzjoUqfAcSXdJiPJc35523075;     SzjoUqfAcSXdJiPJc35523075 = SzjoUqfAcSXdJiPJc3079636;     SzjoUqfAcSXdJiPJc3079636 = SzjoUqfAcSXdJiPJc75294733;     SzjoUqfAcSXdJiPJc75294733 = SzjoUqfAcSXdJiPJc36373413;     SzjoUqfAcSXdJiPJc36373413 = SzjoUqfAcSXdJiPJc17034098;     SzjoUqfAcSXdJiPJc17034098 = SzjoUqfAcSXdJiPJc85044879;     SzjoUqfAcSXdJiPJc85044879 = SzjoUqfAcSXdJiPJc79331971;     SzjoUqfAcSXdJiPJc79331971 = SzjoUqfAcSXdJiPJc86152290;     SzjoUqfAcSXdJiPJc86152290 = SzjoUqfAcSXdJiPJc13055243;     SzjoUqfAcSXdJiPJc13055243 = SzjoUqfAcSXdJiPJc74651692;     SzjoUqfAcSXdJiPJc74651692 = SzjoUqfAcSXdJiPJc24083027;     SzjoUqfAcSXdJiPJc24083027 = SzjoUqfAcSXdJiPJc43469474;     SzjoUqfAcSXdJiPJc43469474 = SzjoUqfAcSXdJiPJc78594007;     SzjoUqfAcSXdJiPJc78594007 = SzjoUqfAcSXdJiPJc87245642;     SzjoUqfAcSXdJiPJc87245642 = SzjoUqfAcSXdJiPJc25346409;     SzjoUqfAcSXdJiPJc25346409 = SzjoUqfAcSXdJiPJc86137308;     SzjoUqfAcSXdJiPJc86137308 = SzjoUqfAcSXdJiPJc33878280;     SzjoUqfAcSXdJiPJc33878280 = SzjoUqfAcSXdJiPJc73152158;     SzjoUqfAcSXdJiPJc73152158 = SzjoUqfAcSXdJiPJc80412057;     SzjoUqfAcSXdJiPJc80412057 = SzjoUqfAcSXdJiPJc85335303;     SzjoUqfAcSXdJiPJc85335303 = SzjoUqfAcSXdJiPJc86681593;     SzjoUqfAcSXdJiPJc86681593 = SzjoUqfAcSXdJiPJc45623085;     SzjoUqfAcSXdJiPJc45623085 = SzjoUqfAcSXdJiPJc26735999;     SzjoUqfAcSXdJiPJc26735999 = SzjoUqfAcSXdJiPJc17345164;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZvwrPIyHboAcqisk32854067() {     double TgEBwFkpUSNUmvBgU39732281 = -399221384;    double TgEBwFkpUSNUmvBgU35351494 = -629978469;    double TgEBwFkpUSNUmvBgU10477740 = -17505878;    double TgEBwFkpUSNUmvBgU63595793 = -297824370;    double TgEBwFkpUSNUmvBgU60497051 = -856468928;    double TgEBwFkpUSNUmvBgU75635188 = -102323418;    double TgEBwFkpUSNUmvBgU91096298 = -996387886;    double TgEBwFkpUSNUmvBgU86561176 = -64736408;    double TgEBwFkpUSNUmvBgU68026003 = -833206227;    double TgEBwFkpUSNUmvBgU57569092 = -432345479;    double TgEBwFkpUSNUmvBgU76010695 = -932516953;    double TgEBwFkpUSNUmvBgU89313858 = -453517528;    double TgEBwFkpUSNUmvBgU62070180 = -397021300;    double TgEBwFkpUSNUmvBgU80000348 = -93463163;    double TgEBwFkpUSNUmvBgU23456774 = -758890704;    double TgEBwFkpUSNUmvBgU67528924 = -908367853;    double TgEBwFkpUSNUmvBgU93404059 = -474756984;    double TgEBwFkpUSNUmvBgU71026828 = -333054242;    double TgEBwFkpUSNUmvBgU72756882 = -278635620;    double TgEBwFkpUSNUmvBgU75388847 = -715787528;    double TgEBwFkpUSNUmvBgU18061863 = -297406841;    double TgEBwFkpUSNUmvBgU94921063 = -161111759;    double TgEBwFkpUSNUmvBgU46298991 = -856060720;    double TgEBwFkpUSNUmvBgU94065338 = -530451960;    double TgEBwFkpUSNUmvBgU78448655 = -575008378;    double TgEBwFkpUSNUmvBgU26199014 = -801084108;    double TgEBwFkpUSNUmvBgU47780149 = -35707147;    double TgEBwFkpUSNUmvBgU46850509 = 13083372;    double TgEBwFkpUSNUmvBgU36788958 = 31948402;    double TgEBwFkpUSNUmvBgU16007170 = -61779442;    double TgEBwFkpUSNUmvBgU48236804 = -361066644;    double TgEBwFkpUSNUmvBgU58260110 = -621043825;    double TgEBwFkpUSNUmvBgU68216140 = -540443608;    double TgEBwFkpUSNUmvBgU81888497 = -484346847;    double TgEBwFkpUSNUmvBgU41062691 = -470696935;    double TgEBwFkpUSNUmvBgU36936837 = -133852420;    double TgEBwFkpUSNUmvBgU32929032 = -667720841;    double TgEBwFkpUSNUmvBgU45444106 = -640714962;    double TgEBwFkpUSNUmvBgU6613189 = -162982427;    double TgEBwFkpUSNUmvBgU53399215 = -946833792;    double TgEBwFkpUSNUmvBgU67170929 = -85169423;    double TgEBwFkpUSNUmvBgU54874616 = -242600316;    double TgEBwFkpUSNUmvBgU56244668 = -673607107;    double TgEBwFkpUSNUmvBgU6926713 = -829595292;    double TgEBwFkpUSNUmvBgU32469182 = -8716848;    double TgEBwFkpUSNUmvBgU75713957 = 80245202;    double TgEBwFkpUSNUmvBgU13820004 = -321295775;    double TgEBwFkpUSNUmvBgU28255306 = -143270197;    double TgEBwFkpUSNUmvBgU50354084 = -445642632;    double TgEBwFkpUSNUmvBgU47648172 = -810646211;    double TgEBwFkpUSNUmvBgU29566916 = -173066479;    double TgEBwFkpUSNUmvBgU64742014 = -510819392;    double TgEBwFkpUSNUmvBgU4381631 = -574880208;    double TgEBwFkpUSNUmvBgU23380988 = -210673803;    double TgEBwFkpUSNUmvBgU14792204 = -440688595;    double TgEBwFkpUSNUmvBgU44811218 = -138109625;    double TgEBwFkpUSNUmvBgU89052503 = -773917749;    double TgEBwFkpUSNUmvBgU16412402 = -487053919;    double TgEBwFkpUSNUmvBgU85147138 = -722815992;    double TgEBwFkpUSNUmvBgU34298037 = 44615179;    double TgEBwFkpUSNUmvBgU27855039 = 33383728;    double TgEBwFkpUSNUmvBgU44245790 = -909471258;    double TgEBwFkpUSNUmvBgU49772219 = 3315190;    double TgEBwFkpUSNUmvBgU52018834 = -671426785;    double TgEBwFkpUSNUmvBgU9332288 = 28721165;    double TgEBwFkpUSNUmvBgU17750585 = -211473129;    double TgEBwFkpUSNUmvBgU21097719 = -913073920;    double TgEBwFkpUSNUmvBgU80181682 = -912674453;    double TgEBwFkpUSNUmvBgU38937658 = -622766228;    double TgEBwFkpUSNUmvBgU86519936 = -525038284;    double TgEBwFkpUSNUmvBgU34599893 = -140647013;    double TgEBwFkpUSNUmvBgU47959954 = -834042023;    double TgEBwFkpUSNUmvBgU64413640 = -70071816;    double TgEBwFkpUSNUmvBgU19357668 = -331801828;    double TgEBwFkpUSNUmvBgU8217918 = -530618106;    double TgEBwFkpUSNUmvBgU63187247 = 45193475;    double TgEBwFkpUSNUmvBgU38676395 = -487504652;    double TgEBwFkpUSNUmvBgU39372278 = 73534572;    double TgEBwFkpUSNUmvBgU61596156 = -421735112;    double TgEBwFkpUSNUmvBgU2734699 = -555253581;    double TgEBwFkpUSNUmvBgU12379011 = -379788334;    double TgEBwFkpUSNUmvBgU19524844 = -892436951;    double TgEBwFkpUSNUmvBgU96496424 = -541273997;    double TgEBwFkpUSNUmvBgU89140786 = -157405388;    double TgEBwFkpUSNUmvBgU86440253 = -888712963;    double TgEBwFkpUSNUmvBgU83494790 = -850247253;    double TgEBwFkpUSNUmvBgU53878480 = 53836383;    double TgEBwFkpUSNUmvBgU44835152 = -229769805;    double TgEBwFkpUSNUmvBgU67096293 = 56341747;    double TgEBwFkpUSNUmvBgU96251472 = -232587310;    double TgEBwFkpUSNUmvBgU47884334 = -359934671;    double TgEBwFkpUSNUmvBgU16516630 = -80666922;    double TgEBwFkpUSNUmvBgU60296967 = -917898970;    double TgEBwFkpUSNUmvBgU72315151 = -107597606;    double TgEBwFkpUSNUmvBgU25544176 = -880217521;    double TgEBwFkpUSNUmvBgU22925140 = -175698166;    double TgEBwFkpUSNUmvBgU5102397 = -145915507;    double TgEBwFkpUSNUmvBgU4225834 = 97819678;    double TgEBwFkpUSNUmvBgU97594424 = -758316458;    double TgEBwFkpUSNUmvBgU14718597 = -399221384;     TgEBwFkpUSNUmvBgU39732281 = TgEBwFkpUSNUmvBgU35351494;     TgEBwFkpUSNUmvBgU35351494 = TgEBwFkpUSNUmvBgU10477740;     TgEBwFkpUSNUmvBgU10477740 = TgEBwFkpUSNUmvBgU63595793;     TgEBwFkpUSNUmvBgU63595793 = TgEBwFkpUSNUmvBgU60497051;     TgEBwFkpUSNUmvBgU60497051 = TgEBwFkpUSNUmvBgU75635188;     TgEBwFkpUSNUmvBgU75635188 = TgEBwFkpUSNUmvBgU91096298;     TgEBwFkpUSNUmvBgU91096298 = TgEBwFkpUSNUmvBgU86561176;     TgEBwFkpUSNUmvBgU86561176 = TgEBwFkpUSNUmvBgU68026003;     TgEBwFkpUSNUmvBgU68026003 = TgEBwFkpUSNUmvBgU57569092;     TgEBwFkpUSNUmvBgU57569092 = TgEBwFkpUSNUmvBgU76010695;     TgEBwFkpUSNUmvBgU76010695 = TgEBwFkpUSNUmvBgU89313858;     TgEBwFkpUSNUmvBgU89313858 = TgEBwFkpUSNUmvBgU62070180;     TgEBwFkpUSNUmvBgU62070180 = TgEBwFkpUSNUmvBgU80000348;     TgEBwFkpUSNUmvBgU80000348 = TgEBwFkpUSNUmvBgU23456774;     TgEBwFkpUSNUmvBgU23456774 = TgEBwFkpUSNUmvBgU67528924;     TgEBwFkpUSNUmvBgU67528924 = TgEBwFkpUSNUmvBgU93404059;     TgEBwFkpUSNUmvBgU93404059 = TgEBwFkpUSNUmvBgU71026828;     TgEBwFkpUSNUmvBgU71026828 = TgEBwFkpUSNUmvBgU72756882;     TgEBwFkpUSNUmvBgU72756882 = TgEBwFkpUSNUmvBgU75388847;     TgEBwFkpUSNUmvBgU75388847 = TgEBwFkpUSNUmvBgU18061863;     TgEBwFkpUSNUmvBgU18061863 = TgEBwFkpUSNUmvBgU94921063;     TgEBwFkpUSNUmvBgU94921063 = TgEBwFkpUSNUmvBgU46298991;     TgEBwFkpUSNUmvBgU46298991 = TgEBwFkpUSNUmvBgU94065338;     TgEBwFkpUSNUmvBgU94065338 = TgEBwFkpUSNUmvBgU78448655;     TgEBwFkpUSNUmvBgU78448655 = TgEBwFkpUSNUmvBgU26199014;     TgEBwFkpUSNUmvBgU26199014 = TgEBwFkpUSNUmvBgU47780149;     TgEBwFkpUSNUmvBgU47780149 = TgEBwFkpUSNUmvBgU46850509;     TgEBwFkpUSNUmvBgU46850509 = TgEBwFkpUSNUmvBgU36788958;     TgEBwFkpUSNUmvBgU36788958 = TgEBwFkpUSNUmvBgU16007170;     TgEBwFkpUSNUmvBgU16007170 = TgEBwFkpUSNUmvBgU48236804;     TgEBwFkpUSNUmvBgU48236804 = TgEBwFkpUSNUmvBgU58260110;     TgEBwFkpUSNUmvBgU58260110 = TgEBwFkpUSNUmvBgU68216140;     TgEBwFkpUSNUmvBgU68216140 = TgEBwFkpUSNUmvBgU81888497;     TgEBwFkpUSNUmvBgU81888497 = TgEBwFkpUSNUmvBgU41062691;     TgEBwFkpUSNUmvBgU41062691 = TgEBwFkpUSNUmvBgU36936837;     TgEBwFkpUSNUmvBgU36936837 = TgEBwFkpUSNUmvBgU32929032;     TgEBwFkpUSNUmvBgU32929032 = TgEBwFkpUSNUmvBgU45444106;     TgEBwFkpUSNUmvBgU45444106 = TgEBwFkpUSNUmvBgU6613189;     TgEBwFkpUSNUmvBgU6613189 = TgEBwFkpUSNUmvBgU53399215;     TgEBwFkpUSNUmvBgU53399215 = TgEBwFkpUSNUmvBgU67170929;     TgEBwFkpUSNUmvBgU67170929 = TgEBwFkpUSNUmvBgU54874616;     TgEBwFkpUSNUmvBgU54874616 = TgEBwFkpUSNUmvBgU56244668;     TgEBwFkpUSNUmvBgU56244668 = TgEBwFkpUSNUmvBgU6926713;     TgEBwFkpUSNUmvBgU6926713 = TgEBwFkpUSNUmvBgU32469182;     TgEBwFkpUSNUmvBgU32469182 = TgEBwFkpUSNUmvBgU75713957;     TgEBwFkpUSNUmvBgU75713957 = TgEBwFkpUSNUmvBgU13820004;     TgEBwFkpUSNUmvBgU13820004 = TgEBwFkpUSNUmvBgU28255306;     TgEBwFkpUSNUmvBgU28255306 = TgEBwFkpUSNUmvBgU50354084;     TgEBwFkpUSNUmvBgU50354084 = TgEBwFkpUSNUmvBgU47648172;     TgEBwFkpUSNUmvBgU47648172 = TgEBwFkpUSNUmvBgU29566916;     TgEBwFkpUSNUmvBgU29566916 = TgEBwFkpUSNUmvBgU64742014;     TgEBwFkpUSNUmvBgU64742014 = TgEBwFkpUSNUmvBgU4381631;     TgEBwFkpUSNUmvBgU4381631 = TgEBwFkpUSNUmvBgU23380988;     TgEBwFkpUSNUmvBgU23380988 = TgEBwFkpUSNUmvBgU14792204;     TgEBwFkpUSNUmvBgU14792204 = TgEBwFkpUSNUmvBgU44811218;     TgEBwFkpUSNUmvBgU44811218 = TgEBwFkpUSNUmvBgU89052503;     TgEBwFkpUSNUmvBgU89052503 = TgEBwFkpUSNUmvBgU16412402;     TgEBwFkpUSNUmvBgU16412402 = TgEBwFkpUSNUmvBgU85147138;     TgEBwFkpUSNUmvBgU85147138 = TgEBwFkpUSNUmvBgU34298037;     TgEBwFkpUSNUmvBgU34298037 = TgEBwFkpUSNUmvBgU27855039;     TgEBwFkpUSNUmvBgU27855039 = TgEBwFkpUSNUmvBgU44245790;     TgEBwFkpUSNUmvBgU44245790 = TgEBwFkpUSNUmvBgU49772219;     TgEBwFkpUSNUmvBgU49772219 = TgEBwFkpUSNUmvBgU52018834;     TgEBwFkpUSNUmvBgU52018834 = TgEBwFkpUSNUmvBgU9332288;     TgEBwFkpUSNUmvBgU9332288 = TgEBwFkpUSNUmvBgU17750585;     TgEBwFkpUSNUmvBgU17750585 = TgEBwFkpUSNUmvBgU21097719;     TgEBwFkpUSNUmvBgU21097719 = TgEBwFkpUSNUmvBgU80181682;     TgEBwFkpUSNUmvBgU80181682 = TgEBwFkpUSNUmvBgU38937658;     TgEBwFkpUSNUmvBgU38937658 = TgEBwFkpUSNUmvBgU86519936;     TgEBwFkpUSNUmvBgU86519936 = TgEBwFkpUSNUmvBgU34599893;     TgEBwFkpUSNUmvBgU34599893 = TgEBwFkpUSNUmvBgU47959954;     TgEBwFkpUSNUmvBgU47959954 = TgEBwFkpUSNUmvBgU64413640;     TgEBwFkpUSNUmvBgU64413640 = TgEBwFkpUSNUmvBgU19357668;     TgEBwFkpUSNUmvBgU19357668 = TgEBwFkpUSNUmvBgU8217918;     TgEBwFkpUSNUmvBgU8217918 = TgEBwFkpUSNUmvBgU63187247;     TgEBwFkpUSNUmvBgU63187247 = TgEBwFkpUSNUmvBgU38676395;     TgEBwFkpUSNUmvBgU38676395 = TgEBwFkpUSNUmvBgU39372278;     TgEBwFkpUSNUmvBgU39372278 = TgEBwFkpUSNUmvBgU61596156;     TgEBwFkpUSNUmvBgU61596156 = TgEBwFkpUSNUmvBgU2734699;     TgEBwFkpUSNUmvBgU2734699 = TgEBwFkpUSNUmvBgU12379011;     TgEBwFkpUSNUmvBgU12379011 = TgEBwFkpUSNUmvBgU19524844;     TgEBwFkpUSNUmvBgU19524844 = TgEBwFkpUSNUmvBgU96496424;     TgEBwFkpUSNUmvBgU96496424 = TgEBwFkpUSNUmvBgU89140786;     TgEBwFkpUSNUmvBgU89140786 = TgEBwFkpUSNUmvBgU86440253;     TgEBwFkpUSNUmvBgU86440253 = TgEBwFkpUSNUmvBgU83494790;     TgEBwFkpUSNUmvBgU83494790 = TgEBwFkpUSNUmvBgU53878480;     TgEBwFkpUSNUmvBgU53878480 = TgEBwFkpUSNUmvBgU44835152;     TgEBwFkpUSNUmvBgU44835152 = TgEBwFkpUSNUmvBgU67096293;     TgEBwFkpUSNUmvBgU67096293 = TgEBwFkpUSNUmvBgU96251472;     TgEBwFkpUSNUmvBgU96251472 = TgEBwFkpUSNUmvBgU47884334;     TgEBwFkpUSNUmvBgU47884334 = TgEBwFkpUSNUmvBgU16516630;     TgEBwFkpUSNUmvBgU16516630 = TgEBwFkpUSNUmvBgU60296967;     TgEBwFkpUSNUmvBgU60296967 = TgEBwFkpUSNUmvBgU72315151;     TgEBwFkpUSNUmvBgU72315151 = TgEBwFkpUSNUmvBgU25544176;     TgEBwFkpUSNUmvBgU25544176 = TgEBwFkpUSNUmvBgU22925140;     TgEBwFkpUSNUmvBgU22925140 = TgEBwFkpUSNUmvBgU5102397;     TgEBwFkpUSNUmvBgU5102397 = TgEBwFkpUSNUmvBgU4225834;     TgEBwFkpUSNUmvBgU4225834 = TgEBwFkpUSNUmvBgU97594424;     TgEBwFkpUSNUmvBgU97594424 = TgEBwFkpUSNUmvBgU14718597;     TgEBwFkpUSNUmvBgU14718597 = TgEBwFkpUSNUmvBgU39732281;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wTUWZezhkXIikZVL47903135() {     double lOWlWxCDriZeQdhDR45849388 = -511123273;    double lOWlWxCDriZeQdhDR78724773 = -339578249;    double lOWlWxCDriZeQdhDR26519924 = -168338800;    double lOWlWxCDriZeQdhDR90670574 = -488573001;    double lOWlWxCDriZeQdhDR29392540 = -277118800;    double lOWlWxCDriZeQdhDR67052391 = -366819653;    double lOWlWxCDriZeQdhDR51585264 = -35352863;    double lOWlWxCDriZeQdhDR36321068 = -351610321;    double lOWlWxCDriZeQdhDR32668499 = -601238892;    double lOWlWxCDriZeQdhDR98798750 = -761320956;    double lOWlWxCDriZeQdhDR36120985 = -809954540;    double lOWlWxCDriZeQdhDR21136902 = -897923730;    double lOWlWxCDriZeQdhDR48279299 = -836912389;    double lOWlWxCDriZeQdhDR63144025 = -625701730;    double lOWlWxCDriZeQdhDR19199200 = -349352870;    double lOWlWxCDriZeQdhDR9034318 = -881367396;    double lOWlWxCDriZeQdhDR87240062 = 8697903;    double lOWlWxCDriZeQdhDR68079981 = 1423115;    double lOWlWxCDriZeQdhDR34586724 = -684404717;    double lOWlWxCDriZeQdhDR55939012 = 27309552;    double lOWlWxCDriZeQdhDR6615057 = -152894296;    double lOWlWxCDriZeQdhDR81442086 = -69041475;    double lOWlWxCDriZeQdhDR81889782 = -602578207;    double lOWlWxCDriZeQdhDR78057712 = -752756947;    double lOWlWxCDriZeQdhDR22148306 = -948204411;    double lOWlWxCDriZeQdhDR50920689 = -679739431;    double lOWlWxCDriZeQdhDR46239928 = 63630650;    double lOWlWxCDriZeQdhDR59957634 = -52845582;    double lOWlWxCDriZeQdhDR2921214 = -190434303;    double lOWlWxCDriZeQdhDR84622806 = -979250309;    double lOWlWxCDriZeQdhDR40479353 = -274387583;    double lOWlWxCDriZeQdhDR55870732 = -330490641;    double lOWlWxCDriZeQdhDR77927088 = -433056978;    double lOWlWxCDriZeQdhDR76719062 = -235888612;    double lOWlWxCDriZeQdhDR2537343 = -9430040;    double lOWlWxCDriZeQdhDR5662751 = -347878527;    double lOWlWxCDriZeQdhDR14190966 = -621356710;    double lOWlWxCDriZeQdhDR79534318 = -412220971;    double lOWlWxCDriZeQdhDR80223442 = -156905924;    double lOWlWxCDriZeQdhDR40901153 = -320273014;    double lOWlWxCDriZeQdhDR72840153 = -150443317;    double lOWlWxCDriZeQdhDR87025072 = -512742595;    double lOWlWxCDriZeQdhDR34311646 = -754411691;    double lOWlWxCDriZeQdhDR94334452 = -848938230;    double lOWlWxCDriZeQdhDR44421647 = -770615003;    double lOWlWxCDriZeQdhDR87653152 = 95029086;    double lOWlWxCDriZeQdhDR31598578 = -440071312;    double lOWlWxCDriZeQdhDR36212247 = -153165729;    double lOWlWxCDriZeQdhDR60029813 = -531379620;    double lOWlWxCDriZeQdhDR60559824 = -897657019;    double lOWlWxCDriZeQdhDR65267594 = -15360150;    double lOWlWxCDriZeQdhDR75551733 = -311545712;    double lOWlWxCDriZeQdhDR52630087 = -366669796;    double lOWlWxCDriZeQdhDR19349869 = -531143949;    double lOWlWxCDriZeQdhDR41288079 = -41220377;    double lOWlWxCDriZeQdhDR64407301 = -342081798;    double lOWlWxCDriZeQdhDR96834990 = -737000043;    double lOWlWxCDriZeQdhDR48462212 = -415581853;    double lOWlWxCDriZeQdhDR68522269 = -540368590;    double lOWlWxCDriZeQdhDR78471850 = -597379369;    double lOWlWxCDriZeQdhDR20812464 = -330450304;    double lOWlWxCDriZeQdhDR91627630 = -982507282;    double lOWlWxCDriZeQdhDR33399854 = -61176019;    double lOWlWxCDriZeQdhDR48045692 = -621988584;    double lOWlWxCDriZeQdhDR58319397 = -386933374;    double lOWlWxCDriZeQdhDR80250253 = -379463900;    double lOWlWxCDriZeQdhDR43209813 = -364866752;    double lOWlWxCDriZeQdhDR71560237 = -501023777;    double lOWlWxCDriZeQdhDR60606683 = -516271691;    double lOWlWxCDriZeQdhDR13536450 = 98525657;    double lOWlWxCDriZeQdhDR94843351 = -160010686;    double lOWlWxCDriZeQdhDR7705745 = -579081126;    double lOWlWxCDriZeQdhDR87856539 = -841670961;    double lOWlWxCDriZeQdhDR93685571 = -264131703;    double lOWlWxCDriZeQdhDR83098858 = -822247131;    double lOWlWxCDriZeQdhDR19589984 = -640151702;    double lOWlWxCDriZeQdhDR47130441 = -314629785;    double lOWlWxCDriZeQdhDR87555329 = -753639977;    double lOWlWxCDriZeQdhDR33636065 = -982141945;    double lOWlWxCDriZeQdhDR34495154 = -943233498;    double lOWlWxCDriZeQdhDR19322112 = -139668119;    double lOWlWxCDriZeQdhDR10027681 = -783203622;    double lOWlWxCDriZeQdhDR99927820 = -521465962;    double lOWlWxCDriZeQdhDR42361389 = -292777284;    double lOWlWxCDriZeQdhDR19355212 = -863890160;    double lOWlWxCDriZeQdhDR64927620 = -962841871;    double lOWlWxCDriZeQdhDR3240645 = -963820845;    double lOWlWxCDriZeQdhDR58577220 = -901913030;    double lOWlWxCDriZeQdhDR35430983 = -94668235;    double lOWlWxCDriZeQdhDR38130042 = -667348243;    double lOWlWxCDriZeQdhDR8827760 = -610878485;    double lOWlWxCDriZeQdhDR65728754 = -105774858;    double lOWlWxCDriZeQdhDR11012050 = -871852382;    double lOWlWxCDriZeQdhDR1751592 = -559526555;    double lOWlWxCDriZeQdhDR20088690 = -989822711;    double lOWlWxCDriZeQdhDR81212523 = -167936036;    double lOWlWxCDriZeQdhDR53625219 = -351566576;    double lOWlWxCDriZeQdhDR86265953 = -32423108;    double lOWlWxCDriZeQdhDR36015055 = -362004857;    double lOWlWxCDriZeQdhDR64171393 = -511123273;     lOWlWxCDriZeQdhDR45849388 = lOWlWxCDriZeQdhDR78724773;     lOWlWxCDriZeQdhDR78724773 = lOWlWxCDriZeQdhDR26519924;     lOWlWxCDriZeQdhDR26519924 = lOWlWxCDriZeQdhDR90670574;     lOWlWxCDriZeQdhDR90670574 = lOWlWxCDriZeQdhDR29392540;     lOWlWxCDriZeQdhDR29392540 = lOWlWxCDriZeQdhDR67052391;     lOWlWxCDriZeQdhDR67052391 = lOWlWxCDriZeQdhDR51585264;     lOWlWxCDriZeQdhDR51585264 = lOWlWxCDriZeQdhDR36321068;     lOWlWxCDriZeQdhDR36321068 = lOWlWxCDriZeQdhDR32668499;     lOWlWxCDriZeQdhDR32668499 = lOWlWxCDriZeQdhDR98798750;     lOWlWxCDriZeQdhDR98798750 = lOWlWxCDriZeQdhDR36120985;     lOWlWxCDriZeQdhDR36120985 = lOWlWxCDriZeQdhDR21136902;     lOWlWxCDriZeQdhDR21136902 = lOWlWxCDriZeQdhDR48279299;     lOWlWxCDriZeQdhDR48279299 = lOWlWxCDriZeQdhDR63144025;     lOWlWxCDriZeQdhDR63144025 = lOWlWxCDriZeQdhDR19199200;     lOWlWxCDriZeQdhDR19199200 = lOWlWxCDriZeQdhDR9034318;     lOWlWxCDriZeQdhDR9034318 = lOWlWxCDriZeQdhDR87240062;     lOWlWxCDriZeQdhDR87240062 = lOWlWxCDriZeQdhDR68079981;     lOWlWxCDriZeQdhDR68079981 = lOWlWxCDriZeQdhDR34586724;     lOWlWxCDriZeQdhDR34586724 = lOWlWxCDriZeQdhDR55939012;     lOWlWxCDriZeQdhDR55939012 = lOWlWxCDriZeQdhDR6615057;     lOWlWxCDriZeQdhDR6615057 = lOWlWxCDriZeQdhDR81442086;     lOWlWxCDriZeQdhDR81442086 = lOWlWxCDriZeQdhDR81889782;     lOWlWxCDriZeQdhDR81889782 = lOWlWxCDriZeQdhDR78057712;     lOWlWxCDriZeQdhDR78057712 = lOWlWxCDriZeQdhDR22148306;     lOWlWxCDriZeQdhDR22148306 = lOWlWxCDriZeQdhDR50920689;     lOWlWxCDriZeQdhDR50920689 = lOWlWxCDriZeQdhDR46239928;     lOWlWxCDriZeQdhDR46239928 = lOWlWxCDriZeQdhDR59957634;     lOWlWxCDriZeQdhDR59957634 = lOWlWxCDriZeQdhDR2921214;     lOWlWxCDriZeQdhDR2921214 = lOWlWxCDriZeQdhDR84622806;     lOWlWxCDriZeQdhDR84622806 = lOWlWxCDriZeQdhDR40479353;     lOWlWxCDriZeQdhDR40479353 = lOWlWxCDriZeQdhDR55870732;     lOWlWxCDriZeQdhDR55870732 = lOWlWxCDriZeQdhDR77927088;     lOWlWxCDriZeQdhDR77927088 = lOWlWxCDriZeQdhDR76719062;     lOWlWxCDriZeQdhDR76719062 = lOWlWxCDriZeQdhDR2537343;     lOWlWxCDriZeQdhDR2537343 = lOWlWxCDriZeQdhDR5662751;     lOWlWxCDriZeQdhDR5662751 = lOWlWxCDriZeQdhDR14190966;     lOWlWxCDriZeQdhDR14190966 = lOWlWxCDriZeQdhDR79534318;     lOWlWxCDriZeQdhDR79534318 = lOWlWxCDriZeQdhDR80223442;     lOWlWxCDriZeQdhDR80223442 = lOWlWxCDriZeQdhDR40901153;     lOWlWxCDriZeQdhDR40901153 = lOWlWxCDriZeQdhDR72840153;     lOWlWxCDriZeQdhDR72840153 = lOWlWxCDriZeQdhDR87025072;     lOWlWxCDriZeQdhDR87025072 = lOWlWxCDriZeQdhDR34311646;     lOWlWxCDriZeQdhDR34311646 = lOWlWxCDriZeQdhDR94334452;     lOWlWxCDriZeQdhDR94334452 = lOWlWxCDriZeQdhDR44421647;     lOWlWxCDriZeQdhDR44421647 = lOWlWxCDriZeQdhDR87653152;     lOWlWxCDriZeQdhDR87653152 = lOWlWxCDriZeQdhDR31598578;     lOWlWxCDriZeQdhDR31598578 = lOWlWxCDriZeQdhDR36212247;     lOWlWxCDriZeQdhDR36212247 = lOWlWxCDriZeQdhDR60029813;     lOWlWxCDriZeQdhDR60029813 = lOWlWxCDriZeQdhDR60559824;     lOWlWxCDriZeQdhDR60559824 = lOWlWxCDriZeQdhDR65267594;     lOWlWxCDriZeQdhDR65267594 = lOWlWxCDriZeQdhDR75551733;     lOWlWxCDriZeQdhDR75551733 = lOWlWxCDriZeQdhDR52630087;     lOWlWxCDriZeQdhDR52630087 = lOWlWxCDriZeQdhDR19349869;     lOWlWxCDriZeQdhDR19349869 = lOWlWxCDriZeQdhDR41288079;     lOWlWxCDriZeQdhDR41288079 = lOWlWxCDriZeQdhDR64407301;     lOWlWxCDriZeQdhDR64407301 = lOWlWxCDriZeQdhDR96834990;     lOWlWxCDriZeQdhDR96834990 = lOWlWxCDriZeQdhDR48462212;     lOWlWxCDriZeQdhDR48462212 = lOWlWxCDriZeQdhDR68522269;     lOWlWxCDriZeQdhDR68522269 = lOWlWxCDriZeQdhDR78471850;     lOWlWxCDriZeQdhDR78471850 = lOWlWxCDriZeQdhDR20812464;     lOWlWxCDriZeQdhDR20812464 = lOWlWxCDriZeQdhDR91627630;     lOWlWxCDriZeQdhDR91627630 = lOWlWxCDriZeQdhDR33399854;     lOWlWxCDriZeQdhDR33399854 = lOWlWxCDriZeQdhDR48045692;     lOWlWxCDriZeQdhDR48045692 = lOWlWxCDriZeQdhDR58319397;     lOWlWxCDriZeQdhDR58319397 = lOWlWxCDriZeQdhDR80250253;     lOWlWxCDriZeQdhDR80250253 = lOWlWxCDriZeQdhDR43209813;     lOWlWxCDriZeQdhDR43209813 = lOWlWxCDriZeQdhDR71560237;     lOWlWxCDriZeQdhDR71560237 = lOWlWxCDriZeQdhDR60606683;     lOWlWxCDriZeQdhDR60606683 = lOWlWxCDriZeQdhDR13536450;     lOWlWxCDriZeQdhDR13536450 = lOWlWxCDriZeQdhDR94843351;     lOWlWxCDriZeQdhDR94843351 = lOWlWxCDriZeQdhDR7705745;     lOWlWxCDriZeQdhDR7705745 = lOWlWxCDriZeQdhDR87856539;     lOWlWxCDriZeQdhDR87856539 = lOWlWxCDriZeQdhDR93685571;     lOWlWxCDriZeQdhDR93685571 = lOWlWxCDriZeQdhDR83098858;     lOWlWxCDriZeQdhDR83098858 = lOWlWxCDriZeQdhDR19589984;     lOWlWxCDriZeQdhDR19589984 = lOWlWxCDriZeQdhDR47130441;     lOWlWxCDriZeQdhDR47130441 = lOWlWxCDriZeQdhDR87555329;     lOWlWxCDriZeQdhDR87555329 = lOWlWxCDriZeQdhDR33636065;     lOWlWxCDriZeQdhDR33636065 = lOWlWxCDriZeQdhDR34495154;     lOWlWxCDriZeQdhDR34495154 = lOWlWxCDriZeQdhDR19322112;     lOWlWxCDriZeQdhDR19322112 = lOWlWxCDriZeQdhDR10027681;     lOWlWxCDriZeQdhDR10027681 = lOWlWxCDriZeQdhDR99927820;     lOWlWxCDriZeQdhDR99927820 = lOWlWxCDriZeQdhDR42361389;     lOWlWxCDriZeQdhDR42361389 = lOWlWxCDriZeQdhDR19355212;     lOWlWxCDriZeQdhDR19355212 = lOWlWxCDriZeQdhDR64927620;     lOWlWxCDriZeQdhDR64927620 = lOWlWxCDriZeQdhDR3240645;     lOWlWxCDriZeQdhDR3240645 = lOWlWxCDriZeQdhDR58577220;     lOWlWxCDriZeQdhDR58577220 = lOWlWxCDriZeQdhDR35430983;     lOWlWxCDriZeQdhDR35430983 = lOWlWxCDriZeQdhDR38130042;     lOWlWxCDriZeQdhDR38130042 = lOWlWxCDriZeQdhDR8827760;     lOWlWxCDriZeQdhDR8827760 = lOWlWxCDriZeQdhDR65728754;     lOWlWxCDriZeQdhDR65728754 = lOWlWxCDriZeQdhDR11012050;     lOWlWxCDriZeQdhDR11012050 = lOWlWxCDriZeQdhDR1751592;     lOWlWxCDriZeQdhDR1751592 = lOWlWxCDriZeQdhDR20088690;     lOWlWxCDriZeQdhDR20088690 = lOWlWxCDriZeQdhDR81212523;     lOWlWxCDriZeQdhDR81212523 = lOWlWxCDriZeQdhDR53625219;     lOWlWxCDriZeQdhDR53625219 = lOWlWxCDriZeQdhDR86265953;     lOWlWxCDriZeQdhDR86265953 = lOWlWxCDriZeQdhDR36015055;     lOWlWxCDriZeQdhDR36015055 = lOWlWxCDriZeQdhDR64171393;     lOWlWxCDriZeQdhDR64171393 = lOWlWxCDriZeQdhDR45849388;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void eTLAGchucVnQpSbd15194735() {     double csjMJuRYxnmZEJoKS81308356 = -977400172;    double csjMJuRYxnmZEJoKS91996851 = -220129856;    double csjMJuRYxnmZEJoKS63906400 = -934005875;    double csjMJuRYxnmZEJoKS62893929 = -309724072;    double csjMJuRYxnmZEJoKS85677809 = -382717645;    double csjMJuRYxnmZEJoKS3275263 = -629167549;    double csjMJuRYxnmZEJoKS62242670 = -980078597;    double csjMJuRYxnmZEJoKS98452628 = 4155538;    double csjMJuRYxnmZEJoKS34186104 = 10429701;    double csjMJuRYxnmZEJoKS13701463 = -426007954;    double csjMJuRYxnmZEJoKS74302720 = -611962269;    double csjMJuRYxnmZEJoKS21705050 = -762044501;    double csjMJuRYxnmZEJoKS86513944 = -288924894;    double csjMJuRYxnmZEJoKS16284889 = -602065140;    double csjMJuRYxnmZEJoKS77393716 = -852777885;    double csjMJuRYxnmZEJoKS92575618 = -517179003;    double csjMJuRYxnmZEJoKS43075999 = -518276650;    double csjMJuRYxnmZEJoKS67840173 = -72004953;    double csjMJuRYxnmZEJoKS35137054 = 99031862;    double csjMJuRYxnmZEJoKS49776230 = -399251514;    double csjMJuRYxnmZEJoKS19701920 = -910168985;    double csjMJuRYxnmZEJoKS84863611 = -155677769;    double csjMJuRYxnmZEJoKS52415162 = -240210391;    double csjMJuRYxnmZEJoKS71850657 = -163581552;    double csjMJuRYxnmZEJoKS30166257 = -972189610;    double csjMJuRYxnmZEJoKS61618861 = -368316238;    double csjMJuRYxnmZEJoKS6660502 = -842653491;    double csjMJuRYxnmZEJoKS686060 = -222617756;    double csjMJuRYxnmZEJoKS49589428 = -764529105;    double csjMJuRYxnmZEJoKS90156217 = 86242520;    double csjMJuRYxnmZEJoKS59062585 = -826368070;    double csjMJuRYxnmZEJoKS93693750 = -103462055;    double csjMJuRYxnmZEJoKS90145099 = -580652956;    double csjMJuRYxnmZEJoKS6467489 = -300582426;    double csjMJuRYxnmZEJoKS68345796 = 39469709;    double csjMJuRYxnmZEJoKS57223594 = -562443833;    double csjMJuRYxnmZEJoKS63602782 = -486052750;    double csjMJuRYxnmZEJoKS30749722 = -394876553;    double csjMJuRYxnmZEJoKS10614004 = -907782200;    double csjMJuRYxnmZEJoKS5172686 = -885535560;    double csjMJuRYxnmZEJoKS6291761 = 80357768;    double csjMJuRYxnmZEJoKS76390199 = -107287886;    double csjMJuRYxnmZEJoKS23695025 = -307592477;    double csjMJuRYxnmZEJoKS34465791 = -353270640;    double csjMJuRYxnmZEJoKS44438106 = -597126164;    double csjMJuRYxnmZEJoKS33614481 = 56105454;    double csjMJuRYxnmZEJoKS50943742 = -924057673;    double csjMJuRYxnmZEJoKS53938558 = -319692323;    double csjMJuRYxnmZEJoKS82922479 = -195116500;    double csjMJuRYxnmZEJoKS85089414 = -288683835;    double csjMJuRYxnmZEJoKS46038318 = -989636708;    double csjMJuRYxnmZEJoKS59037206 = -525198059;    double csjMJuRYxnmZEJoKS21251236 = -122715350;    double csjMJuRYxnmZEJoKS35341715 = -722652448;    double csjMJuRYxnmZEJoKS95492815 = -387710523;    double csjMJuRYxnmZEJoKS96444744 = -721722404;    double csjMJuRYxnmZEJoKS39581690 = -979919466;    double csjMJuRYxnmZEJoKS92055743 = -670424324;    double csjMJuRYxnmZEJoKS32727673 = -337534463;    double csjMJuRYxnmZEJoKS24058948 = 85598593;    double csjMJuRYxnmZEJoKS96614761 = -786514058;    double csjMJuRYxnmZEJoKS61556611 = -657460841;    double csjMJuRYxnmZEJoKS48863200 = -231315357;    double csjMJuRYxnmZEJoKS44029887 = 24187180;    double csjMJuRYxnmZEJoKS54638878 = -599639885;    double csjMJuRYxnmZEJoKS80608970 = -408500215;    double csjMJuRYxnmZEJoKS31559951 = -81391546;    double csjMJuRYxnmZEJoKS80046456 = -988342468;    double csjMJuRYxnmZEJoKS47939092 = -541534849;    double csjMJuRYxnmZEJoKS20170123 = -190334052;    double csjMJuRYxnmZEJoKS28972836 = 68873747;    double csjMJuRYxnmZEJoKS12326277 = -23400098;    double csjMJuRYxnmZEJoKS57226169 = -164222754;    double csjMJuRYxnmZEJoKS29964369 = -15432578;    double csjMJuRYxnmZEJoKS43484470 = -379609283;    double csjMJuRYxnmZEJoKS43311720 = -702881100;    double csjMJuRYxnmZEJoKS61168587 = -848085293;    double csjMJuRYxnmZEJoKS17949372 = -886939751;    double csjMJuRYxnmZEJoKS27412552 = -566455388;    double csjMJuRYxnmZEJoKS96551776 = -928295064;    double csjMJuRYxnmZEJoKS10675120 = -444258566;    double csjMJuRYxnmZEJoKS52721943 = -422961169;    double csjMJuRYxnmZEJoKS17763580 = 72498744;    double csjMJuRYxnmZEJoKS64500014 = -375845271;    double csjMJuRYxnmZEJoKS44117899 = 75879227;    double csjMJuRYxnmZEJoKS25380 = -201170011;    double csjMJuRYxnmZEJoKS72442514 = -980746705;    double csjMJuRYxnmZEJoKS54803384 = -858000508;    double csjMJuRYxnmZEJoKS10974674 = -912871903;    double csjMJuRYxnmZEJoKS71901051 = -238807888;    double csjMJuRYxnmZEJoKS17641905 = -582524368;    double csjMJuRYxnmZEJoKS71547039 = -815628427;    double csjMJuRYxnmZEJoKS98022048 = 42657909;    double csjMJuRYxnmZEJoKS86555056 = -893380793;    double csjMJuRYxnmZEJoKS8557924 = 978497;    double csjMJuRYxnmZEJoKS44735149 = -262181391;    double csjMJuRYxnmZEJoKS27526999 = -875972529;    double csjMJuRYxnmZEJoKS79665137 = -231779657;    double csjMJuRYxnmZEJoKS79826912 = -753630756;    double csjMJuRYxnmZEJoKS63829136 = -977400172;     csjMJuRYxnmZEJoKS81308356 = csjMJuRYxnmZEJoKS91996851;     csjMJuRYxnmZEJoKS91996851 = csjMJuRYxnmZEJoKS63906400;     csjMJuRYxnmZEJoKS63906400 = csjMJuRYxnmZEJoKS62893929;     csjMJuRYxnmZEJoKS62893929 = csjMJuRYxnmZEJoKS85677809;     csjMJuRYxnmZEJoKS85677809 = csjMJuRYxnmZEJoKS3275263;     csjMJuRYxnmZEJoKS3275263 = csjMJuRYxnmZEJoKS62242670;     csjMJuRYxnmZEJoKS62242670 = csjMJuRYxnmZEJoKS98452628;     csjMJuRYxnmZEJoKS98452628 = csjMJuRYxnmZEJoKS34186104;     csjMJuRYxnmZEJoKS34186104 = csjMJuRYxnmZEJoKS13701463;     csjMJuRYxnmZEJoKS13701463 = csjMJuRYxnmZEJoKS74302720;     csjMJuRYxnmZEJoKS74302720 = csjMJuRYxnmZEJoKS21705050;     csjMJuRYxnmZEJoKS21705050 = csjMJuRYxnmZEJoKS86513944;     csjMJuRYxnmZEJoKS86513944 = csjMJuRYxnmZEJoKS16284889;     csjMJuRYxnmZEJoKS16284889 = csjMJuRYxnmZEJoKS77393716;     csjMJuRYxnmZEJoKS77393716 = csjMJuRYxnmZEJoKS92575618;     csjMJuRYxnmZEJoKS92575618 = csjMJuRYxnmZEJoKS43075999;     csjMJuRYxnmZEJoKS43075999 = csjMJuRYxnmZEJoKS67840173;     csjMJuRYxnmZEJoKS67840173 = csjMJuRYxnmZEJoKS35137054;     csjMJuRYxnmZEJoKS35137054 = csjMJuRYxnmZEJoKS49776230;     csjMJuRYxnmZEJoKS49776230 = csjMJuRYxnmZEJoKS19701920;     csjMJuRYxnmZEJoKS19701920 = csjMJuRYxnmZEJoKS84863611;     csjMJuRYxnmZEJoKS84863611 = csjMJuRYxnmZEJoKS52415162;     csjMJuRYxnmZEJoKS52415162 = csjMJuRYxnmZEJoKS71850657;     csjMJuRYxnmZEJoKS71850657 = csjMJuRYxnmZEJoKS30166257;     csjMJuRYxnmZEJoKS30166257 = csjMJuRYxnmZEJoKS61618861;     csjMJuRYxnmZEJoKS61618861 = csjMJuRYxnmZEJoKS6660502;     csjMJuRYxnmZEJoKS6660502 = csjMJuRYxnmZEJoKS686060;     csjMJuRYxnmZEJoKS686060 = csjMJuRYxnmZEJoKS49589428;     csjMJuRYxnmZEJoKS49589428 = csjMJuRYxnmZEJoKS90156217;     csjMJuRYxnmZEJoKS90156217 = csjMJuRYxnmZEJoKS59062585;     csjMJuRYxnmZEJoKS59062585 = csjMJuRYxnmZEJoKS93693750;     csjMJuRYxnmZEJoKS93693750 = csjMJuRYxnmZEJoKS90145099;     csjMJuRYxnmZEJoKS90145099 = csjMJuRYxnmZEJoKS6467489;     csjMJuRYxnmZEJoKS6467489 = csjMJuRYxnmZEJoKS68345796;     csjMJuRYxnmZEJoKS68345796 = csjMJuRYxnmZEJoKS57223594;     csjMJuRYxnmZEJoKS57223594 = csjMJuRYxnmZEJoKS63602782;     csjMJuRYxnmZEJoKS63602782 = csjMJuRYxnmZEJoKS30749722;     csjMJuRYxnmZEJoKS30749722 = csjMJuRYxnmZEJoKS10614004;     csjMJuRYxnmZEJoKS10614004 = csjMJuRYxnmZEJoKS5172686;     csjMJuRYxnmZEJoKS5172686 = csjMJuRYxnmZEJoKS6291761;     csjMJuRYxnmZEJoKS6291761 = csjMJuRYxnmZEJoKS76390199;     csjMJuRYxnmZEJoKS76390199 = csjMJuRYxnmZEJoKS23695025;     csjMJuRYxnmZEJoKS23695025 = csjMJuRYxnmZEJoKS34465791;     csjMJuRYxnmZEJoKS34465791 = csjMJuRYxnmZEJoKS44438106;     csjMJuRYxnmZEJoKS44438106 = csjMJuRYxnmZEJoKS33614481;     csjMJuRYxnmZEJoKS33614481 = csjMJuRYxnmZEJoKS50943742;     csjMJuRYxnmZEJoKS50943742 = csjMJuRYxnmZEJoKS53938558;     csjMJuRYxnmZEJoKS53938558 = csjMJuRYxnmZEJoKS82922479;     csjMJuRYxnmZEJoKS82922479 = csjMJuRYxnmZEJoKS85089414;     csjMJuRYxnmZEJoKS85089414 = csjMJuRYxnmZEJoKS46038318;     csjMJuRYxnmZEJoKS46038318 = csjMJuRYxnmZEJoKS59037206;     csjMJuRYxnmZEJoKS59037206 = csjMJuRYxnmZEJoKS21251236;     csjMJuRYxnmZEJoKS21251236 = csjMJuRYxnmZEJoKS35341715;     csjMJuRYxnmZEJoKS35341715 = csjMJuRYxnmZEJoKS95492815;     csjMJuRYxnmZEJoKS95492815 = csjMJuRYxnmZEJoKS96444744;     csjMJuRYxnmZEJoKS96444744 = csjMJuRYxnmZEJoKS39581690;     csjMJuRYxnmZEJoKS39581690 = csjMJuRYxnmZEJoKS92055743;     csjMJuRYxnmZEJoKS92055743 = csjMJuRYxnmZEJoKS32727673;     csjMJuRYxnmZEJoKS32727673 = csjMJuRYxnmZEJoKS24058948;     csjMJuRYxnmZEJoKS24058948 = csjMJuRYxnmZEJoKS96614761;     csjMJuRYxnmZEJoKS96614761 = csjMJuRYxnmZEJoKS61556611;     csjMJuRYxnmZEJoKS61556611 = csjMJuRYxnmZEJoKS48863200;     csjMJuRYxnmZEJoKS48863200 = csjMJuRYxnmZEJoKS44029887;     csjMJuRYxnmZEJoKS44029887 = csjMJuRYxnmZEJoKS54638878;     csjMJuRYxnmZEJoKS54638878 = csjMJuRYxnmZEJoKS80608970;     csjMJuRYxnmZEJoKS80608970 = csjMJuRYxnmZEJoKS31559951;     csjMJuRYxnmZEJoKS31559951 = csjMJuRYxnmZEJoKS80046456;     csjMJuRYxnmZEJoKS80046456 = csjMJuRYxnmZEJoKS47939092;     csjMJuRYxnmZEJoKS47939092 = csjMJuRYxnmZEJoKS20170123;     csjMJuRYxnmZEJoKS20170123 = csjMJuRYxnmZEJoKS28972836;     csjMJuRYxnmZEJoKS28972836 = csjMJuRYxnmZEJoKS12326277;     csjMJuRYxnmZEJoKS12326277 = csjMJuRYxnmZEJoKS57226169;     csjMJuRYxnmZEJoKS57226169 = csjMJuRYxnmZEJoKS29964369;     csjMJuRYxnmZEJoKS29964369 = csjMJuRYxnmZEJoKS43484470;     csjMJuRYxnmZEJoKS43484470 = csjMJuRYxnmZEJoKS43311720;     csjMJuRYxnmZEJoKS43311720 = csjMJuRYxnmZEJoKS61168587;     csjMJuRYxnmZEJoKS61168587 = csjMJuRYxnmZEJoKS17949372;     csjMJuRYxnmZEJoKS17949372 = csjMJuRYxnmZEJoKS27412552;     csjMJuRYxnmZEJoKS27412552 = csjMJuRYxnmZEJoKS96551776;     csjMJuRYxnmZEJoKS96551776 = csjMJuRYxnmZEJoKS10675120;     csjMJuRYxnmZEJoKS10675120 = csjMJuRYxnmZEJoKS52721943;     csjMJuRYxnmZEJoKS52721943 = csjMJuRYxnmZEJoKS17763580;     csjMJuRYxnmZEJoKS17763580 = csjMJuRYxnmZEJoKS64500014;     csjMJuRYxnmZEJoKS64500014 = csjMJuRYxnmZEJoKS44117899;     csjMJuRYxnmZEJoKS44117899 = csjMJuRYxnmZEJoKS25380;     csjMJuRYxnmZEJoKS25380 = csjMJuRYxnmZEJoKS72442514;     csjMJuRYxnmZEJoKS72442514 = csjMJuRYxnmZEJoKS54803384;     csjMJuRYxnmZEJoKS54803384 = csjMJuRYxnmZEJoKS10974674;     csjMJuRYxnmZEJoKS10974674 = csjMJuRYxnmZEJoKS71901051;     csjMJuRYxnmZEJoKS71901051 = csjMJuRYxnmZEJoKS17641905;     csjMJuRYxnmZEJoKS17641905 = csjMJuRYxnmZEJoKS71547039;     csjMJuRYxnmZEJoKS71547039 = csjMJuRYxnmZEJoKS98022048;     csjMJuRYxnmZEJoKS98022048 = csjMJuRYxnmZEJoKS86555056;     csjMJuRYxnmZEJoKS86555056 = csjMJuRYxnmZEJoKS8557924;     csjMJuRYxnmZEJoKS8557924 = csjMJuRYxnmZEJoKS44735149;     csjMJuRYxnmZEJoKS44735149 = csjMJuRYxnmZEJoKS27526999;     csjMJuRYxnmZEJoKS27526999 = csjMJuRYxnmZEJoKS79665137;     csjMJuRYxnmZEJoKS79665137 = csjMJuRYxnmZEJoKS79826912;     csjMJuRYxnmZEJoKS79826912 = csjMJuRYxnmZEJoKS63829136;     csjMJuRYxnmZEJoKS63829136 = csjMJuRYxnmZEJoKS81308356;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uMoZoVzVfqVXcALZ51839709() {     double QnyDGjBvQqHykYGFs3695474 = -611804136;    double QnyDGjBvQqHykYGFs76344352 = -168672911;    double QnyDGjBvQqHykYGFs92675679 = -73659719;    double QnyDGjBvQqHykYGFs77900617 = -316131603;    double QnyDGjBvQqHykYGFs83852063 = -973774646;    double QnyDGjBvQqHykYGFs18158380 = -489775927;    double QnyDGjBvQqHykYGFs54398409 = -802065903;    double QnyDGjBvQqHykYGFs74086487 = -466441106;    double QnyDGjBvQqHykYGFs31349235 = -973766338;    double QnyDGjBvQqHykYGFs5465048 = -168749287;    double QnyDGjBvQqHykYGFs11844580 = -608586670;    double QnyDGjBvQqHykYGFs77607998 = 87210205;    double QnyDGjBvQqHykYGFs53522125 = -907642213;    double QnyDGjBvQqHykYGFs58899641 = -29773897;    double QnyDGjBvQqHykYGFs14128994 = -480255598;    double QnyDGjBvQqHykYGFs36831531 = -898846545;    double QnyDGjBvQqHykYGFs92899350 = -795556470;    double QnyDGjBvQqHykYGFs73816589 = -523747644;    double QnyDGjBvQqHykYGFs76418685 = -36070263;    double QnyDGjBvQqHykYGFs28292514 = -651885967;    double QnyDGjBvQqHykYGFs20585027 = -901656294;    double QnyDGjBvQqHykYGFs48678830 = -660444083;    double QnyDGjBvQqHykYGFs55708485 = -416290983;    double QnyDGjBvQqHykYGFs98350444 = -727574409;    double QnyDGjBvQqHykYGFs65706503 = -593748734;    double QnyDGjBvQqHykYGFs49921856 = -812210462;    double QnyDGjBvQqHykYGFs53749922 = -600239984;    double QnyDGjBvQqHykYGFs6597510 = -11072210;    double QnyDGjBvQqHykYGFs56481989 = -8786225;    double QnyDGjBvQqHykYGFs99313396 = -934053346;    double QnyDGjBvQqHykYGFs3353391 = -907684222;    double QnyDGjBvQqHykYGFs51234941 = -755533410;    double QnyDGjBvQqHykYGFs71183769 = -9996450;    double QnyDGjBvQqHykYGFs65856176 = -624709276;    double QnyDGjBvQqHykYGFs44575161 = -24286713;    double QnyDGjBvQqHykYGFs98916463 = -877839210;    double QnyDGjBvQqHykYGFs57042495 = -219000702;    double QnyDGjBvQqHykYGFs84375822 = -262502025;    double QnyDGjBvQqHykYGFs58922135 = -208828232;    double QnyDGjBvQqHykYGFs25358400 = 78240410;    double QnyDGjBvQqHykYGFs35049132 = -422819898;    double QnyDGjBvQqHykYGFs64898590 = -965196577;    double QnyDGjBvQqHykYGFs29245217 = -195123061;    double QnyDGjBvQqHykYGFs33909910 = -350634289;    double QnyDGjBvQqHykYGFs12421373 = -406269642;    double QnyDGjBvQqHykYGFs41714762 = -126123642;    double QnyDGjBvQqHykYGFs63241139 = -402467926;    double QnyDGjBvQqHykYGFs98537232 = -76227313;    double QnyDGjBvQqHykYGFs77382384 = -144833199;    double QnyDGjBvQqHykYGFs82173159 = -430704094;    double QnyDGjBvQqHykYGFs54907535 = -244712985;    double QnyDGjBvQqHykYGFs32888463 = -956017341;    double QnyDGjBvQqHykYGFs99565639 = -894626580;    double QnyDGjBvQqHykYGFs95628260 = -998333257;    double QnyDGjBvQqHykYGFs31254683 = -528414639;    double QnyDGjBvQqHykYGFs55016643 = -951360053;    double QnyDGjBvQqHykYGFs20635867 = -752381928;    double QnyDGjBvQqHykYGFs94325234 = -346085310;    double QnyDGjBvQqHykYGFs12194115 = -722382870;    double QnyDGjBvQqHykYGFs33930208 = -61564185;    double QnyDGjBvQqHykYGFs64408458 = -889535943;    double QnyDGjBvQqHykYGFs47800900 = -690993694;    double QnyDGjBvQqHykYGFs17604498 = -357654882;    double QnyDGjBvQqHykYGFs32035839 = 60287007;    double QnyDGjBvQqHykYGFs2111658 = -261065066;    double QnyDGjBvQqHykYGFs60609639 = -853053261;    double QnyDGjBvQqHykYGFs6424230 = -902793345;    double QnyDGjBvQqHykYGFs87665949 = -182932938;    double QnyDGjBvQqHykYGFs14324480 = 94512816;    double QnyDGjBvQqHykYGFs15212530 = -602416389;    double QnyDGjBvQqHykYGFs79789036 = -579845844;    double QnyDGjBvQqHykYGFs8523528 = -433054446;    double QnyDGjBvQqHykYGFs14894454 = -214919413;    double QnyDGjBvQqHykYGFs51060285 = -14310674;    double QnyDGjBvQqHykYGFs93243381 = -129066070;    double QnyDGjBvQqHykYGFs55686436 = -936459717;    double QnyDGjBvQqHykYGFs19433614 = -365321022;    double QnyDGjBvQqHykYGFs21798575 = 34343306;    double QnyDGjBvQqHykYGFs85929072 = -221304767;    double QnyDGjBvQqHykYGFs23991741 = -367625093;    double QnyDGjBvQqHykYGFs86680716 = -309742537;    double QnyDGjBvQqHykYGFs55212689 = -424012671;    double QnyDGjBvQqHykYGFs29215125 = -866239012;    double QnyDGjBvQqHykYGFs74308829 = -578082131;    double QnyDGjBvQqHykYGFs44405862 = -589340362;    double QnyDGjBvQqHykYGFs70464927 = -951666881;    double QnyDGjBvQqHykYGFs51669302 = -860906830;    double QnyDGjBvQqHykYGFs75555509 = -11663194;    double QnyDGjBvQqHykYGFs34601493 = 3705362;    double QnyDGjBvQqHykYGFs89558517 = -72926660;    double QnyDGjBvQqHykYGFs78280597 = -25457282;    double QnyDGjBvQqHykYGFs62717260 = -872915392;    double QnyDGjBvQqHykYGFs72181708 = -540119155;    double QnyDGjBvQqHykYGFs24991928 = -47264047;    double QnyDGjBvQqHykYGFs60949942 = -32223647;    double QnyDGjBvQqHykYGFs87248231 = -731826204;    double QnyDGjBvQqHykYGFs47294092 = -507541695;    double QnyDGjBvQqHykYGFs97209377 = -155410068;    double QnyDGjBvQqHykYGFs31798253 = 10430776;    double QnyDGjBvQqHykYGFs51811734 = -611804136;     QnyDGjBvQqHykYGFs3695474 = QnyDGjBvQqHykYGFs76344352;     QnyDGjBvQqHykYGFs76344352 = QnyDGjBvQqHykYGFs92675679;     QnyDGjBvQqHykYGFs92675679 = QnyDGjBvQqHykYGFs77900617;     QnyDGjBvQqHykYGFs77900617 = QnyDGjBvQqHykYGFs83852063;     QnyDGjBvQqHykYGFs83852063 = QnyDGjBvQqHykYGFs18158380;     QnyDGjBvQqHykYGFs18158380 = QnyDGjBvQqHykYGFs54398409;     QnyDGjBvQqHykYGFs54398409 = QnyDGjBvQqHykYGFs74086487;     QnyDGjBvQqHykYGFs74086487 = QnyDGjBvQqHykYGFs31349235;     QnyDGjBvQqHykYGFs31349235 = QnyDGjBvQqHykYGFs5465048;     QnyDGjBvQqHykYGFs5465048 = QnyDGjBvQqHykYGFs11844580;     QnyDGjBvQqHykYGFs11844580 = QnyDGjBvQqHykYGFs77607998;     QnyDGjBvQqHykYGFs77607998 = QnyDGjBvQqHykYGFs53522125;     QnyDGjBvQqHykYGFs53522125 = QnyDGjBvQqHykYGFs58899641;     QnyDGjBvQqHykYGFs58899641 = QnyDGjBvQqHykYGFs14128994;     QnyDGjBvQqHykYGFs14128994 = QnyDGjBvQqHykYGFs36831531;     QnyDGjBvQqHykYGFs36831531 = QnyDGjBvQqHykYGFs92899350;     QnyDGjBvQqHykYGFs92899350 = QnyDGjBvQqHykYGFs73816589;     QnyDGjBvQqHykYGFs73816589 = QnyDGjBvQqHykYGFs76418685;     QnyDGjBvQqHykYGFs76418685 = QnyDGjBvQqHykYGFs28292514;     QnyDGjBvQqHykYGFs28292514 = QnyDGjBvQqHykYGFs20585027;     QnyDGjBvQqHykYGFs20585027 = QnyDGjBvQqHykYGFs48678830;     QnyDGjBvQqHykYGFs48678830 = QnyDGjBvQqHykYGFs55708485;     QnyDGjBvQqHykYGFs55708485 = QnyDGjBvQqHykYGFs98350444;     QnyDGjBvQqHykYGFs98350444 = QnyDGjBvQqHykYGFs65706503;     QnyDGjBvQqHykYGFs65706503 = QnyDGjBvQqHykYGFs49921856;     QnyDGjBvQqHykYGFs49921856 = QnyDGjBvQqHykYGFs53749922;     QnyDGjBvQqHykYGFs53749922 = QnyDGjBvQqHykYGFs6597510;     QnyDGjBvQqHykYGFs6597510 = QnyDGjBvQqHykYGFs56481989;     QnyDGjBvQqHykYGFs56481989 = QnyDGjBvQqHykYGFs99313396;     QnyDGjBvQqHykYGFs99313396 = QnyDGjBvQqHykYGFs3353391;     QnyDGjBvQqHykYGFs3353391 = QnyDGjBvQqHykYGFs51234941;     QnyDGjBvQqHykYGFs51234941 = QnyDGjBvQqHykYGFs71183769;     QnyDGjBvQqHykYGFs71183769 = QnyDGjBvQqHykYGFs65856176;     QnyDGjBvQqHykYGFs65856176 = QnyDGjBvQqHykYGFs44575161;     QnyDGjBvQqHykYGFs44575161 = QnyDGjBvQqHykYGFs98916463;     QnyDGjBvQqHykYGFs98916463 = QnyDGjBvQqHykYGFs57042495;     QnyDGjBvQqHykYGFs57042495 = QnyDGjBvQqHykYGFs84375822;     QnyDGjBvQqHykYGFs84375822 = QnyDGjBvQqHykYGFs58922135;     QnyDGjBvQqHykYGFs58922135 = QnyDGjBvQqHykYGFs25358400;     QnyDGjBvQqHykYGFs25358400 = QnyDGjBvQqHykYGFs35049132;     QnyDGjBvQqHykYGFs35049132 = QnyDGjBvQqHykYGFs64898590;     QnyDGjBvQqHykYGFs64898590 = QnyDGjBvQqHykYGFs29245217;     QnyDGjBvQqHykYGFs29245217 = QnyDGjBvQqHykYGFs33909910;     QnyDGjBvQqHykYGFs33909910 = QnyDGjBvQqHykYGFs12421373;     QnyDGjBvQqHykYGFs12421373 = QnyDGjBvQqHykYGFs41714762;     QnyDGjBvQqHykYGFs41714762 = QnyDGjBvQqHykYGFs63241139;     QnyDGjBvQqHykYGFs63241139 = QnyDGjBvQqHykYGFs98537232;     QnyDGjBvQqHykYGFs98537232 = QnyDGjBvQqHykYGFs77382384;     QnyDGjBvQqHykYGFs77382384 = QnyDGjBvQqHykYGFs82173159;     QnyDGjBvQqHykYGFs82173159 = QnyDGjBvQqHykYGFs54907535;     QnyDGjBvQqHykYGFs54907535 = QnyDGjBvQqHykYGFs32888463;     QnyDGjBvQqHykYGFs32888463 = QnyDGjBvQqHykYGFs99565639;     QnyDGjBvQqHykYGFs99565639 = QnyDGjBvQqHykYGFs95628260;     QnyDGjBvQqHykYGFs95628260 = QnyDGjBvQqHykYGFs31254683;     QnyDGjBvQqHykYGFs31254683 = QnyDGjBvQqHykYGFs55016643;     QnyDGjBvQqHykYGFs55016643 = QnyDGjBvQqHykYGFs20635867;     QnyDGjBvQqHykYGFs20635867 = QnyDGjBvQqHykYGFs94325234;     QnyDGjBvQqHykYGFs94325234 = QnyDGjBvQqHykYGFs12194115;     QnyDGjBvQqHykYGFs12194115 = QnyDGjBvQqHykYGFs33930208;     QnyDGjBvQqHykYGFs33930208 = QnyDGjBvQqHykYGFs64408458;     QnyDGjBvQqHykYGFs64408458 = QnyDGjBvQqHykYGFs47800900;     QnyDGjBvQqHykYGFs47800900 = QnyDGjBvQqHykYGFs17604498;     QnyDGjBvQqHykYGFs17604498 = QnyDGjBvQqHykYGFs32035839;     QnyDGjBvQqHykYGFs32035839 = QnyDGjBvQqHykYGFs2111658;     QnyDGjBvQqHykYGFs2111658 = QnyDGjBvQqHykYGFs60609639;     QnyDGjBvQqHykYGFs60609639 = QnyDGjBvQqHykYGFs6424230;     QnyDGjBvQqHykYGFs6424230 = QnyDGjBvQqHykYGFs87665949;     QnyDGjBvQqHykYGFs87665949 = QnyDGjBvQqHykYGFs14324480;     QnyDGjBvQqHykYGFs14324480 = QnyDGjBvQqHykYGFs15212530;     QnyDGjBvQqHykYGFs15212530 = QnyDGjBvQqHykYGFs79789036;     QnyDGjBvQqHykYGFs79789036 = QnyDGjBvQqHykYGFs8523528;     QnyDGjBvQqHykYGFs8523528 = QnyDGjBvQqHykYGFs14894454;     QnyDGjBvQqHykYGFs14894454 = QnyDGjBvQqHykYGFs51060285;     QnyDGjBvQqHykYGFs51060285 = QnyDGjBvQqHykYGFs93243381;     QnyDGjBvQqHykYGFs93243381 = QnyDGjBvQqHykYGFs55686436;     QnyDGjBvQqHykYGFs55686436 = QnyDGjBvQqHykYGFs19433614;     QnyDGjBvQqHykYGFs19433614 = QnyDGjBvQqHykYGFs21798575;     QnyDGjBvQqHykYGFs21798575 = QnyDGjBvQqHykYGFs85929072;     QnyDGjBvQqHykYGFs85929072 = QnyDGjBvQqHykYGFs23991741;     QnyDGjBvQqHykYGFs23991741 = QnyDGjBvQqHykYGFs86680716;     QnyDGjBvQqHykYGFs86680716 = QnyDGjBvQqHykYGFs55212689;     QnyDGjBvQqHykYGFs55212689 = QnyDGjBvQqHykYGFs29215125;     QnyDGjBvQqHykYGFs29215125 = QnyDGjBvQqHykYGFs74308829;     QnyDGjBvQqHykYGFs74308829 = QnyDGjBvQqHykYGFs44405862;     QnyDGjBvQqHykYGFs44405862 = QnyDGjBvQqHykYGFs70464927;     QnyDGjBvQqHykYGFs70464927 = QnyDGjBvQqHykYGFs51669302;     QnyDGjBvQqHykYGFs51669302 = QnyDGjBvQqHykYGFs75555509;     QnyDGjBvQqHykYGFs75555509 = QnyDGjBvQqHykYGFs34601493;     QnyDGjBvQqHykYGFs34601493 = QnyDGjBvQqHykYGFs89558517;     QnyDGjBvQqHykYGFs89558517 = QnyDGjBvQqHykYGFs78280597;     QnyDGjBvQqHykYGFs78280597 = QnyDGjBvQqHykYGFs62717260;     QnyDGjBvQqHykYGFs62717260 = QnyDGjBvQqHykYGFs72181708;     QnyDGjBvQqHykYGFs72181708 = QnyDGjBvQqHykYGFs24991928;     QnyDGjBvQqHykYGFs24991928 = QnyDGjBvQqHykYGFs60949942;     QnyDGjBvQqHykYGFs60949942 = QnyDGjBvQqHykYGFs87248231;     QnyDGjBvQqHykYGFs87248231 = QnyDGjBvQqHykYGFs47294092;     QnyDGjBvQqHykYGFs47294092 = QnyDGjBvQqHykYGFs97209377;     QnyDGjBvQqHykYGFs97209377 = QnyDGjBvQqHykYGFs31798253;     QnyDGjBvQqHykYGFs31798253 = QnyDGjBvQqHykYGFs51811734;     QnyDGjBvQqHykYGFs51811734 = QnyDGjBvQqHykYGFs3695474;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iSQRYDZYCdJptqmV66888777() {     double QtvJXRdmgCuUcreUk9812580 = -723706024;    double QtvJXRdmgCuUcreUk19717631 = -978272691;    double QtvJXRdmgCuUcreUk8717863 = -224492640;    double QtvJXRdmgCuUcreUk4975399 = -506880234;    double QtvJXRdmgCuUcreUk52747552 = -394424518;    double QtvJXRdmgCuUcreUk9575583 = -754272162;    double QtvJXRdmgCuUcreUk14887376 = -941030881;    double QtvJXRdmgCuUcreUk23846378 = -753315019;    double QtvJXRdmgCuUcreUk95991730 = -741799004;    double QtvJXRdmgCuUcreUk46694707 = -497724765;    double QtvJXRdmgCuUcreUk71954870 = -486024257;    double QtvJXRdmgCuUcreUk9431042 = -357195996;    double QtvJXRdmgCuUcreUk39731244 = -247533302;    double QtvJXRdmgCuUcreUk42043318 = -562012465;    double QtvJXRdmgCuUcreUk9871421 = -70717765;    double QtvJXRdmgCuUcreUk78336925 = -871846089;    double QtvJXRdmgCuUcreUk86735353 = -312101583;    double QtvJXRdmgCuUcreUk70869742 = -189270287;    double QtvJXRdmgCuUcreUk38248527 = -441839360;    double QtvJXRdmgCuUcreUk8842679 = 91211113;    double QtvJXRdmgCuUcreUk9138220 = -757143749;    double QtvJXRdmgCuUcreUk35199854 = -568373799;    double QtvJXRdmgCuUcreUk91299277 = -162808470;    double QtvJXRdmgCuUcreUk82342818 = -949879397;    double QtvJXRdmgCuUcreUk9406154 = -966944767;    double QtvJXRdmgCuUcreUk74643531 = -690865785;    double QtvJXRdmgCuUcreUk52209700 = -500902187;    double QtvJXRdmgCuUcreUk19704635 = -77001164;    double QtvJXRdmgCuUcreUk22614245 = -231168929;    double QtvJXRdmgCuUcreUk67929033 = -751524214;    double QtvJXRdmgCuUcreUk95595939 = -821005160;    double QtvJXRdmgCuUcreUk48845563 = -464980225;    double QtvJXRdmgCuUcreUk80894718 = 97390179;    double QtvJXRdmgCuUcreUk60686740 = -376251041;    double QtvJXRdmgCuUcreUk6049813 = -663019819;    double QtvJXRdmgCuUcreUk67642376 = 8134683;    double QtvJXRdmgCuUcreUk38304430 = -172636572;    double QtvJXRdmgCuUcreUk18466035 = -34008034;    double QtvJXRdmgCuUcreUk32532389 = -202751729;    double QtvJXRdmgCuUcreUk12860338 = -395198812;    double QtvJXRdmgCuUcreUk40718356 = -488093792;    double QtvJXRdmgCuUcreUk97049047 = -135338855;    double QtvJXRdmgCuUcreUk7312195 = -275927644;    double QtvJXRdmgCuUcreUk21317651 = -369977228;    double QtvJXRdmgCuUcreUk24373838 = -68167796;    double QtvJXRdmgCuUcreUk53653957 = -111339758;    double QtvJXRdmgCuUcreUk81019713 = -521243463;    double QtvJXRdmgCuUcreUk6494174 = -86122846;    double QtvJXRdmgCuUcreUk87058113 = -230570187;    double QtvJXRdmgCuUcreUk95084812 = -517714902;    double QtvJXRdmgCuUcreUk90608213 = -87006656;    double QtvJXRdmgCuUcreUk43698182 = -756743661;    double QtvJXRdmgCuUcreUk47814097 = -686416168;    double QtvJXRdmgCuUcreUk91597140 = -218803402;    double QtvJXRdmgCuUcreUk57750557 = -128946421;    double QtvJXRdmgCuUcreUk74612726 = -55332226;    double QtvJXRdmgCuUcreUk28418354 = -715464222;    double QtvJXRdmgCuUcreUk26375045 = -274613244;    double QtvJXRdmgCuUcreUk95569244 = -539935467;    double QtvJXRdmgCuUcreUk78104020 = -703558733;    double QtvJXRdmgCuUcreUk57365882 = -153369975;    double QtvJXRdmgCuUcreUk95182741 = -764029717;    double QtvJXRdmgCuUcreUk1232134 = -422146091;    double QtvJXRdmgCuUcreUk28062697 = -990274791;    double QtvJXRdmgCuUcreUk51098767 = -676719606;    double QtvJXRdmgCuUcreUk23109307 = 78955968;    double QtvJXRdmgCuUcreUk28536323 = -354586176;    double QtvJXRdmgCuUcreUk79044503 = -871282261;    double QtvJXRdmgCuUcreUk35993505 = -898992646;    double QtvJXRdmgCuUcreUk42229044 = 21147552;    double QtvJXRdmgCuUcreUk40032496 = -599209518;    double QtvJXRdmgCuUcreUk68269318 = -178093549;    double QtvJXRdmgCuUcreUk38337353 = -986518559;    double QtvJXRdmgCuUcreUk25388190 = 53359452;    double QtvJXRdmgCuUcreUk68124322 = -420695095;    double QtvJXRdmgCuUcreUk12089173 = -521804895;    double QtvJXRdmgCuUcreUk27887659 = -192446155;    double QtvJXRdmgCuUcreUk69981626 = -792831243;    double QtvJXRdmgCuUcreUk57968981 = -781711601;    double QtvJXRdmgCuUcreUk55752196 = -755605010;    double QtvJXRdmgCuUcreUk93623817 = -69622322;    double QtvJXRdmgCuUcreUk45715526 = -314779341;    double QtvJXRdmgCuUcreUk32646521 = -846430977;    double QtvJXRdmgCuUcreUk27529432 = -713454028;    double QtvJXRdmgCuUcreUk77320820 = -564517558;    double QtvJXRdmgCuUcreUk51897757 = 35738501;    double QtvJXRdmgCuUcreUk1031467 = -778564058;    double QtvJXRdmgCuUcreUk89297577 = -683806419;    double QtvJXRdmgCuUcreUk2936183 = -147304621;    double QtvJXRdmgCuUcreUk31437086 = -507687593;    double QtvJXRdmgCuUcreUk39224023 = -276401096;    double QtvJXRdmgCuUcreUk11929385 = -898023328;    double QtvJXRdmgCuUcreUk22896790 = -494072568;    double QtvJXRdmgCuUcreUk54428368 = -499192996;    double QtvJXRdmgCuUcreUk55494455 = -141828837;    double QtvJXRdmgCuUcreUk45535615 = -724064075;    double QtvJXRdmgCuUcreUk95816913 = -713192765;    double QtvJXRdmgCuUcreUk79249497 = -285652854;    double QtvJXRdmgCuUcreUk70218883 = -693257622;    double QtvJXRdmgCuUcreUk1264531 = -723706024;     QtvJXRdmgCuUcreUk9812580 = QtvJXRdmgCuUcreUk19717631;     QtvJXRdmgCuUcreUk19717631 = QtvJXRdmgCuUcreUk8717863;     QtvJXRdmgCuUcreUk8717863 = QtvJXRdmgCuUcreUk4975399;     QtvJXRdmgCuUcreUk4975399 = QtvJXRdmgCuUcreUk52747552;     QtvJXRdmgCuUcreUk52747552 = QtvJXRdmgCuUcreUk9575583;     QtvJXRdmgCuUcreUk9575583 = QtvJXRdmgCuUcreUk14887376;     QtvJXRdmgCuUcreUk14887376 = QtvJXRdmgCuUcreUk23846378;     QtvJXRdmgCuUcreUk23846378 = QtvJXRdmgCuUcreUk95991730;     QtvJXRdmgCuUcreUk95991730 = QtvJXRdmgCuUcreUk46694707;     QtvJXRdmgCuUcreUk46694707 = QtvJXRdmgCuUcreUk71954870;     QtvJXRdmgCuUcreUk71954870 = QtvJXRdmgCuUcreUk9431042;     QtvJXRdmgCuUcreUk9431042 = QtvJXRdmgCuUcreUk39731244;     QtvJXRdmgCuUcreUk39731244 = QtvJXRdmgCuUcreUk42043318;     QtvJXRdmgCuUcreUk42043318 = QtvJXRdmgCuUcreUk9871421;     QtvJXRdmgCuUcreUk9871421 = QtvJXRdmgCuUcreUk78336925;     QtvJXRdmgCuUcreUk78336925 = QtvJXRdmgCuUcreUk86735353;     QtvJXRdmgCuUcreUk86735353 = QtvJXRdmgCuUcreUk70869742;     QtvJXRdmgCuUcreUk70869742 = QtvJXRdmgCuUcreUk38248527;     QtvJXRdmgCuUcreUk38248527 = QtvJXRdmgCuUcreUk8842679;     QtvJXRdmgCuUcreUk8842679 = QtvJXRdmgCuUcreUk9138220;     QtvJXRdmgCuUcreUk9138220 = QtvJXRdmgCuUcreUk35199854;     QtvJXRdmgCuUcreUk35199854 = QtvJXRdmgCuUcreUk91299277;     QtvJXRdmgCuUcreUk91299277 = QtvJXRdmgCuUcreUk82342818;     QtvJXRdmgCuUcreUk82342818 = QtvJXRdmgCuUcreUk9406154;     QtvJXRdmgCuUcreUk9406154 = QtvJXRdmgCuUcreUk74643531;     QtvJXRdmgCuUcreUk74643531 = QtvJXRdmgCuUcreUk52209700;     QtvJXRdmgCuUcreUk52209700 = QtvJXRdmgCuUcreUk19704635;     QtvJXRdmgCuUcreUk19704635 = QtvJXRdmgCuUcreUk22614245;     QtvJXRdmgCuUcreUk22614245 = QtvJXRdmgCuUcreUk67929033;     QtvJXRdmgCuUcreUk67929033 = QtvJXRdmgCuUcreUk95595939;     QtvJXRdmgCuUcreUk95595939 = QtvJXRdmgCuUcreUk48845563;     QtvJXRdmgCuUcreUk48845563 = QtvJXRdmgCuUcreUk80894718;     QtvJXRdmgCuUcreUk80894718 = QtvJXRdmgCuUcreUk60686740;     QtvJXRdmgCuUcreUk60686740 = QtvJXRdmgCuUcreUk6049813;     QtvJXRdmgCuUcreUk6049813 = QtvJXRdmgCuUcreUk67642376;     QtvJXRdmgCuUcreUk67642376 = QtvJXRdmgCuUcreUk38304430;     QtvJXRdmgCuUcreUk38304430 = QtvJXRdmgCuUcreUk18466035;     QtvJXRdmgCuUcreUk18466035 = QtvJXRdmgCuUcreUk32532389;     QtvJXRdmgCuUcreUk32532389 = QtvJXRdmgCuUcreUk12860338;     QtvJXRdmgCuUcreUk12860338 = QtvJXRdmgCuUcreUk40718356;     QtvJXRdmgCuUcreUk40718356 = QtvJXRdmgCuUcreUk97049047;     QtvJXRdmgCuUcreUk97049047 = QtvJXRdmgCuUcreUk7312195;     QtvJXRdmgCuUcreUk7312195 = QtvJXRdmgCuUcreUk21317651;     QtvJXRdmgCuUcreUk21317651 = QtvJXRdmgCuUcreUk24373838;     QtvJXRdmgCuUcreUk24373838 = QtvJXRdmgCuUcreUk53653957;     QtvJXRdmgCuUcreUk53653957 = QtvJXRdmgCuUcreUk81019713;     QtvJXRdmgCuUcreUk81019713 = QtvJXRdmgCuUcreUk6494174;     QtvJXRdmgCuUcreUk6494174 = QtvJXRdmgCuUcreUk87058113;     QtvJXRdmgCuUcreUk87058113 = QtvJXRdmgCuUcreUk95084812;     QtvJXRdmgCuUcreUk95084812 = QtvJXRdmgCuUcreUk90608213;     QtvJXRdmgCuUcreUk90608213 = QtvJXRdmgCuUcreUk43698182;     QtvJXRdmgCuUcreUk43698182 = QtvJXRdmgCuUcreUk47814097;     QtvJXRdmgCuUcreUk47814097 = QtvJXRdmgCuUcreUk91597140;     QtvJXRdmgCuUcreUk91597140 = QtvJXRdmgCuUcreUk57750557;     QtvJXRdmgCuUcreUk57750557 = QtvJXRdmgCuUcreUk74612726;     QtvJXRdmgCuUcreUk74612726 = QtvJXRdmgCuUcreUk28418354;     QtvJXRdmgCuUcreUk28418354 = QtvJXRdmgCuUcreUk26375045;     QtvJXRdmgCuUcreUk26375045 = QtvJXRdmgCuUcreUk95569244;     QtvJXRdmgCuUcreUk95569244 = QtvJXRdmgCuUcreUk78104020;     QtvJXRdmgCuUcreUk78104020 = QtvJXRdmgCuUcreUk57365882;     QtvJXRdmgCuUcreUk57365882 = QtvJXRdmgCuUcreUk95182741;     QtvJXRdmgCuUcreUk95182741 = QtvJXRdmgCuUcreUk1232134;     QtvJXRdmgCuUcreUk1232134 = QtvJXRdmgCuUcreUk28062697;     QtvJXRdmgCuUcreUk28062697 = QtvJXRdmgCuUcreUk51098767;     QtvJXRdmgCuUcreUk51098767 = QtvJXRdmgCuUcreUk23109307;     QtvJXRdmgCuUcreUk23109307 = QtvJXRdmgCuUcreUk28536323;     QtvJXRdmgCuUcreUk28536323 = QtvJXRdmgCuUcreUk79044503;     QtvJXRdmgCuUcreUk79044503 = QtvJXRdmgCuUcreUk35993505;     QtvJXRdmgCuUcreUk35993505 = QtvJXRdmgCuUcreUk42229044;     QtvJXRdmgCuUcreUk42229044 = QtvJXRdmgCuUcreUk40032496;     QtvJXRdmgCuUcreUk40032496 = QtvJXRdmgCuUcreUk68269318;     QtvJXRdmgCuUcreUk68269318 = QtvJXRdmgCuUcreUk38337353;     QtvJXRdmgCuUcreUk38337353 = QtvJXRdmgCuUcreUk25388190;     QtvJXRdmgCuUcreUk25388190 = QtvJXRdmgCuUcreUk68124322;     QtvJXRdmgCuUcreUk68124322 = QtvJXRdmgCuUcreUk12089173;     QtvJXRdmgCuUcreUk12089173 = QtvJXRdmgCuUcreUk27887659;     QtvJXRdmgCuUcreUk27887659 = QtvJXRdmgCuUcreUk69981626;     QtvJXRdmgCuUcreUk69981626 = QtvJXRdmgCuUcreUk57968981;     QtvJXRdmgCuUcreUk57968981 = QtvJXRdmgCuUcreUk55752196;     QtvJXRdmgCuUcreUk55752196 = QtvJXRdmgCuUcreUk93623817;     QtvJXRdmgCuUcreUk93623817 = QtvJXRdmgCuUcreUk45715526;     QtvJXRdmgCuUcreUk45715526 = QtvJXRdmgCuUcreUk32646521;     QtvJXRdmgCuUcreUk32646521 = QtvJXRdmgCuUcreUk27529432;     QtvJXRdmgCuUcreUk27529432 = QtvJXRdmgCuUcreUk77320820;     QtvJXRdmgCuUcreUk77320820 = QtvJXRdmgCuUcreUk51897757;     QtvJXRdmgCuUcreUk51897757 = QtvJXRdmgCuUcreUk1031467;     QtvJXRdmgCuUcreUk1031467 = QtvJXRdmgCuUcreUk89297577;     QtvJXRdmgCuUcreUk89297577 = QtvJXRdmgCuUcreUk2936183;     QtvJXRdmgCuUcreUk2936183 = QtvJXRdmgCuUcreUk31437086;     QtvJXRdmgCuUcreUk31437086 = QtvJXRdmgCuUcreUk39224023;     QtvJXRdmgCuUcreUk39224023 = QtvJXRdmgCuUcreUk11929385;     QtvJXRdmgCuUcreUk11929385 = QtvJXRdmgCuUcreUk22896790;     QtvJXRdmgCuUcreUk22896790 = QtvJXRdmgCuUcreUk54428368;     QtvJXRdmgCuUcreUk54428368 = QtvJXRdmgCuUcreUk55494455;     QtvJXRdmgCuUcreUk55494455 = QtvJXRdmgCuUcreUk45535615;     QtvJXRdmgCuUcreUk45535615 = QtvJXRdmgCuUcreUk95816913;     QtvJXRdmgCuUcreUk95816913 = QtvJXRdmgCuUcreUk79249497;     QtvJXRdmgCuUcreUk79249497 = QtvJXRdmgCuUcreUk70218883;     QtvJXRdmgCuUcreUk70218883 = QtvJXRdmgCuUcreUk1264531;     QtvJXRdmgCuUcreUk1264531 = QtvJXRdmgCuUcreUk9812580;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uJNZughOFzjSpren34180376() {     double HMGaRsTQXHYkisWPf45271549 = -89982924;    double HMGaRsTQXHYkisWPf32989710 = -858824298;    double HMGaRsTQXHYkisWPf46104339 = -990159715;    double HMGaRsTQXHYkisWPf77198753 = -328031305;    double HMGaRsTQXHYkisWPf9032821 = -500023363;    double HMGaRsTQXHYkisWPf45798454 = 83379943;    double HMGaRsTQXHYkisWPf25544782 = -785756615;    double HMGaRsTQXHYkisWPf85977939 = -397549160;    double HMGaRsTQXHYkisWPf97509335 = -130130411;    double HMGaRsTQXHYkisWPf61597419 = -162411763;    double HMGaRsTQXHYkisWPf10136605 = -288031986;    double HMGaRsTQXHYkisWPf9999190 = -221316768;    double HMGaRsTQXHYkisWPf77965889 = -799545807;    double HMGaRsTQXHYkisWPf95184180 = -538375874;    double HMGaRsTQXHYkisWPf68065936 = -574142780;    double HMGaRsTQXHYkisWPf61878226 = -507657696;    double HMGaRsTQXHYkisWPf42571289 = -839076135;    double HMGaRsTQXHYkisWPf70629934 = -262698355;    double HMGaRsTQXHYkisWPf38798857 = -758402781;    double HMGaRsTQXHYkisWPf2679897 = -335349952;    double HMGaRsTQXHYkisWPf22225083 = -414418438;    double HMGaRsTQXHYkisWPf38621378 = -655010093;    double HMGaRsTQXHYkisWPf61824657 = -900440654;    double HMGaRsTQXHYkisWPf76135763 = -360704001;    double HMGaRsTQXHYkisWPf17424105 = -990929966;    double HMGaRsTQXHYkisWPf85341702 = -379442592;    double HMGaRsTQXHYkisWPf12630275 = -307186328;    double HMGaRsTQXHYkisWPf60433059 = -246773338;    double HMGaRsTQXHYkisWPf69282459 = -805263731;    double HMGaRsTQXHYkisWPf73462444 = -786031384;    double HMGaRsTQXHYkisWPf14179172 = -272985647;    double HMGaRsTQXHYkisWPf86668581 = -237951640;    double HMGaRsTQXHYkisWPf93112728 = -50205798;    double HMGaRsTQXHYkisWPf90435166 = -440944855;    double HMGaRsTQXHYkisWPf71858266 = -614120069;    double HMGaRsTQXHYkisWPf19203221 = -206430623;    double HMGaRsTQXHYkisWPf87716246 = -37332612;    double HMGaRsTQXHYkisWPf69681438 = -16663615;    double HMGaRsTQXHYkisWPf62922950 = -953628005;    double HMGaRsTQXHYkisWPf77131869 = -960461358;    double HMGaRsTQXHYkisWPf74169963 = -257292706;    double HMGaRsTQXHYkisWPf86414174 = -829884146;    double HMGaRsTQXHYkisWPf96695572 = -929108430;    double HMGaRsTQXHYkisWPf61448989 = -974309638;    double HMGaRsTQXHYkisWPf24390296 = -994678958;    double HMGaRsTQXHYkisWPf99615285 = -150263390;    double HMGaRsTQXHYkisWPf364878 = 94770176;    double HMGaRsTQXHYkisWPf24220486 = -252649439;    double HMGaRsTQXHYkisWPf9950780 = -994307067;    double HMGaRsTQXHYkisWPf19614402 = 91258282;    double HMGaRsTQXHYkisWPf71378937 = 38716787;    double HMGaRsTQXHYkisWPf27183655 = -970396008;    double HMGaRsTQXHYkisWPf16435246 = -442461722;    double HMGaRsTQXHYkisWPf7588988 = -410311902;    double HMGaRsTQXHYkisWPf11955294 = -475436567;    double HMGaRsTQXHYkisWPf6650171 = -434972832;    double HMGaRsTQXHYkisWPf71165053 = -958383645;    double HMGaRsTQXHYkisWPf69968576 = -529455715;    double HMGaRsTQXHYkisWPf59774648 = -337101340;    double HMGaRsTQXHYkisWPf23691119 = -20580771;    double HMGaRsTQXHYkisWPf33168180 = -609433730;    double HMGaRsTQXHYkisWPf65111722 = -438983277;    double HMGaRsTQXHYkisWPf16695480 = -592285429;    double HMGaRsTQXHYkisWPf24046892 = -344099027;    double HMGaRsTQXHYkisWPf47418248 = -889426117;    double HMGaRsTQXHYkisWPf23468024 = 49919653;    double HMGaRsTQXHYkisWPf16886461 = -71110970;    double HMGaRsTQXHYkisWPf87530722 = -258600953;    double HMGaRsTQXHYkisWPf23325915 = -924255805;    double HMGaRsTQXHYkisWPf48862716 = -267712157;    double HMGaRsTQXHYkisWPf74161980 = -370325084;    double HMGaRsTQXHYkisWPf72889851 = -722412520;    double HMGaRsTQXHYkisWPf7706984 = -309070351;    double HMGaRsTQXHYkisWPf61666987 = -797941424;    double HMGaRsTQXHYkisWPf28509934 = 21942753;    double HMGaRsTQXHYkisWPf35810909 = -584534293;    double HMGaRsTQXHYkisWPf41925805 = -725901663;    double HMGaRsTQXHYkisWPf375669 = -926131017;    double HMGaRsTQXHYkisWPf51745467 = -366025044;    double HMGaRsTQXHYkisWPf17808819 = -740666576;    double HMGaRsTQXHYkisWPf84976825 = -374212768;    double HMGaRsTQXHYkisWPf88409788 = 45463111;    double HMGaRsTQXHYkisWPf50482280 = -252466271;    double HMGaRsTQXHYkisWPf49668058 = -796522014;    double HMGaRsTQXHYkisWPf2083508 = -724748171;    double HMGaRsTQXHYkisWPf86995516 = -302589640;    double HMGaRsTQXHYkisWPf70233336 = -795489918;    double HMGaRsTQXHYkisWPf85523741 = -639893897;    double HMGaRsTQXHYkisWPf78479872 = -965508288;    double HMGaRsTQXHYkisWPf65208096 = -79147238;    double HMGaRsTQXHYkisWPf48038168 = -248046979;    double HMGaRsTQXHYkisWPf17747670 = -507876898;    double HMGaRsTQXHYkisWPf9906790 = -679562276;    double HMGaRsTQXHYkisWPf39231832 = -833047234;    double HMGaRsTQXHYkisWPf43963690 = -251027629;    double HMGaRsTQXHYkisWPf9058242 = -818309429;    double HMGaRsTQXHYkisWPf69718694 = -137598717;    double HMGaRsTQXHYkisWPf72648681 = -485009404;    double HMGaRsTQXHYkisWPf14030741 = 15116479;    double HMGaRsTQXHYkisWPf922273 = -89982924;     HMGaRsTQXHYkisWPf45271549 = HMGaRsTQXHYkisWPf32989710;     HMGaRsTQXHYkisWPf32989710 = HMGaRsTQXHYkisWPf46104339;     HMGaRsTQXHYkisWPf46104339 = HMGaRsTQXHYkisWPf77198753;     HMGaRsTQXHYkisWPf77198753 = HMGaRsTQXHYkisWPf9032821;     HMGaRsTQXHYkisWPf9032821 = HMGaRsTQXHYkisWPf45798454;     HMGaRsTQXHYkisWPf45798454 = HMGaRsTQXHYkisWPf25544782;     HMGaRsTQXHYkisWPf25544782 = HMGaRsTQXHYkisWPf85977939;     HMGaRsTQXHYkisWPf85977939 = HMGaRsTQXHYkisWPf97509335;     HMGaRsTQXHYkisWPf97509335 = HMGaRsTQXHYkisWPf61597419;     HMGaRsTQXHYkisWPf61597419 = HMGaRsTQXHYkisWPf10136605;     HMGaRsTQXHYkisWPf10136605 = HMGaRsTQXHYkisWPf9999190;     HMGaRsTQXHYkisWPf9999190 = HMGaRsTQXHYkisWPf77965889;     HMGaRsTQXHYkisWPf77965889 = HMGaRsTQXHYkisWPf95184180;     HMGaRsTQXHYkisWPf95184180 = HMGaRsTQXHYkisWPf68065936;     HMGaRsTQXHYkisWPf68065936 = HMGaRsTQXHYkisWPf61878226;     HMGaRsTQXHYkisWPf61878226 = HMGaRsTQXHYkisWPf42571289;     HMGaRsTQXHYkisWPf42571289 = HMGaRsTQXHYkisWPf70629934;     HMGaRsTQXHYkisWPf70629934 = HMGaRsTQXHYkisWPf38798857;     HMGaRsTQXHYkisWPf38798857 = HMGaRsTQXHYkisWPf2679897;     HMGaRsTQXHYkisWPf2679897 = HMGaRsTQXHYkisWPf22225083;     HMGaRsTQXHYkisWPf22225083 = HMGaRsTQXHYkisWPf38621378;     HMGaRsTQXHYkisWPf38621378 = HMGaRsTQXHYkisWPf61824657;     HMGaRsTQXHYkisWPf61824657 = HMGaRsTQXHYkisWPf76135763;     HMGaRsTQXHYkisWPf76135763 = HMGaRsTQXHYkisWPf17424105;     HMGaRsTQXHYkisWPf17424105 = HMGaRsTQXHYkisWPf85341702;     HMGaRsTQXHYkisWPf85341702 = HMGaRsTQXHYkisWPf12630275;     HMGaRsTQXHYkisWPf12630275 = HMGaRsTQXHYkisWPf60433059;     HMGaRsTQXHYkisWPf60433059 = HMGaRsTQXHYkisWPf69282459;     HMGaRsTQXHYkisWPf69282459 = HMGaRsTQXHYkisWPf73462444;     HMGaRsTQXHYkisWPf73462444 = HMGaRsTQXHYkisWPf14179172;     HMGaRsTQXHYkisWPf14179172 = HMGaRsTQXHYkisWPf86668581;     HMGaRsTQXHYkisWPf86668581 = HMGaRsTQXHYkisWPf93112728;     HMGaRsTQXHYkisWPf93112728 = HMGaRsTQXHYkisWPf90435166;     HMGaRsTQXHYkisWPf90435166 = HMGaRsTQXHYkisWPf71858266;     HMGaRsTQXHYkisWPf71858266 = HMGaRsTQXHYkisWPf19203221;     HMGaRsTQXHYkisWPf19203221 = HMGaRsTQXHYkisWPf87716246;     HMGaRsTQXHYkisWPf87716246 = HMGaRsTQXHYkisWPf69681438;     HMGaRsTQXHYkisWPf69681438 = HMGaRsTQXHYkisWPf62922950;     HMGaRsTQXHYkisWPf62922950 = HMGaRsTQXHYkisWPf77131869;     HMGaRsTQXHYkisWPf77131869 = HMGaRsTQXHYkisWPf74169963;     HMGaRsTQXHYkisWPf74169963 = HMGaRsTQXHYkisWPf86414174;     HMGaRsTQXHYkisWPf86414174 = HMGaRsTQXHYkisWPf96695572;     HMGaRsTQXHYkisWPf96695572 = HMGaRsTQXHYkisWPf61448989;     HMGaRsTQXHYkisWPf61448989 = HMGaRsTQXHYkisWPf24390296;     HMGaRsTQXHYkisWPf24390296 = HMGaRsTQXHYkisWPf99615285;     HMGaRsTQXHYkisWPf99615285 = HMGaRsTQXHYkisWPf364878;     HMGaRsTQXHYkisWPf364878 = HMGaRsTQXHYkisWPf24220486;     HMGaRsTQXHYkisWPf24220486 = HMGaRsTQXHYkisWPf9950780;     HMGaRsTQXHYkisWPf9950780 = HMGaRsTQXHYkisWPf19614402;     HMGaRsTQXHYkisWPf19614402 = HMGaRsTQXHYkisWPf71378937;     HMGaRsTQXHYkisWPf71378937 = HMGaRsTQXHYkisWPf27183655;     HMGaRsTQXHYkisWPf27183655 = HMGaRsTQXHYkisWPf16435246;     HMGaRsTQXHYkisWPf16435246 = HMGaRsTQXHYkisWPf7588988;     HMGaRsTQXHYkisWPf7588988 = HMGaRsTQXHYkisWPf11955294;     HMGaRsTQXHYkisWPf11955294 = HMGaRsTQXHYkisWPf6650171;     HMGaRsTQXHYkisWPf6650171 = HMGaRsTQXHYkisWPf71165053;     HMGaRsTQXHYkisWPf71165053 = HMGaRsTQXHYkisWPf69968576;     HMGaRsTQXHYkisWPf69968576 = HMGaRsTQXHYkisWPf59774648;     HMGaRsTQXHYkisWPf59774648 = HMGaRsTQXHYkisWPf23691119;     HMGaRsTQXHYkisWPf23691119 = HMGaRsTQXHYkisWPf33168180;     HMGaRsTQXHYkisWPf33168180 = HMGaRsTQXHYkisWPf65111722;     HMGaRsTQXHYkisWPf65111722 = HMGaRsTQXHYkisWPf16695480;     HMGaRsTQXHYkisWPf16695480 = HMGaRsTQXHYkisWPf24046892;     HMGaRsTQXHYkisWPf24046892 = HMGaRsTQXHYkisWPf47418248;     HMGaRsTQXHYkisWPf47418248 = HMGaRsTQXHYkisWPf23468024;     HMGaRsTQXHYkisWPf23468024 = HMGaRsTQXHYkisWPf16886461;     HMGaRsTQXHYkisWPf16886461 = HMGaRsTQXHYkisWPf87530722;     HMGaRsTQXHYkisWPf87530722 = HMGaRsTQXHYkisWPf23325915;     HMGaRsTQXHYkisWPf23325915 = HMGaRsTQXHYkisWPf48862716;     HMGaRsTQXHYkisWPf48862716 = HMGaRsTQXHYkisWPf74161980;     HMGaRsTQXHYkisWPf74161980 = HMGaRsTQXHYkisWPf72889851;     HMGaRsTQXHYkisWPf72889851 = HMGaRsTQXHYkisWPf7706984;     HMGaRsTQXHYkisWPf7706984 = HMGaRsTQXHYkisWPf61666987;     HMGaRsTQXHYkisWPf61666987 = HMGaRsTQXHYkisWPf28509934;     HMGaRsTQXHYkisWPf28509934 = HMGaRsTQXHYkisWPf35810909;     HMGaRsTQXHYkisWPf35810909 = HMGaRsTQXHYkisWPf41925805;     HMGaRsTQXHYkisWPf41925805 = HMGaRsTQXHYkisWPf375669;     HMGaRsTQXHYkisWPf375669 = HMGaRsTQXHYkisWPf51745467;     HMGaRsTQXHYkisWPf51745467 = HMGaRsTQXHYkisWPf17808819;     HMGaRsTQXHYkisWPf17808819 = HMGaRsTQXHYkisWPf84976825;     HMGaRsTQXHYkisWPf84976825 = HMGaRsTQXHYkisWPf88409788;     HMGaRsTQXHYkisWPf88409788 = HMGaRsTQXHYkisWPf50482280;     HMGaRsTQXHYkisWPf50482280 = HMGaRsTQXHYkisWPf49668058;     HMGaRsTQXHYkisWPf49668058 = HMGaRsTQXHYkisWPf2083508;     HMGaRsTQXHYkisWPf2083508 = HMGaRsTQXHYkisWPf86995516;     HMGaRsTQXHYkisWPf86995516 = HMGaRsTQXHYkisWPf70233336;     HMGaRsTQXHYkisWPf70233336 = HMGaRsTQXHYkisWPf85523741;     HMGaRsTQXHYkisWPf85523741 = HMGaRsTQXHYkisWPf78479872;     HMGaRsTQXHYkisWPf78479872 = HMGaRsTQXHYkisWPf65208096;     HMGaRsTQXHYkisWPf65208096 = HMGaRsTQXHYkisWPf48038168;     HMGaRsTQXHYkisWPf48038168 = HMGaRsTQXHYkisWPf17747670;     HMGaRsTQXHYkisWPf17747670 = HMGaRsTQXHYkisWPf9906790;     HMGaRsTQXHYkisWPf9906790 = HMGaRsTQXHYkisWPf39231832;     HMGaRsTQXHYkisWPf39231832 = HMGaRsTQXHYkisWPf43963690;     HMGaRsTQXHYkisWPf43963690 = HMGaRsTQXHYkisWPf9058242;     HMGaRsTQXHYkisWPf9058242 = HMGaRsTQXHYkisWPf69718694;     HMGaRsTQXHYkisWPf69718694 = HMGaRsTQXHYkisWPf72648681;     HMGaRsTQXHYkisWPf72648681 = HMGaRsTQXHYkisWPf14030741;     HMGaRsTQXHYkisWPf14030741 = HMGaRsTQXHYkisWPf922273;     HMGaRsTQXHYkisWPf922273 = HMGaRsTQXHYkisWPf45271549;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mRxEsuqmkGFOUUDc49229444() {     double gQIqekKNNgyaNZPyb51388655 = -201884813;    double gQIqekKNNgyaNZPyb76362989 = -568424079;    double gQIqekKNNgyaNZPyb62146523 = -40992636;    double gQIqekKNNgyaNZPyb4273535 = -518779935;    double gQIqekKNNgyaNZPyb77928309 = 79326766;    double gQIqekKNNgyaNZPyb37215657 = -181116292;    double gQIqekKNNgyaNZPyb86033747 = -924721592;    double gQIqekKNNgyaNZPyb35737830 = -684423073;    double gQIqekKNNgyaNZPyb62151831 = -998163077;    double gQIqekKNNgyaNZPyb2827079 = -491387240;    double gQIqekKNNgyaNZPyb70246895 = -165469573;    double gQIqekKNNgyaNZPyb41822232 = -665722970;    double gQIqekKNNgyaNZPyb64175007 = -139436896;    double gQIqekKNNgyaNZPyb78327857 = 29385558;    double gQIqekKNNgyaNZPyb63808363 = -164604946;    double gQIqekKNNgyaNZPyb3383620 = -480657239;    double gQIqekKNNgyaNZPyb36407293 = -355621248;    double gQIqekKNNgyaNZPyb67683087 = 71779002;    double gQIqekKNNgyaNZPyb628699 = -64171878;    double gQIqekKNNgyaNZPyb83230061 = -692252872;    double gQIqekKNNgyaNZPyb10778276 = -269905893;    double gQIqekKNNgyaNZPyb25142402 = -562939809;    double gQIqekKNNgyaNZPyb97415448 = -646958141;    double gQIqekKNNgyaNZPyb60128137 = -583008989;    double gQIqekKNNgyaNZPyb61123755 = -264125999;    double gQIqekKNNgyaNZPyb10063378 = -258097915;    double gQIqekKNNgyaNZPyb11090053 = -207848531;    double gQIqekKNNgyaNZPyb73540185 = -312702292;    double gQIqekKNNgyaNZPyb35414715 = 72353564;    double gQIqekKNNgyaNZPyb42078081 = -603502252;    double gQIqekKNNgyaNZPyb6421721 = -186306585;    double gQIqekKNNgyaNZPyb84279203 = 52601545;    double gQIqekKNNgyaNZPyb2823678 = 57180832;    double gQIqekKNNgyaNZPyb85265730 = -192486620;    double gQIqekKNNgyaNZPyb33332919 = -152853175;    double gQIqekKNNgyaNZPyb87929133 = -420456731;    double gQIqekKNNgyaNZPyb68978180 = 9031519;    double gQIqekKNNgyaNZPyb3771651 = -888169625;    double gQIqekKNNgyaNZPyb36533204 = -947551502;    double gQIqekKNNgyaNZPyb64633807 = -333900581;    double gQIqekKNNgyaNZPyb79839187 = -322566600;    double gQIqekKNNgyaNZPyb18564632 = -26424;    double gQIqekKNNgyaNZPyb74762551 = 90086986;    double gQIqekKNNgyaNZPyb48856729 = -993652576;    double gQIqekKNNgyaNZPyb36342761 = -656577112;    double gQIqekKNNgyaNZPyb11554481 = -135479507;    double gQIqekKNNgyaNZPyb18143452 = -24005362;    double gQIqekKNNgyaNZPyb32177427 = -262544972;    double gQIqekKNNgyaNZPyb19626509 = 19955945;    double gQIqekKNNgyaNZPyb32526054 = 4247474;    double gQIqekKNNgyaNZPyb7079616 = -903576885;    double gQIqekKNNgyaNZPyb37993374 = -771122328;    double gQIqekKNNgyaNZPyb64683702 = -234251310;    double gQIqekKNNgyaNZPyb3557868 = -730782047;    double gQIqekKNNgyaNZPyb38451168 = -75968349;    double gQIqekKNNgyaNZPyb26246254 = -638945004;    double gQIqekKNNgyaNZPyb78947540 = -921465938;    double gQIqekKNNgyaNZPyb2018386 = -457983648;    double gQIqekKNNgyaNZPyb43149779 = -154653937;    double gQIqekKNNgyaNZPyb67864931 = -662575319;    double gQIqekKNNgyaNZPyb26125604 = -973267762;    double gQIqekKNNgyaNZPyb12493563 = -512019301;    double gQIqekKNNgyaNZPyb323116 = -656776637;    double gQIqekKNNgyaNZPyb20073750 = -294660826;    double gQIqekKNNgyaNZPyb96405357 = -205080656;    double gQIqekKNNgyaNZPyb85967692 = -118071118;    double gQIqekKNNgyaNZPyb38998555 = -622903802;    double gQIqekKNNgyaNZPyb78909276 = -946950276;    double gQIqekKNNgyaNZPyb44994939 = -817761268;    double gQIqekKNNgyaNZPyb75879229 = -744148216;    double gQIqekKNNgyaNZPyb34405439 = -389688758;    double gQIqekKNNgyaNZPyb32635642 = -467451624;    double gQIqekKNNgyaNZPyb31149883 = 19330503;    double gQIqekKNNgyaNZPyb35994891 = -730271298;    double gQIqekKNNgyaNZPyb3390875 = -269686272;    double gQIqekKNNgyaNZPyb92213644 = -169879470;    double gQIqekKNNgyaNZPyb50379851 = -553026796;    double gQIqekKNNgyaNZPyb48558719 = -653305566;    double gQIqekKNNgyaNZPyb23785377 = -926431877;    double gQIqekKNNgyaNZPyb49569274 = -28646492;    double gQIqekKNNgyaNZPyb91919926 = -134092554;    double gQIqekKNNgyaNZPyb78912625 = -945303559;    double gQIqekKNNgyaNZPyb53913676 = -232658237;    double gQIqekKNNgyaNZPyb2888661 = -931893911;    double gQIqekKNNgyaNZPyb34998466 = -699925368;    double gQIqekKNNgyaNZPyb68428346 = -415184258;    double gQIqekKNNgyaNZPyb19595501 = -713147146;    double gQIqekKNNgyaNZPyb99265810 = -212037122;    double gQIqekKNNgyaNZPyb46814562 = -16518271;    double gQIqekKNNgyaNZPyb7086666 = -513908171;    double gQIqekKNNgyaNZPyb8981594 = -498990793;    double gQIqekKNNgyaNZPyb66959795 = -532984834;    double gQIqekKNNgyaNZPyb60621871 = -633515688;    double gQIqekKNNgyaNZPyb68668273 = -184976183;    double gQIqekKNNgyaNZPyb38508203 = -360632820;    double gQIqekKNNgyaNZPyb67345624 = -810547300;    double gQIqekKNNgyaNZPyb18241516 = -343249787;    double gQIqekKNNgyaNZPyb54688801 = -615252189;    double gQIqekKNNgyaNZPyb52451371 = -688571920;    double gQIqekKNNgyaNZPyb50375069 = -201884813;     gQIqekKNNgyaNZPyb51388655 = gQIqekKNNgyaNZPyb76362989;     gQIqekKNNgyaNZPyb76362989 = gQIqekKNNgyaNZPyb62146523;     gQIqekKNNgyaNZPyb62146523 = gQIqekKNNgyaNZPyb4273535;     gQIqekKNNgyaNZPyb4273535 = gQIqekKNNgyaNZPyb77928309;     gQIqekKNNgyaNZPyb77928309 = gQIqekKNNgyaNZPyb37215657;     gQIqekKNNgyaNZPyb37215657 = gQIqekKNNgyaNZPyb86033747;     gQIqekKNNgyaNZPyb86033747 = gQIqekKNNgyaNZPyb35737830;     gQIqekKNNgyaNZPyb35737830 = gQIqekKNNgyaNZPyb62151831;     gQIqekKNNgyaNZPyb62151831 = gQIqekKNNgyaNZPyb2827079;     gQIqekKNNgyaNZPyb2827079 = gQIqekKNNgyaNZPyb70246895;     gQIqekKNNgyaNZPyb70246895 = gQIqekKNNgyaNZPyb41822232;     gQIqekKNNgyaNZPyb41822232 = gQIqekKNNgyaNZPyb64175007;     gQIqekKNNgyaNZPyb64175007 = gQIqekKNNgyaNZPyb78327857;     gQIqekKNNgyaNZPyb78327857 = gQIqekKNNgyaNZPyb63808363;     gQIqekKNNgyaNZPyb63808363 = gQIqekKNNgyaNZPyb3383620;     gQIqekKNNgyaNZPyb3383620 = gQIqekKNNgyaNZPyb36407293;     gQIqekKNNgyaNZPyb36407293 = gQIqekKNNgyaNZPyb67683087;     gQIqekKNNgyaNZPyb67683087 = gQIqekKNNgyaNZPyb628699;     gQIqekKNNgyaNZPyb628699 = gQIqekKNNgyaNZPyb83230061;     gQIqekKNNgyaNZPyb83230061 = gQIqekKNNgyaNZPyb10778276;     gQIqekKNNgyaNZPyb10778276 = gQIqekKNNgyaNZPyb25142402;     gQIqekKNNgyaNZPyb25142402 = gQIqekKNNgyaNZPyb97415448;     gQIqekKNNgyaNZPyb97415448 = gQIqekKNNgyaNZPyb60128137;     gQIqekKNNgyaNZPyb60128137 = gQIqekKNNgyaNZPyb61123755;     gQIqekKNNgyaNZPyb61123755 = gQIqekKNNgyaNZPyb10063378;     gQIqekKNNgyaNZPyb10063378 = gQIqekKNNgyaNZPyb11090053;     gQIqekKNNgyaNZPyb11090053 = gQIqekKNNgyaNZPyb73540185;     gQIqekKNNgyaNZPyb73540185 = gQIqekKNNgyaNZPyb35414715;     gQIqekKNNgyaNZPyb35414715 = gQIqekKNNgyaNZPyb42078081;     gQIqekKNNgyaNZPyb42078081 = gQIqekKNNgyaNZPyb6421721;     gQIqekKNNgyaNZPyb6421721 = gQIqekKNNgyaNZPyb84279203;     gQIqekKNNgyaNZPyb84279203 = gQIqekKNNgyaNZPyb2823678;     gQIqekKNNgyaNZPyb2823678 = gQIqekKNNgyaNZPyb85265730;     gQIqekKNNgyaNZPyb85265730 = gQIqekKNNgyaNZPyb33332919;     gQIqekKNNgyaNZPyb33332919 = gQIqekKNNgyaNZPyb87929133;     gQIqekKNNgyaNZPyb87929133 = gQIqekKNNgyaNZPyb68978180;     gQIqekKNNgyaNZPyb68978180 = gQIqekKNNgyaNZPyb3771651;     gQIqekKNNgyaNZPyb3771651 = gQIqekKNNgyaNZPyb36533204;     gQIqekKNNgyaNZPyb36533204 = gQIqekKNNgyaNZPyb64633807;     gQIqekKNNgyaNZPyb64633807 = gQIqekKNNgyaNZPyb79839187;     gQIqekKNNgyaNZPyb79839187 = gQIqekKNNgyaNZPyb18564632;     gQIqekKNNgyaNZPyb18564632 = gQIqekKNNgyaNZPyb74762551;     gQIqekKNNgyaNZPyb74762551 = gQIqekKNNgyaNZPyb48856729;     gQIqekKNNgyaNZPyb48856729 = gQIqekKNNgyaNZPyb36342761;     gQIqekKNNgyaNZPyb36342761 = gQIqekKNNgyaNZPyb11554481;     gQIqekKNNgyaNZPyb11554481 = gQIqekKNNgyaNZPyb18143452;     gQIqekKNNgyaNZPyb18143452 = gQIqekKNNgyaNZPyb32177427;     gQIqekKNNgyaNZPyb32177427 = gQIqekKNNgyaNZPyb19626509;     gQIqekKNNgyaNZPyb19626509 = gQIqekKNNgyaNZPyb32526054;     gQIqekKNNgyaNZPyb32526054 = gQIqekKNNgyaNZPyb7079616;     gQIqekKNNgyaNZPyb7079616 = gQIqekKNNgyaNZPyb37993374;     gQIqekKNNgyaNZPyb37993374 = gQIqekKNNgyaNZPyb64683702;     gQIqekKNNgyaNZPyb64683702 = gQIqekKNNgyaNZPyb3557868;     gQIqekKNNgyaNZPyb3557868 = gQIqekKNNgyaNZPyb38451168;     gQIqekKNNgyaNZPyb38451168 = gQIqekKNNgyaNZPyb26246254;     gQIqekKNNgyaNZPyb26246254 = gQIqekKNNgyaNZPyb78947540;     gQIqekKNNgyaNZPyb78947540 = gQIqekKNNgyaNZPyb2018386;     gQIqekKNNgyaNZPyb2018386 = gQIqekKNNgyaNZPyb43149779;     gQIqekKNNgyaNZPyb43149779 = gQIqekKNNgyaNZPyb67864931;     gQIqekKNNgyaNZPyb67864931 = gQIqekKNNgyaNZPyb26125604;     gQIqekKNNgyaNZPyb26125604 = gQIqekKNNgyaNZPyb12493563;     gQIqekKNNgyaNZPyb12493563 = gQIqekKNNgyaNZPyb323116;     gQIqekKNNgyaNZPyb323116 = gQIqekKNNgyaNZPyb20073750;     gQIqekKNNgyaNZPyb20073750 = gQIqekKNNgyaNZPyb96405357;     gQIqekKNNgyaNZPyb96405357 = gQIqekKNNgyaNZPyb85967692;     gQIqekKNNgyaNZPyb85967692 = gQIqekKNNgyaNZPyb38998555;     gQIqekKNNgyaNZPyb38998555 = gQIqekKNNgyaNZPyb78909276;     gQIqekKNNgyaNZPyb78909276 = gQIqekKNNgyaNZPyb44994939;     gQIqekKNNgyaNZPyb44994939 = gQIqekKNNgyaNZPyb75879229;     gQIqekKNNgyaNZPyb75879229 = gQIqekKNNgyaNZPyb34405439;     gQIqekKNNgyaNZPyb34405439 = gQIqekKNNgyaNZPyb32635642;     gQIqekKNNgyaNZPyb32635642 = gQIqekKNNgyaNZPyb31149883;     gQIqekKNNgyaNZPyb31149883 = gQIqekKNNgyaNZPyb35994891;     gQIqekKNNgyaNZPyb35994891 = gQIqekKNNgyaNZPyb3390875;     gQIqekKNNgyaNZPyb3390875 = gQIqekKNNgyaNZPyb92213644;     gQIqekKNNgyaNZPyb92213644 = gQIqekKNNgyaNZPyb50379851;     gQIqekKNNgyaNZPyb50379851 = gQIqekKNNgyaNZPyb48558719;     gQIqekKNNgyaNZPyb48558719 = gQIqekKNNgyaNZPyb23785377;     gQIqekKNNgyaNZPyb23785377 = gQIqekKNNgyaNZPyb49569274;     gQIqekKNNgyaNZPyb49569274 = gQIqekKNNgyaNZPyb91919926;     gQIqekKNNgyaNZPyb91919926 = gQIqekKNNgyaNZPyb78912625;     gQIqekKNNgyaNZPyb78912625 = gQIqekKNNgyaNZPyb53913676;     gQIqekKNNgyaNZPyb53913676 = gQIqekKNNgyaNZPyb2888661;     gQIqekKNNgyaNZPyb2888661 = gQIqekKNNgyaNZPyb34998466;     gQIqekKNNgyaNZPyb34998466 = gQIqekKNNgyaNZPyb68428346;     gQIqekKNNgyaNZPyb68428346 = gQIqekKNNgyaNZPyb19595501;     gQIqekKNNgyaNZPyb19595501 = gQIqekKNNgyaNZPyb99265810;     gQIqekKNNgyaNZPyb99265810 = gQIqekKNNgyaNZPyb46814562;     gQIqekKNNgyaNZPyb46814562 = gQIqekKNNgyaNZPyb7086666;     gQIqekKNNgyaNZPyb7086666 = gQIqekKNNgyaNZPyb8981594;     gQIqekKNNgyaNZPyb8981594 = gQIqekKNNgyaNZPyb66959795;     gQIqekKNNgyaNZPyb66959795 = gQIqekKNNgyaNZPyb60621871;     gQIqekKNNgyaNZPyb60621871 = gQIqekKNNgyaNZPyb68668273;     gQIqekKNNgyaNZPyb68668273 = gQIqekKNNgyaNZPyb38508203;     gQIqekKNNgyaNZPyb38508203 = gQIqekKNNgyaNZPyb67345624;     gQIqekKNNgyaNZPyb67345624 = gQIqekKNNgyaNZPyb18241516;     gQIqekKNNgyaNZPyb18241516 = gQIqekKNNgyaNZPyb54688801;     gQIqekKNNgyaNZPyb54688801 = gQIqekKNNgyaNZPyb52451371;     gQIqekKNNgyaNZPyb52451371 = gQIqekKNNgyaNZPyb50375069;     gQIqekKNNgyaNZPyb50375069 = gQIqekKNNgyaNZPyb51388655;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void EMTLTcZAUupwRgcL16521044() {     double RatelFYDEsungPWsv86847623 = -668161713;    double RatelFYDEsungPWsv89635067 = -448975685;    double RatelFYDEsungPWsv99532999 = -806659711;    double RatelFYDEsungPWsv76496889 = -339931007;    double RatelFYDEsungPWsv34213579 = -26272080;    double RatelFYDEsungPWsv73438528 = -443464187;    double RatelFYDEsungPWsv96691154 = -769447326;    double RatelFYDEsungPWsv97869390 = -328657213;    double RatelFYDEsungPWsv63669436 = -386494484;    double RatelFYDEsungPWsv17729791 = -156074238;    double RatelFYDEsungPWsv8428630 = 32522698;    double RatelFYDEsungPWsv42390381 = -529843741;    double RatelFYDEsungPWsv2409653 = -691449401;    double RatelFYDEsungPWsv31468721 = 53022149;    double RatelFYDEsungPWsv22002880 = -668029961;    double RatelFYDEsungPWsv86924920 = -116468846;    double RatelFYDEsungPWsv92243228 = -882595801;    double RatelFYDEsungPWsv67443278 = -1649066;    double RatelFYDEsungPWsv1179029 = -380735300;    double RatelFYDEsungPWsv77067280 = -18813938;    double RatelFYDEsungPWsv23865139 = 72819418;    double RatelFYDEsungPWsv28563927 = -649576103;    double RatelFYDEsungPWsv67940828 = -284590325;    double RatelFYDEsungPWsv53921082 = 6166407;    double RatelFYDEsungPWsv69141706 = -288111197;    double RatelFYDEsungPWsv20761550 = 53325278;    double RatelFYDEsungPWsv71510626 = -14132671;    double RatelFYDEsungPWsv14268610 = -482474466;    double RatelFYDEsungPWsv82082929 = -501741238;    double RatelFYDEsungPWsv47611492 = -638009422;    double RatelFYDEsungPWsv25004952 = -738287072;    double RatelFYDEsungPWsv22102222 = -820369870;    double RatelFYDEsungPWsv15041689 = -90415146;    double RatelFYDEsungPWsv15014157 = -257180433;    double RatelFYDEsungPWsv99141372 = -103953425;    double RatelFYDEsungPWsv39489978 = -635022037;    double RatelFYDEsungPWsv18389997 = -955664522;    double RatelFYDEsungPWsv54987054 = -870825206;    double RatelFYDEsungPWsv66923766 = -598427778;    double RatelFYDEsungPWsv28905340 = -899163127;    double RatelFYDEsungPWsv13290795 = -91765514;    double RatelFYDEsungPWsv7929758 = -694571715;    double RatelFYDEsungPWsv64145929 = -563093800;    double RatelFYDEsungPWsv88988067 = -497984986;    double RatelFYDEsungPWsv36359220 = -483088273;    double RatelFYDEsungPWsv57515809 = -174403139;    double RatelFYDEsungPWsv37488615 = -507991722;    double RatelFYDEsungPWsv49903738 = -429071565;    double RatelFYDEsungPWsv42519174 = -743780935;    double RatelFYDEsungPWsv57055643 = -486779341;    double RatelFYDEsungPWsv87850338 = -777853442;    double RatelFYDEsungPWsv21478847 = -984774674;    double RatelFYDEsungPWsv33304851 = 9703136;    double RatelFYDEsungPWsv19549714 = -922290547;    double RatelFYDEsungPWsv92655904 = -422458495;    double RatelFYDEsungPWsv58283697 = 81414390;    double RatelFYDEsungPWsv21694239 = -64385361;    double RatelFYDEsungPWsv45611917 = -712826119;    double RatelFYDEsungPWsv7355183 = 48180190;    double RatelFYDEsungPWsv13452029 = 20402642;    double RatelFYDEsungPWsv1927903 = -329331516;    double RatelFYDEsungPWsv82422544 = -186972861;    double RatelFYDEsungPWsv15786462 = -826915975;    double RatelFYDEsungPWsv16057945 = -748485062;    double RatelFYDEsungPWsv92724838 = -417787167;    double RatelFYDEsungPWsv86326408 = -147107433;    double RatelFYDEsungPWsv27348692 = -339428596;    double RatelFYDEsungPWsv87395495 = -334268968;    double RatelFYDEsungPWsv32327349 = -843024426;    double RatelFYDEsungPWsv82512902 = 66992075;    double RatelFYDEsungPWsv68534923 = -160804325;    double RatelFYDEsungPWsv37256174 = 88229405;    double RatelFYDEsungPWsv519513 = -403221289;    double RatelFYDEsungPWsv72273688 = -481572173;    double RatelFYDEsungPWsv63776485 = -927048424;    double RatelFYDEsungPWsv15935381 = -232608868;    double RatelFYDEsungPWsv64417997 = 13517696;    double RatelFYDEsungPWsv78952761 = -786605340;    double RatelFYDEsungPWsv17561863 = -510745320;    double RatelFYDEsungPWsv11625897 = -13708058;    double RatelFYDEsungPWsv83272934 = -438683000;    double RatelFYDEsungPWsv21606888 = -585061107;    double RatelFYDEsungPWsv71749435 = -738693531;    double RatelFYDEsungPWsv25027286 = 85038103;    double RatelFYDEsungPWsv59761153 = -860155981;    double RatelFYDEsungPWsv3526106 = -753512398;    double RatelFYDEsungPWsv88797370 = -730073006;    double RatelFYDEsungPWsv95491974 = -168124599;    double RatelFYDEsungPWsv22358253 = -834721939;    double RatelFYDEsungPWsv40857675 = -85367816;    double RatelFYDEsungPWsv17795739 = -470636676;    double RatelFYDEsungPWsv72778080 = -142838403;    double RatelFYDEsungPWsv47631871 = -819005396;    double RatelFYDEsungPWsv53471737 = -518830420;    double RatelFYDEsungPWsv26977438 = -469831611;    double RatelFYDEsungPWsv30868251 = -904792654;    double RatelFYDEsungPWsv92143296 = -867655740;    double RatelFYDEsungPWsv48087985 = -814608739;    double RatelFYDEsungPWsv96263228 = 19802181;    double RatelFYDEsungPWsv50032811 = -668161713;     RatelFYDEsungPWsv86847623 = RatelFYDEsungPWsv89635067;     RatelFYDEsungPWsv89635067 = RatelFYDEsungPWsv99532999;     RatelFYDEsungPWsv99532999 = RatelFYDEsungPWsv76496889;     RatelFYDEsungPWsv76496889 = RatelFYDEsungPWsv34213579;     RatelFYDEsungPWsv34213579 = RatelFYDEsungPWsv73438528;     RatelFYDEsungPWsv73438528 = RatelFYDEsungPWsv96691154;     RatelFYDEsungPWsv96691154 = RatelFYDEsungPWsv97869390;     RatelFYDEsungPWsv97869390 = RatelFYDEsungPWsv63669436;     RatelFYDEsungPWsv63669436 = RatelFYDEsungPWsv17729791;     RatelFYDEsungPWsv17729791 = RatelFYDEsungPWsv8428630;     RatelFYDEsungPWsv8428630 = RatelFYDEsungPWsv42390381;     RatelFYDEsungPWsv42390381 = RatelFYDEsungPWsv2409653;     RatelFYDEsungPWsv2409653 = RatelFYDEsungPWsv31468721;     RatelFYDEsungPWsv31468721 = RatelFYDEsungPWsv22002880;     RatelFYDEsungPWsv22002880 = RatelFYDEsungPWsv86924920;     RatelFYDEsungPWsv86924920 = RatelFYDEsungPWsv92243228;     RatelFYDEsungPWsv92243228 = RatelFYDEsungPWsv67443278;     RatelFYDEsungPWsv67443278 = RatelFYDEsungPWsv1179029;     RatelFYDEsungPWsv1179029 = RatelFYDEsungPWsv77067280;     RatelFYDEsungPWsv77067280 = RatelFYDEsungPWsv23865139;     RatelFYDEsungPWsv23865139 = RatelFYDEsungPWsv28563927;     RatelFYDEsungPWsv28563927 = RatelFYDEsungPWsv67940828;     RatelFYDEsungPWsv67940828 = RatelFYDEsungPWsv53921082;     RatelFYDEsungPWsv53921082 = RatelFYDEsungPWsv69141706;     RatelFYDEsungPWsv69141706 = RatelFYDEsungPWsv20761550;     RatelFYDEsungPWsv20761550 = RatelFYDEsungPWsv71510626;     RatelFYDEsungPWsv71510626 = RatelFYDEsungPWsv14268610;     RatelFYDEsungPWsv14268610 = RatelFYDEsungPWsv82082929;     RatelFYDEsungPWsv82082929 = RatelFYDEsungPWsv47611492;     RatelFYDEsungPWsv47611492 = RatelFYDEsungPWsv25004952;     RatelFYDEsungPWsv25004952 = RatelFYDEsungPWsv22102222;     RatelFYDEsungPWsv22102222 = RatelFYDEsungPWsv15041689;     RatelFYDEsungPWsv15041689 = RatelFYDEsungPWsv15014157;     RatelFYDEsungPWsv15014157 = RatelFYDEsungPWsv99141372;     RatelFYDEsungPWsv99141372 = RatelFYDEsungPWsv39489978;     RatelFYDEsungPWsv39489978 = RatelFYDEsungPWsv18389997;     RatelFYDEsungPWsv18389997 = RatelFYDEsungPWsv54987054;     RatelFYDEsungPWsv54987054 = RatelFYDEsungPWsv66923766;     RatelFYDEsungPWsv66923766 = RatelFYDEsungPWsv28905340;     RatelFYDEsungPWsv28905340 = RatelFYDEsungPWsv13290795;     RatelFYDEsungPWsv13290795 = RatelFYDEsungPWsv7929758;     RatelFYDEsungPWsv7929758 = RatelFYDEsungPWsv64145929;     RatelFYDEsungPWsv64145929 = RatelFYDEsungPWsv88988067;     RatelFYDEsungPWsv88988067 = RatelFYDEsungPWsv36359220;     RatelFYDEsungPWsv36359220 = RatelFYDEsungPWsv57515809;     RatelFYDEsungPWsv57515809 = RatelFYDEsungPWsv37488615;     RatelFYDEsungPWsv37488615 = RatelFYDEsungPWsv49903738;     RatelFYDEsungPWsv49903738 = RatelFYDEsungPWsv42519174;     RatelFYDEsungPWsv42519174 = RatelFYDEsungPWsv57055643;     RatelFYDEsungPWsv57055643 = RatelFYDEsungPWsv87850338;     RatelFYDEsungPWsv87850338 = RatelFYDEsungPWsv21478847;     RatelFYDEsungPWsv21478847 = RatelFYDEsungPWsv33304851;     RatelFYDEsungPWsv33304851 = RatelFYDEsungPWsv19549714;     RatelFYDEsungPWsv19549714 = RatelFYDEsungPWsv92655904;     RatelFYDEsungPWsv92655904 = RatelFYDEsungPWsv58283697;     RatelFYDEsungPWsv58283697 = RatelFYDEsungPWsv21694239;     RatelFYDEsungPWsv21694239 = RatelFYDEsungPWsv45611917;     RatelFYDEsungPWsv45611917 = RatelFYDEsungPWsv7355183;     RatelFYDEsungPWsv7355183 = RatelFYDEsungPWsv13452029;     RatelFYDEsungPWsv13452029 = RatelFYDEsungPWsv1927903;     RatelFYDEsungPWsv1927903 = RatelFYDEsungPWsv82422544;     RatelFYDEsungPWsv82422544 = RatelFYDEsungPWsv15786462;     RatelFYDEsungPWsv15786462 = RatelFYDEsungPWsv16057945;     RatelFYDEsungPWsv16057945 = RatelFYDEsungPWsv92724838;     RatelFYDEsungPWsv92724838 = RatelFYDEsungPWsv86326408;     RatelFYDEsungPWsv86326408 = RatelFYDEsungPWsv27348692;     RatelFYDEsungPWsv27348692 = RatelFYDEsungPWsv87395495;     RatelFYDEsungPWsv87395495 = RatelFYDEsungPWsv32327349;     RatelFYDEsungPWsv32327349 = RatelFYDEsungPWsv82512902;     RatelFYDEsungPWsv82512902 = RatelFYDEsungPWsv68534923;     RatelFYDEsungPWsv68534923 = RatelFYDEsungPWsv37256174;     RatelFYDEsungPWsv37256174 = RatelFYDEsungPWsv519513;     RatelFYDEsungPWsv519513 = RatelFYDEsungPWsv72273688;     RatelFYDEsungPWsv72273688 = RatelFYDEsungPWsv63776485;     RatelFYDEsungPWsv63776485 = RatelFYDEsungPWsv15935381;     RatelFYDEsungPWsv15935381 = RatelFYDEsungPWsv64417997;     RatelFYDEsungPWsv64417997 = RatelFYDEsungPWsv78952761;     RatelFYDEsungPWsv78952761 = RatelFYDEsungPWsv17561863;     RatelFYDEsungPWsv17561863 = RatelFYDEsungPWsv11625897;     RatelFYDEsungPWsv11625897 = RatelFYDEsungPWsv83272934;     RatelFYDEsungPWsv83272934 = RatelFYDEsungPWsv21606888;     RatelFYDEsungPWsv21606888 = RatelFYDEsungPWsv71749435;     RatelFYDEsungPWsv71749435 = RatelFYDEsungPWsv25027286;     RatelFYDEsungPWsv25027286 = RatelFYDEsungPWsv59761153;     RatelFYDEsungPWsv59761153 = RatelFYDEsungPWsv3526106;     RatelFYDEsungPWsv3526106 = RatelFYDEsungPWsv88797370;     RatelFYDEsungPWsv88797370 = RatelFYDEsungPWsv95491974;     RatelFYDEsungPWsv95491974 = RatelFYDEsungPWsv22358253;     RatelFYDEsungPWsv22358253 = RatelFYDEsungPWsv40857675;     RatelFYDEsungPWsv40857675 = RatelFYDEsungPWsv17795739;     RatelFYDEsungPWsv17795739 = RatelFYDEsungPWsv72778080;     RatelFYDEsungPWsv72778080 = RatelFYDEsungPWsv47631871;     RatelFYDEsungPWsv47631871 = RatelFYDEsungPWsv53471737;     RatelFYDEsungPWsv53471737 = RatelFYDEsungPWsv26977438;     RatelFYDEsungPWsv26977438 = RatelFYDEsungPWsv30868251;     RatelFYDEsungPWsv30868251 = RatelFYDEsungPWsv92143296;     RatelFYDEsungPWsv92143296 = RatelFYDEsungPWsv48087985;     RatelFYDEsungPWsv48087985 = RatelFYDEsungPWsv96263228;     RatelFYDEsungPWsv96263228 = RatelFYDEsungPWsv50032811;     RatelFYDEsungPWsv50032811 = RatelFYDEsungPWsv86847623;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void aCpWInfnCZCykKcK31570112() {     double uhFPXgBpyXcyfhQzK92964730 = -780063601;    double uhFPXgBpyXcyfhQzK33008347 = -158575466;    double uhFPXgBpyXcyfhQzK15575184 = -957492632;    double uhFPXgBpyXcyfhQzK3571670 = -530679637;    double uhFPXgBpyXcyfhQzK3109068 = -546921951;    double uhFPXgBpyXcyfhQzK64855731 = -707960422;    double uhFPXgBpyXcyfhQzK57180120 = -908412303;    double uhFPXgBpyXcyfhQzK47629282 = -615531126;    double uhFPXgBpyXcyfhQzK28311932 = -154527149;    double uhFPXgBpyXcyfhQzK58959450 = -485049716;    double uhFPXgBpyXcyfhQzK68538920 = -944914888;    double uhFPXgBpyXcyfhQzK74213423 = -974249943;    double uhFPXgBpyXcyfhQzK88618771 = -31340489;    double uhFPXgBpyXcyfhQzK14612398 = -479216419;    double uhFPXgBpyXcyfhQzK17745307 = -258492127;    double uhFPXgBpyXcyfhQzK28430315 = -89468389;    double uhFPXgBpyXcyfhQzK86079231 = -399140913;    double uhFPXgBpyXcyfhQzK64496431 = -767171709;    double uhFPXgBpyXcyfhQzK63008870 = -786504397;    double uhFPXgBpyXcyfhQzK57617445 = -375716857;    double uhFPXgBpyXcyfhQzK12418332 = -882668038;    double uhFPXgBpyXcyfhQzK15084951 = -557505820;    double uhFPXgBpyXcyfhQzK3531621 = -31107812;    double uhFPXgBpyXcyfhQzK37913456 = -216138581;    double uhFPXgBpyXcyfhQzK12841356 = -661307230;    double uhFPXgBpyXcyfhQzK45483225 = -925330045;    double uhFPXgBpyXcyfhQzK69970404 = 85205126;    double uhFPXgBpyXcyfhQzK27375735 = -548403420;    double uhFPXgBpyXcyfhQzK48215185 = -724123943;    double uhFPXgBpyXcyfhQzK16227129 = -455480290;    double uhFPXgBpyXcyfhQzK17247502 = -651608010;    double uhFPXgBpyXcyfhQzK19712844 = -529816685;    double uhFPXgBpyXcyfhQzK24752637 = 16971484;    double uhFPXgBpyXcyfhQzK9844722 = -8722199;    double uhFPXgBpyXcyfhQzK60616024 = -742686531;    double uhFPXgBpyXcyfhQzK8215891 = -849048144;    double uhFPXgBpyXcyfhQzK99651931 = -909300391;    double uhFPXgBpyXcyfhQzK89077266 = -642331216;    double uhFPXgBpyXcyfhQzK40534020 = -592351275;    double uhFPXgBpyXcyfhQzK16407278 = -272602350;    double uhFPXgBpyXcyfhQzK18960019 = -157039409;    double uhFPXgBpyXcyfhQzK40080215 = -964713993;    double uhFPXgBpyXcyfhQzK42212908 = -643898384;    double uhFPXgBpyXcyfhQzK76395807 = -517327924;    double uhFPXgBpyXcyfhQzK48311685 = -144986428;    double uhFPXgBpyXcyfhQzK69455004 = -159619255;    double uhFPXgBpyXcyfhQzK55267189 = -626767260;    double uhFPXgBpyXcyfhQzK57860679 = -438967098;    double uhFPXgBpyXcyfhQzK52194903 = -829517923;    double uhFPXgBpyXcyfhQzK69967296 = -573790150;    double uhFPXgBpyXcyfhQzK23551017 = -620147113;    double uhFPXgBpyXcyfhQzK32288566 = -785500994;    double uhFPXgBpyXcyfhQzK81553308 = -882086451;    double uhFPXgBpyXcyfhQzK15518595 = -142760692;    double uhFPXgBpyXcyfhQzK19151780 = -22990277;    double uhFPXgBpyXcyfhQzK77879780 = -122557782;    double uhFPXgBpyXcyfhQzK29476727 = -27467655;    double uhFPXgBpyXcyfhQzK77661727 = -641354052;    double uhFPXgBpyXcyfhQzK90730313 = -869372408;    double uhFPXgBpyXcyfhQzK57625842 = -621591906;    double uhFPXgBpyXcyfhQzK94885326 = -693165548;    double uhFPXgBpyXcyfhQzK29804385 = -260008884;    double uhFPXgBpyXcyfhQzK99414096 = -891407184;    double uhFPXgBpyXcyfhQzK12084803 = -699046860;    double uhFPXgBpyXcyfhQzK41711948 = -833441706;    double uhFPXgBpyXcyfhQzK48826077 = -315098204;    double uhFPXgBpyXcyfhQzK49460786 = -891221428;    double uhFPXgBpyXcyfhQzK78774050 = 77381709;    double uhFPXgBpyXcyfhQzK53996373 = -736529889;    double uhFPXgBpyXcyfhQzK9529416 = -409443984;    double uhFPXgBpyXcyfhQzK28778383 = -180167998;    double uhFPXgBpyXcyfhQzK97001964 = -756809698;    double uhFPXgBpyXcyfhQzK23962412 = -74820435;    double uhFPXgBpyXcyfhQzK46601593 = -413902048;    double uhFPXgBpyXcyfhQzK38657426 = -118677449;    double uhFPXgBpyXcyfhQzK72338117 = -917954045;    double uhFPXgBpyXcyfhQzK72872042 = -913607436;    double uhFPXgBpyXcyfhQzK27135813 = -513779888;    double uhFPXgBpyXcyfhQzK89601771 = 28847847;    double uhFPXgBpyXcyfhQzK43386352 = -401687975;    double uhFPXgBpyXcyfhQzK90216035 = -198562786;    double uhFPXgBpyXcyfhQzK12109726 = -475827777;    double uhFPXgBpyXcyfhQzK75180831 = -718885497;    double uhFPXgBpyXcyfhQzK78247888 = -50333794;    double uhFPXgBpyXcyfhQzK92676111 = -835333177;    double uhFPXgBpyXcyfhQzK84958935 = -866107016;    double uhFPXgBpyXcyfhQzK38159535 = -647730234;    double uhFPXgBpyXcyfhQzK9234043 = -840267825;    double uhFPXgBpyXcyfhQzK90692942 = -985731922;    double uhFPXgBpyXcyfhQzK82736244 = -520128749;    double uhFPXgBpyXcyfhQzK78739164 = -721580490;    double uhFPXgBpyXcyfhQzK21990205 = -167946340;    double uhFPXgBpyXcyfhQzK98346953 = -772958808;    double uhFPXgBpyXcyfhQzK82908177 = -970759369;    double uhFPXgBpyXcyfhQzK21521951 = -579436802;    double uhFPXgBpyXcyfhQzK89155634 = -897030525;    double uhFPXgBpyXcyfhQzK40666118 = 26693191;    double uhFPXgBpyXcyfhQzK30128105 = -944851524;    double uhFPXgBpyXcyfhQzK34683859 = -683886218;    double uhFPXgBpyXcyfhQzK99485607 = -780063601;     uhFPXgBpyXcyfhQzK92964730 = uhFPXgBpyXcyfhQzK33008347;     uhFPXgBpyXcyfhQzK33008347 = uhFPXgBpyXcyfhQzK15575184;     uhFPXgBpyXcyfhQzK15575184 = uhFPXgBpyXcyfhQzK3571670;     uhFPXgBpyXcyfhQzK3571670 = uhFPXgBpyXcyfhQzK3109068;     uhFPXgBpyXcyfhQzK3109068 = uhFPXgBpyXcyfhQzK64855731;     uhFPXgBpyXcyfhQzK64855731 = uhFPXgBpyXcyfhQzK57180120;     uhFPXgBpyXcyfhQzK57180120 = uhFPXgBpyXcyfhQzK47629282;     uhFPXgBpyXcyfhQzK47629282 = uhFPXgBpyXcyfhQzK28311932;     uhFPXgBpyXcyfhQzK28311932 = uhFPXgBpyXcyfhQzK58959450;     uhFPXgBpyXcyfhQzK58959450 = uhFPXgBpyXcyfhQzK68538920;     uhFPXgBpyXcyfhQzK68538920 = uhFPXgBpyXcyfhQzK74213423;     uhFPXgBpyXcyfhQzK74213423 = uhFPXgBpyXcyfhQzK88618771;     uhFPXgBpyXcyfhQzK88618771 = uhFPXgBpyXcyfhQzK14612398;     uhFPXgBpyXcyfhQzK14612398 = uhFPXgBpyXcyfhQzK17745307;     uhFPXgBpyXcyfhQzK17745307 = uhFPXgBpyXcyfhQzK28430315;     uhFPXgBpyXcyfhQzK28430315 = uhFPXgBpyXcyfhQzK86079231;     uhFPXgBpyXcyfhQzK86079231 = uhFPXgBpyXcyfhQzK64496431;     uhFPXgBpyXcyfhQzK64496431 = uhFPXgBpyXcyfhQzK63008870;     uhFPXgBpyXcyfhQzK63008870 = uhFPXgBpyXcyfhQzK57617445;     uhFPXgBpyXcyfhQzK57617445 = uhFPXgBpyXcyfhQzK12418332;     uhFPXgBpyXcyfhQzK12418332 = uhFPXgBpyXcyfhQzK15084951;     uhFPXgBpyXcyfhQzK15084951 = uhFPXgBpyXcyfhQzK3531621;     uhFPXgBpyXcyfhQzK3531621 = uhFPXgBpyXcyfhQzK37913456;     uhFPXgBpyXcyfhQzK37913456 = uhFPXgBpyXcyfhQzK12841356;     uhFPXgBpyXcyfhQzK12841356 = uhFPXgBpyXcyfhQzK45483225;     uhFPXgBpyXcyfhQzK45483225 = uhFPXgBpyXcyfhQzK69970404;     uhFPXgBpyXcyfhQzK69970404 = uhFPXgBpyXcyfhQzK27375735;     uhFPXgBpyXcyfhQzK27375735 = uhFPXgBpyXcyfhQzK48215185;     uhFPXgBpyXcyfhQzK48215185 = uhFPXgBpyXcyfhQzK16227129;     uhFPXgBpyXcyfhQzK16227129 = uhFPXgBpyXcyfhQzK17247502;     uhFPXgBpyXcyfhQzK17247502 = uhFPXgBpyXcyfhQzK19712844;     uhFPXgBpyXcyfhQzK19712844 = uhFPXgBpyXcyfhQzK24752637;     uhFPXgBpyXcyfhQzK24752637 = uhFPXgBpyXcyfhQzK9844722;     uhFPXgBpyXcyfhQzK9844722 = uhFPXgBpyXcyfhQzK60616024;     uhFPXgBpyXcyfhQzK60616024 = uhFPXgBpyXcyfhQzK8215891;     uhFPXgBpyXcyfhQzK8215891 = uhFPXgBpyXcyfhQzK99651931;     uhFPXgBpyXcyfhQzK99651931 = uhFPXgBpyXcyfhQzK89077266;     uhFPXgBpyXcyfhQzK89077266 = uhFPXgBpyXcyfhQzK40534020;     uhFPXgBpyXcyfhQzK40534020 = uhFPXgBpyXcyfhQzK16407278;     uhFPXgBpyXcyfhQzK16407278 = uhFPXgBpyXcyfhQzK18960019;     uhFPXgBpyXcyfhQzK18960019 = uhFPXgBpyXcyfhQzK40080215;     uhFPXgBpyXcyfhQzK40080215 = uhFPXgBpyXcyfhQzK42212908;     uhFPXgBpyXcyfhQzK42212908 = uhFPXgBpyXcyfhQzK76395807;     uhFPXgBpyXcyfhQzK76395807 = uhFPXgBpyXcyfhQzK48311685;     uhFPXgBpyXcyfhQzK48311685 = uhFPXgBpyXcyfhQzK69455004;     uhFPXgBpyXcyfhQzK69455004 = uhFPXgBpyXcyfhQzK55267189;     uhFPXgBpyXcyfhQzK55267189 = uhFPXgBpyXcyfhQzK57860679;     uhFPXgBpyXcyfhQzK57860679 = uhFPXgBpyXcyfhQzK52194903;     uhFPXgBpyXcyfhQzK52194903 = uhFPXgBpyXcyfhQzK69967296;     uhFPXgBpyXcyfhQzK69967296 = uhFPXgBpyXcyfhQzK23551017;     uhFPXgBpyXcyfhQzK23551017 = uhFPXgBpyXcyfhQzK32288566;     uhFPXgBpyXcyfhQzK32288566 = uhFPXgBpyXcyfhQzK81553308;     uhFPXgBpyXcyfhQzK81553308 = uhFPXgBpyXcyfhQzK15518595;     uhFPXgBpyXcyfhQzK15518595 = uhFPXgBpyXcyfhQzK19151780;     uhFPXgBpyXcyfhQzK19151780 = uhFPXgBpyXcyfhQzK77879780;     uhFPXgBpyXcyfhQzK77879780 = uhFPXgBpyXcyfhQzK29476727;     uhFPXgBpyXcyfhQzK29476727 = uhFPXgBpyXcyfhQzK77661727;     uhFPXgBpyXcyfhQzK77661727 = uhFPXgBpyXcyfhQzK90730313;     uhFPXgBpyXcyfhQzK90730313 = uhFPXgBpyXcyfhQzK57625842;     uhFPXgBpyXcyfhQzK57625842 = uhFPXgBpyXcyfhQzK94885326;     uhFPXgBpyXcyfhQzK94885326 = uhFPXgBpyXcyfhQzK29804385;     uhFPXgBpyXcyfhQzK29804385 = uhFPXgBpyXcyfhQzK99414096;     uhFPXgBpyXcyfhQzK99414096 = uhFPXgBpyXcyfhQzK12084803;     uhFPXgBpyXcyfhQzK12084803 = uhFPXgBpyXcyfhQzK41711948;     uhFPXgBpyXcyfhQzK41711948 = uhFPXgBpyXcyfhQzK48826077;     uhFPXgBpyXcyfhQzK48826077 = uhFPXgBpyXcyfhQzK49460786;     uhFPXgBpyXcyfhQzK49460786 = uhFPXgBpyXcyfhQzK78774050;     uhFPXgBpyXcyfhQzK78774050 = uhFPXgBpyXcyfhQzK53996373;     uhFPXgBpyXcyfhQzK53996373 = uhFPXgBpyXcyfhQzK9529416;     uhFPXgBpyXcyfhQzK9529416 = uhFPXgBpyXcyfhQzK28778383;     uhFPXgBpyXcyfhQzK28778383 = uhFPXgBpyXcyfhQzK97001964;     uhFPXgBpyXcyfhQzK97001964 = uhFPXgBpyXcyfhQzK23962412;     uhFPXgBpyXcyfhQzK23962412 = uhFPXgBpyXcyfhQzK46601593;     uhFPXgBpyXcyfhQzK46601593 = uhFPXgBpyXcyfhQzK38657426;     uhFPXgBpyXcyfhQzK38657426 = uhFPXgBpyXcyfhQzK72338117;     uhFPXgBpyXcyfhQzK72338117 = uhFPXgBpyXcyfhQzK72872042;     uhFPXgBpyXcyfhQzK72872042 = uhFPXgBpyXcyfhQzK27135813;     uhFPXgBpyXcyfhQzK27135813 = uhFPXgBpyXcyfhQzK89601771;     uhFPXgBpyXcyfhQzK89601771 = uhFPXgBpyXcyfhQzK43386352;     uhFPXgBpyXcyfhQzK43386352 = uhFPXgBpyXcyfhQzK90216035;     uhFPXgBpyXcyfhQzK90216035 = uhFPXgBpyXcyfhQzK12109726;     uhFPXgBpyXcyfhQzK12109726 = uhFPXgBpyXcyfhQzK75180831;     uhFPXgBpyXcyfhQzK75180831 = uhFPXgBpyXcyfhQzK78247888;     uhFPXgBpyXcyfhQzK78247888 = uhFPXgBpyXcyfhQzK92676111;     uhFPXgBpyXcyfhQzK92676111 = uhFPXgBpyXcyfhQzK84958935;     uhFPXgBpyXcyfhQzK84958935 = uhFPXgBpyXcyfhQzK38159535;     uhFPXgBpyXcyfhQzK38159535 = uhFPXgBpyXcyfhQzK9234043;     uhFPXgBpyXcyfhQzK9234043 = uhFPXgBpyXcyfhQzK90692942;     uhFPXgBpyXcyfhQzK90692942 = uhFPXgBpyXcyfhQzK82736244;     uhFPXgBpyXcyfhQzK82736244 = uhFPXgBpyXcyfhQzK78739164;     uhFPXgBpyXcyfhQzK78739164 = uhFPXgBpyXcyfhQzK21990205;     uhFPXgBpyXcyfhQzK21990205 = uhFPXgBpyXcyfhQzK98346953;     uhFPXgBpyXcyfhQzK98346953 = uhFPXgBpyXcyfhQzK82908177;     uhFPXgBpyXcyfhQzK82908177 = uhFPXgBpyXcyfhQzK21521951;     uhFPXgBpyXcyfhQzK21521951 = uhFPXgBpyXcyfhQzK89155634;     uhFPXgBpyXcyfhQzK89155634 = uhFPXgBpyXcyfhQzK40666118;     uhFPXgBpyXcyfhQzK40666118 = uhFPXgBpyXcyfhQzK30128105;     uhFPXgBpyXcyfhQzK30128105 = uhFPXgBpyXcyfhQzK34683859;     uhFPXgBpyXcyfhQzK34683859 = uhFPXgBpyXcyfhQzK99485607;     uhFPXgBpyXcyfhQzK99485607 = uhFPXgBpyXcyfhQzK92964730;}
// Junk Finished
