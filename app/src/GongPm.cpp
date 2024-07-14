#include <GongPm.h>

GongPm::GongPm()
{

    printk("\nGet device stm32lpm\n");
    pm = DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(st_stm32pm));
    
    if (!pm) {
        printk("Device STM32PM not found.\n");
        return;
    }

}


/**
 * Gets the wakeup pin that has awaken the system
 */
int32_t GongPm::getWakeupPin()
{
    int wakeupPin = stm32pm_wakeup_pin_get(pm);
    printk("wakeupPin: %d\n", wakeupPin);

    return wakeupPin;
}

/**
 * Checks if the wakeup pin is still active
 */
int GongPm::isWakeupPinActive()
{
    int isActive = stm32pm_wakeup_pin_active(pm);
    printk("Is WakeupPin active: %d\n", isActive);

    return isActive;
}


/**
 * Puts the system into the standby mode
 */
void GongPm::standby()
{
    stm32pm_state_set(pm, PM_STATE_STANDBY, 0);
}



