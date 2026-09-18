#include "usb/usb_product_name.h"

extern void KeyboardUsb_SetProductName(const char *name);

void Customer_UsbProductName_Init(void)
{
    KeyboardUsb_SetProductName("Hall_Keyboard");
}
