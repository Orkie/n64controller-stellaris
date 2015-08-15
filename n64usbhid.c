#define PART_TM4C123GH6PM
#include <stdint.h>
#include <stdbool.h>

#include <pin_map.h>
#include <hw_memmap.h>
#include <sysctl.h>
#include <gpio.h>
#include <usb.h>
#include <usblib.h>
#include <usb-ids.h>
#include <usbdevice.h>
#include <usbhid.h>
#include <usbdhid.h>
#include <usbdmsc.h>

#include "ustdlib.h"

#define N64_CONTROLLER_PID 0x8B
#define USB_HID_Z               0x32
#define USB_HID_RX              0x33
#define USB_HID_GAMEPAD        0x05

uint32_t rxCallback(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgParam, void *pvMsgData);
uint32_t txCallback(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgParam, void *pvMsgData);

const uint8_t languageDescriptor[] =
{
    4,
    USB_DTYPE_STRING,
    USBShort(USB_LANG_EN_US)
};

//*****************************************************************************
//
// The manufacturer string.
//
//*****************************************************************************
const uint8_t manufacturerString[] =
{
    (12 + 1) * 2,
    USB_DTYPE_STRING,
    'A', 0, 'd', 0, 'a', 0, 'n', 0, ' ', 0, 'S', 0, 'c', 0, 'o', 0, 't', 0,
    'n', 0, 'e', 0, 'y', 0,
};

//*****************************************************************************
//
// The product string.
//
//*****************************************************************************
const uint8_t productString[] =
{
    2 + (7 * 2),
    USB_DTYPE_STRING,
    'N', 0, '6', 0, '4', 0, ' ', 0, 'P', 0, 'a', 0, 'd', 0,
};

//*****************************************************************************
//
// The serial number string.
//
//*****************************************************************************
const uint8_t serialNumberString[] =
{
    2 + (8 * 2),
    USB_DTYPE_STRING,
    '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0
};

//*****************************************************************************
//
// The control interface description string.
//
//*****************************************************************************
const uint8_t controlInterfaceString[] =
{
	    (8 + 1) * 2,
	    USB_DTYPE_STRING,
	    'H', 0, 'I', 0, 'D', 0, ' ', 0, 'M', 0, 'i', 0, 's', 0, 'c', 0
};

//*****************************************************************************
//
// The configuration description string.
//
//*****************************************************************************
const uint8_t configString[] =
{
		(21 + 1) * 2,
		USB_DTYPE_STRING,
		'H', 0, 'I', 0, 'D', 0, ' ', 0, 'M', 0, 'i', 0, 's', 0, 'c', 0,
		' ', 0, 'C', 0, 'o', 0, 'n', 0, 'f', 0, 'i', 0, 'g', 0,
		'u', 0, 'r', 0, 'a', 0, 't', 0, 'i', 0, 'o', 0, 'n', 0
};

const uint8_t * const stringDescriptors[] =
{
    languageDescriptor,
    manufacturerString,
    productString,
    serialNumberString,
    controlInterfaceString,
    configString
};
#define NUM_STRING_DESCRIPTORS (sizeof(stringDescriptors) / sizeof(uint8_t *))

tHIDReportIdle reportIdle = {
	0,
	0,
	0,
	0,
};

