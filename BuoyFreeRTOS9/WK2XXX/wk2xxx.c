/***************************************************************************/
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ���ο�����ʹ�ã�δ��������ɣ��������������κ���;
//WK2124 DEMO
//wkоƬ��غ���
//�ɶ�Ϊ��΢����
//������̳:www.wkmic.com
//�޸�����:2017/8/10
//�汾��V1.0
//��Ȩ���У�����ؾ���
////////////////////////////////////////////////////////////////////////////////// 

/***************************************************************************/
#include "wk2xxx.h"
#include "spi.h"
#include "uart.h"
#include "gpio.h"
//#include "delay.h"

void WK2XXX_RST_Init(void)
{/*
 GPIO_InitTypeDef  GPIO_InitStructure;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PA,PD�˿�ʱ��
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 //PA.4 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.4
 GPIO_SetBits(GPIOA,GPIO_Pin_3);		
	//PA.4 �����	
	*/
	
	
}
void WK2XXX_Reset_Init(void)
{
	/*
GPIO_SetBits(GPIOA,GPIO_Pin_3);//1
GPIO_ResetBits(GPIOA,GPIO_Pin_3);//0
delay_ms(10);
GPIO_SetBits(GPIOA,GPIO_Pin_3);//1	
delay_ms(100);
	*/
	
}
/*************************************************************************/
//�������ܣ���ʼ��SPIƬѡ�ź�CS,����CS��Ĭ��״̬����Ϊ�ߵ�ƽ
//
//
/*************************************************************************/
void SPI_CS_Init(void)
{
	/*
 GPIO_InitTypeDef  GPIO_InitStructure;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PA,PD�˿�ʱ��
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_3;	//PA.4 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.4
 GPIO_SetBits(GPIOA,GPIO_Pin_4);						 //PA.4 �����	
	*/
}
/*************************************************************************/
//�������ܣ���ʼ��SPI���ߣ�����SPI����Ϊ0ģʽ
/*************************************************************************/
void SPI_BUS_Init(void)
{

	//SPI1_Init();		   //��ʼ��SPI
	//SPI1_SetSpeed(SPI_BaudRatePrescaler_8);	//����Ϊ10Mʱ��,����ģʽ
}
/*************************************************************************/
//�������ܣ�����CS�ź�Ϊ�ߵ�ƽ
/*************************************************************************/
void SPI_CS_H(void)
{
//	GPIO_SetBits(GPIOA,GPIO_Pin_4);
		//	PDout(4)  = 1;
}
/*************************************************************************/
//�������ܣ�����CS�ź�Ϊ�͵�ƽ
/*************************************************************************/
void SPI_CS_L(void)
{
	//GPIO_ResetBits(GPIOA,GPIO_Pin_4);
}
/*************************************************************************/
//�������ܣ���ʼ��SPI�ӿ�
/*************************************************************************/
void WK2XXX_SPI_Init(void)
{
	SPI_CS_Init();
	SPI_BUS_Init();
	
	
}

/*************************************************************************/
//�������ܣ�д�Ĵ���������ǰ���ǸüĴ�����д��ĳЩ�Ĵ��������д1�����ܻ��Զ���1������������ֲ�)
//������port:Ϊ�Ӵ��ڵ���(C0C1)
//      reg:Ϊ�Ĵ����ĵ�ַ(A3A2A1A0)
//      dat:Ϊд��Ĵ���������
//ע�⣺���Ӵ��ڱ���ͨ������£���FDATд������ݻ�ͨ��TX�������
//*************************************************************************/
void Wk2xxxWriteReg(unsigned char port,unsigned char reg,unsigned char dat)
{	 
	 SPI_CS_L();//Ƭѡʹ��
	 SPI1_ReadWriteByte(((port-1)<<4)+reg); //д�����ֽ�
	 SPI1_ReadWriteByte(dat); //д����
	 //spi_xfer(value, kSPI_PCS_ReturnInactive);
	 SPI_CS_H();//Ƭѡ��Ч
}


