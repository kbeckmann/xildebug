#pragma once

#include "errors.h"

#define EHID_DAP_BRIDGE_NO_INIT			(EHID_DAP_BRIDGE_BASE + 0)
#define EHID_DAP_BRIDGE_TASK_CREATE		(EHID_DAP_BRIDGE_BASE + 1)

err_t hid_dap_bridge_init(void);
