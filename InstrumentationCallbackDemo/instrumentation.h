#pragma once
#include <stdint.h>

struct InstrumentationCallbackInfo_t {
	uint32_t version;
	uint32_t reserved;
	void (*callback)();
};
