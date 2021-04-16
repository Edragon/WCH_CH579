/********************************** (C) COPYRIGHT ******************************
* File Name          : MQTT_Pub.c
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/01
* Description        : ����TCP/IP��MQTTЭ��ͨѶ���̣�ʵ����MQTT��ʽ��ͨ���ٶȡ�������������������ʵ���豸��ͨ��ʹ��ǰ�������ʺź�����
*					(1)��CH579 Examples by KEIL;
*					(2)������1��������Ϣ,115200bps,�򿪻��߹رյ�����Ϣ�����IDE�궨��������;
*					(3)��������������ʾ����TCP/IP��MQTTЭ��ͨѶ��
					     �˳�����ʾPUB�Ĺ���
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ�����*/
#include <stdio.h>
#include <string.h>
#include "CH57x_common.h"
#include "core_cm0.h"
#include "CH57xNET.H"
#include "MQTTPacket.H"

#define KEEPLIVE_ENABLE                      1                                  /* ����KEEPLIVE���� */

/* ����Ļ�������ȫ�ֱ�������Ҫ���壬���е��� */
__align(16)UINT8    CH57xMACRxDesBuf[(RX_QUEUE_ENTRIES )*16];                   /* MAC������������������16�ֽڶ��� */
__align(4) UINT8    CH57xMACRxBuf[RX_QUEUE_ENTRIES*RX_BUF_SIZE];                /* MAC���ջ�������4�ֽڶ��� */
__align(4) SOCK_INF SocketInf[CH57xNET_MAX_SOCKET_NUM];                         /* Socket��Ϣ��4�ֽڶ��� */

UINT16 MemNum[8] = {CH57xNET_NUM_IPRAW,
                    CH57xNET_NUM_UDP,
                    CH57xNET_NUM_TCP,
                    CH57xNET_NUM_TCP_LISTEN,
                    CH57xNET_NUM_TCP_SEG,
                    CH57xNET_NUM_IP_REASSDATA,
                    CH57xNET_NUM_PBUF,
                    CH57xNET_NUM_POOL_BUF
                    };
UINT16 MemSize[8] = {CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_IPRAW_PCB),
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
/******************************************************************************/
/* ����ʾ�������غ� */
#define RECE_BUF_LEN                          536                               /* ���ջ������Ĵ�С */
/* CH57xNET��TCP��MSS����Ϊ536�ֽڣ���һ��TCP��������ݲ����Ϊ536�ֽ� */
/* TCPЭ��ջ���û������ڽ������أ��������ֵΪsocket�Ľ��ջ��������ȡ����趨 */
/* RX_QUEUE_ENTRIESʱҪ����MSS�ʹ���֮��Ĺ�ϵ�����細��ֵΪ4*MSS����Զ��һ�λᷢ�� */
/* 4��TCP�������RX_QUEUE_ENTRIESС��4�����Ȼ�ᵼ�����ݰ���ʧ���Ӷ�����ͨѶЧ�ʽ��� */
/* ����RX_QUEUE_ENTRIESҪ����( ����/MSS ),������socketͬʱ���д������������ݣ��� */ 
/* ����RX_QUEUE_ENTRIESҪ����(( ����/MSS )*socket����) �ڶ��socketͬʱ���д��������շ�ʱ */
/* Ϊ�˽�ԼRAM���뽫���ջ������ĳ�������ΪMSS */

/* CH579MQTT��ض��壬��Ҫ����ʵ������޸� */
UINT8 MACAddr[6] = {0x84,0xc2,0xe4,0x02,0x03,0x04};                              /* CH579MAC��ַ */
UINT8 IPAddr[4]  = {192,168,1,200};                                              /* CH579IP��ַ */
UINT8 GWIPAddr[4]= {192,168,1,1};                                                /* CH579���� */
UINT8 IPMask[4]  = {255,255,255,0};                                              /* CH579�������� */
UINT8 DESIP[4]   = {58,213,74,190};                                              /* Ŀ��IP��ַ */
UINT16 aport=1000;											                     /* CH579Դ�˿� */

/* ���ڵƶ��� PB�ڵ�ʮ��λ��Ч */
UINT16 CH57xNET_LEDCONN=0x0010;                                                 /* ����ָʾ�� PB4 */
UINT16 CH57xNET_LEDDATA=0x0080;                                                 /* ͨѶָʾ�� PB7 */ 

