/********************************** (C) COPYRIGHT ******************************
* File Name          : TCP_server.C
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/01
* Description        : CH57xNET����ʾ�ļ�
*                      (1)��CH579 Examples by KEIL;
*                      (2)������0��������Ϣ,115200bps;
*                      (3)��������������ʾFTP �ͻ���
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ�����*/
#include <stdio.h>
#include <string.h>
#include "CH57x_common.h"
#include "core_cm0.h"
#include "CH57xNET.H"
#include "FTPINC.H"

#define KEEPLIVE_ENABLE                      1                                  /* ����KEEPLIVE���� */

/* ����Ļ�������ȫ�ֱ�������Ҫ���壬���е��� */
__align(16)UINT8    CH57xMACRxDesBuf[(RX_QUEUE_ENTRIES )*16];                   /* MAC������������������16�ֽڶ��� */
__align(4) UINT8    CH57xMACRxBuf[RX_QUEUE_ENTRIES*RX_BUF_SIZE];                /* MAC���ջ�������4�ֽڶ��� */
__align(4) SOCK_INF SocketInf[CH57xNET_MAX_SOCKET_NUM];                         /* Socket��Ϣ��4�ֽڶ��� */
const UINT16 MemNum[8] = {CH57xNET_NUM_IPRAW,
                         CH57xNET_NUM_UDP,
                         CH57xNET_NUM_TCP,
                         CH57xNET_NUM_TCP_LISTEN,
                         CH57xNET_NUM_TCP_SEG,
                         CH57xNET_NUM_IP_REASSDATA,
                         CH57xNET_NUM_PBUF,
                         CH57xNET_NUM_POOL_BUF
                         };
const UINT16 MemSize[8] = {CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_IPRAW_PCB),
                          CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_UDP_PCB),
                          CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_TCP_PCB),
                          CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_TCP_PCB_LISTEN),
                          CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_TCP_SEG),
                          CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_IP_REASSDATA),
                          CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_PBUF) + CH57xNET_MEM_ALIGN_SIZE(0),
                          CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_PBUF) + CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_POOL_BUF)
                         };
__align(4)UINT8 Memp_Memory[CH57xNET_MEMP_SIZE];
__align(4)UINT8 Mem_Heap_Memory[CH57xNET_RAM_HEAP_SIZE];
__align(4)UINT8 Mem_ArpTable[CH57xNET_RAM_ARP_TABLE_SIZE];

#define CH57xNET_TIMEPERIOD                   10                                /* ��ʱ����ʱ���ڣ���λmS*/

/******************************************************************************/
/* ����ʾ�������غ� */
#define RECE_BUF_LEN                          536                               /* ���ջ������Ĵ�С */
#define READ_RECV_BUF_MODE                    0                                 /* socket���ջ�������ȡģʽ��1�����ƣ�0�������� */
										 									 
UINT8 SocketRecvBuf[CH57xNET_MAX_SOCKET_NUM][RECE_BUF_LEN];                     /* socket���ջ����� */
/* CH579��ض��� */                                                                   
UINT8 MACAddr[6]  = {0x84,0x21,0x04,0x05,0x06,0x07};                            /* CH579MAC��ַ */ 
const UINT8 IPAddr[4]   = {192,168,1,200};                                       /* CH579IP��ַ */ 
const UINT8 GWIPAddr[4] = {192,168,1,1};                                        /* CH579���� */ 
const UINT8 IPMask[4]   = {255,255,255,0};                                      /* CH579�������� */ 
const UINT8 DestIP[4]   = {192,168,1,100};                                      /* Ŀ��IP */ 
const UINT8 *pUserName  = "qinheng";                                            /* ������½ */
const UINT8 *pPassword  = "123";                                                /* e-mail ������Ϊ��½���� */
UINT16 DESPORT = 3000;                                                            /* Ŀ��˿ں� */
char  ListName[24];                                                             /* ���ڱ���Ŀ¼�� */
char  ListMkd[24];                                                              /* ���ڱ����´�����Ŀ¼�� */
char  FileName[24];                                                             /* ���ڱ����ļ��� */
char  CharIP[17];                                                               /* ���ڱ���ת�����ַ���IP��ַ */

