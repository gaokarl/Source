/*******************************************************************************
* Copyright (C) 2013 Spansion LLC. All Rights Reserved. 
*
* This software is owned and published by: 
* Spansion LLC, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with Spansion 
* components. This software is licensed by Spansion to be adapted only 
* for use in systems utilizing Spansion components. Spansion shall not be 
* responsible for misuse or illegal use of this software for devices not 
* supported herein.  Spansion is providing this software "AS IS" and will 
* not be responsible for issues arising from incorrect user implementation 
* of the software.  
*
* SPANSION MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS), 
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING, 
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED 
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED 
* WARRANTY OF NONINFRINGEMENT.  
* SPANSION SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, 
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT 
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, 
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR 
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, 
* SAVINGS OR PROFITS, 
* EVEN IF SPANSION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED 
* FROM, THE SOFTWARE.  
*
* This software may be replicated in part or whole for the licensed use, 
* with the restriction that this Disclaimer and Copyright notice must be 
* included with each copy of this software, whether used in part or whole, 
* at all times.  
*/
/******************************************************************************/
/** \file UsbDevice.h
 **
 ** A detailed description is available at 
 ** @link UsbDeviceGroup USB Device Module description @endlink
 **
 ** History:
 **   - 2012-08-28  2.0  MSc  First version (starting at version 2.0).
 **   - 2012-10-02  2.1  MSc  use of external interrupts without L3 implemented
 **   - 2012-11-13  2.2  MSc  Some problems while switching from Host to Device fixed
 **   - 2013-01-31  2.3  MSc  DMA added
 **   - 2013-05-07  2.4  MSc  Bug with sending wrong package sizes solved
 **   - 2013-05-21  2.5  MSc  Some Windows versions request only 4 bytes of string descriptor
 **                           ->fixed: more than only 4 bytes will be sent
 **   - 2013-06-04  2.6  MSc  FM4 support added
 **   - 2013-10-17  2.7  MSc  Device Configuration change: support for more than one report descriptor
 **   - 2014-02-28  2.8  MSc  Busy wait added to send and receive initiate procedure
 **   - 2015-05-13  2.9  MSCH Better timeout handling added
 **   - 2015-05-29  3.0  MSCH bit access for EPnS register changed for more stable access
 **                           added better data overflow handling
 **   - 2015-07-21  3.1  MSCH check DRQ bit before continue data transfers for safety reasons
 **
 ******************************************************************************/

#ifndef __USBDEVICE_H__
#define __USBDEVICE_H__
     
#include "usb.h"
     
/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup UsbDeviceGroup USB Device Mode Functionality
 **
 ** Provided functions of USB module:
 ** 
 ** - UsbDevice_ReceiveData()
 ** - UsbDevice_SendData()
 ** - UsbDevice_SendDataControl()
 ** - UsbDevice_ReceiveDataControl()
 ** - UsbDevice_RegisterVendorClass()
 ** - UsbDevice_GetStatus()
 ** - UsbDevice_InitCallback()
 ** - UsbDevice_Init()
 ** - UsbDevice_SetupEndpoint()
 **
 ** Used to transfer data with EP0..EP5, to do enumeration process and support different USB class drivers.
 **
 ******************************************************************************/
//@{