char *username =  "";							                                 /* �豸����ÿ���豸Ψһ�����á�/�����ּ� */
char *password =  "";								                             /* ��������½���� */
char *sub_topic = "";								                             /* ���ĵĻỰ����Ϊ���Է����գ�Ӧ�뷢���ĻỰ����ͬ */
char *pub_topic = "";									                         /* �����ĻỰ*/

UINT8 SocketId;                                                                  /* ����socket���������Բ��ö��� */
UINT8 SocketRecvBuf[RECE_BUF_LEN];                                               /* socket���ջ����� */
UINT8 MyBuf[RECE_BUF_LEN];                                                       /* ����һ����ʱ������ */

UINT8 con_flag=0;										                         /* ������MQTT��������־λ */
UINT8 pub_flag=1;											                     /* �ѷ����Ự��Ϣ��־λ */
UINT8 sub_flag=0;											                     /* �Ѷ��ĻỰ��־λ */
UINT8 tout_flag=0;											                     /* ��ʱ��־λ */
UINT16 packetid=0;											                     /* ��ID */



/*******************************************************************************
* Function Name  : IRQ_Handler
* Description    : IRQ�жϷ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ETH_IRQHandler( void )						                             	/* ��̫���ж� */
{
	CH57xNET_ETHIsr();								                            /* ��̫���ж��жϷ����� */
}

void TMR0_IRQHandler( void ) 						                            /* ��ʱ���ж� */
{
	CH57xNET_TimeIsr(CH57xNETTIMEPERIOD);                                       /* ��ʱ���жϷ����� */
	R8_TMR0_INT_FLAG |= 0xff;					                             	/* �����ʱ���жϱ�־ */
}



/*******************************************************************************
* Function Name  : mStopIfError
* Description    : ����ʹ�ã���ʾ�������
* Input          : iError �������
* Output         : None
* Return         : None
*******************************************************************************/
void mStopIfError(UINT8 iError)
{
    if (iError == CH57xNET_ERR_SUCCESS) return;                                /* �����ɹ� */
    PRINT("mStopIfError: %02X\r\n", (UINT16)iError);                           /* ��ʾ���� */
}

/*******************************************************************************
* Function Name	: Transport_Open
* Description	: ����TCP����
* Input			: None
* Output		: None
* Return		: None
*******************************************************************************/
UINT8 Transport_Open()
{
	UINT8 i;																														 
	SOCK_INF TmpSocketInf;								                 	/* ������ʱsocket���� */

	memset((void *)&TmpSocketInf,0,sizeof(SOCK_INF));	                 	/* ���ڲ��Ὣ�˱������ƣ�������ý���ʱ������ȫ������ */
	memcpy((void *)TmpSocketInf.IPAddr,DESIP,4);		                   	/* ����Ŀ��IP��ַ */
	TmpSocketInf.DesPort = 1883;							                /* ����Ŀ�Ķ˿� */
	TmpSocketInf.SourPort = aport++;						                /* ����Դ�˿� */
	TmpSocketInf.ProtoType = PROTO_TYPE_TCP;			                	/* ����socekt���� */
	TmpSocketInf.RecvStartPoint = (UINT32)SocketRecvBuf;	                /* ���ý��ջ������Ľ��ջ����� */
	TmpSocketInf.RecvBufLen = RECE_BUF_LEN ;			                 	/* ���ý��ջ������Ľ��ճ��� */
	i = CH57xNET_SocketCreat(&SocketId,&TmpSocketInf);		                /* ����socket�������ص�socket����������SocketId�� */
	mStopIfError(i);										                /* ������ */
	
	i = CH57xNET_SocketConnect(SocketId);				                	/* TCP���� */
	mStopIfError(i);										                /* ������ */
	return SocketId;
}


/*******************************************************************************
* Function Name	: Transport_Close
* Description	: �ر�TCP����
* Input			: None
* Output		: None
* Return		: None
*******************************************************************************/
UINT8 Transport_Close()
{
	UINT8 i;
	i=CH57xNET_SocketClose(SocketId,TCP_CLOSE_NORMAL);
	mStopIfError(i);
	return i;
}


