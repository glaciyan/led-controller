#include <stdint.h>
#include "services/ans/ble_svc_ans.h"

// The compiler refuses to find this method when it's not in a .c file
void ble_ans_init() {
    ble_svc_ans_init();
}