#include <windows.h>
#include <stdio.h>
#include "RsComm.h"
#include "skel.h"

#define TERM_CR
//#define TERM_CRLF

/*　初期化以外の各関数の戻り値は無視しています。
　必要であればMSDNライブラリで確認して処置してください*/

rscommdata rs;
extern globaldata g;
extern void rs_init_proc(void);
extern void rs_end_proc(void);
extern void rs_rcv_proc(void);

/*=======================================
		受信バイト数のチェック
 =======================================*/
long rs_check(void)
{
	COMSTAT	ComStat;
	DWORD	dwErrorFlags;

	/*エラーがあったらクリア*/
	ClearCommError(rs.hComm,&dwErrorFlags,&ComStat);

	/*受信バイト数を返す*/
	return(ComStat.cbInQue);
}

/*=======================================
		受信データの読み込み
		n1バイトをsに読み込む
 =======================================*/
long rs_read(long n1,unsigned char *s)
{
	unsigned long	n2;	/*実際に読み込んだ数*/
	unsigned long	err=0;

	if (!ReadFile(rs.hComm,s,n1,&n2,&rs.ol)) {
		if (GetLastError() == ERROR_IO_PENDING) {
			GetOverlappedResult(rs.hComm,&rs.ol,&n2,TRUE);
		}
		else {
			printf("%%rs_read error (%d)\n",err);
		}
	}
	return(n2);

}

/*===================================
		データの送信
		n1バイトをsから送信
 ====================================*/
long rs_write(long n1,unsigned char *s)
{
	unsigned long n2;	/*実際に送信した数*/
	unsigned long err;
	DWORD Transfer;

	if (!WriteFile(rs.hComm,s,n1,&n2,&rs.ol)) {
		if ((err=GetLastError()) == ERROR_IO_PENDING) {
			GetOverlappedResult(rs.hComm,&rs.ol,&Transfer,TRUE);
		}
		else {
			printf("%%rs_write error (%d)\n",err);
		}
	}
	return(n2);
}

/*=================================
		RS-232C end
 =================================*/
void rs_end(void)
{
	if (rs.ol.hEvent) CloseHandle(rs.ol.hEvent);
	if (rs.hComm) CloseHandle(rs.hComm);
	rs_end_proc();
}

/*===========================================================
		RS-232C initial
		chan:ﾁｬﾝﾈﾙ (0=COM1,1=COM2,～)
		speed:[b/s] (50 ～ 57600)
		length:charctor length (5=5bit ～ 8=8bit)
		parities:0=non,1=odd,2=even
		stops:stop bits (0,1,2=1,1.5,2)
 ===========================================================*/
long rs_init(char *portName, unsigned long speed,
	unsigned short length, unsigned short parities, unsigned short stops)
{
	char c[10];
	sprintf(c,"\\\\.\\%s",portName);

	/*通信ファイルのオープン*/
	rs.hComm=CreateFile(c,				/*COM1～*/
		GENERIC_READ | GENERIC_WRITE,	/*送受信*/
		0,								/*非共有モード*/
		NULL,							/*非セキュリティーモード*/
		OPEN_EXISTING,					/*オープン*/
//		FILE_ATTRIBUTE_NORMAL,			/*非オーバーラップドモード*/
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL);							/*通信ではNULL*/

	if(rs.hComm==INVALID_HANDLE_VALUE)	{
		printf("%%rs_init CreateFile error\n");
		return(-1);	/*オープン失敗*/
	}

	// 受信イベント生成
	ZeroMemory( &rs.ol, sizeof(rs.ol) );  // オーバラップ構造体クリア
	rs.ol.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (rs.ol.hEvent == NULL){
        printf("%%rs_init create event error\n");
        return 1;
    }

	/*通信マスク設定*/
//	if(!SetCommMask(rs.hComm,0))			/*イベントなし*/
	if(!SetCommMask(rs.hComm,EV_RXCHAR))	/*文字を受信したらイベント発生*/
	{
        printf("%%rs_init SetCommMask error\n");
		rs_end();
		return(-2);						/*通信マスク設定失敗*/
	}

		/*入出力バッファ数を設定*/
    if(!SetupComm(rs.hComm,sizeof(g.rcvbuf),sizeof(g.sndbuf))) /*バッファ数*/
	{
        printf("%%rs_init SetupComm error\n");
		rs_end();
		return(-3);						/*設定失敗*/
	}
	/*タイムアウト設定*/
	/*ReadFileの動作すぐに終了する*/
	rs.CommTimeOuts.ReadIntervalTimeout=0xffffffff;
	rs.CommTimeOuts.ReadTotalTimeoutMultiplier=0;
	rs.CommTimeOuts.ReadTotalTimeoutConstant=0;
	/*１バイト書き込みタイムアウト=20ms*/
	rs.CommTimeOuts.WriteTotalTimeoutMultiplier=20;
	rs.CommTimeOuts.WriteTotalTimeoutConstant=0;
	if(!SetCommTimeouts(rs.hComm,&rs.CommTimeOuts))
	{
		printf("%%rs_init SetupCommTimeouts error\n");
		rs_end();
		return(-4);						/*設定失敗*/
	}

	/*DCB構造体に通信デバイスの設定条件を読み込む*/
	GetCommState(rs.hComm,&rs.siodcb);
	rs.siodcb.DCBlength=sizeof(DCB);
	rs.siodcb.BaudRate=speed;				/*bps*/
	rs.siodcb.fBinary=TRUE;				/*ﾊﾞｲﾅﾘｰﾓｰﾄﾞ,EOFﾁｪｯｸしない*/
	rs.siodcb.ByteSize=(unsigned char)length;	/*Bit*/
	rs.siodcb.Parity=(unsigned char)parities;	/*パリティ*/
	rs.siodcb.StopBits=(unsigned char)stops;	/*Stop Bit (0,1,2=1,1.5,2)*/
	rs.siodcb.fRtsControl=RTS_CONTROL_DISABLE;	/*RTSフロー制御しない*/
	rs.siodcb.fOutxCtsFlow=FALSE;		/*CTSフロー制御なし*/
	rs.siodcb.fInX=rs.siodcb.fOutX=FALSE;	/*XON/XOFFフロー制御なし*/
	rs.siodcb.fParity=TRUE;			/*パリティチェック有効*/
	/*DCB構造体に通信デバイスの設定条件を書き込み*/
	if(!SetCommState(rs.hComm,&rs.siodcb))
	{
        printf("%%rs_init SetupCommState error\n");
		rs_end();
		return(-5);					/*設定失敗*/
	}
	rs_init_proc();
	return(0);
}

