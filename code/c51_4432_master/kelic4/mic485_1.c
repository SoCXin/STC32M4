
//2009.05.13 433mhz testing program
#include  "AT89X52.h"
//#include  "lcd.h"
//#include  "absacc.h"
#define   uchar  unsigned  char
#define   uint  unsigned int
#define   Uchar  unsigned  char
#define   Uint  unsigned int

#define   U8  unsigned  char
#define   U16  unsigned  int

#define    Wait1us        _nop_();
#define    Wait2us        {_nop_();_nop_();}
#define    Wait4us        {Wait2us;Wait2us;}
#define    Wait8us        {Wait4us;Wait4us;}
#define    Wait10us    {Wait8us;Wait2us;}

#define   TXRX_FRAME_BYTES 4   //UART 发送接收一帧字节数为５

sbit NSS = P1 ^ 0;
sbit NIRQ= P3 ^ 2;
sbit SDN= P0 ^ 0;
sbit RX_LED= P2 ^ 4; 
sbit TX_LED= P2 ^ 5;   
sbit key1= P2 ^ 6; 
sbit key2= P2 ^ 7;   

sbit SCK = P1 ^ 3; 
sbit SDI = P1 ^ 2;
sbit SDO = P1 ^ 1;

#define   ROW(x)       (0x80|(x << 0x06))
#define   COL(x)       (x)
#define   LEN(x)       ((x-0x01) << 0x03)

#define   LEN_MASK     0x38
#define   COL_MASK     0x07
#define   ROW_MASK     0xC0
#define   ROW_COL      (ROW_MASK|COL_MASK)
	
#define   HexDisp(x)   HexDispArray[x]
#define   BYTE_LOW(x)  (x & 0x0F) 
#define   BYTE_HIGH(x) ((x & 0xF0)>>0x04)

// lcd bit define
sbit     LCD_RW = P2^2;
sbit     LCD_RS = P2^1;
sbit     LCD_EN = P2^0;
#define  LCD_DATA P0
// sfr      LCD_DATA = 0x80;



								/* ======================================================== *
								 *							Function PROTOTYPES							* 
								 * ======================================================== */
//MCU initialization
void MCU_Init(void);
//SPI functions						
void SpiWriteRegister (U8, U8);
U8 SpiReadRegister (U8);
void delay_2us(U16 delay_cnt);
void sending();

void ini_lcd(void);
void lcd_printf_char(U8 disp_char, U8 row_col);
void lcd_printf_string(U8 *disp_str, U8 row_len_col);
void lcd_write_reg(U8 command);
void lcd_write_data(U8 value);
void lcd_wait(void);
void delay_2us(U16 delay_cnt);
void hex_to_assic(U8 hex_data, U8 *str);