static const uint8_t hidReportDescriptor[] =
{
    UsagePage(USB_HID_GENERIC_DESKTOP),
    Usage(USB_HID_GAMEPAD),
    Collection(USB_HID_APPLICATION),
        Collection(USB_HID_PHYSICAL),

//		//
//		// 8 - 1 bit values for the first set of buttons.
//		//
//		UsagePage(USB_HID_BUTTONS),
//		UsageMinimum(1),
//		UsageMaximum(8),
//		LogicalMinimum(0),
//		LogicalMaximum(1),
//		ReportSize(1),
//		ReportCount(8),
//		Input(USB_HID_INPUT_DATA | USB_HID_INPUT_VARIABLE | USB_HID_INPUT_ABS),
//
//		//
//		// 8 - 1 bit values for the second set of buttons.
//		//
//		UsagePage(USB_HID_BUTTONS),
//		UsageMinimum(1),
//		UsageMaximum(8),
//		LogicalMinimum(0),
//		LogicalMaximum(1),
//		ReportSize(1),
//		ReportCount(8),
//		Input(USB_HID_INPUT_DATA | USB_HID_INPUT_VARIABLE | USB_HID_INPUT_ABS),

		//
		// The X, Y Z and Rx (Will appear as two thumb controls.
		//
		UsagePage(USB_HID_GENERIC_DESKTOP),
		Usage(USB_HID_X),
		Usage(USB_HID_Y),
		Usage(USB_HID_Z),
		Usage(USB_HID_RX),
		LogicalMinimum(-127),
		LogicalMaximum(127),
		ReportSize(8),
		ReportCount(36),
		Input(USB_HID_INPUT_DATA | USB_HID_INPUT_VARIABLE | USB_HID_INPUT_ABS),


        EndCollection,
    EndCollection,
};

static const tHIDDescriptor hidDescriptor = {
    9,                                 // bLength
    USB_HID_DTYPE_HID,                 // bDescriptorType
    0x111,                             // bcdHID (version 1.11 compliant)
    0,                                 // bCountryCode (not localized)
    1,                                 // bNumDescriptors
    {
        {
            USB_HID_DTYPE_REPORT,                  // Report descriptor
            sizeof(hidReportDescriptor)     // Size of report descriptor
        }
    }
};

static const uint8_t *const hidClassDescriptors[] = {
    hidReportDescriptor
};

uint8_t g_pui8MouseDescriptor[] =
{
    //
    // Configuration descriptor header.
    //
    9,                          // Size of the configuration descriptor.
    USB_DTYPE_CONFIGURATION,    // Type of this descriptor.
    USBShort(34),               // The total size of this full structure.
    1,                          // The number of interfaces in this
                                // configuration.
    1,                          // The unique value for this configuration.
    5,                          // The string identifier that describes this
                                // configuration.
    USB_CONF_ATTR_BUS_PWR,     // Bus Powered, Self Powered, remote wake up.
    250,                        // The maximum power in 2mA increments.
};

uint8_t g_pui8HIDInterface[HIDINTERFACE_SIZE] =
{
    //
    // HID Device Class Interface Descriptor.
    //
    9,                          // Size of the interface descriptor.
    USB_DTYPE_INTERFACE,        // Type of this descriptor.
    0,                          // The index for this interface.
    0,                          // The alternate setting for this interface.
    2,                          // The number of endpoints used by this
                                // interface.
    USB_CLASS_HID,              // The interface class
    USB_HID_SCLASS_BOOT,        // The interface sub-class.
    USB_HID_PROTOCOL_NONE,     // The interface protocol for the sub-class
                                // specified above.
    4,                          // The string index for this interface.
};

const uint8_t g_pui8HIDInEndpoint[HIDINENDPOINT_SIZE] =
{
    //
    // Interrupt IN endpoint descriptor
    //
    7,                          // The size of the endpoint descriptor.
    USB_DTYPE_ENDPOINT,         // Descriptor type is an endpoint.
    USB_EP_DESC_IN | USBEPToIndex(USB_EP_1),
    USB_EP_ATTR_INT,            // Endpoint is an interrupt endpoint.
    USBShort(USBFIFOSizeToBytes(USB_FIFO_SZ_64)),
                                // The maximum packet size.
    1,                         // The polling interval for this endpoint.
};

const uint8_t g_pui8HIDOutEndpoint[HIDINENDPOINT_SIZE] =
{
    //
    // Interrupt OUT endpoint descriptor
    //
    7,                          // The size of the endpoint descriptor.
    USB_DTYPE_ENDPOINT,         // Descriptor type is an endpoint.
    USB_EP_DESC_OUT | USBEPToIndex(USB_EP_2),
    USB_EP_ATTR_INT,            // Endpoint is an interrupt endpoint.
    USBShort(USBFIFOSizeToBytes(USB_FIFO_SZ_64)),
                                // The maximum packet size.
    1,                         // The polling interval for this endpoint.
};