/*******************************************************************************
* Function Name	: Transport_SendPacket
* Description	: ��̫����������
* Input			: UINT8 *buf �������ݵ����ֽڵ�ַ
				  UINT32 len �������ݵĳ���
* Output		: None
* Return		: None
*******************************************************************************/
void Transport_SendPacket(UINT8 *buf,UINT32 len)
{
	UINT32 totallen;
	UINT8 *p=buf;
	
	totallen=len;
	while(1)
	{
		len = totallen;
		CH57xNET_SocketSend(SocketId,p,&len);				              	/* ��MyBuf�е����ݷ��� */
		totallen -= len;					 					            /* ���ܳ��ȼ�ȥ�Լ�������ϵĳ��� */
		p += len;											             	/* ��������ָ��ƫ��*/
		if(totallen)continue;									            /* �������δ������ϣ����������*/
		break;													            /* ������ϣ��˳� */
	}
}


/*******************************************************************************
* Function Name	: MQTT_Connect
* Description	: ����MQTT����
* Input			: char *username �豸��
				  char *password ��������������
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Connect(char *username,char *password)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	UINT32 len;
	UINT8 buf[200];

	data.clientID.cstring = "11";
	data.keepAliveInterval = 20;
	data.cleansession = 1;
	data.username.cstring = username;																		
	data.password.cstring = password;
	 
	len=MQTTSerialize_connect(buf,sizeof(buf),&data);											
	Transport_SendPacket(buf,len);						
}


/*******************************************************************************
* Function Name	: MQTT_Subscribe
* Description	: MQTT����һ������
* Input			: char *topic ���ĵ�������
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Subscribe( char *topic)
{
	MQTTString topicString = MQTTString_initializer;
	int req_qos=0;
	UINT32 len;
	UINT32 msgid=1;
	UINT8 buf[200];
	
	topicString.cstring=topic;
	len=MQTTSerialize_subscribe(buf,sizeof(buf),0,msgid,1,&topicString,&req_qos);
	Transport_SendPacket(buf,len);
}


/*******************************************************************************
* Function Name	: MQTT_Unsubscribe
* Description	: MQTTȡ������һ������
* Input			: char *topic ȡ�����ĵ�������
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Unsubscribe(char *topic)
{
	MQTTString topicString = MQTTString_initializer;
	UINT32 len;
	UINT32 msgid=1;
	UINT8 buf[200];
	
	topicString.cstring=topic;
	len=MQTTSerialize_unsubscribe(buf,sizeof(buf),0,msgid,1,&topicString);
	Transport_SendPacket(buf,len);
}


/*******************************************************************************
* Function Name	: MQTT_Subscribe
* Description	: MQTT����һ������
* Input			: char *topic ���ĵ�������
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Publish(char *topic,char *payload)
{
	MQTTString topicString = MQTTString_initializer;
	UINT32 payloadlen;
	UINT32 len;
	UINT8 buf[1024];
	
	topicString.cstring=topic; 
	payloadlen=strlen(payload);
	len= MQTTSerialize_publish(buf,sizeof(buf),0,0,0,packetid++,topicString,payload,payloadlen);	
	Transport_SendPacket(buf,len);
}


/*******************************************************************************
* Function Name	: MQTT_Pingreq
* Description	: MQTT����������
* Input			: None
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Pingreq()
{
	UINT32 len;
	UINT8 buf[200];
	
	len=MQTTSerialize_pingreq(buf,sizeof(buf));
	Transport_SendPacket(buf,len);
}


/*******************************************************************************
* Function Name	: MQTT_Disconnect
* Description	: �Ͽ�MQTT����
* Input			: None
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Disconnect()
{
	UINT32 len;
	UINT8 buf[50];
	len=MQTTSerialize_disconnect(buf,sizeof(buf));
	Transport_SendPacket(buf,len);
}



/*******************************************************************************
* Function Name  : CH57xNET_CreatTcpSocket
* Description    : ����TCP Client socket
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_CreatTcpSocket(void)
{
   UINT8 i;                                                             
   SOCK_INF TmpSocketInf;                                                       /* ������ʱsocket���� */

   memset((void *)&TmpSocketInf,0,sizeof(SOCK_INF));                            /* ���ڲ��Ὣ�˱������ƣ�������ý���ʱ������ȫ������ */
   memcpy((void *)TmpSocketInf.IPAddr,DESIP,4);                                 /* ����Ŀ��IP��ַ */
   TmpSocketInf.DesPort = 1000;                     
   TmpSocketInf.SourPort = 2000;                                                /* ����Դ�˿� */
   TmpSocketInf.ProtoType = PROTO_TYPE_TCP;                                     /* ����socekt���� */
   TmpSocketInf.RecvStartPoint = (UINT32)SocketRecvBuf;                         /* ���ý��ջ������Ľ��ջ����� */
   TmpSocketInf.RecvBufLen = RECE_BUF_LEN ;                                     /* ���ý��ջ������Ľ��ճ��� */
   i = CH57xNET_SocketCreat(&SocketId,&TmpSocketInf);                           /* ����socket�������ص�socket����������SocketId�� */
   mStopIfError(i);                                                             /* ������ */