/**
 ******************************************************************************
 ** \page example_usbdevice_classinit Example initializing a vendor class by use of USB descriptors
 **  
 ** For initializing a USB class by USB descriptor, the u8InterfaceNumber in the stc_usbdevice_class_config_t
 ** must be set to 0xFF. This does only work if the interface can be found by interface class, subclass and protocol. 
 **
 ** @code 
 ** stcUsbClassConfig.u8InterfaceNumber          = 0xFF;                          // do not use fix interface number, choose by class, subclass, protocol
 ** stcUsbClassConfig.u8InterfaceClass           = 0x02;                          // CDC Class
 ** stcUsbClassConfig.u8InterfaceSubClass        = 0x02;                          // Custom Sub Class
 ** stcUsbClassConfig.u8InterfaceProtocol        = 0x01;                          // Custom Protocol
 ** @endcode
 **   
 ** Additional callbacks can be initialized
 ** @code 
 ** stcUsbClassConfig.pfnSetupRequestCallback    = DecodeSetupRequest;            // setup requests handled
 ** stcUsbClassConfig.pfnConnectionCallback      = NULL;                          // No connection callback handled
 ** stcUsbClassConfig.pfnDisconnectionCallback   = NULL;                          // No disconnection callback handled
 ** stcUsbClassConfig.pfnConfCallback            = ConfCallback;                  // Callback for configuration set    
 ** @endcode   
 **   
 ** UsbDevice_RegisterVendorClass registers the class configuration and searches automatically for the correct interface
 ** and returns this interface in &pu8Interface with the length of it in &u8InterfaceLength.   
 **    
 ** @code     
 ** UsbDevice_RegisterVendorClass(pstcUsb,&stcUsbClassConfig,&pu8Interface,&u8InterfaceLength);   
 ** @endcode   
 **
 ** This example gives an idea how to setup the endpoints with the returned interface data by use of
 ** UsbDevice_SetupEndpoint.
 ** @code    
 **    u8NumEndpoints = 0;
 **    u8ControlInterface = pu8Interface[2];
 **    for(u8i = 0;u8i < u8InterfaceLength;)
 **    {
 **        u8i += pu8Interface[u8i];
 **        if (USBDESCR_ENDPOINT == pu8Interface[u8i + 1])
 **        {
 **            stcEndpointConfig.u8EndpointAddress = pu8Interface[u8i + 2];
 **            stcEndpointConfig.pfnRxTxCallback = NULL;
 **            if (((stcEndpointConfig.u8EndpointAddress) & 0x80) != 0)
 **            {
 **                stcEndpointConfig.pstcEndpointBuffer = &stcEndpointBufferControlIN;
 **                pstcEndpointControlIN = UsbDevice_SetupEndpoint(pstcUsb, &stcEndpointConfig);
 **                pstcEndpointControlIN->bAutomaticNullTermination = TRUE;
 **            }
 **            u8NumEndpoints++;
 **        }
 **    } 
 ** @endcode
 **
 ** To initialize the whole class, the whole initialization must be called within the pfnInitClassesCallback
 ** of the stc_usbdevice_config_t config in usbconfig.c. Normally all active classes are initialized in usbconfig.c
 ** within the default callback for pfnInitClassesCallback called UsbConfig_UsbDeviceClassesInitCallback<n>.
 ** @code
 ** stcUsbDeviceConfig.pfnInitClassesCallback = UsbConfig_UsbDeviceClassesInitCallback0; //for USB0
 ** @endcode   
 **
 ** If the initialization of the example class is called UsbDeviceCdcCom_Init, the code in UsbConfig_UsbDeviceClassesInitCallback0
 ** can look like this:
 ** @code    
 ** #if FM_PERIPHERAL_ENABLE_USB0_DEVICE == ON
 **     static void UsbConfig_UsbDeviceClassesInitCallback0(stc_usbn_t* pstcUSB)
 **     {
 **         #if (USBDEVICECDCCOM_ENABLED == ON)
 **             UsbDeviceCdcCom_Init((stc_usbn_t*)&USB0);
 **         #endif
 **     }    
 ** #endif 
 ** #if FM_PERIPHERAL_ENABLE_USB1_DEVICE == ON
 **     static void UsbConfig_UsbDeviceClassesInitCallback1(stc_usbn_t* pstcUSB)
 **     {
 **         #if (USBDEVICECDCCOM_ENABLED == ON)
 **             UsbDeviceCdcCom_Init((stc_usbn_t*)&USB1);
 **         #endif
 **     }    
 ** #endif     
 ** @endcode    
 **   
 **   
 ** The whole UsbDeviceCdcCom_Init example:   
 ** @code 
 **   
 ** static stc_usbn_endpoint_data_t* pstcEndpointControlIN;
 ** static stc_usbn_endpoint_buffer_t stcEndpointBufferControlIN;  
 **   
 ** static void DecodeSetupRequest(stc_usbn_t* pstcUsb, stc_usb_request_t* pstcSetup);   
 ** static void ConfCallback(stc_usbn_t* pstcUsb);   
 **   
 ** void UsbDeviceCdcCom_Init(stc_usbn_t* pstcUsb)
 ** {
 **     stc_usbdevice_class_config_t stcUsbClassConfig;
 **     stc_usbdevice_endpoint_config_t stcEndpointConfig;
 **     uint8_t* pu8Interface = NULL;
 **     uint8_t u8InterfaceLength = 0;
 **     uint8_t u8i = 0;
 **     uint8_t u8NumEndpoints = 0;
 **     pstcUsbHandle = pstcUsb;
 **     
 **     stcUsbClassConfig.u8InterfaceNumber          = 0xFF;                          // do not use fix interface number, choose by class, subclass, protocol
 **     stcUsbClassConfig.u8InterfaceClass           = 0x02;                          // CDC Class
 **     stcUsbClassConfig.u8InterfaceSubClass        = 0x02;                          // Custom Sub Class
 **     stcUsbClassConfig.u8InterfaceProtocoll       = 0x01;                          // Custom Protocol
 **     stcUsbClassConfig.pfnSetupRequestCallback    = DecodeSetupRequest;            // setup requests handled
 **     stcUsbClassConfig.pfnConnectionCallback      = NULL;                          // No connection callback handled
 **     stcUsbClassConfig.pfnDisconnectionCallback   = NULL;                          // No disconnection callback handled
 **     stcUsbClassConfig.pfnConfCallback            = ConfCallback;                  // Callback for configuration set
 **     UsbDevice_RegisterVendorClass(pstcUsb,&stcUsbClassConfig,&pu8Interface,&u8InterfaceLength);
 **
 **    u8NumEndpoints = 0;
 **    u8ControlInterface = pu8Interface[2];
 **    for(u8i = 0;u8i < u8InterfaceLength;)
 **    {
 **        u8i += pu8Interface[u8i];
 **        if (USBDESCR_ENDPOINT == pu8Interface[u8i + 1])
 **        {
 **            stcEndpointConfig.u8EndpointAddress = pu8Interface[u8i + 2];
 **            stcEndpointConfig.pfnRxTxCallback = NULL;
 **            if (((stcEndpointConfig.u8EndpointAddress) & 0x80) != 0)
 **            {
 **                stcEndpointConfig.pstcEndpointBuffer = &stcEndpointBufferControlIN;
 **                pstcEndpointControlIN = UsbDevice_SetupEndpoint(pstcUsb, &stcEndpointConfig);
 **                pstcEndpointControlIN->bAutomaticNullTermination = TRUE;
 **            }
 **            u8NumEndpoints++;
 **        }
 **    } 
 **   
 ** @endcode
 **
 ******************************************************************************/
    