/* ���ڵƶ��� PB�ڵ�ʮ��λ��Ч */
UINT16 CH57xNET_LEDCONN=0x0010;                                                 /* ����ָʾ�� PB4 */
UINT16 CH57xNET_LEDDATA=0x0080;                                                 /* ͨѶָʾ�� PB7 */ 

UINT8 SocketId;                                                                 /* ����socket���������Բ��ö��� */
UINT8 gPort;                                                                    /* ���ڸı�˿�ֵ */
/*******************************************************************************
* Function Name  : IRQ_Handler
* Description    : IRQ�жϷ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ETH_IRQHandler( void )							/* ��̫���ж� */
{
	CH57xNET_ETHIsr();								/* ��̫���ж��жϷ����� */
}

void TMR0_IRQHandler( void ) 						/* ��ʱ���ж� */
{
	CH57xNET_TimeIsr(CH57xNETTIMEPERIOD);           /* ��ʱ���жϷ����� */
	R8_TMR0_INT_FLAG |= 0xff;						/* �����ʱ���жϱ�־ */
}

/*******************************************************************************
* Function Name  : SysTimeInit
* Description    : ϵͳ��ʱ����ʼ����CH579@100MHZ TIME0 10ms������CH57xNET_TIMEPERIOD
*                ������ʼ����ʱ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Timer0Init(UINT32 time)
{
	R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;		//������м���ֵ
	R8_TMR0_CTRL_MOD = 0;						//���ö�ʱ��ģʽ
	R32_TMR0_CNT_END = FREQ_SYS/1000000*time;	//���ö�ʱʱ��
	R8_TMR0_INT_FLAG = R8_TMR0_INT_FLAG;		//�����־
	R8_TMR0_INTER_EN = RB_TMR_IE_CYC_END;		//��ʱ�ж�
	R8_TMR0_CTRL_MOD |= RB_TMR_COUNT_EN;
	NVIC_EnableIRQ(TMR0_IRQn);	
}

/*******************************************************************************
* Function Name  : mStopIfError
* Description    : ����ʹ�ã���ʾ�������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void mStopIfError(UINT8 iError)
{
    if (iError == CH57xNET_ERR_SUCCESS) return;                                 /* �����ɹ� */
    PRINT("mStopIfError: %02X\r\n", (UINT16)iError);                           /* ��ʾ���� */
}

/*******************************************************************************
* Function Name  : CH57xNET_LibInit
* Description    : ���ʼ������
* Input          : ip      ip��ַָ��
*                ��gwip    ����ip��ַָ��
*                : mask    ����ָ��
*                : macaddr MAC��ַFָ�� 
* Output         : None
* Return         : ִ��״̬
*******************************************************************************/
UINT8 CH57xNET_LibInit(const UINT8 *ip,const UINT8 *gwip,const UINT8 *mask,const UINT8 *macaddr)
{
    UINT8 i;
    struct _CH57x_CFG cfg;
    if(CH57xNET_GetVer() != CH57xNET_LIB_VER)return 0xfc;                       /* ��ȡ��İ汾�ţ�����Ƿ��ͷ�ļ�һ�� */
    CH57xNETConfig = LIB_CFG_VALUE;                                             /* ��������Ϣ���ݸ�������ñ��� */
    cfg.RxBufSize = RX_BUF_SIZE; 
    cfg.TCPMss   = CH57xNET_TCP_MSS;
    cfg.HeapSize = CH57x_MEM_HEAP_SIZE;
    cfg.ARPTableNum = CH57xNET_NUM_ARP_TABLE;
    cfg.MiscConfig0 = CH57xNET_MISC_CONFIG0;
    CH57xNET_ConfigLIB(&cfg);
    i = CH57xNET_Init(ip,gwip,mask,macaddr);                                                         
    PRINT("CH57xNET_Config: %x\n",CH57xNETConfig);                                
    return (i);                                                                 /* ���ʼ�� */

}

