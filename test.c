
#include "xparameters.h"
#include "xuartlite.h"
#include "stdio.h"
#include "xil_printf.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_printf.h"


// ----------------------------------------------------
// CONSTANT
// ----------------------------------------------------
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define UARTLITE_DEVICE_ID	  XPAR_UARTLITE_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define UARTLITE_INT_IRQ_ID     XPAR_INTC_0_UARTLITE_0_VEC_ID

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the UartLite device.
 */
#define TEST_BUFFER_SIZE        5

// ----------------------------------------------------
// FUNCTION PROTOTYPE
// ----------------------------------------------------
int pmod_rs485_uart_example(u16 DeviceId);
int SetupInterruptSystem(XUartLite *UartLitePtr);
void SendHandler(void *CallBackRef, unsigned int EventData);
void RecvHandler(void *CallBackRef, unsigned int EventData);

// ----------------------------------------------------
// VARIABLE DEFINITIONS
// ----------------------------------------------------
XUartLite UartLite;            		// The instance of the UartLite Device
XIntc InterruptController;     		// The instance of the Interrupt Controller

u8 SendBuffer[TEST_BUFFER_SIZE];	// UART TX buffer (shared with interrupt code)
u8 ReceiveBuffer[TEST_BUFFER_SIZE];	// UART RX buffer (shared with interrupt code)

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
static volatile int TotalReceivedCount; // UART RX char counter (shared with interrupt code)
static volatile int TotalSentCount;     // UART TX char counter (shared with interrupt code)
static volatile int TxIsDone=TRUE;		// UART TX flag (TRUE=tx is possible) (shared with interrupt code)
static volatile int RxIsDone=TRUE;		// UART RX flag (TRUE=rx is possible) (shared with interrupt code)
static volatile int TxMaxChr;			// UART TX chars number to transmit (shared with interrupt code)


/***************************************************************************
* MAIN
****************************************************************************/
int main(void)
{
	int Status;

	xil_printf("System start\n\r");

	/* run example UART read/write */
	Status = pmod_rs485_uart_example(UARTLITE_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return 0;
	}

}

// ----------------------------------------------------
// FUNCTION DEFINITION
// ----------------------------------------------------
/**
 * sends and receives data from pmod device
 */
int pmod_rs485_uart_example(u16 DeviceId)
{
	int Status;
	int Index;
	XUartLite_Config *Config;

	/*
	 * Initialize the UART driver so that it's ready to use
	 * Look up the configuration in the config table and then initialize it.
	 */
	Config = XUartLite_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XUartLite_Initialize(&UartLite, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XUartLite_SelfTest(&UartLite);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the UartLite to the interrupt subsystem such that interrupts can
	 * occur. This function is application specific.
	 */
	Status = SetupInterruptSystem(&UartLite);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the UartLite that will be called from the
	 * interrupt context when data has been sent and received, specify a
	 * pointer to the UartLite driver instance as the callback reference so
	 * that the handlers are able to access the instance data.
	 */
	XUartLite_SetSendHandler(&UartLite, SendHandler, &UartLite);
	XUartLite_SetRecvHandler(&UartLite, RecvHandler, &UartLite);

	/*
	 * Enable the interrupt of the UartLite so that interrupts will occur.
	 */
	XUartLite_EnableInterrupt(&UartLite);

	/*
	 * Initialize the send buffer bytes with a pattern to send and the
	 * the receive buffer bytes to zero to allow the receive data to be
	 * verified.
	 */

	u8 SendBuffer[] = {0x56, 0x00, 0x36, 0x01, 0x00};
	TxMaxChr = sizeof(SendBuffer);

	/* Send the buffer using the UartLite.
	 */
	XUartLite_Send(&UartLite, SendBuffer, TxMaxChr);
	if (TxIsDone == TRUE) {
		xil_printf("Transmit OK\n\r");
	}

	for (Index = 0; Index < TxMaxChr; Index++) {
		ReceiveBuffer[Index] = 0;
	}

	/*
	 * Receive the buffer using the UartLite.
	 */
	XUartLite_Recv(&UartLite, ReceiveBuffer, TxMaxChr);
	if (RxIsDone == TRUE) {
		xil_printf("Receive OK\n\r");
	}
	//len = XUartLite_Send(&UartLite, SendBuffer, TEST_BUFFER_SIZE);
	//if (len == sizeof(SendBuffer)) {
	//	xil_printf("Sent to device\n\r");
	//}

	/*
	 * Start receiving data before sending it since there is a loopback.
	 */
	//XUartLite_Recv(&UartLite, ReceiveBuffer, TEST_BUFFER_SIZE);

	/*


	/*
	 * Wait for the entire buffer to be received, letting the interrupt
	 * processing work in the background, this function may get locked
	 * up in this loop if the interrupts are not working correctly.
	 */
	//while ((TotalReceivedCount != TEST_BUFFER_SIZE) ||
	//	(TotalSentCount != TEST_BUFFER_SIZE)) {
	//}


	/*
	 * Verify the entire receive buffer was successfully received.
	 */
	//for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
	//	if (ReceiveBuffer[Index] != SendBuffer[Index]) {
	//		xil_printf("Data not received\n\r");
	//		return XST_FAILURE;
	//	} else xil_printf("Data Received\n\r");
	//}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the handler which performs processing to send data to the
* UartLite. It is called from an interrupt context such that the amount of
* processing performed should be minimized. It is called when the transmit
* FIFO of the UartLite is empty and more data can be sent through the UartLite.
*
* This handler provides an example of how to handle data for the UartLite,
* but is application specific.
*
* @param	CallBackRef contains a callback reference from the driver.
*		In this case it is the instance pointer for the UartLite driver.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void SendHandler(void *CallBackRef, unsigned int EventData)
{
	// Get the number of transmitted chars
	TotalSentCount = EventData;

	// Check if all is done
	if (TotalSentCount >= TxMaxChr){
		TxIsDone = TRUE;
	}else{
		TxIsDone = FALSE;
	}

}


/****************************************************************************/
/**
*
* This function is the handler which performs processing to receive data from
* the UartLite. It is called from an interrupt context such that the amount of
* processing performed should be minimized.  It is called data is present in
* the receive FIFO of the UartLite such that the data can be retrieved from
* the UartLite. The size of the data present in the FIFO is not known when
* this function is called.
*
* This handler provides an example of how to handle data for the UartLite,
* but is application specific.
*
* @param	CallBackRef contains a callback reference from the driver, in
*		this case it is the instance pointer for the UartLite driver.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void RecvHandler(void *CallBackRef, unsigned int EventData)
{

	// Get the number of received chars
	TotalReceivedCount = EventData;

	// Check if all is done
	if (TotalReceivedCount >= TxMaxChr){
		RxIsDone = TRUE;
	}else{
		RxIsDone = FALSE;
	}
}


/****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the UartLite device. This function is application specific since the
* actual system may or may not have an interrupt controller. The UartLite
* could be directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param    UartLitePtr contains a pointer to the instance of the UartLite
*           component which is going to be connected to the interrupt
*           controller.
*
* @return   XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note     None.
*
****************************************************************************/
int SetupInterruptSystem(XUartLite *UartLitePtr)
{

	int Status;


	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&InterruptController, UARTLITE_INT_IRQ_ID,
			   (XInterruptHandler)XUartLite_InterruptHandler,
			   (void *)UartLitePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the UartLite can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the UartLite device.
	 */
	XIntc_Enable(&InterruptController, UARTLITE_INT_IRQ_ID);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)XIntc_InterruptHandler,
			 &InterruptController);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
