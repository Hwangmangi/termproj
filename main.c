#include <stdio.h>
#include <stdlib.h>
#include "xil_exception.h"
#include "xparameters.h"
#include "xscugic.h"
#include "pushbutton.h"

#include "xil_types.h"
#include "xparameters.h"
#include "xiicps.h"
#include "seven_seg.h"
#include "textlcd.h"

#include "xil_printf.h"
#include "xil_io.h"
#include "xparameters.h"
#include "ff.h"



#define	IIC_SLAVE_ADDR	0x51
#define	IIC_SCLK_RATE	100000



#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define INTC_DEVICE_INT_ID	31

void	WriteTLCDReg(char *pRegVal, int val);
int		ReadRTC(XIicPs Iic, u8 *SendBuffer, u8 *RecvBuffer);

int GicConfigure(u16 DeviceId);
void ServiceRoutine(void *CallbackRef); //
/************ SD card parameters ************/
static FATFS fatfs;
static FIL fil;
static char filename[32] = "lenna.bin";
FRESULT Res;
TCHAR *Path = "0:/";
u32 * buffer[65280];
u32 data_size = 4 * 65280; // 4byte * buffer_size
u32 NumBytesRead;


XScuGic InterruptController; 	     // Instance of the Interrupt Controller
static XScuGic_Config *GicConfig;    // The configuration parameters of the controller

int main(void)
{
	int Status;

	xil_printf("Interrupt Test\r\n");

	/*
	 *  Run the Gic configure, specify the Device ID generated in xparameters.h
	 */
	Status = GicConfigure(INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("GIC Configure Failed\r\n");
		return XST_FAILURE;
	}

	while(TRUE){}	// Main loop

	return XST_SUCCESS;
}
void main2(void)
{
	XIicPs	Iic;			/**< Instance of the IIC Device */
	int 	IicStatus;
	u8		*SendBuffer;	/**< Buffer for Transmitting Data 총 8bit*/
	u8		RecvBuffer[3];	/**< Buffer for Receiving Data   총 24bit*/

	int		SegReg;
	char	TlcdReg_upline[16];
	char	TlcdReg_downline[16];

	int		i;
	int		wait;


	sprintf(TlcdReg_upline, "Digital Clock   ");

	while(TRUE)
	{
		IicStatus = ReadRTC(Iic, SendBuffer, RecvBuffer);//무슨 객체값, 8bit, 24bit들어감
		//이건 32bit 정수값, peripheral 상태값인듯
		if (IicStatus != XST_SUCCESS)
		{
			return XST_FAILURE;
		}
		SegReg=(RecvBuffer[2] << 24) | (0b1010 << 20) | (RecvBuffer[1] << 12) | (0b1010 << 8) | RecvBuffer[0];
		SEVEN_SEG_mWriteReg(XPAR_SEVEN_SEG_0_S00_AXI_BASEADDR,SEVEN_SEG_S00_AXI_SLV_REG0_OFFSET,SegReg);

		// Coding here!
		//

		WriteTLCDReg(TlcdReg_downline, SegReg);
		for(i = 0; i < 4; i++)
		{
			TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, TEXTLCD_S00_AXI_SLV_REG0_OFFSET+i, TlcdReg_upline[TEXTLCD_S00_AXI_SLV_REG0_OFFSET+i]);
			TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, TEXTLCD_S00_AXI_SLV_REG1_OFFSET+i, TlcdReg_upline[TEXTLCD_S00_AXI_SLV_REG1_OFFSET+i]);
			TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, TEXTLCD_S00_AXI_SLV_REG2_OFFSET+i, TlcdReg_upline[TEXTLCD_S00_AXI_SLV_REG2_OFFSET+i]);
			TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, TEXTLCD_S00_AXI_SLV_REG3_OFFSET+i, TlcdReg_upline[TEXTLCD_S00_AXI_SLV_REG3_OFFSET+i]);

			TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, TEXTLCD_S00_AXI_SLV_REG4_OFFSET+i, TlcdReg_downline[TEXTLCD_S00_AXI_SLV_REG4_OFFSET+i]);
			TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, TEXTLCD_S00_AXI_SLV_REG5_OFFSET+i, TlcdReg_downline[TEXTLCD_S00_AXI_SLV_REG5_OFFSET+i]);
			TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, TEXTLCD_S00_AXI_SLV_REG6_OFFSET+i, TlcdReg_downline[TEXTLCD_S00_AXI_SLV_REG6_OFFSET+i]);
			TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, TEXTLCD_S00_AXI_SLV_REG7_OFFSET+i, TlcdReg_downline[TEXTLCD_S00_AXI_SLV_REG7_OFFSET+i]);
			// Coding here!
			//

		}

		for(wait = 0; wait < 1200; wait++);
	}
}