#define USBDEVICE_MAXCLASSES 5
     
/**
 ******************************************************************************
 ** \brief setup request callback handle
 **
 ** \param pstcUsb USB handle
 **
 ** \param pstcSetup Pointer to setup request
 **
 ******************************************************************************/
typedef void (*usbdevice_setuprequest_func_ptr_t)(stc_usbn_t* pstcUsb, stc_usb_request_t* pstcSetup);

/**
 ******************************************************************************
 ** \brief control data transferred
 **
 ** \param pstcUsb USB handle
 **
 ** \param pstcEndpoint Endpoint handle
 **
 ******************************************************************************/
typedef void (*usbdevice_controltranfered_func_ptr_t)(stc_usbn_t* pstcUsb, stc_usbn_endpoint_data_t* pstcEndpoint);

/**
 ******************************************************************************
 ** \brief init class callback
 **
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usbdevice_initclasses_func_ptr_t)(stc_usbn_t* pstcUsb);



/**
 ******************************************************************************
 ** \brief USB device class configuration
 **
 ** used to setup USB classes
 **
 ******************************************************************************/
typedef struct stc_usbdevice_class_config
{
    uint8_t u8InterfaceNumber;                                  ///< Interface Number, set to 0xFF, if automatically search for
    uint8_t u8InterfaceClass;                                   ///< Interface Class
    uint8_t u8InterfaceSubClass;                                ///< Interface Sub Class
    union
    {
        uint8_t u8InterfaceProtocoll;                               ///< Interface Protocoll
        uint8_t u8InterfaceProtocol;                                ///< Interface Protocol
    };
    usbdevice_setuprequest_func_ptr_t pfnSetupRequestCallback;  ///< Callback for setup requests
    usb_connect_func_ptr_t pfnConnectionCallback;               ///< Callback on device connect
    usb_disconnect_func_ptr_t pfnDisconnectionCallback;         ///< Callback on device disconnect
    usb_conf_isr_func_ptr_t pfnConfCallback;                    ///< Callback on device configuration changed
} stc_usbdevice_class_config_t;