/*************************************************************************/
//�������ܣ����Ĵ�������
//������port:Ϊ�Ӵ��ڵ���(C0C1)
//      reg:Ϊ�Ĵ����ĵ�ַ(A3A2A1A0)
//      rec_data:Ϊ��ȡ���ļĴ���ֵ
//ע�⣺���Ӵ��ڱ���ͨ������£���FDAT��ʵ���Ͼ��Ƕ�ȡuart��rx���յ�����
/*************************************************************************/
unsigned char Wk2xxxReadReg(unsigned char port,unsigned char reg)
{	
	unsigned char rec_data; 
	SPI_CS_L();	//Ƭѡʹ��
	SPI1_ReadWriteByte(0x40+((port-1)<<4)+reg);//д�����ֽڣ���������ɼ������ֲ�
	rec_data=SPI1_ReadWriteByte(0);//���շ��ص�����
	SPI_CS_H();	//Ƭѡ��Ч	
	return rec_data;
}
/**************************** Wk2xxxWriteFifo*********************************************/
//�������ܣ��ú���ΪдFIFO������ͨ���ú���д������ݻ�ֱ�ӽ����Ӵ��ڵķ���FIFO��Ȼ��ͨ��TX���ŷ���
//������port��Ϊ�Ӵ��ڵĶ˿ں�(C0\C1)
//      *wbuf:д�����ݲ���
//      len��  д�����ݳ���
//
/*************************************************************************/
void Wk2xxxWriteFifo(unsigned char port,unsigned char *wbuf,unsigned int len)
{	 unsigned char n;
	 SPI_CS_L(); // Ƭѡ��Ч
	 SPI1_ReadWriteByte(0x80+((port-1)<<4)); //дFIFO����ָ��
	  for(n=0;n<len;n++)
	    {
	     SPI1_ReadWriteByte(*(wbuf+n));
		} 
	 SPI_CS_H();	//Ƭѡ��Ч

}

/**************************** Wk2xxxReadFifo*********************************************/
//�������ܣ��ú���Ϊ��FIFO������ͨ���ú�������һ�ζ����������FIFO�е����ݣ����256���ֽ�
//������port��Ϊ�Ӵ��ڵĶ˿ں�(C0\C1)
//      *rbuf:д�����ݲ���
//      len��  д�����ݳ���
//
/*************************************************************************/
void Wk2xxxReadFifo(unsigned char port,unsigned char *rbuf,unsigned int len)
{	 unsigned char n;
	 SPI_CS_L();//Ƭѡ��Ч
	 SPI1_ReadWriteByte(0xc0+((port-1)<<4));	//д��fifo����ָ��
	 for(n=0;n<len;n++)
	   {
		*(rbuf+n)=SPI1_ReadWriteByte(0); 
	   }
	 SPI_CS_H();//Ƭѡ��Ч										
	 //return 0;
}

/*************************************************************************/
//��������:�˺�����Ҫ��ͨ����дwk2xxx�ļĴ������ж����ӿڵ�ͨ��ʱ���Ƿ�������
//��������
//����ֵ��rv��ʾ����ֵ��0�ɹ�  
/*************************************************************************/
 unsigned char Wk2xxxTest(void)
 {
	unsigned char rec_data,rv;
//���ӿ�ΪSPI	
	rec_data=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GENA);
	if(rec_data==0x30)
		return rv;
	else
		{
			rv=1;
			return rv;
		}

 }
/******************************Wk2xxxInit*******************************************/
//�������ܣ���������Ҫ���ʼ��һЩоƬ�����Ĵ�����
/*********************************************************************************/
void Wk2xxxInit(unsigned char port)
{
    unsigned char gena,grst,gier,sier,scr;
	//ʹ���Ӵ���ʱ��
    gena=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GENA); 
	switch (port)
    {
          case 1://ʹ���Ӵ���1��ʱ��
              gena|=WK2XXX_UT1EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		  case 2://ʹ���Ӵ���2��ʱ��
              gena|=WK2XXX_UT2EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 3://ʹ���Ӵ���3��ʱ��
              gena|=WK2XXX_UT3EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 4://ʹ���Ӵ���4��ʱ��
              gena|=WK2XXX_UT4EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
	 }	
	//�����λ�Ӵ���
	grst=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GRST); 
	switch (port)
    {
          case 1://�����λ�Ӵ���1
              grst|=WK2XXX_UT1RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		  case 2://�����λ�Ӵ���2
              grst|=WK2XXX_UT2RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 3://�����λ�Ӵ���3
              grst|=WK2XXX_UT3RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 4://�����λ�Ӵ���4
             grst|=WK2XXX_UT4RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
	 }	
  //ʹ���Ӵ����жϣ������Ӵ������жϺ��Ӵ����ڲ��Ľ����жϣ��������жϴ���
	gier=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GIER); 
	switch (port)
    {
          case 1://�����λ�Ӵ���1
              gier|=WK2XXX_UT1RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
              break;
		  case 2://�����λ�Ӵ���2
              gier|=WK2XXX_UT2RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
              break;
		   case 3://�����λ�Ӵ���3
              gier|=WK2XXX_UT3RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
              break;
		   case 4://�����λ�Ӵ���4
              gier|=WK2XXX_UT4RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
              break;
	 }	 
	 //ʹ���Ӵ��ڽ��մ����жϺͳ�ʱ�ж�
	 sier=Wk2xxxReadReg(port,WK2XXX_SIER); 