unsigned char Rf_Rec_Cnt;
unsigned char temp_array[8] = "        ";
U8 ItStatus1,ItStatus2;
U8 length,temp8;
U8 payload[10];
U16 delay;
bit successful_flag;
void main(void)
{
	
	unsigned long timeout;
	

	//Initialize the MCU: 
	//	- set IO ports for the Software Development board
	//	- set MCU clock source
	//	- initialize the SPI port 
	//	- turn off LEDs
	MCU_Init();
	
	ini_lcd();
	delay_2us(50000);

	lcd_printf_string(" master ", ROW(0x00)|LEN(0x08)|COL(0x00));
	lcd_printf_string(" ia4432 ", ROW(0x01)|LEN(0x08)|COL(0x00));
	
	
							/* ======================================================== *
							 *						Initialize the Si443x ISM chip				* 
							 * ======================================================== */
	//Turn on the radio by pulling down the PWRDN pin
	SDN = 0;
	//Wait at least 15ms befory any initialization SPI commands are sent to the radio
	// (wait for the power on reset sequence) 
	for (temp8=0;temp8<15;temp8++)
	{
		for(delay=0;delay<10000;delay++);
	}	 
	//read interrupt status registers to clear the interrupt flags and release NIRQ pin
	
	ItStatus1 = SpiReadRegister(0x03);													//read the Interrupt Status1 register
	ItStatus2 = SpiReadRegister(0x04);													//read the Interrupt Status2 register
    
	//SW reset   
   SpiWriteRegister(0x07, 0x80);															//write 0x80 to the Operating & Function Control1 register 
	
	//wait for POR interrupt from the radio (while the nIRQ pin is high)
	while ( NIRQ == 1);  
	//read interrupt status registers to clear the interrupt flags and release NIRQ pin
	ItStatus1 = SpiReadRegister(0x03);													//read the Interrupt Status1 register
	ItStatus2 = SpiReadRegister(0x04);													//read the Interrupt Status2 register
     
	//wait for chip ready interrupt from the radio (while the nIRQ pin is high)
	while ( NIRQ == 1);  
	//read interrupt status registers to clear the interrupt flags and release NIRQ pin
	ItStatus1 = SpiReadRegister(0x03);													//read the Interrupt Status1 register
	ItStatus2 = SpiReadRegister(0x04);													//read the Interrupt Status2 register
						
							/*set the physical parameters*/
	//set the center frequency to 915 MHz
	//SpiWriteRegister(0x75, 0x75);															//write 0x75 to the Frequency Band Select register             
	//SpiWriteRegister(0x76, 0xBB);															//write 0xBB to the Nominal Carrier Frequency1 register
	//SpiWriteRegister(0x77, 0x80); 
 														//write 0x80 to the Nominal Carrier Frequency0 register
	
	SpiWriteRegister(0x75, 0x53);															//write 0x75 to the Frequency Band Select register             
	SpiWriteRegister(0x76, 0x64);															//write 0xBB to the Nominal Carrier Frequency1 register
	SpiWriteRegister(0x77, 0x00); 
	
	
	//set the desired TX data rate (9.6kbps)
	//SpiWriteRegister(0x6E, 0x4E);															//write 0x4E to the TXDataRate 1 register
	//SpiWriteRegister(0x6F, 0xA5);															//write 0xA5 to the TXDataRate 0 register
	//SpiWriteRegister(0x70, 0x2C);															//write 0x2C to the Modulation Mode Control 1 register
    
	
	//set the desired TX data rate (1.2kbps)
	SpiWriteRegister(0x2a, 0x14);	
	SpiWriteRegister(0x6E, 0x09);															//write 0x4E to the TXDataRate 1 register
	SpiWriteRegister(0x6F, 0xd5);															//write 0xA5 to the TXDataRate 0 register
	SpiWriteRegister(0x70, 0x2C);	
	
													

	
	//set the Tx deviation register (+-45kHz)
	//SpiWriteRegister(0x72, 0x48);	//(9.6kbps)														//write 0x48 to the Frequency Deviation register 
    SpiWriteRegister(0x72, 0x38);	//(1.2kbps)	

								/*set the modem parameters according to the exel calculator(parameters: 9.6 kbps, deviation: 45 kHz, channel filter BW: 102.2 kHz*/
	SpiWriteRegister(0x1C, 0x1b);															//write 0x1E to the IF Filter Bandwidth register		
	SpiWriteRegister(0x20, 0x83);															//write 0xD0 to the Clock Recovery Oversampling Ratio register		
	SpiWriteRegister(0x21, 0xc0);															//write 0x00 to the Clock Recovery Offset 2 register		
	SpiWriteRegister(0x22, 0x13);															//write 0x9D to the Clock Recovery Offset 1 register		
	SpiWriteRegister(0x23, 0xa9);															//write 0x49 to the Clock Recovery Offset 0 register		
	SpiWriteRegister(0x24, 0x00);															//write 0x00 to the Clock Recovery Timing Loop Gain 1 register		
	SpiWriteRegister(0x25, 0x03);															//write 0x24 to the Clock Recovery Timing Loop Gain 0 register		
	SpiWriteRegister(0x1D, 0x40);															//write 0x40 to the AFC Loop Gearshift Override register		
	SpiWriteRegister(0x1E, 0x0A);															//write 0x0A to the AFC Timing Control register		
	SpiWriteRegister(0x2A, 0x14);															//write 0x20 to the AFC Limiter register		
						
							/*set the packet structure and the modulation type*/
	//set the preamble length to 5bytes 
	SpiWriteRegister(0x34, 0x0A);															//write 0x0A to the Preamble Length register
	//set preamble detection threshold to 20bits
	SpiWriteRegister(0x35, 0x2A); 														//write 0x2A to the Preamble Detection Control  register

	//Disable header bytes; set variable packet length (the length of the payload is defined by the
	//received packet length field of the packet); set the synch word to two bytes long
	SpiWriteRegister(0x33, 0x02);															//write 0x02 to the Header Control2 register    
	
	//Set the sync word pattern to 0x2DD4
	SpiWriteRegister(0x36, 0x2D);															//write 0x2D to the Sync Word 3 register
	SpiWriteRegister(0x37, 0xD4);															//write 0xD4 to the Sync Word 2 register

	//enable the TX & RX packet handler and CRC-16 (IBM) check
	SpiWriteRegister(0x30, 0x8D);															//write 0x8D to the Data Access Control register
	//Disable the receive header filters
   SpiWriteRegister(0x32, 0x00 );														//write 0x00 to the Header Control1 register            
	//enable FIFO mode and GFSK modulation
	SpiWriteRegister(0x71, 0x63);															//write 0x63 to the Modulation Mode Control 2 register

											/*set the GPIO's according to the RF switch */
   //SpiWriteRegister(0x0C, 0x12);															//write 0x12 to the GPIO1 Configuration(set the TX state)
	//SpiWriteRegister(0x0b, 0x15);															//write 0x15 to the GPIO2 Configuration(set the RX state) 
	SpiWriteRegister(0x0C, 0x15);															//write 0x15 to the GPIO1 Configuration(set the rX state)
	SpiWriteRegister(0x0b, 0x12);															//write 0x12 to the GPIO0Configuration(set the tX state) 

											/*set the non-default Si443x registers*/
	//set  cap. bank
	SpiWriteRegister(0x09, 0xD7);															//write 0xD7 to the Crystal Oscillator Load Capacitance register
	// Set AGC Override1 Register
	SpiWriteRegister(0x69, 0x60);															//write 0x60 to the AGC Override1 register	

	//set tx power 20dbm max
	//SpiWriteRegister(0x6d, 0x1f);	 
	SpiWriteRegister(0x6d, 0x1e);	
	
	/*enable receiver chain*/
	SpiWriteRegister(0x07, 0x05);															//write 0x05 to the Operating Function Control 1 register
	//Enable two interrupts: 
	// a) one which shows that a valid packet received: 'ipkval'
	// b) second shows if the packet received with incorrect CRC: 'icrcerror' 
	SpiWriteRegister(0x05, 0x03); 														//write 0x03 to the Interrupt Enable 1 register
	SpiWriteRegister(0x06, 0x00); 														//write 0x00 to the Interrupt Enable 2 register
	//read interrupt status registers to release all pending interrupts
	ItStatus1 = SpiReadRegister(0x03);													//read the Interrupt Status1 register
	ItStatus2 = SpiReadRegister(0x04);													//read the Interrupt Status2 register
    //while((key1)&&(key2))
	//{
		/*enable receiver chain*/	    
		SpiWriteRegister(0x07, 0x05);	
		
		SpiWriteRegister(0x0d, 0xf4);	
		while((key1)&&(key2));
	//}

	/*MAIN Loop*/
	while(1)
		{
		//Poll the port pins of the MCU to figure out whether the push button is pressed or not
		if (key1 == 0)
		{
			lcd_printf_string("wait... ", ROW(0x00)|LEN(0x08)|COL(0x00));
			lcd_printf_string("        ", ROW(0x01)|LEN(0x08)|COL(0x00));
			while (!key1);

			sending();
			//write 0x05 to the Operating Function Control 1 register
			successful_flag=0;
			timeout=0x3fff;
			while (timeout!=0)
			{
				timeout--;
				if ( NIRQ == 0 )
				{
					//disable the receiver chain
					SpiWriteRegister(0x07, 0x01);													//write 0x01 to the Operating Function Control 1 register
					//read interrupt status registers
					ItStatus1 = SpiReadRegister(0x03);											//read the Interrupt Status1 register
					ItStatus2 = SpiReadRegister(0x04);											//read the Interrupt Status2 register

					/*CRC Error interrupt occured*/
					if ( (ItStatus1 & 0x01) == 0x01 )
					{
						//reset the RX FIFO
						SpiWriteRegister(0x08, 0x02);												//write 0x02 to the Operating Function Control 2 register
						SpiWriteRegister(0x08, 0x00);												//write 0x00 to the Operating Function Control 2 register
						//blink all LEDs to show the error
						TX_LED = 1;
						RX_LED = 1;
						for (delay = 0; delay < 10000;delay++);
						//for (delay = 0; delay < 60000;delay++);
						TX_LED = 0;
						RX_LED = 0;
					}

					/*packet received interrupt occured*/
					if ( (ItStatus1 & 0x02) == 0x02 )
					{
						//Read the length of the received payload
						length = SpiReadRegister(0x4B);											//read the Received Packet Length register
						//check whether the received payload is not longer than the allocated buffer in the MCU
						if (length < 11)
						{
							//Get the reeived payload from the RX FIFO
							for (temp8=0;temp8 < length;temp8++)
							{
								payload[temp8] = SpiReadRegister(0x7F);						//read the FIFO Access register
							}

							//check whether the acknowledgement packet received
							successful_flag = 1;
							timeout=0;
							//check whether an expected packet received, this should be acknowledged

						}
					}



					//reset the RX FIFO
					SpiWriteRegister(0x08, 0x02);													//write 0x02 to the Operating Function Control 2 register
					SpiWriteRegister(0x08, 0x00);													//write 0x00 to the Operating Function Control 2 register
					//enable the receiver chain again
					SpiWriteRegister(0x07, 0x05);													//write 0x05 to the Operating Function Control 1 register
				}
			}

			if (successful_flag)
			{
				lcd_printf_string("        ", ROW(0x00)|LEN(0x08)|COL(0x00));
				lcd_printf_string("ok      ", ROW(0x01)|LEN(0x08)|COL(0x00));

			}
			else
			{
				lcd_printf_string("        ", ROW(0x00)|LEN(0x08)|COL(0x00));
				lcd_printf_string("fail    ", ROW(0x01)|LEN(0x08)|COL(0x00));

			}
			
			/*
			while((key1)&&(key2))
	        {
		
		            sending();
	        }
*/

		}
		
		


	}   
}