/**
 ******************************************************************************
 ** \brief USB device class configuration
 **
 ** used to setup USB classes
 **
 ******************************************************************************/
typedef struct stc_usbdevice_intern_data
{
  uint8_t u8Dummy;
  en_usb_status_t enStatus;                                              ///< USB HAL status
  uint8_t* pu8DeviceDescriptor;                                          ///< Pointer to USB device descriptor
  uint8_t* pu8ConfigDescriptor;                                          ///< Pointer to USB configuration descriptor
  stc_usbdevice_reportdescriptor_t* astcReportDescriptors;               ///< Pointer to USB report descriptor struct array
  stc_usbdevice_stringdescriptor_t* pstcStringDescriptors;               ///< Pointer to USB string descriptors
  uint8_t u8StringDescriptorCount;                                       ///< Number of USB string descriptors
  stc_usbn_endpoint_data_t* pstcEp0OUT;                                  ///< Pointer of Endpoint 0 OUT handle
  stc_usbn_endpoint_data_t* pstcEp0IN;                                   ///< Pointer of Endpoint 0 IN handle
  en_usb_control_stage_t enControlStage;                                 ///< Control endpoint stage
  uint8_t pu8ControlReceiveBuffer[64];                                   ///< Buffer for control endpoint
  stc_usbn_endpoint_buffer_t stcEndpointBuffer;                          ///< Buffer struct of endpoint buffer
  stc_usbdevice_class_config_t stcUsbClasses[USBDEVICE_MAXCLASSES];      ///< USB class configurations
  usbdevice_initclasses_func_ptr_t pfnInitClassesCallback;               ///< Callback to initialize classes
  usbdevice_controltranfered_func_ptr_t pfnControlTransferred;           ///< Callback control data transferred
  volatile uint8_t* pu8LastControlTransfer;
} stc_usbdevice_intern_data_t;

/**
 ******************************************************************************
 ** \brief USB device configuration
 **
 ******************************************************************************/
typedef struct stc_usbdevice_config
{
    uint8_t* pu8DeviceDescriptor;                                 ///< Pointer to USB device descriptor
    uint8_t* pu8ConfigDescriptor;                                 ///< Pointer to USB configuration descriptor
    stc_usbdevice_reportdescriptor_t* astcReportDescriptors;      ///< Pointer to USB report descriptor struct array
    stc_usbdevice_stringdescriptor_t* pstcStringDescriptors;      ///< Pointer to USB string descriptor
    uint8_t u8StringDescriptorCount;                              ///< Number of string descriptors
    usbdevice_initclasses_func_ptr_t pfnInitClassesCallback;      ///< Class initialization callback
} stc_usbdevice_config_t;


