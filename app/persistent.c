#include "persistent.h"

struct persistent persistent_data __attribute__((section (".persistent")));