/*===================================
		CTS , DSR Status Read
 ====================================*/
void modem_status(short *cts_flg,short *dsr_flg)
{
	unsigned long ModemStatus;
	GetCommModemStatus(rs.hComm,&ModemStatus);
	if(ModemStatus&MS_CTS_ON)	*cts_flg=1;
	else	*cts_flg=0;
	if(ModemStatus&MS_DSR_ON)	*dsr_flg=1;
	else	*dsr_flg=0;
}

/*RTS ON/OFF*/
void rts_onoff(short flg)
{
	if(!flg)
		EscapeCommFunction(rs.hComm,CLRRTS);
	else
		EscapeCommFunction(rs.hComm,SETRTS);
}
/*DTR ON/OFF*/
void dtr_onoff(short flg)
{
	if(!flg)
		EscapeCommFunction(rs.hComm,CLRDTR);
	else
		EscapeCommFunction(rs.hComm,SETDTR);
}
/*Break Char ON/OFF */
void break_onoff(short flg)
{
	if(flg)
		SetCommBreak(rs.hComm);
	else
		ClearCommBreak(rs.hComm);
}

// 受信スレッド
// 基本的にターミネータ（CRorCRLF）のあるASCIIデータを処理する（切替えはソース書換え要）
// ターミネータを受信したら、rs_rcv_procを実行
void rs_thread(void *)
{
	DWORD Event;
	DWORD Transfer;
	DWORD dwSize;
	DWORD dwErr;
	COMSTAT comst;
	int err,i;
	str255 line;

	while(g.rs_mode) {
		// データ受信のイベント待ち
		if (!WaitCommEvent(rs.hComm,&Event,&rs.ol)) {
			if ((err=GetLastError()) == ERROR_IO_PENDING) {
				GetOverlappedResult(rs.hComm,&rs.ol,&Transfer,TRUE);
			}
			else {
				printf("%%rs_thread WaitCommEvent error (%d)\n",err);
				break;
			}
        }
		// g.cmdを送信
		if (strlen(g.cmd)) {
			sprintf(line,"%s\x0D",g.cmd);
			rs_write(strlen(line),(unsigned char*)line);
			SbarPrintf(g.hSbar,2,"%s",g.cmd);
			strcpy(g.cmd,"");
		}

		if (Event&EV_RXCHAR) {	// イベントがEV_RXCHARなら受信
			ClearCommError(rs.hComm,&dwErr,&comst);
			if (!ReadFile(rs.hComm,g.rcvbuf,comst.cbInQue,&dwSize,&rs.ol)) {
				return;
			}
			else {
				g.rcvbuf[dwSize] = '\0';
				if (dwSize>sizeof(g.rcvbuf)-1) {
					printf(">>>>rs_thread rcvbuf over! (%d)\n%s\n>>>>\n",dwSize,g.rcvbuf);
					MessageBox(NULL,"%%rs_thread rcvbuf over!","Error",MB_OK);
					dwSize = sizeof(g.rcvbuf)-1;
				}

				// 受信成功
				for (i=0; i<(int)dwSize ;i++) {
					g.rbuf[g.rbuf_n++] = g.rcvbuf[i];
//					if (g.rbuf_n>2 && g.rbuf[g.rbuf_n-2]==0x0d && g.rbuf[g.rbuf_n-1]==0x0a) {	// CRLF処理
					if (g.rbuf_n>1 && g.rbuf[g.rbuf_n-1]==0x0d) {	// CR処理
						g.rbuf_n -= 1;
						//g.rbuf[g.rbuf_n] = 0x00;

						// 受信処理（ターミネータ受信毎に受信処理を行う）
						rs_rcv_proc();
						g.rbuf_n = 0;
					}
				}
				if (g.rbuf_n>POINT_N*4+2) g.rbuf_n = 0;
			}		
		}
	}

	rs_end();
}

