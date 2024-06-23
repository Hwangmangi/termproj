#include <stdio.h>
#include "xil_printf.h"
#include "xil_io.h"
#include "xparameters.h"
#include "ff.h"

#include <stdlib.h>
#include "xil_exception.h"
#include "xscugic.h"
#include "pushbutton.h"

#include "bs.h"

#include "xil_types.h"
#include "xparameters.h"
#include "xiicps.h"
#include "seven_seg.h"
#include "textlcd.h"

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define INTC_DEVICE_INT_ID	31


//---------------------------------------------function---------------------------------------------------------
void tftlcd_print(u32 * buffer);
void receiveBuffer(u32 *buffer, char filename[], int size);
void maskBuffer(u32* buffer, u32* buffer2, int startX, int startY, int maskWidth, int maskHeight);

int GicConfigure(u16 DeviceId);
void ServiceRoutine(void *CallbackRef); //

void DisplayScoreAndState(int score, int state);
//------------------------------------------global parameter--------------------------------------------------------------

XScuGic InterruptController; 	     // Instance of the Interrupt Controller
static XScuGic_Config *GicConfig;    // The configuration parameters of the controller

/************ SD card parameters ************/
static FATFS fatfs;
static FIL fil;
FRESULT Res;
TCHAR *Path = "0:/";
u32 * buffer[65280];
u32 data_size = 4 * 65280; // 4byte * buffer_size

//expand variable
u32 * buffer2[37640];
u32 data_size2 = 4 * 36740;

static int state = 0;
static int score = 0;

char urf[4];

u32 NumBytesRead;
//----------------------------------------main function-----------------------------------------------------------------
int main()
{
    NumBytesRead = 0;
    xil_printf("terminal");
    int Status;

    Status = GicConfigure(INTC_DEVICE_ID);
    if (Status != XST_SUCCESS) {
    		xil_printf("GIC Configure Failed\r\n");
    		return XST_FAILURE;
    }

    char	TlcdReg_upline[16] = "  COMMON SENSE  ";
    char	TlcdReg_downline[16] = "  QUIZ TOTAL 10 ";
    int i;
    for(i = 0; i < 4; i++)
    		{

    			TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i*4, (TlcdReg_upline[i*4]<<24) + (TlcdReg_upline[i*4+1]<<16) + (TlcdReg_upline[i*4+2]<<8) + TlcdReg_upline[i*4+3]);
    			TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i*4+16, (TlcdReg_downline[i*4]<<24) + (TlcdReg_downline[i*4+1]<<16) + (TlcdReg_downline[i*4+2]<<8) + TlcdReg_downline[i*4+3]);

    		}


    while(TRUE){}




    return XST_SUCCESS;


    return 0;
}

