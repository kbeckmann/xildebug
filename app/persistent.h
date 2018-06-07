#pragma once

#include <stdint.h>
#include <stdbool.h>

struct persistent {
	bool reboot_to_bootloader;
};

extern struct persistent persistent_data;
