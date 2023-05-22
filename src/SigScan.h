#pragma once

#define SIG_SCAN(name, hint, signature, mask)                       \
	void *name ();                                                  \
	void *name##Addr = name ();                                     \
	void *name () {                                                 \
		if (!name##Addr) {                                          \
			name##Addr = sigScan (signature, mask, (void *)(hint)); \
			if (name##Addr) return name##Addr;                      \
			sigValid = false;                                       \
		}                                                           \
		return name##Addr;                                          \
	}

#define SIG_SCAN_STRING(name, string)            \
	void *name ();                               \
	void *name##Addr = name ();                  \
	void *name () {                              \
		if (!name##Addr) {                       \
			name##Addr = sigScanString (string); \
			if (name##Addr) return name##Addr;   \
			sigValid = false;                    \
		}                                        \
		return name##Addr;                       \
	}

void *sigScan (const char *signature, const char *mask, void *hint);
void *sigScanString (const char *signature);

// Automatically scanned signatures, these are expected to exist in all game
// versions sigValid is going to be false if any automatic signature scan fails
extern bool sigValid;
