#pragma once

#include <stdbool.h>
#include <stdint.h>

struct persistent {
	bool reboot_to_bootloader;
};

extern struct persistent persistent_data;
