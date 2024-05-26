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

#include "button_moore.h"

#define	IIC_SLAVE_ADDR	0x51
#define	IIC_SCLK_RATE	100000
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define INTC_DEVICE_INT_ID	31

void	WriteTLCDReg(char *pRegVal, int val);
int		ReadRTC(XIicPs Iic, u8 *SendBuffer, u8 *RecvBuffer);
int GicConfigure(u16 DeviceId);
void ServiceRoutine(void *CallbackRef); //
void DataRead(char filename[], FATFS fatfs, TCHAR Path, FIL fil, u32 buffer[], u32 data_size, u32 NumBytesRead);
/************ SD card parameters ************/
//tftlcd瑜� �쐞�븳 �쟾�뿭蹂��닔 �떆�옉
static FATFS fatfs;
static FIL fil;
static char board[32] = "board.bin";
static char blank[32] = "black.bin"
static char general[32] = "general.bin";
static char merchant[32]  = "merchant.bin";
static char king[32] = "king.bin";
static char prince[32] = "prince.bin";
static char hoo[32] = "hoo.bin";
static char setting[32] = "setting.bin";
static char ending[32] = "ending.bin";

FRESULT Res;
TCHAR *Path = "0:/";
u32 * buffer[65280];
u32 data_size = 4 * 65280; // 4byte * buffer_size

u32 *buffer2[1800];
u32 data_size2 = 4*1800;

u32 *buffer3[450];
u32 data_size3 = 4*450;
u32 NumBytesRead;
//�뿬湲곌퉴吏� tftlcd瑜� �쐞�븳 �쟾�뿭蹂��닔
int *button_state = 0;
//0: 留�,�룷濡쒖꽑�깮 �긽�깭, 1:留먯꽑�깮�긽�깭, 2:諛⑺뼢�꽑�깮 �긽�깭, 3:蹂대뱶�쐞移� �꽑�깮�긽�깭
typedef struct board_pos
{
	int small1_x;
	int small2_x;
	int small3_x;
	int small4_x;
	int small5_x;
	int small6_x;

	int pos1_x;
	int pos2_x;
	int pos3_x;
	int pos4_x;

	int pos5_x;
	int pos6_x;
	int pos7_x;
	int pos8_x;

	int pos9_x;
	int pos10_x;
	int pos11_x;
	int pos12_x;

	int small1_y;
	int small2_y;
	int small3_y;
	int small4_y;
	int small5_y;
	int small6_y;

	int pos1_y;
	int pos2_y;
	int pos3_y;
	int pos4_y;

	int pos5_y;
	int pos6_y;
	int pos7_y;
	int pos8_y;

	int pos9_y;
	int pos10_y;
	int pos11_y;
	int pos12_y;
	//0: None, 1: king, 2:general, 3:merchant, 4:prince, 5:hoo
	int small1;
	int small2;
	int small3;
	int small4;
	int small5;
	int small6;

	int pos1;
	int pos2;
	int pos3;
	int pos4;

	int pos5;
	int pos6;
	int pos7;
	int pos8;

	int pos9;
	int pos10;
	int pos11;
	int pos12;
	
}board_pos;
/*
--------------------------------------------------------------------------------------------
|                |          |         |         |         |
|                | pos1     | pos2    | pos7    | pos8    |       small4
|                |          |         |         |         |       small5
|                |          |         |         |         |       small6
|                |----------|---------|---------|---------|
|                |          |         |         |         |
|                | pos3     | pos4    | pos9    | pos10   |
|                |          |         |         |         |
|                |          |         |         |         |
|  small1        |----------|---------|---------|---------|
|  small2        |          |         |         |         |
|  small3        | pos5     | pos6    | pos11   | pos12   |
|                |          |         |         |         |
|                |          |         |         |         |
---------------------------------------------------------------------------------------------

*/


XScuGic InterruptController; 	     // Instance of the Interrupt Controller
static XScuGic_Config *GicConfig;    // The configuration parameters of the controller