/*******************************************************************************
* Function Name  : CH57xNET_HandleSokcetInt
* Description    : socket�жϴ�����
* Input          : sockeid  socket����
*                ��initstat �ж�״̬
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_HandleSokcetInt(UINT8 sockeid,UINT8 initstat)
{
    UINT32 len;

    PRINT("sockeid##= %02x\r\n",(UINT16)sockeid);
    if(initstat & SINT_STAT_RECV)                                               /* �����ж� */
    {
        PRINT("ftp sockid:%d\r\n",ftp.SocketCtl);
        len = CH57xNET_SocketRecvLen(sockeid,NULL);                             /* ��ѯ���� */
        if(len){

        ftp.RecDatLen = len; 
        CH57xNET_SocketRecv(sockeid,SocketRecvBuf[sockeid],&len);               /* �����ջ����������ݶ���MyBuf��*/
        PRINT("check type= %02x\r\n",(UINT16)ftp.FileCmd);
        PRINT("len= %08d\n",ftp.RecDatLen);
        PRINT("MyBuffer=\r\n\n%s\r\n",(char *)SocketRecvBuf[sockeid]);
        CH57xNET_FTPProcessReceDat((char *)SocketRecvBuf[sockeid],ftp.FileCmd,sockeid);
        memset((void *)SocketRecvBuf[sockeid],'\0',sizeof(SocketRecvBuf[sockeid]));    
        
        }          
    }
    if(initstat & SINT_STAT_CONNECT)                                            /* TCP�����ж� */
    {                                                                /* �������жϱ�ʾTCP�Ѿ����ӣ����Խ����շ����� */
        PRINT("TCP Connect Success\r\n");                           
        //ftp.TcpStatus = CH57xNET_FTPGetSockeID( );
			  if( (sockeid != ftp.DatMonitor) && (sockeid!= ftp.SocketCtl))
					{
                ftp.DatTransfer = sockeid;
								PRINT("CH57xNET_FTPGetSockeID:%d!\r\n",ftp.DatTransfer); 
                CH57xNET_ModifyRecvBuf(sockeid,(UINT32)SocketRecvBuf[sockeid],RECE_BUF_LEN);  
					}
					
    }
    if(initstat & SINT_STAT_DISCONNECT)                                         /* TCP�Ͽ��ж� */
    {                                                               /* �������жϣ�CH57xNET_���ڲ��Ὣ��socket�������Ϊ�ر�*/
        PRINT("TCP Disconnect\r\n");                                             /* Ӧ������������´������� */
        if(sockeid == ftp.DatTransfer){
            ftp.TcpStatus = 0;
            ftp.CmdDataS = FTP_MACH_DATAOVER;
            if(ftp.DatMonitor != 255){
            CH57xNET_SocketClose( ftp.DatMonitor,TCP_CLOSE_NORMAL );
            }
        }
        
    }
    if(initstat & SINT_STAT_TIM_OUT)                                            /* TCP��ʱ�ж� */
    {                                                                       /* �������жϣ�CH57xNET_���ڲ��Ὣ��socket�������Ϊ�ر�*/
        PRINT("TCP Timout\r\n");                                                 /* Ӧ������������´������� */
    }
}