const tConfigSection configSection =
{
    sizeof(g_pui8MouseDescriptor),
    g_pui8MouseDescriptor
};

const tConfigSection g_sHIDInterfaceSection =
{
    sizeof(g_pui8HIDInterface),
    g_pui8HIDInterface
};

const tConfigSection g_sHIDInEndpointSection =
{
    sizeof(g_pui8HIDInEndpoint),
    g_pui8HIDInEndpoint
};

const tConfigSection g_sHIDOutEndpointSection =
{
    sizeof(g_pui8HIDOutEndpoint),
    g_pui8HIDOutEndpoint
};

tConfigSection g_sHIDDescriptorSection =
{
   sizeof(hidDescriptor),
   (const uint8_t *)&hidDescriptor
};

const tConfigSection *configSections[] ={
    &configSection,
    &g_sHIDInterfaceSection,
    &g_sHIDDescriptorSection,
    &g_sHIDInEndpointSection,
    &g_sHIDOutEndpointSection
};

#define NUM_HID_SECTIONS        (sizeof(configSections) / sizeof(configSections[0]))

tConfigHeader configHeader = {
    NUM_HID_SECTIONS,
    configSections
};

const tConfigHeader * const configDescriptors[] = {
    &configHeader
};

tUSBDHIDDevice hidDevice = {
	USB_VID_TI_1CBE,
    N64_CONTROLLER_PID,
    250,
    USB_CONF_ATTR_BUS_PWR,
    USB_HID_SCLASS_BOOT,
    USB_HID_PROTOCOL_NONE,
    1,
    &reportIdle,
    rxCallback,
    (void*)&hidDevice,
    txCallback,
    (void*)&hidDevice,
    false,
    &hidDescriptor,
    &hidClassDescriptors,
    &stringDescriptors,
    NUM_STRING_DESCRIPTORS,
    configDescriptors
};

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3
void initN64USB() {
	// Configure USB pins
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

	USBStackModeSet(0, eUSBModeForceDevice, 0);

	USBDHIDInit(0, &hidDevice);
}


bool isRxDataAvailable() {
	return USBDHIDRxPacketAvailable(&hidDevice);
}

bool isTxDataAvailable() {
	return USBDHIDTxPacketAvailable(&hidDevice);
}

static volatile bool connectionActive = false;
static volatile bool dataReadyToRead = false;

bool isConnected() {
	return connectionActive;
}

unsigned long isDataReadyToRead() {
	return USBDHIDRxPacketAvailable(&hidDevice);
}

void sendString(const char *pcString, ...) {
    va_list vaArgP;

	va_start(vaArgP, pcString);

	char buf[64];
	uvsnprintf(buf, 64, pcString, vaArgP);
    va_end(vaArgP);
	if(isTxDataAvailable()) {
		USBDHIDReportWrite(&hidDevice, buf, ustrlen(buf), true);
	}
}

void sendUsbData(uint8_t* buffer, int length) {
	while(!isTxDataAvailable());
	USBDHIDReportWrite(&hidDevice, buffer, length, true);
}

uint8_t readBuffer[64];
uint8_t* receiveUsbData(unsigned long length) {
	USBDHIDPacketRead(&hidDevice, readBuffer, length, true);
	return readBuffer;
}

uint32_t rxCallback(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgParam, void *pvMsgData) {
    switch (ui32Event) {
    case USB_EVENT_CONNECTED:
    	connectionActive = true;
    	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    	break;
    case USB_EVENT_DISCONNECTED:
    	connectionActive = false;
    	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);
    	break;
    case USB_EVENT_RX_AVAILABLE:
    	dataReadyToRead = true;
    	break;
    }
	return 0;
}


uint32_t txCallback(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgParam, void *pvMsgData) {
	return 0;
}