// 1. 보드의 정보를 담은 구조체를 선언하고
board_pos bp = {
        .small1_x = 6, .small2_x = 6, .small3_x = 6, .small4_x = 427, .small5_x = 427, .small6_x = 427,
        .pos1_x = 81, .pos2_x = 163, .pos3_x = 243, .pos4_x = 327,
        .pos5_x = 81, .pos6_x = 163, .pos7_x = 243, .pos8_x = 327,
        .pos9_x = 81, .pos10_x = 163, .pos11_x = 243, .pos12_x = 327,
        .small1_y = 151, .small2_y = 151, .small3_y = 151, .small4_y = 29, .small5_y = 29, .small6_y = 29,
        .pos1_y = 35, .pos2_y = 35, .pos3_y = 35, .pos4_y = 35,
        .pos5_y = 108, .pos6_y = 108, .pos7_y = 108, .pos8_y = 108,
        .pos9_y = 180, .pos10_y = 180, .pos11_y = 180, .pos12_y = 180,
        .small1 = 0, .small2 = 0, .small3 = 0, .small4 = 0, .small5 = 0, .small6 = 0,
        .pos1 = 3, .pos2 = 0, .pos3 = 1, .pos4 = 4, .pos5 = 2, .pos6 = 0,
        .pos7 = 0, .pos8 = 2, .pos9 = 4, .pos10 = 1, .pos11 = 0, .pos12 = 3
    };

    // 구조체 포인터 선언 및 초기화
//2. 포인터로도 접근할수잇도록 해준다
board_pos *pBp = &bp;

//3. 누구 차례인지도 여기 저장
int *turn;
*turn=0;

//4. 게임이 얼마나 진행되었는지
int *count;
*count =0;