void main3()
{
    NumBytesRead = 0;
/*********************SD card read*********************/
	Res = f_mount(&fatfs, Path, 0);
    if(Res != FR_OK){
    	xil_printf("mount_fail\n");
    	return 0;
    }

    Res = f_open(&fil, filename, FA_READ);
    if(Res){
        xil_printf("file_open_fail\n");
        return 0;
    }

    Res = f_lseek(&fil, 0);
    if (Res) {
    	xil_printf("fseek_fail\n");
    	return 0;
    }

    Res = f_read(&fil, buffer, data_size, &NumBytesRead);
    if(Res){
        xil_printf("data_read_fail\n");
        return 0;
    }

    Res = f_close(&fil);

	xil_printf("file_read_success\n");


    int Data;
    int R;
    int G;
    int B;
    /****************************TFT-LCD write(RGB565)****************************/
    for (int i = 0; i < 272; i++){
    	for (int j = 0; j < 240; j++){
    		// 1
			Data = (int)buffer[j + 240*i] & 0x0000ffff;
			//xil_printf("1. Data:%08x\n", Data);
			R = (Data >> 11) & 0x0000001f;
			G = Data & 0x000007E0;
			B = Data & 0x0000001f;
			Data = (B<<11)| G | R;
			//xil_printf("2. R:%08x, G:%08x, B:%08x, Data:%08x\n", R, G, B, Data);
			Xil_Out32(XPAR_TFTLCD_0_S00_AXI_BASEADDR + (2*j + 480*i)*4, Data);

			// 2
			Data = (int)buffer[j + 240*i] >> 16;
			//xil_printf("3. Data:%08x\n", Data);
			R = (Data >> 11) & 0x0000001f;
			G = Data & 0x000007E0;
			B = Data & 0x0000001f;
			Data = (B<<11)| G | R;
			//xil_printf("4. R:%08x, G:%08x, B:%08x, Data:%08x\n", R, G, B, Data);
			Xil_Out32(XPAR_TFTLCD_0_S00_AXI_BASEADDR + (1 + 2*j + 480*i)*4, Data);
    	}
    }


    return 0;
}
int GicConfigure(u16 DeviceId)
{
	int Status;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig,
					GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			&InterruptController);

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(&InterruptController, INTC_DEVICE_INT_ID,
			   (Xil_ExceptionHandler)ServiceRoutine,
			   (void *)&InterruptController);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the device and then cause (simulate) an
	 * interrupt so the handlers will be called
	 */
	XScuGic_Enable(&InterruptController, INTC_DEVICE_INT_ID);

	return XST_SUCCESS;
}

void ServiceRoutine(void *CallbackRef)
{
	char pb;

	pb = PUSHBUTTON_mReadReg(XPAR_PUSHBUTTON_0_S00_AXI_BASEADDR, 0);

	PUSHBUTTON_mWriteReg(XPAR_PUSHBUTTON_0_S00_AXI_BASEADDR, 0, 0);

	if ((pb & 1) == 1){
		xil_printf("S1 Switch is pushed\r\n");
	}
	else if ((pb & 2) == 2){
		xil_printf("S2 Switch is pushed\r\n");
	}
	else if ((pb & 4) == 4){
		xil_printf("S3 Switch is pushed\r\n");
	}
	else if ((pb & 8) == 8){
		xil_printf("S4 Switch is pushed\r\n");
	}
}
void WriteTLCDReg(char *pRegVal, int val) //pRegVal은 16Byte의 배열
{
	int		i = 0;
	char	temp;

	for(i = 0; i < 8; i++)
	{
		temp=((u8)(val>>(28-i)) & 0x0F)|0x30;
		// Coding here!
		pRegVal[i] = temp;
		pRegVal[i+8] = 0x20;
	}
}

int ReadRTC(XIicPs Iic, u8 *SendBuffer, u8 *RecvBuffer)
{
	int				Status;
	XIicPs_Config	*Config;

	Config = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (Config == NULL)
	{
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);

	*SendBuffer		= 0x02;
	RecvBuffer[0]	= 0x00;
	RecvBuffer[1]	= 0x00;
	RecvBuffer[2]	= 0x00;

	Status = XIicPs_MasterSendPolled(&Iic, SendBuffer, 1, IIC_SLAVE_ADDR);
	if(Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	while(XIicPs_BusIsBusy(&Iic))
	{
		/* NOP */
	}

	Status = XIicPs_MasterRecvPolled(&Iic, RecvBuffer, 3, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