//	 sier |= WK2XXX_RFTRIG_IEN|WK2XXX_RXOUT_IEN;
	 	 sier |= WK2XXX_RFTRIG_IEN;
	 Wk2xxxWriteReg(port,WK2XXX_SIER,sier);
	 // ��ʼ��FIFO�����ù̶��жϴ���
	 Wk2xxxWriteReg(port,WK2XXX_FCR,0XFF);
	 //���������жϴ��㣬��������������Ч����ô����FCR�Ĵ����жϵĹ̶��жϴ��㽫ʧЧ
	 Wk2xxxWriteReg(port,WK2XXX_SPAGE,1);//�л���page1
	 Wk2xxxWriteReg(port,WK2XXX_RFTL,0X08);//���ý��մ���8���ֽ�
	 Wk2xxxWriteReg(port,WK2XXX_TFTL,0X10);//���÷��ʹ���Ϊ16���ֽ�
	 Wk2xxxWriteReg(port,WK2XXX_SPAGE,0);//�л���page0 
	 //ʹ���Ӵ��ڵķ��ͺͽ���ʹ��
	 scr=Wk2xxxReadReg(port,WK2XXX_SCR); 
	 scr|=WK2XXX_TXEN|WK2XXX_RXEN;
	 Wk2xxxWriteReg(port,WK2XXX_SCR,scr);
}

/******************************Wk2xxxClose*******************************************/
//�������ܣ���������رյ�ǰ�Ӵ��ڣ��͸�λ��ʼֵ��
/*********************************************************************************/

void Wk2xxxClose(unsigned char port)
{
    unsigned char gena,grst;
	//��λ�Ӵ���
	grst=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GRST); 
	switch (port)
    {
          case 1://�����λ�Ӵ���1
              grst|=WK2XXX_UT1RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		  case 2://�����λ�Ӵ���2
              grst|=WK2XXX_UT2RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 3://�����λ�Ӵ���3
              grst|=WK2XXX_UT3RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 4://�����λ�Ӵ���4
              grst|=WK2XXX_UT4RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
	 }	
	//�ر��Ӵ���ʱ��
    gena=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GENA); 
	switch (port)
    {
          case 1://ʹ���Ӵ���1��ʱ��
              gena&=~WK2XXX_UT1EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		  case 2://ʹ���Ӵ���2��ʱ��
              gena&=~WK2XXX_UT2EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 3://ʹ���Ӵ���3��ʱ��
              gena&=~WK2XXX_UT3EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 4://ʹ���Ӵ���4��ʱ��
              gena&=~WK2XXX_UT4EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
	 }	
}






