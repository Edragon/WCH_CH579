/********************************** (C) COPYRIGHT ******************************
* File Name          : FTP_CLIENT.C
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/01
* Description        : CH579оƬFTP�ӿ�����                    
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ����� */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CH57x_common.h"
#include "core_cm0.h"
#include "CH57xNET.h"
#include "FTPINC.h"
#include "FTPCMD.c"
#include "FTPfile.c"

/*******************************************************************************
* Function Name  : CH57xNET_FTPProcessReceDat
* Description    : ��ȡ��������   
* Input          : recv_buff  -579���յ�������
*                  check_type -��ǰ��������
*                  socketid   -socket����
* Output         : None               
* Return         : None
*******************************************************************************/
void CH57xNET_FTPProcessReceDat( char *recv_buff,UINT8 check_type,UINT8 socketid )
{
    UINT8 S;
    if( socketid == ftp.DatTransfer ){
        if(ftp.CmdDataS == FTP_MACH_RECEDATA){                                  /* �����ļ����� */
            if( ftp.InterCmdS == FTP_MACH_GETFILE ){ 
                S = CH57xNET_FTPFileWrite(recv_buff,strlen(recv_buff));
            }
            else if(ftp.InterCmdS == FTP_MACH_FINDLIST ){
                S = CH57xNET_FTPFindList( recv_buff );                          /* ��֤�������ݣ����ڲ���ָ����Ŀ¼�� */
                if( S == FTP_CHECK_SUCCESS ) ftp.FindList = 1;                  /* �鵽ָ����Ŀ¼�� */
            }
            else if(ftp.InterCmdS == FTP_MACH_FINDFILE ){
                S = CH57xNET_FTPFindFile( recv_buff );                          /* �����ļ� */    
                if( S == FTP_CHECK_SUCCESS ) ftp.FindFile = 1;                  /* �ҵ��ļ� */
            }
        }
    }
    else if( socketid == ftp.SocketCtl ){   
        /* ����Ϊ����Ӧ�� */           
        S = CH57xNET_FTPCheckRespond(recv_buff,check_type);
    }
}

/*******************************************************************************
* Function Name  : CH395_FTPSendFile
* Description    : ��������
* Input          : NONE
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_FTPSendFile( void )
{
    UINT8 S;    
    
    S = CH57xNET_FTPFileOpen( FileName );
    if(S == FTP_CHECK_SUCCESS)    CH57xNET_FTPFileRead( );
    CH57xNET_FTPSendData( send_buff, strlen(send_buff),ftp.DatTransfer );
    if(ftp.CmdDataS == FTP_MACH_DATAOVER){
        CH57xNET_SocketClose( ftp.DatTransfer,TCP_CLOSE_NORMAL );
    }    
}

/*******************************************************************************
* Function Name  : CH57xNET_FTPCheckRespond
* Description    : ��ѯ״̬��ִ����Ӧ����
* Input          : recv_buff  -������Ϣ
                   check_type -��������
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_FTPClientCmd( void )
{
    
    if(ftp.CmdDataS == FTP_MACH_SENDDATA){//printf("client1\r\n"); 
        if(ftp.TcpStatus == FTP_MACH_CONNECT) CH57xNET_FTPSendFile( );          /* ��������������� */
        return ;     
    }
    if(ftp.CmdDataS == FTP_MACH_DATAOVER){//printf("client2\r\n"); 
        if(ftp.FindList == 1){                                                  /* �ҵ�Ŀ¼�� */
            ftp.FindList = 0;
            CH57xNET_FTPCwd( 0 );
        }
    }
    if(ftp.FileCmd){//printf("client3\r\n"); 
        CH57xNET_FTPInterCmd( );                                                /* ִ�ж�Ӧ�Ľӿ����� */
    }
}

/*******************************************************************************
* Function Name  : CH57xNET_FTPInterCmd
* Description    : ִ�ж�Ӧ����������е���ִ��˳��
* Input          : NONE
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_FTPInterCmd( void )
{
    UINT8 i;
    switch(ftp.FileCmd){
        case FTP_CMD_LOGIN:                                                     /* ��½ */
           i= CH57xNET_FTPLogin( ); //printf("CH57xNET_FTPLogin %02x\r\n",i); 
           if( /*CH57xNET_FTPLogin( )*/i == FTP_COMMAND_SUCCESS ){                  /* ��½�ɹ�,�ɽ����������� */             
               printf("login!\r\n"); 
						 CH57xNET_FTPSearch("C:\Users\FJY\Desktop\192","FILELIST.txt" );                     /* ��ʼ��ѯָ����Ŀ¼ */   
             
            }
            break;
        case FTP_CMD_SEARCH:                                                    /* �Ѳ��ļ���������Ŀ¼�����ļ�����*/
            if( CH57xNET_FTPSearch("USER","FILELIST.txt" ) == FTP_COMMAND_SUCCESS ){        
                if( ftp.FindFile )CH57xNET_FTPGetFile("FILELIST.txt" );         /* �Ѳ鵽ָ����Ŀ¼�µ��ļ���ʼ�����ļ� */
                else CH57xNET_FTPQuit( );                                       /* û�Ѳ鵽ָ����Ŀ¼�µ��ļ����˳���Ҳ�ɽ����������������ϴ���*/
            }
            break;
        case FTP_CMD_GETFILE:                                                   /* �����ļ����������ļ�����*/ 
            if(CH57xNET_FTPGetFile("FILELIST.txt" ) == FTP_COMMAND_SUCCESS ){   /* �����ļ��ɹ����ɽ����������� */
                CH57xNET_FTPPutFile("TEXT","abc.txt");                          /* �ϴ��ļ� */
            }
            break;
        case FTP_CMD_PUTFILE:                                                   /* �ϴ��ļ���������Ŀ¼�����ļ�����*/
            if( CH57xNET_FTPPutFile("TEXT","abc.txt")== FTP_COMMAND_SUCCESS ){  /* �ϴ��ļ��ɹ����ɽ����������� */
                CH57xNET_FTPQuit( );                                            /* �˳� */
            }
            break;
        default:
            break;
    }
}

/*********************************** endfile **********************************/