#ifdef  KEEPLIVE_ENABLE
   CH57xNET_SocketSetKeepLive( SocketId, 1 );                                   /* ����socket��KEEPLIVE���ܣ�V06�汾֧�֣� */
#endif

   i = CH57xNET_SocketConnect(SocketId);                                        /* TCP���� */
   mStopIfError(i);                                                             /* ������ */
   i = CH57xNET_SetSocketTTL( SocketId,10 );
   mStopIfError(i);                                                             /* ������ */
}


/*******************************************************************************
* Function Name  : net_initkeeplive
* Description    : keeplive��ʼ��
* Input          : None      
* Output         : None
* Return         : None
*******************************************************************************/
#ifdef  KEEPLIVE_ENABLE
void net_initkeeplive(void)
{
    struct _KEEP_CFG  klcfg;

    klcfg.KLIdle = 20000;                                                       /* ���� */
    klcfg.KLIntvl = 10000;                                                      /* ��� */
    klcfg.KLCount = 5;                                                          /* ���� */
    CH57xNET_ConfigKeepLive(&klcfg);
}
#endif

/*******************************************************************************
* Function Name  : CH57xNET_LibInit
* Description    : ���ʼ������
* Input          : ip      ip��ַָ��
*                ��gwip    ����ip��ַָ��
*                : mask    ����ָ��
*                : macaddr MAC��ַָ�� 
* Output         : None
* Return         : ִ��״̬
*******************************************************************************/
UINT8 CH57xNET_LibInit(/*const*/ UINT8 *ip,/*const*/ UINT8 *gwip,/*const*/ UINT8 *mask,/*const*/ UINT8 *macaddr)
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
#ifdef  KEEPLIVE_ENABLE
    net_initkeeplive( );
#endif
    return (i);                    
}

/*******************************************************************************
* Function Name  : CH57xNET_HandleSockInt
* Description    : Socket�жϴ�����
* Input          : sockeid  socket����
*                ��initstat �ж�״̬
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_HandleSockInt(UINT8 sockeid,UINT8 initstat)
{
    UINT32 len;
    UINT8 i;
    unsigned char* dup;
	unsigned short* packetid;

	int* qos;
	unsigned char* retained;
	MQTTString* topicName;
	unsigned char* payload;
	int payloadlen;
	unsigned char *p=payload;

    if(initstat & SINT_STAT_RECV)                                             /* �����ж� */
    {
        len = CH57xNET_SocketRecvLen(sockeid,NULL);		                      /* ��ѯ���� */
		CH57xNET_SocketRecv(sockeid,MyBuf,&len);			                  /* �����ջ����������ݶ���MyBuf��*/
		switch(MyBuf[0]>>4)
		{
			case FLAG_CONNACK:
				PRINT("connack\r\n");
				con_flag=1;
				break;

			case FLAG_PUBLISH:
				MQTTDeserialize_publish(dup,qos,retained,packetid,topicName,&payload,&payloadlen,MyBuf,len);
				PRINT("payloadlen=%d\r\n",(UINT16)payloadlen);
				p=payload;
				for(i=0;i<payloadlen;i++)
				{
					PRINT("%c ",(UINT16)*p);
					p++;
				}
				PRINT("\r\n");
				break;
				
			case FLAG_SUBACK:
				sub_flag=1;
				PRINT("suback\r\n");
				break;

			default:

				break;
		}
    }
    if(initstat & SINT_STAT_CONNECT)                                          /* TCP�����ж� */
    {                                                                         /* �������жϱ�ʾTCP�Ѿ����ӣ����Խ����շ����� */
        PRINT("TCP Connect Success\n");   
        MQTT_Connect(username, password);        
    }
    if(initstat & SINT_STAT_DISCONNECT)                                       /* TCP�Ͽ��ж� */
    {                                                                         /* �������жϣ�CH579���ڲ��Ὣ��socket�������Ϊ�ر�*/
        PRINT("TCP Disconnect\n");                                            /* Ӧ������������´������� */
        CH57xNET_CreatTcpSocket();
    }
    if(initstat & SINT_STAT_TIM_OUT)                                          /* TCP��ʱ�ж� */
    {                                                                         /* �������жϣ�CH579���ڲ��Ὣ��socket�������Ϊ�ر�*/
        PRINT("TCP Timout\n");                                                /* Ӧ������������´������� */
        Transport_Open();
    }
}


