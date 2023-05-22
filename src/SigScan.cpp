#include "SigScan.h"
#include <psapi.h>

MODULEINFO moduleInfo;

const MODULEINFO &
getModuleInfo () {
	if (moduleInfo.SizeOfImage) return moduleInfo;

	ZeroMemory (&moduleInfo, sizeof (MODULEINFO));
	GetModuleInformation (GetCurrentProcess (), GetModuleHandle (nullptr), &moduleInfo, sizeof (MODULEINFO));

	return moduleInfo;
}

void *
sigScan_memory (const char *signature, const char *mask, size_t sigSize, void *memory, const size_t memorySize) {
	if (sigSize == 0) sigSize = strlen (mask);

	for (size_t i = 0; i < memorySize; i++) {
		char *currMemory = (char *)memory + i;

		size_t j;
		for (j = 0; j < sigSize; j++)
			if (mask[j] != '?' && signature[j] != currMemory[j]) break;

		if (j == sigSize) return currMemory;
	}

	return nullptr;
}

void *
sigScan (const char *signature, const char *mask, void *hint) {
	const MODULEINFO &info = getModuleInfo ();
	const size_t sigSize   = strlen (mask);

	// Ensure hint address is within the process memory region so there are no
	// crashes.
	if ((hint >= info.lpBaseOfDll) && ((char *)hint + sigSize <= (char *)info.lpBaseOfDll + info.SizeOfImage)) {
		void *result = sigScan_memory (signature, mask, sigSize, hint, sigSize);

		if (result) return result;
	}

	return sigScan_memory (signature, mask, sigSize, info.lpBaseOfDll, info.SizeOfImage);
}

void *
sigScanString (const char *signature) {
	const MODULEINFO &info = getModuleInfo ();
	const u64 sigSize      = strlen (signature);

	for (u64 i = 0; i < info.SizeOfImage; i++) {
		u8 *currMemory = (u8 *)info.lpBaseOfDll + i;

		u64 j;
		for (j = 0; j < sigSize; j++)
			if (signature[j] != currMemory[j]) break;

		if (j == sigSize) return currMemory;
	}

	return 0;
}

bool sigValid = true;