//-----------------------------------------fuction----------------------------------------------------------------
void maskBuffer(u32* buffer, u32* buffer2, int startX, int startY, int maskWidth, int maskHeight) {
    // Dimensions of buffer
    int width = 240;
    int height = 272;

    // Define the region to mask
    int maskX = startX;
    int maskY = startY;
    int maskEndX = maskX + maskWidth;
    int maskEndY = maskY + maskHeight;

    // Iterate through the buffer
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (i >= maskY && i < maskEndY && j >= maskX && j < maskEndX) {
                // Calculate index for buffer2
                int maskIndex = (i - maskY) * maskWidth + (j - maskX);
                // Replace buffer data with buffer2 data
                buffer[j + width * i] = buffer2[maskIndex];
            }
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------

void receiveBuffer(u32 *buffer, char filename[], int data_size){ //65280, 1800, 450

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


}
//---------------------------------------------------------------------------------------------------------------------

void tftlcd_print(u32 * buffer){
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
}
//---------------------------------------------------------------------------------------------------------------------


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
//---------------------------------------------------------------------------------------------------------------------


void ServiceRoutine(void *CallbackRef)
{
	char pb;
	int pb2;
	int i;
	pb = PUSHBUTTON_mReadReg(XPAR_PUSHBUTTON_0_S00_AXI_BASEADDR, 0);
	pb2 = BS_mReadReg(XPAR_BS_0_S00_AXI_BASEADDR,0);
	PUSHBUTTON_mWriteReg(XPAR_PUSHBUTTON_0_S00_AXI_BASEADDR, 0, 0);
	xil_printf("S1 Switch is pushed\r\n");

	switch (state) //pos4
			{

				case 0:
					receiveBuffer(buffer, "start.bin", data_size);
					tftlcd_print(buffer);
					state+=1;
					receiveBuffer(buffer, "quiz1.bin", data_size);
				    tftlcd_print(buffer);
					break;

				case 1:

				    state+=1;
				    if (pb==1){
				    	score+=1;
				    	DisplayScoreAndState(score, state);
				    	receiveBuffer(buffer, "quiz2.bin", data_size);
				    	tftlcd_print(buffer);

						break;
				    }
				    else{
				    	DisplayScoreAndState(score, state);
				    	receiveBuffer(buffer, "quiz2.bin", data_size);
				    	tftlcd_print(buffer);
						break;
				    }

				case 2:

					state+=1;
				    if (pb==1){
				    	score+=1;
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz3.bin", data_size);
						tftlcd_print(buffer);
						break;
				    }
				    else{
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz3.bin", data_size);
						tftlcd_print(buffer);
						break;}

				case 3:

					state+=1;
				    if (pb==1){
				    	score+=1;
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz4.bin", data_size);
						tftlcd_print(buffer);
						break;
				    }
				    else{
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz4.bin", data_size);
						tftlcd_print(buffer);
						break;}

				case 4:

					state+=1;
				    if (pb==1){
				    	score+=1;
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz5.bin", data_size);
						tftlcd_print(buffer);

						break;
				    }
				    else{
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz5.bin", data_size);
						tftlcd_print(buffer);
						break;}

				case 5:

					state+=1;
				    if (pb==1){
				    	score+=1;
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz6.bin", data_size);
						tftlcd_print(buffer);
						break;
				    }
				    else{
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz6.bin", data_size);
						tftlcd_print(buffer);
						break;}

				case 6:

					state+=1;
				    if (pb==1){
				    	score+=1;
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz7.bin", data_size);
						tftlcd_print(buffer);
						break;
				    }
				    else{
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz7.bin", data_size);
						tftlcd_print(buffer);
						break;}

				case 7:

					state+=1;
				    if (pb==2){
				    	score+=1;
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz8.bin", data_size);
						tftlcd_print(buffer);
						break;
				    }
				    else{
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz8.bin", data_size);
						tftlcd_print(buffer);
						break;}

				case 8:

					state+=1;
				    if (pb==8){
				    	score+=1;
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz9.bin", data_size);
						tftlcd_print(buffer);
						break;
				    }
				    else{
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz9.bin", data_size);
						tftlcd_print(buffer);
						break;}

				case 9:

					state+=1;
				    if (pb==8){
				    	score+=1;
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz10.bin", data_size);
						tftlcd_print(buffer);
						break;
				    }
				    else{
				    	DisplayScoreAndState(score, state);
						receiveBuffer(buffer, "quiz10.bin", data_size);
						tftlcd_print(buffer);
						break;}

				case 10:

					state+=1;
				    if (pb==1){
				    	score+=1;
				    	DisplayScoreAndState(score, state);
				    	receiveBuffer(buffer, "press.bin", data_size);
				    	tftlcd_print(buffer);
						break;
				    }
				    else{
				    	DisplayScoreAndState(score, state);
				    	receiveBuffer(buffer, "press.bin", data_size);
				    	tftlcd_print(buffer);
						break;}

				case 11:
					if(score > 8){
						int i;
				    	for(i=2;i<101;i++){
				    		sprintf(urf, "a%d.bin", i);
				    		receiveBuffer(buffer, urf, data_size);
				    		tftlcd_print(buffer);
				    	}
				    	receiveBuffer(buffer, "end.bin", data_size);
				    	tftlcd_print(buffer);
				    	state+=1;
				    	break;
					}
				    	else{
				    		receiveBuffer(buffer, "end.bin", data_size);
				    		tftlcd_print(buffer);
				    		DisplayScoreAndState(score, state);
				    		state+=1;
				    		break;
				    	}

				default:

					break;

	}
}

void DisplayScoreAndState(int score, int state)
{
    char TlcdReg_upline[16];
    char TlcdReg_downline[16];

    sprintf(TlcdReg_upline, "score: %d        ", score);
    sprintf(TlcdReg_downline, "quiz number: %d  ", state);

    for (int i = 0; i < 4; i++)
    {
        TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i * 4, (TlcdReg_upline[i * 4] << 24) + (TlcdReg_upline[i * 4 + 1] << 16) + (TlcdReg_upline[i * 4 + 2] << 8) + TlcdReg_upline[i * 4 + 3]);
        TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i * 4 + 16, (TlcdReg_downline[i * 4] << 24) + (TlcdReg_downline[i * 4 + 1] << 16) + (TlcdReg_downline[i * 4 + 2] << 8) + TlcdReg_downline[i * 4 + 3]);
    }
}