/**
 ******************************************************************************
 ** \brief USB device endpoint configuration
 **
 ******************************************************************************/
typedef struct stc_usbdevice_endpoint_config
{
  uint8_t u8EndpointAddress;                                        ///< Endpointaddress
  volatile usb_endpoint_datatransferred_func_ptr_t pfnRxTxCallback; ///< Data transferred callback
  stc_usbn_endpoint_buffer_t* pstcEndpointBuffer;                   ///< Pointer to endpoint buffer struct
} stc_usbdevice_endpoint_config_t;



/**
 ******************************************************************************
 ** \brief USB device instance
 **
 ******************************************************************************/
typedef struct stc_usbdevicen_instance_data
{
    volatile stc_usbn_t*  pstcInstance;  ///< pointer to registers of an instance
    stc_usbdevice_intern_data_t stcInternData; ///< module internal data of instance
} stc_usbdevicen_instance_data_t;

/**
 ******************************************************************************
 ** \brief USB device sending mode
 **
 ******************************************************************************/
typedef enum en_usbsend_mode
{
  UsbPOLL  = 0,  ///< send polled
  UsbIRQ   = 1,  ///< send IRQ
  UsbDMA   = 2,  ///< send DMA
} en_usbsend_mode_t;




en_result_t UsbDevice_ReceiveData(stc_usbn_t* pstcUsb, stc_usbn_endpoint_data_t* pstcEndpoint, uint8_t* pu8Buffer, uint32_t u32DataSize, en_usbsend_mode_t enMode);

en_result_t UsbDevice_SendData(stc_usbn_t* pstcUsb, stc_usbn_endpoint_data_t* pstcEndpoint, uint8_t* pu8Buffer, uint32_t u32DataSize, en_usbsend_mode_t enMode);

void UsbDevice_SendDataControl(stc_usbn_t * pstcUsb,uint8_t* pu8Buffer, uint32_t u32DataLength);

void UsbDevice_ReceiveDataControl(stc_usbn_t * pstcUsb, usbdevice_controltranfered_func_ptr_t pfnCompletionCallback);

en_result_t UsbDevice_RegisterVendorClass(stc_usbn_t* pstcUsb, stc_usbdevice_class_config_t* pstcUsbClassConfig, uint8_t** ppu8Buffer, uint8_t* pu8InterfaceLength);

en_usb_status_t UsbDevice_GetStatus(stc_usbn_t* pstcUsb);

en_usb_endpoint_status_t UsbDevice_HalEndpointStatus(stc_usbn_endpoint_data_t* pstcEpHandle);

void UsbDevice_InitCallback(stc_usbn_t* pstcUsb, stc_usb_config_t* pstcUsbConfig);

void UsbDevice_Init(stc_usbn_t* pstcUsb, stc_usb_config_t* pstcUsbConfig, stc_usbdevice_config_t* pstcConfig);

stc_usbn_endpoint_data_t* UsbDevice_SetupEndpoint(stc_usbn_t* pstcUsb, stc_usbdevice_endpoint_config_t* pstcConfig);


#define UsbDevice_HalEndpointStatus(x) Usb_HalEndpointStatus(x)

#define UsbDevice_HalEndpointIsBusy(x) Usb_HalEndpointIsBusy(x)

#define UsbDevice_HalStallEndpoint(x) Usb_HalStallEndpoint(x)

#define UsbDevice_HalUnstallEndpoint(x) Usb_HalUnstallEndpoint(x)

#if (L3_PERIPHERAL_ENABLE_USB0 == L3_ON)
    void UsbDevice_Usb0ExintCallback(void);
#endif
#if (L3_PERIPHERAL_ENABLE_USB1 == L3_ON)
    void UsbDevice_Usb1ExintCallback(void);
#endif
#ifdef __cplusplus
}
#endif
//@} // UsbDeviceGroup
#endif