/**************************Wk2xxxSetBaud*******************************************************/
//�������ܣ������Ӵ��ڲ����ʺ������˺����в����ʵ�ƥ��ֵ�Ǹ���11.0592Mhz�µ��ⲿ��������
// port:�Ӵ��ں�
// baud:�����ʴ�С.�����ʱ�ʾ��ʽ��
//
/**************************Wk2xxxSetBaud*******************************************************/
void Wk2xxxSetBaud(unsigned char port,int baud)
{  
	unsigned char baud1,baud0,pres,scr;
	//���²�������Ӧ�ļĴ���ֵ�������ⲿʱ��Ϊ11.0592������¼������ã����ʹ������������Ҫ���¼���
	switch (baud) 
	{
      case 600:
			baud1=0x4;
			baud0=0x7f;
			pres=0;
      break;
      case 1200:
			baud1=0x2;
			baud0=0x3F;
			pres=0;
			break;
      case 2400:
			baud1=0x1;
			baud0=0x1f;
			pres=0;
			break;
      case 4800:
			baud1=0x00;
			baud0=0x8f;
			pres=0;
			break;
      case 9600:
			baud1=0x00;
			baud0=0x47;
			pres=0;
			break;
      case 19200:
			baud1=0x00;
			baud0=0x23;
			pres=0;
			break;
      case 38400:
			baud1=0x00;
			baud0=0x11;
			pres=0;
			break;
			
      case 76800:
			baud1=0x00;
			baud0=0x08;
			pres=0;
			break; 
       
      case 1800:
			baud1=0x01;
			baud0=0x7f;
			pres=0;
			break;
      case 3600:
			baud1=0x00;
			baud0=0xbf;
			pres=0;
			break;
      case 7200:
			baud1=0x00;
			baud0=0x5f;
			pres=0;
			break;
      case 14400:
			baud1=0x00;
			baud0=0x2f;
			pres=0;
			break;
      case 28800:
			baud1=0x00;
			baud0=0x17;
			pres=0;
			break;
      case 57600:
			baud1=0x00;
			baud0=0x0b;
			pres=0;
      break;
      case 115200:
			baud1=0x00;
			baud0=0x05;
			pres=0;
			break;
      case 230400:
			baud1=0x00;
			baud0=0x02;
			pres=0;
			break;
      default:
			baud1=0x00;
			baud0=0x00;
			pres=0;
    }
	//�ص��Ӵ����շ�ʹ��
	scr=Wk2xxxReadReg(port,WK2XXX_SCR); 
	Wk2xxxWriteReg(port,WK2XXX_SCR,0);
	//���ò�������ؼĴ���
	Wk2xxxWriteReg(port,WK2XXX_SPAGE,1);//�л���page1
	Wk2xxxWriteReg(port,WK2XXX_BAUD1,baud1);
	Wk2xxxWriteReg(port,WK2XXX_BAUD0,baud0);
	Wk2xxxWriteReg(port,WK2XXX_PRES,pres);
	Wk2xxxWriteReg(port,WK2XXX_SPAGE,0);//�л���page0 
	//ʹ���Ӵ����շ�ʹ��
	Wk2xxxWriteReg(port,WK2XXX_SCR,scr);
	
	
}
/*****************************Wk2xxxSendBuf****************************************/
//������Ϊ�Ӵ��ڷ������ݵĺ������������ݵ��Ӵ��ڵ�FIFO.Ȼ��ͨ���ٷ���
//����˵����port���Ӵ��ڶ˿ں�
//          *sendbuf:��Ҫ���͵�����buf
//          len����Ҫ�������ݵĳ���
// ��������ֵ��ʵ�ʳɹ����͵�����
//˵�������ô˺���ֻ�ǰ�����д���Ӵ��ڵķ���FIFO��Ȼ���ٷ��͡�1������ȷ���Ӵ��ڵķ���FIFO�ж������ݣ����ݾ��������
//ȷ��д��FIFO���ݵĸ�����
/*********************************************************************/
unsigned int Wk2xxxSendBuf(unsigned char port,unsigned char *sendbuf,unsigned int len)
{
	 unsigned int ret,tfcnt,sendlen;
	 unsigned char  fsr;
	  
	 fsr=Wk2xxxReadReg(port,WK2XXX_FSR);
	 if(~fsr&WK2XXX_TFULL )//�Ӵ��ڷ���FIFOδ��
	 {

	     tfcnt=Wk2xxxReadReg(port,WK2XXX_TFCNT);//���Ӵ��ڷ���fifo�����ݸ���
		 sendlen=256-tfcnt;//FIFO��д�������ֽ���
		 
		 if(sendlen<len)
		 {
			ret=sendlen; 
			Wk2xxxWriteFifo(port,sendbuf,sendlen);
		 }
		 else
		 {
			 Wk2xxxWriteFifo(port,sendbuf,len);
			 ret=len;
		 }
	  }
	 
	 return ret;
}

/*****************************Wk2xxxGetBuf****************************************/
//������Ϊ�Ӵ��ڽ������ݺ���
//����˵����port���Ӵ��ڶ˿ں�
//          *getbuf:���յ�������buf
// ��������ֵ��ʵ�ʽ��յ������ݸ���
/*********************************************************************/
unsigned int Wk2xxxGetBuf(unsigned char port,unsigned char *getbuf)
{
	unsigned int ret=0,rfcnt;
	unsigned char fsr;
	fsr=Wk2xxxReadReg(port,WK2XXX_FSR);
	if(fsr&WK2XXX_RDAT )//�Ӵ��ڷ���FIFOδ��
	  {
	     rfcnt=Wk2xxxReadReg(port,WK2XXX_RFCNT);//���Ӵ��ڷ���fifo�����ݸ���
		 if(rfcnt==0)//��RFCNT�Ĵ���Ϊ0��ʱ�������������������256������0�����ʱ��ͨ��FSR���жϣ����FSR��ʾ����FIFO��Ϊ�գ���Ϊ256���ֽ�
		 {rfcnt=256;}
		 Wk2xxxReadFifo(port,getbuf,rfcnt);
		 ret=rfcnt;
	   }
	 return ret;	
}

