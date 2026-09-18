extern void USB_BSP_IRQHandler(void);
extern void USB_BSP_WkupHandler(void);

void USB_HS_WKUP_IRQHandler(void)
{
    USB_BSP_WkupHandler();
}

void USB_HS_IRQHandler(void)
{
    USB_BSP_IRQHandler();
}

#ifdef USB_DEDICATED_EP_ENABLED
extern void USB_BSP_EpOutHandler(void);
extern void USB_BSP_EpInHandler(void);

void USB_HS_EPx_OUT_IRQHandler(void)
{
    USB_BSP_EpOutHandler();
}

void USB_HS_EPx_IN_IRQHandler(void)
{
    USB_BSP_EpInHandler();
}
#endif