int main(void)
{	NumBytesRead = 0;
   // 1. 처음에는 아무것도 없는 보드판을 출력해준다
	Res = f_mount(&fatfs, Path, 0);
    if(Res != FR_OK){
    	xil_printf("mount_fail\n");
    	return 0;
    }

    Res = f_open(&fil, board, FA_READ);
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

	
	tftlcd_print(buffer)  // 화면을 출력해준다
    
	//2. 아무것도 없는 보드가 초기세팅이고 
	while(TRUE){

	


	}	// Main loop

	return XST_SUCCESS;
}
void main_textlcd_sevenseg(void)
{
	XIicPs	Iic;			/**< Instance of the IIC Device */
	int 	IicStatus;
	u8		*SendBuffer;	/**< Buffer for Transmitting Data 占쏙옙 8bit*/
	u8		RecvBuffer[3];	/**< Buffer for Receiving Data   占쏙옙 24bit*/

	int		SegReg;
	char	TlcdReg_upline[16];
	char	TlcdReg_downline[16];

	int		i;
	int		wait;


	sprintf(TlcdReg_upline, "Digital Clock   ");

	while(TRUE)
	{
		IicStatus = ReadRTC(Iic, SendBuffer, RecvBuffer);//占쏙옙占쏙옙 占쏙옙체占쏙옙, 8bit, 24bit占쏙옙載�
		//占싱곤옙 32bit 占쏙옙占쏙옙占쏙옙, peripheral 占쏙옙占승곤옙占싸듸옙
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
	char state;

	a= *button_state;
	pb = PUSHBUTTON_mReadReg(XPAR_PUSHBUTTON_0_S00_AXI_BASEADDR, 0);

	PUSHBUTTON_mWriteReg(XPAR_PUSHBUTTON_0_S00_AXI_BASEADDR, 0, 0);
	//여기서 0을 데이터 값으로 줘서 눌렸다는것을 알려줫다
	state = BUTTON_MOORE_mReadReg(XPAR_BUTTON_MOORE_0_S00_AXI_BASEADDR, 0);
	//BUTTON_MOORE_mWriteReg(XPAR_BUTTON_MOORE_0_S00_AXI_BASEADDR, 0, 0);
	state=0x0f&state;

	switch (state)
	{
	case 0://0000 (0) : 내 말 또는 상대 말 선택
		switch (pb)//11_1000(0x38), 11_0100(0x34)
		{
		case 0x38:
			break;

		case 0x34:
			break;
		
		default:
			break;
		}
		break;
	
	case 1://0001 (1) : 내 말 중 어떤 말을 선택할지
		switch ()// 1000(king), 0100(general), 0010(merchant), 0001(prince)
		{
		case :
			break;
		
		default:
			break;
		}
		break;
	
	case 5://0101 (5) : 내 말의 방향을 선택
		switch () //11_1000, 11_0100, 11_0010, 11_0001, 10_1000, 10_0100, 10_0010, 10_0001
		{
		case :
			break;
		
		default:
			break;
		}
		break;
	
	case 2://0010 (2) : 어떤 포로를 선택할건지
		switch ()
		{
		case :
			break;
		
		default:
			break;
		}
		break;
	
	case 6://0110 (6) : 포로를 어디에 배치할건지
		switch ()
		{
		case :
			break;
		
		default:
			break;
		}
		break;
	
	default:
		break;
	}
	int state=0;
	if (((pb & 1) == 1)&&((pb & 16) == 1)){
		xil_printf("S1 Switch is pushed\r\n");


	
	}
	}
void WriteTLCDReg(char *pRegVal, int val) //pRegVal占쏙옙 16Byte占쏙옙 占썼열
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

void DataRead(char filename, FATFS fatfs,TCHAR *Path,FIL fil, u32 *buffer,u32 data_size, u32 NumBytesRead){
	//filename�쓣 以섏꽌 buffer �룷�씤�꽣�뿉 �뙆�씪�쓣 �꽔�뼱以��떎
	NumBytesRead = 0;

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




    return 0;

}



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
void receiveBuffer(u32 *buffer, char filename[], int size){ //65280, 1800, 450

	static FATFS fatfs;
	static FIL fil;
	//static char filename[32] = "lenna.bin";
	FRESULT Res;
	TCHAR *Path = "0:/";
	//u32 * buffer[size];
	u32 data_size = 4 * size; // 4byte * buffer_size
	u32 NumBytesRead;
	NumBytesRead = 0;
   // 1. 처음에는 아무것도 없는 보드판을 출력해준다
	Res = f_mount(&fatfs, Path, 0);
    if(Res != FR_OK){
    	xil_printf("mount_fail\n");
    	return 0;
    }

    Res = f_open(&fil, board, FA_READ);
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

void concatenate(board_pos *b, u32 *buffer, u32 *buffer2, u32 *buffer3){
	//빈 boar를 생성한다
	receiveBuffer(buffer,"board.bin",65280)
	//말을 마스킹하는 switch문을 18개 깐다
	switch (b->small1) //small1
	{
	case 0:
		receiveBuffer(buffer3,"blank_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 1:
		receiveBuffer(buffer3,"king_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 2:
		receiveBuffer(buffer3,"general_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 3:
		receiveBuffer(buffer3,"merchant_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;	
	case 4:
		receiveBuffer(buffer3,"prince_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
	default:
		break;
	}
	switch (b->small2) //small2
	{
	case 0:
		receiveBuffer(buffer3,"blank_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 1:
		receiveBuffer(buffer3,"king_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 2:
		receiveBuffer(buffer3,"general_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 3:
		receiveBuffer(buffer3,"merchant_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;	
	case 4:
		receiveBuffer(buffer3,"prince_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
	default:
		break;
	}
	switch (b->small3)//small3
	{
	case 0:
		receiveBuffer(buffer3,"blank_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 1:
		receiveBuffer(buffer3,"king_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 2:
		receiveBuffer(buffer3,"general_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 3:
		receiveBuffer(buffer3,"merchant_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;	
	case 4:
		receiveBuffer(buffer3,"prince_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
	default:
		break;
	}
	switch (b->small4)//small4
	{
	case 0:
		receiveBuffer(buffer3,"blank_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 1:
		receiveBuffer(buffer3,"king_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 2:
		receiveBuffer(buffer3,"general_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 3:
		receiveBuffer(buffer3,"merchant_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;	
	case 4:
		receiveBuffer(buffer3,"prince_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
	default:
		break;
	}
	switch (b->small5) //small5
	{
	case 0:
		receiveBuffer(buffer3,"blank_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 1:
		receiveBuffer(buffer3,"king_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 2:
		receiveBuffer(buffer3,"general_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 3:
		receiveBuffer(buffer3,"merchant_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;	
	case 4:
		receiveBuffer(buffer3,"prince_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
	default:
		break;
	}
	switch (b->small6) //small6
	{
	case 0:
		receiveBuffer(buffer3,"blank_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 1:
		receiveBuffer(buffer3,"king_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 2:
		receiveBuffer(buffer3,"general_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;
	case 3:
		receiveBuffer(buffer3,"merchant_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
		break;	
	case 4:
		receiveBuffer(buffer3,"prince_small.bin",450);
		maskBuffer(buffer,buffer3,b->small1_x,b->small1_y,30,30)
	default:
		break;
	}

	switch (b->pos1) //pos1
	{
	case 0:
		receiveBuffer(buffer2,"blank_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos1_x,b->pos1_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos1_x,b->pos1_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos1_x,b->pos1_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos1_x,b->pos1_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos1_x,b->pos1_y,60,60)
	default:
		break;
	}
	switch (b->pos2) //pos2
	{
	case 0:
		receiveBuffer(buffer2,"blank_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos2_x,b->pos2_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos2_x,b->pos2_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos2_x,b->pos2_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos2_x,b->pos2_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos2_x,b->pos2_y,60,60)
	default:
		break;
	}
	switch (b->pos3) //pos3
	{
	case 0:
		receiveBuffer(buffer2,"blank_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos3_x,b->pos3_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos3_x,b->pos3_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos3_x,b->pos3_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos3_x,b->pos3_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos3_x,b->pos3_y,60,60)
	default:
		break;
	}
	switch (b->pos4) //pos4
	{
	case 0:
		receiveBuffer(buffer2,"blank_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos4_x,b->pos4_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos4_x,b->pos4_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos4_x,b->pos4_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos4_x,b->pos4_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos4_x,b->pos4_y,60,60)
	default:
		break;
	}
	switch (b->pos5) //pos5
	{
	case 0:
		receiveBuffer(buffer2,"blank_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos5_x,b->pos5_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos5_x,b->pos5_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos5_x,b->pos5_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos5_x,b->pos5_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos5_x,b->pos5_y,60,60)
	default:
		break;
	}
	switch (b->pos6) //pos6
	{
	case 0:
		receiveBuffer(buffer2,"blank_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos6_x,b->pos6_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos6_x,b->pos6_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos6_x,b->pos6_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos6_x,b->pos6_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_red.bin",1800);
		maskBuffer(buffer,buffer2,b->pos6_x,b->pos6_y,60,60)
	default:
		break;
	}
	
	switch (b->pos7) //pos7
	{
	case 0:
		receiveBuffer(buffer2,"blank_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos7_x,b->pos7_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos7_x,b->pos7_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos7_x,b->pos7_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos7_x,b->pos7_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos7_x,b->pos7_y,60,60)
	default:
		break;
	}
	switch (b->pos8) //pos8
	{
	case 0:
		receiveBuffer(buffer2,"blank_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos8_x,b->pos8_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos8_x,b->pos8_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos8_x,b->pos8_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos8_x,b->pos8_y,60,60)
		break;
	case 4:
		receiveBuffer(buffer2,"prince_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos8_x,b->pos8_y,60,60)
	default:
		break;
	}
	switch (b->pos9) //pos9
	{
	case 0:
		receiveBuffer(buffer2,"blank_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos9_x,b->pos9_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos9_x,b->pos9_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos9_x,b->pos9_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos9_x,b->pos9_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos9_x,b->pos9_y,60,60)
	default:
		break;
	}
	switch (b->pos10) //pos10
	{
	case 0:
		receiveBuffer(buffer2,"blank_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos10_x,b->pos10_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos10_x,b->pos10_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos10_x,b->pos10_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos10_x,b->pos10_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos10_x,b->pos10_y,60,60)
	default:
		break;
	}
	switch (b->pos11) //pos11
	{
	case 0:
		receiveBuffer(buffer2,"blank_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos11_x,b->pos11_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos11_x,b->pos11_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos11_x,b->pos11_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos11_x,b->pos11_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos11_x,b->pos11_y,60,60)
	default:
		break;
	}
	switch (b->pos12) //pos12

	{
	case 0:
		receiveBuffer(buffer2,"blank_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos12_x,b->pos12_y,60,60)
		break;
	case 1:
		receiveBuffer(buffer2,"king_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos12_x,b->pos12_y,60,60)
		break;
	case 2:
		receiveBuffer(buffer2,"general_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos12_x,b->pos12_y,60,60)
		break;
	case 3:
		receiveBuffer(buffer2,"merchant_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos12_x,b->pos12_y,60,60)
		break;	
	case 4:
		receiveBuffer(buffer2,"prince_green.bin",1800);
		maskBuffer(buffer,buffer2,b->pos12_x,b->pos12_y,60,60)
	default:
		break;
	}


}