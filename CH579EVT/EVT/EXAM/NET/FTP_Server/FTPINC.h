/********************************** (C) COPYRIGHT *********************************
* File Name          : FTPINC.H
* Author             : WCH
* Version            : V1.0
* Date               : 2013/11/15
* Description        : Define for CH579FTP.c
**********************************************************************************/



/*********************************************************************************/
#ifndef _FTPINC_H_
#define _FTPINC_H_
#include "ch579sfr.h"


#ifndef Access_Authflag
#define Access_Authflag                0            /* ����ָ���û������ʣ������û����޷����ʷ�������*/
#endif
typedef struct _FTP
{
    UINT8 TimeCount;                   // ��ʱ������
    UINT8 FileFlag;                    // �ж��ļ����ȴ���64�ֽڱ�־λ��������֧������ϴ��ļ�����64��
    UINT8 BufStatus;                   // ��������־λ��0-3λ�ֱ��Ӧ�ĸ�������״̬��0Ϊ��1Ϊ��ʹ�ã�
    UINT8 AuthFlag;                    // �û�Ȩ�ޱ�־λ
    UINT8 ListFlag;                    // 
    UINT8 CmdDataS;                    // ���ݴ���״̬
    UINT8 DataOver;                    // ���ݶ�ȡ���
    UINT8 ListState;                   // 0Ϊ��ǰĿ¼ 1Ϊ��һ��Ŀ¼
    UINT8 TcpCtlSta;                   // TCP��������״̬
    UINT8 TcpDatSta;                   // TCP��������״̬
    UINT8 CmdReceDatS;                 // ��������״̬
    UINT8 SocketCtlMonitor;            // ftp�������ӣ�����socket����
    UINT8 SocketCtlConnect;            // ftp�������ӣ�����socket����
    UINT8 SocketCtlSecond;             // ftp�������ӣ�����socket����
    UINT8 SocketDatMonitor;            // ftp�������ӣ�����socket����
    UINT8 SocketDatConnect;            // ftp�������ӣ�����socket����
}FTP;
extern FTP ftp;
extern     char  UserName[16];         // ���ڱ����û���
extern     char  SourIP[17];           // ���ڱ���ת�����ַ���IP��ַ
#if Access_Authflag
extern     char  *pUserName;           // ���Է��ʷ�����û����������û����޷����ʷ�������
#endif
extern     char  *pPassWord;           // ��Ȩ�޵����루��������ֻ֧�ֶ���
/* CH579��ض��� */                                                                   
extern UINT8 MACAddr[6];                         // CH579MAC��ַ 
extern const UINT8 IPAddr[4];                    // CH579IP��ַ 
extern const UINT8 GWIPAddr[4];                  // CH579���� 
extern const UINT8 IPMask[4];                    // CH579�������� 
/* TCP����״̬ */
#define FTP_MACH_DISCONT             0x01        // TCPδ����
#define FTP_MACH_CONNECT             0x02        // TCP������
#define FTP_MACH_KEPCONT             0x04        // TCP��������
/*  ���ݴ���״̬ */
#define FTP_MACH_SENDDATA            0x01        // ��Ҫ��������
#define FTP_MACH_RECEDATA            0x02        // ��Ҫ��������
#define FTP_MACH_DATAOVER            0x04        // ���ݴ������
#define FTP_MACH_CLOSECTL            0x08        // ��������
/*    */
#define FTP_MACH_LIST                0x00        // ��ȡ�б�
#define FTP_MACH_FILE                0x01        // �����ļ�
//==============================================================================
void  CH57xNET_FTPSendData( char *PSend, UINT32 Len,UINT8 index  );             // ��������
     
void  CH57xNET_FTPConnect( void );                                              // 579��ʼ��

void  CH57xNET_FTPInitVari( void );                                             // ������ʼ��

void  mInitFtpList( void );                                                     // ��ʼ��ģ���ļ�Ŀ¼��Ϣ�Լ��ļ� 

void CH57xNET_FTPListRenew( UINT8 index );                                      // ����Ŀ¼
                                  
void CH57xNET_FTPHandleDatRece( char *recv_buff,UINT8 index );                  // ������յ�������

void CH57xNET_FTPCmdRespond( char *recv_buff );                                 // �����յ�������������Ӧ�Ļظ� 

void CH57xNET_FTPDataReady( char *pSendDat );                                   // ׼��Ҫ���͵�����

void CH57xNET_FTPGetPortNum( char *recv_buff );                                 // ��ȡ�������ӵĶ˿�

void CH57xNET_FTPGetFileName( char *recv_buff,char *pBuf );                     // ��ȡ���� 

UINT8 CH57xNET_FTPFileOpen(  char *pName,UINT8 index  );                        // ���ļ�

UINT32 CH57xNET_FTPFileSize ( char *pName );                                    // ��ȡ�ļ�����

void CH57xNET_FTPFileRead( char *pName );                                       // ���ļ�

void CH57xNET_FTPFileWrite( char *recv_buff, UINT16 LEN );                      // д�ļ�

void CH57xNET_FTPCtlServer(void);                                               // ����FTP��������

void CH57xNET_FTPClientDat(  UINT16 port,UINT8 *pAddr );                        // ����FTP��������
                                                      
void CH57xNET_FTPServerDat( UINT16 *port );                                     // ����FTP��������

void CH57xNET_FTPGetSockeID( UINT8 socketid  );                                 // ��ȡ��������socket����

void CH57xNET_FTPServerCmd( void );                                             // ��ѯ״̬��ִ����Ӧ���ӳ���

void mStopIfError(UINT8 iError);

void CH57xNET_HandleGloableInt( void );

#endif    // __ftpinc_H__
 //================================================================================ 