void sending()
{
	        //Wait for releasing the push button
			//while( PB == 0 );
			//disable the receiver chain (but keep the XTAL running to have shorter TX on time!)
			SpiWriteRegister(0x07, 0x01);													//write 0x01 to the Operating Function Control 1 register			

			//turn on the LED to show the packet transmission
			TX_LED = 1; 																			
			/*SET THE CONTENT OF THE PACKET*/
			//set the length of the payload to 8bytes	
			SpiWriteRegister(0x3E, 8);														//write 8 to the Transmit Packet Length register		
			//fill the payload into the transmit FIFO
			SpiWriteRegister(0x7F, 0x42);													//write 0x42 ('B') to the FIFO Access register	
			SpiWriteRegister(0x7F, 0x55);													//write 0x55 ('U') to the FIFO Access register	
			SpiWriteRegister(0x7F, 0x54);													//write 0x54 ('T') to the FIFO Access register	
			SpiWriteRegister(0x7F, 0x54);													//write 0x54 ('T') to the FIFO Access register	
			SpiWriteRegister(0x7F, 0x4F);													//write 0x4F ('O') to the FIFO Access register	
			SpiWriteRegister(0x7F, 0x4E);													//write 0x4E ('N') to the FIFO Access register	
			SpiWriteRegister(0x7F, 0x31);													//write 0x31 ('1') to the FIFO Access register	
			SpiWriteRegister(0x7F, 0x0D);													//write 0x0D (CR) to the FIFO Access register	

			//Disable all other interrupts and enable the packet sent interrupt only.
			//This will be used for indicating the successfull packet transmission for the MCU
			SpiWriteRegister(0x05, 0x04);													//write 0x04 to the Interrupt Enable 1 register	
			SpiWriteRegister(0x06, 0x00);													//write 0x03 to the Interrupt Enable 2 register	
			//Read interrupt status regsiters. It clear all pending interrupts and the nIRQ pin goes back to high.
			ItStatus1 = SpiReadRegister(0x03);											//read the Interrupt Status1 register
			ItStatus2 = SpiReadRegister(0x04);											//read the Interrupt Status2 register

			/*enable transmitter*/
			//The radio forms the packet and send it automatically.
			SpiWriteRegister(0x07, 0x09);													//write 0x09 to the Operating Function Control 1 register

			/*wait for the packet sent interrupt*/
			//The MCU just needs to wait for the 'ipksent' interrupt.
			while(NIRQ == 1);
			//read interrupt status registers to release the interrupt flags
			ItStatus1 = SpiReadRegister(0x03);											//read the Interrupt Status1 register
			ItStatus2 = SpiReadRegister(0x04);											//read the Interrupt Status2 register

			//wait a bit for showing the LED a bit longer
			for(delay = 0; delay < 10000;delay++);
			//for(delay = 0; delay < 60000;delay++);
			//turn off the LED
			TX_LED = 0; 
			
			//after packet transmission set the interrupt enable bits according receiving mode
			//Enable two interrupts: 
			// a) one which shows that a valid packet received: 'ipkval'
			// b) second shows if the packet received with incorrect CRC: 'icrcerror' 
			SpiWriteRegister(0x05, 0x03); 												//write 0x03 to the Interrupt Enable 1 register
			SpiWriteRegister(0x06, 0x00); 												//write 0x00 to the Interrupt Enable 2 register
			//read interrupt status registers to release all pending interrupts
			ItStatus1 = SpiReadRegister(0x03);											//read the Interrupt Status1 register
			ItStatus2 = SpiReadRegister(0x04);											//read the Interrupt Status2 register

			/*enable receiver chain again*/
			SpiWriteRegister(0x07, 0x05);	   
	
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  +
  + FUNCTION NAME:  void MCU_Init(void)
  +
  + DESCRIPTION:   	This function configures the MCU 
  + 		
  + INPUT:			None
  +
  + RETURN:         None
  +
  + NOTES:          None
  +
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void MCU_Init(void)
{
	/*
	//Disable the Watch Dog timer of the MCU
   PCA0MD   &= ~0x40;                  
	
	// Set the clock source of the MCU: 10MHz, using the internal RC osc.
   CLKSEL    = 0x14;


   // Initialize the the IO ports and the cross bar
   P0SKIP  |= 0x40;                    // skip P0.6
   XBR1    |= 0x40;                    // Enable SPI1 (3 wire mode)
   P1MDOUT |= 0x01;                    // Enable SCK push pull
   P1MDOUT |= 0x04;                    // Enable MOSI push pull
   P1SKIP  |= 0x08;                    // skip NSS
   P1MDOUT |= 0x08;                    // Enable NSS push pull
   P1SKIP  |= 0x40;                    // skip TX_LED
   P1MDOUT |= 0x40;                    // Enable TX_LED push pull
   P2SKIP  |= 0x01;                    // skip RX_LED
   P2MDOUT |= 0x01;                    // Enable RX_LED push pull
	P0SKIP  |= 0x02; 					// skip SDN
	P0MDOUT |= 0x02;					// Enable SDN push pull
   P0SKIP  |= 0x80;                    // skip PB
   SFRPAGE  = CONFIG_PAGE;
   P0DRV    = 0x12;                   // TX high current mode
   P1DRV    = 0x4D;                   // MOSI, SCK, NSS, TX_LED high current mode
	P2DRV	  	= 0x01;					// RX_LED high current mode	
   SFRPAGE  = LEGACY_PAGE;
   XBR2    |= 0x40;                    // enable Crossbar

   // For the SPI communication the hardware peripheral of the MCU is used 
   //in 3 wires Single Master Mode. The select pin of the radio is controlled
	//from software
	SPI1CFG   = 0x40;					//Master SPI, CKPHA=0, CKPOL=0
   SPI1CN    = 0x00;					//3-wire Single Master, SPI enabled
   SPI1CKR   = 0x00;
   SPI1EN 	 = 1;                     	// Enable SPI interrupt
   NSS = 1;
	
	// Turn off the LEDs
	TX_LED = 0;
	RX_LED = 0;
	*/
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  +
  + FUNCTION NAME: void SpiWriteRegister(U8 reg, U8 value)
  +
  + DESCRIPTION:   This function writes the registers 
  + 					
  + INPUT:			 U8 reg - register address   
  +					 U8 value - value write to register	
  +
  + RETURN:        None
  +
  + NOTES:         Write uses a Double buffered transfer
  +
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void SpiWriteRegister (U8 reg, U8 value)
{
   uchar i;
   uchar Temp_byte;
   
   NSS = 0;                              
   Temp_byte = (reg|0x80);				//write data into the SPI register
	
	//--------------------------
	

        for(i = 0x00; i < 0x08; i++)
        {
            if(Temp_byte&0x80)
                SDI    = 1;
            else
                SDI    = 0;
			
            SCK         = 1;
            SCK         = 0;            
            Temp_byte <<= 0x01;
        }
	
	
	
	//--------------------------
		Temp_byte = value;	
	    for(i = 0x00; i < 0x08; i++)
        {
            if(Temp_byte&0x80)
                SDI    = 1;
            else
                SDI    = 0;
			
            SCK         = 1;
            SCK         = 0;            
            Temp_byte           <<= 0x01;
        }
	

	NSS = 1;    
                        

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  +
  + FUNCTION NAME: U8 SpiReadRegister(U8 reg)
  +
  + DESCRIPTION:   This function reads the registers 
  + 					
  + INPUT:			 U8 reg - register address   
  +
  + RETURN:        SPI1DAT - the register content 
  +
  + NOTES:         none
  +
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
U8 SpiReadRegister (U8 reg)
{
    uchar i;
    uchar Temp_byte;
	uchar Result;
	
	NSS = 0; 
	Temp_byte = reg;				//write data into the SPI register
	
	//--------------------------
	

        for(i = 0x00; i < 0x08; i++)
        {
            if(Temp_byte&0x80)
                SDI    = 1;
            else
                SDI    = 0;
			
            SCK         = 1;
            SCK         = 0;            
            Temp_byte           <<= 0x01;
        }
		
		Result=0;
		
  	    for(i=0;i<8;i++)
  	    {                    //read fifo data byte
  	 	     Result=Result<<1;
  	 	     SCK=1;
  	 	     
  	 	    if(SDO)
  	 	    {
  	 		        Result|=1;
  	 	    }
  	 	
  	 	    SCK=0;
  	 	     
  	    }
  	
	    
	
	
	
  
	
	NSS = 1;                            

	
   return(Result);
   

}


 


//----------------- 1602 start ------------------
/***********************************************************
name:		ini_lcd		
input:		none
output:		none
describe:	初始化LCD	
notice:
creat date: 2008-7-25
creator:	dengyihong
************************************************************/
void ini_lcd(void)
{
    lcd_write_reg(0x38);
    lcd_write_reg(0x38);
    lcd_write_reg(0x38);
    lcd_write_reg(0x08);
    lcd_wait();
    lcd_write_reg(0x01);
    lcd_wait();
    delay_2us(50000);
    
    lcd_write_reg(0x14);
    lcd_wait();
    lcd_write_reg(0x06);
    lcd_wait();
    lcd_write_reg(0x80);
    lcd_wait();
    lcd_write_reg(0x0c);
    lcd_wait();	
}


/***********************************************************
name:		lcd_printf_string	
input:		*disp_str  ---  存放显示字符的头指针
			row_len_col  ---  显示坐标参数及显示字符数
			row_len_col:
				bit[7:6]: 显示行坐标
				bit[5:3]: 显示字符长度
				bit[2:0]: 显示字符的起始列坐标
output:		none
describe:	在屏幕上显示多个字符		
notice:
creat date: 2008-7-25
creator:	dengyihong
************************************************************/
void lcd_printf_string(U8 *disp_str, U8 row_len_col)
{
    U8 i = 0;
    U8 len = 0;
    
    len = (row_len_col & LEN_MASK) >> 0x03;
    
    lcd_write_reg(row_len_col & ROW_COL);
    lcd_wait();
    
    for(i=0; i<=len; i++)
    {
        lcd_write_data(*disp_str++);
        lcd_wait();	
    }
}


/***********************************************************
name:		lcd_printf_char		
input:		disp_char  ---  要显示的字符
			row_col    ---  显示坐标
output:		none
describe:	在屏幕上显示一个字符		
notice:
creat date: 2008-7-25
creator:	dengyihong
************************************************************/
/*
void lcd_printf_char(U8 disp_char, U8 row_col)
{
    lcd_write_reg(row_col & ROW_COL);
    lcd_wait();
    lcd_write_data(disp_char);
    lcd_wait();	
}
*/

/***********************************************************
name:		lcd_write_reg		
input:		command  ---  要写入的命令
output:		none
describe:	写控制命令到LCD	
notice:
creat date: 2008-7-25
creator:	dengyihong
************************************************************/
void lcd_write_reg(U8 command)
{
    LCD_DATA = command;		// 写控制命令
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_EN = 1;
    delay_2us(100);
    LCD_EN = 0;
}


/***********************************************************
name:		lcd_write_data		
input:		value  ---  要写入的数据
output:		none
describe:	写数据到LCD	
notice:
creat date: 2008-7-25
creator:	dengyihong
************************************************************/
void lcd_write_data(U8 value)
{
   LCD_DATA =  value;		//写数据
   LCD_RS = 1;
   LCD_RW = 0;
   LCD_EN = 1;
   delay_2us(100);
   LCD_EN = 0; 	
}

/***********************************************************
name:		lcd_wait	
input:		none
output:		none
describe:	等待LCD内部操作完成	
notice:
creat date: 2008-7-25
creator:	dengyihong
************************************************************/
void lcd_wait(void)
{
    U8 value = 0;
       
    do
    {
        LCD_RS = 0;
        LCD_RW = 1;
        LCD_EN = 1;
        value = LCD_DATA;	
        LCD_EN = 0;
    }while(value & 0x80);		// 等待内部操作完成	
}

/**********************************************************
name:		delay_2us
input:		delay_cnt
output:		none
describe:	delay x*2us
notice:
creat date:	2008-7-24
creator:	dengyihong
**********************************************************/
void delay_2us(U16 delay_cnt)
{
    while(delay_cnt--);
}

void hex_to_assic(U8 hex_data, U8 *str)
{
	if(hex_data < 10)
	{
		*str++ = hex_data + '0';
		*str++ = ' ';
		*str++ = ' ';
	}
	else if(hex_data < 100)
	{
		*str++ = hex_data/10 + '0';
		*str++ = hex_data%10 + '0';
		*str++ = ' ';
	}
	else if(hex_data <= 255)
	{
		*str++ = hex_data/100 + '0';
		*str++ = (hex_data/10)%10 + '0';
		*str++ = hex_data%10 + '0';
	}
}
//----------------- 1602 start ------------------ 





 




 


 





 





 





 





 




 


 





 





 





 





 





 





 




 


 





 





 





 





 




 


 





 





 





 





 