/*******************************************************************************
* Function Name  : CH57xNET_HandleGloableInt
* Description    : ȫ���жϴ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_HandleGlobalInt(void)
{
    UINT8 initstat;
    UINT8 i;
    UINT8 socketinit;
    initstat = CH57xNET_GetGlobalInt();                                        /* ��ȫ���ж�״̬����� */
    if(initstat & GINT_STAT_UNREACH)                                           /* ���ɴ��ж� */
    {
        PRINT("UnreachCode ��%d\n",CH57xInf.UnreachCode);                      /* �鿴���ɴ���� */
        PRINT("UnreachProto ��%d\n",CH57xInf.UnreachProto);                    /* �鿴���ɴ�Э������ */
        PRINT("UnreachPort ��%d\n",CH57xInf.UnreachPort);                      /* ��ѯ���ɴ�˿� */     
    }
   if(initstat & GINT_STAT_IP_CONFLI)                                          /* IP��ͻ�ж� */
   {
   
   }
   if(initstat & GINT_STAT_PHY_CHANGE)                                         /* PHY�ı��ж� */
   {
       i = CH57xNET_GetPHYStatus();                                            /* ��ȡPHY״̬ */
       PRINT("GINT_STAT_PHY_CHANGE %02x\n",i);
   }
   if(initstat & GINT_STAT_SOCKET)                                             /* Socket�ж� */
   {
       for(i = 0; i < CH57xNET_MAX_SOCKET_NUM; i ++)                     
       {
           socketinit = CH57xNET_GetSocketInt(i);                              /* ��socket�жϲ����� */
           if(socketinit)CH57xNET_HandleSockInt(i,socketinit);                 /* ������ж������� */
       }    
   }
}


void Timer0Init(UINT32 time)
{
	R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;		                               /* ������м���ֵ */
	R8_TMR0_CTRL_MOD = 0;						                               /* ���ö�ʱ��ģʽ */
	R32_TMR0_CNT_END = FREQ_SYS/1000000*time;	                               /* ���ö�ʱʱ�� */
	R8_TMR0_INT_FLAG = R8_TMR0_INT_FLAG;		                               /* �����־ */
	R8_TMR0_INTER_EN = RB_TMR_IE_CYC_END;	                                   /* ��ʱ�ж� */
	R8_TMR0_CTRL_MOD |= RB_TMR_COUNT_EN;
	NVIC_EnableIRQ(TMR0_IRQn);	
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
int main(void) 
{
    UINT32 i = 0;
    UINT16 TimeDelay=0;
	char payload[500];	
	for(i=0;i<500;i++)
	payload[i]='a';

	SystemClock_UART1_init();
	GetMacAddr(MACAddr);
    i = CH57xNET_LibInit(IPAddr,GWIPAddr,IPMask,MACAddr);                       /* ���ʼ�� */
    mStopIfError(i);                                                            /* ������ */
    PRINT("CH57xNETLibInit Success\r\n");   
	Timer0Init( 10000 );		                                                /* ��ʼ����ʱ��:10ms */
	NVIC_EnableIRQ(ETH_IRQn);
	Transport_Open();    
	while ( CH57xInf.PHYStat < 2 ) {
		DelayMs(50);
	}	                                
    PRINT("CH579 MQTT socket create!Publishing\r\n");   	
    while(1)
    {
        CH57xNET_MainTask();                                                    /* CH57xNET��������������Ҫ����ѭ���в��ϵ��� */
        if(CH57xNET_QueryGlobalInt())CH57xNET_HandleGlobalInt();                /* ��ѯ�жϣ�������жϣ������ȫ���жϴ����� */
        DelayMs(1);
		TimeDelay++;
		if (TimeDelay>500) {
			TimeDelay=0;
			if(con_flag) MQTT_Publish(pub_topic,payload);
		}   
    }
}
/*********************************** endfile **********************************/