/*******************************************************************************
* Function Name  : CH57xNET_HandleGlobalInt
* Description    : ȫ���жϴ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_HandleGlobalInt( void )
{
    UINT8 initstat;
    UINT8 i;
    UINT8 socketinit;
    
    initstat = CH57xNET_GetGlobalInt();                                         /* ��ȫ���ж�״̬����� */
    if(initstat & GINT_STAT_UNREACH)                                            /* ���ɴ��ж� */
    {
        PRINT("UnreachCode ��%d\n",CH57xInf.UnreachCode);                      /* �鿴���ɴ���� */
        PRINT("UnreachProto ��%d\n",CH57xInf.UnreachProto);                    /* �鿴���ɴ�Э������ */
        PRINT("UnreachPort ��%d\n",CH57xInf.UnreachPort);                      /* ��ѯ���ɴ�˿� */       
    }
   if(initstat & GINT_STAT_IP_CONFLI)                                           /* IP��ͻ�ж� */
   {
       PRINT("IP interrupt\n");
   }
   if(initstat & GINT_STAT_PHY_CHANGE)                                          /* PHY�ı��ж� */
   {
       i = CH57xNET_GetPHYStatus();                                             /* ��ȡPHY״̬ */
       PRINT("GINT_STAT_PHY_CHANGE %02x\n",i); 
   }
   if(initstat & GINT_STAT_SOCKET)                                              /* Socket�ж� */
   {
       for(i = 0; i < CH57xNET_MAX_SOCKET_NUM; i ++)                     
       {
           socketinit = CH57xNET_GetSocketInt(i);                               /* ��socket�жϲ����� */
           if(socketinit)CH57xNET_HandleSokcetInt(i,socketinit);                /* ������ж������� */
       }    
   }
}

/*******************************************************************************
* Function Name  : CH57xNET_FTPCtlClient
* Description    : ����TCP Client socket
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_FTPCtlClient(void)
{
   UINT8 i;                                                             
   SOCK_INF TmpSocketInf;                                                       /* ������ʱsocket���� */

   memset((void *)&TmpSocketInf,0,sizeof(SOCK_INF));                            /* ���ڲ��Ὣ�˱������ƣ�������ý���ʱ������ȫ������ */
   memcpy((void *)TmpSocketInf.IPAddr,DestIP,4);                                /* ����Ŀ��IP��ַ */
   TmpSocketInf.DesPort = DESPORT;                                              /* ����Ŀ�Ķ˿� */
   TmpSocketInf.SourPort = 4000;                                                /* ����Դ�˿� */
   TmpSocketInf.ProtoType = PROTO_TYPE_TCP;                                     /* ����socekt���� */
   TmpSocketInf.RecvStartPoint = (UINT32)SocketRecvBuf[0];                      /* ���ý��ջ������Ľ��ջ����� */
   TmpSocketInf.RecvBufLen = RECE_BUF_LEN ;                                     /* ���ý��ջ������Ľ��ճ��� */
   i = CH57xNET_SocketCreat( &SocketId,&TmpSocketInf );                         /* ����socket�������ص�socket����������SocketId�� */
   mStopIfError(i);                                                             /* ������ */
   ftp.SocketCtl = SocketId;    
   PRINT("SocketId TCP FTP = %02x\n",(UINT16)ftp.SocketCtl);
   i = CH57xNET_SocketConnect(SocketId);                                        /* TCP���� */
   mStopIfError(i);                                                             /* ������ */
   memset((void *)SocketRecvBuf[SocketId],'\0',sizeof(SocketRecvBuf[SocketId]));              
}

/*******************************************************************************
* Function Name  : CH57xNET_FTPDatClient
* Description    : ����TCP Client socket
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_FTPDatClient( UINT16 *port)
{
   UINT8 i;                                                             
   SOCK_INF TmpSocketInf;                                                       /* ������ʱsocket���� */

   if(gPort > 200) gPort = 0;
   gPort++;
   memset((void *)&TmpSocketInf,0,sizeof(SOCK_INF));                            /* ���ڲ��Ὣ�˱������ƣ�������ý���ʱ������ȫ������ */
   memcpy((void *)TmpSocketInf.IPAddr,DestIP,4);                                /* ����Ŀ��IP��ַ */
   TmpSocketInf.DesPort =  27;                                                  /* ����Ŀ�Ķ˿� */
   TmpSocketInf.SourPort = 4001;                                                /* ����Դ�˿� */
//  TmpSocketInf.SourPort = 4000+gPort;                                         /* ����Դ�˿� */
   *port = TmpSocketInf.SourPort; 
   TmpSocketInf.ProtoType = PROTO_TYPE_TCP;                                     /* ����socekt���� */
   TmpSocketInf.RecvStartPoint = (UINT32)SocketRecvBuf[1];                      /* ���ý��ջ������Ľ��ջ����� */
   TmpSocketInf.RecvBufLen = RECE_BUF_LEN ;                                     /* ���ý��ջ������Ľ��ճ��� */
   i = CH57xNET_SocketCreat( &SocketId,&TmpSocketInf );                         /* ����socket�������ص�socket����������SocketId�� */
   mStopIfError(i);                                                             /* ������ */
   PRINT("SocketId FTP DATA = %02x\n",(UINT16)ftp.SocketCtl);   
   i = CH57xNET_SocketConnect(SocketId);                                        /* TCP���� */
   mStopIfError(i);                                                             /* ������ */
}

/*******************************************************************************
* Function Name  : CH57xNET_FTPDatServer
* Description    : ����TCP Server socket
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_FTPDatServer( UINT16 *port )
{
   UINT8 i;
   SOCK_INF TmpSocketInf;                                                       /* ������ʱsocket���� */
 
   if(gPort > 200) gPort = 0;
   gPort++;
   memset((void *)&TmpSocketInf,0,sizeof(SOCK_INF));                            /* ���ڲ��Ὣ�˱������ƣ�������ý���ʱ������ȫ������ */
   TmpSocketInf.SourPort = 4000 + gPort;                                        /* ����Դ�˿� */
   *port = TmpSocketInf.SourPort; 
   TmpSocketInf.ProtoType = PROTO_TYPE_TCP;                                     /* ����socekt���� */
   i = CH57xNET_SocketCreat(&SocketId,&TmpSocketInf);                           /* ����socket�������ص�socket����������SocketId�� */
   mStopIfError(i);                                                             /* ������ */
   ftp.DatMonitor = SocketId; 
   i = CH57xNET_SocketListen(SocketId);                                         /* TCP���� */
   PRINT("SocketId dat server:%d\n",(UINT16)SocketId);    
   mStopIfError(i);                                                             /* ������ */
   memset((void *)SocketRecvBuf[SocketId],'\0',sizeof(SocketRecvBuf[SocketId]));              
}

/*******************************************************************************
* Function Name  : CH57xNET_ftp_SendData
* Description    : ��������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_FTPSendData( char *PSend, UINT32 Len,UINT8 index  )
{
    UINT32 length;
    UINT8 i,*p,count;

    p = (UINT8 *)PSend;
    length = Len;
    count = 0;
    while(1){
        Len = length;
        i = CH57xNET_SocketSend(index,p,&Len);                                  /* ��MyBuf�е����ݷ��� */
        mStopIfError(i);                                                        /* ������ */
        if(Len == 0){
            count++;
            if(count>2){
    PRINT("Send Data  fail\n");   
                return;
            } 
        }
        length -= Len;                                                          /* ���ܳ��ȼ�ȥ�Լ�������ϵĳ��� */
        p += Len;                                                               /* ��������ָ��ƫ��*/
        if(length)continue;                                                     /* �������δ������ϣ����������*/
        break;                                                                  /* ������ϣ��˳� */
    }
}

/*******************************************************************************
* Function Name  : CH57xNET_FTPGetSockeID
* Description    : ��ȡ��������socketid
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH57xNET_FTPGetSockeID(  )
{
    UINT8 i;
    for(i=0;i<CH57xNET_MAX_SOCKET_NUM;i++){
        if(SocketInf[i].DesPort == DESPORT){
            if( (i != ftp.DatMonitor) && (i!= ftp.SocketCtl)){
                ftp.DatTransfer = i;
								PRINT("CH57xNET_FTPGetSockeID:i!\r\n",ftp.DatTransfer); 
                CH57xNET_ModifyRecvBuf(i,(UINT32)SocketRecvBuf[i],RECE_BUF_LEN);  
                return TRUE;
            }
        }
    }
    return FALSE; 
}

/*******************************************************************************
* Function Name  : CH57xNET__FTPInitVari
* Description    : ������ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET__FTPInitVari( )
{
    UINT8 i,j;
    FTP   *pFTP;

    pFTP = &ftp;
    memset((void *)pFTP,0,sizeof(FTP));                   
    ftp.SocketCtl = 255; 
    ftp.DatMonitor = 255;
    ftp.DatTransfer = 255;
    ftp.FileCmd = FTP_CMD_LOGIN;                                                /* ִ�е�½���� */
    memset((void *)CharIP,'\0',sizeof(CharIP));                   
    j = 0;
    for(i=0;i<4;i++){                                                           /* ��ʮ���Ƶ�IP��ַת������Ҫ���ַ���ʽ */ 
        if( IPAddr[i]/100 ){
            CharIP[j++] = IPAddr[i]/100 + '0';
            CharIP[j++] = (IPAddr[i]%100)/10 + '0' ;
            CharIP[j++] = IPAddr[i]%10 + '0';
        } 
        else if( IPAddr[i]/10 ){
            CharIP[j++] = IPAddr[i]/10 + '0';
            CharIP[j++] = IPAddr[i]%10 + '0';
        } 
        else CharIP[j++] = IPAddr[i]%10 + '0';
         CharIP[j++] = ',';
    }
}

/*******************************************************************************
* Function Name  : CH57xNET_FTPInit
* Description    : ch579��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_FTPInit( void ) 
{

    UINT8 i = 0;
    CH57xNET__FTPInitVari( );                                                   /* ��ʼ������ */
    i = CH57xNET_LibInit(IPAddr,GWIPAddr,IPMask,MACAddr);                       /* ���ʼ�� */
    mStopIfError(i);                                                            /* ������ */
    PRINT("CH57xNET_LibInit Success\n");   
    DelayMs(50);
    Timer0Init( 10000 );		// ��ʼ����ʱ��:10ms
	NVIC_EnableIRQ(ETH_IRQn);
}

/*******************************************************************************
* Function Name  : SystemClock_UART1_init
* Description    : ϵͳʱ�Ӻʹ���1��ʼ��������1���ڴ�ӡ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SystemClock_UART1_init(void)
{
    PWR_UnitModCfg(ENABLE, UNIT_SYS_PLL);                                      /* PLL�ϵ� */
    DelayMs(3); 
    SetSysClock(CLK_SOURCE_HSE_32MHz);                                          /* �ⲿ���� PLL ���32MHz */
    GPIOA_SetBits( GPIO_Pin_9 );
    GPIOA_ModeCfg( GPIO_Pin_9, GPIO_ModeOut_PP_5mA );                           /* ����1��IO������ */
	UART1_DefInit( );                                                            /* ����1��ʼ�� */
}

/*******************************************************************************
* Function Name  : GetMacAddr
* Description    : ϵͳ��ȡMAC��ַ
* Input          : pMAC:ָ�������洢Mac��ַ�Ļ���
* Output         : None
* Return         : None
*******************************************************************************/
void GetMacAddr(UINT8 *pMAC)
{
	UINT8 transbuf[6],i;
	
	GetMACAddress(transbuf);
	for(i=0;i<6;i++)
	{
		pMAC[5-i]=transbuf[i];
	
	}
}

/*******************************************************************************
* Function Name  : main
* Description    : ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main( void )
{
	SystemClock_UART1_init();
	GetMacAddr(MACAddr);
    CH57xNET_FTPInit( );                                                        /* ��ʼ�� */
    while(1)
    {
        CH57xNET_MainTask( );                                                   /* CH579IP��������������Ҫ����ѭ���в��ϵ��� */
        if(CH57xNET_QueryGlobalInt( ))CH57xNET_HandleGlobalInt();               /* ��ѯ�жϣ�������жϣ������ȫ���жϴ����� */
        CH57xNET_FTPClientCmd( );                                               /* ��ѯ״̬��ִ�������� */
    }
}

/*********************************** endfile **********************************/
