#include <windows.h>
#include <stdio.h>
#include "RsComm.h"
#include "skel.h"

#define TERM_CR
//#define TERM_CRLF

/*�@�������ȊO�̊e�֐��̖߂�l�͖������Ă��܂��B
�@�K�v�ł����MSDN���C�u�����Ŋm�F���ď��u���Ă�������*/

rscommdata rs;
extern globaldata g;
extern void rs_init_proc(void);
extern void rs_end_proc(void);
extern void rs_rcv_proc(void);

/*=======================================
		��M�o�C�g���̃`�F�b�N
 =======================================*/
long rs_check(void)
{
	COMSTAT	ComStat;
	DWORD	dwErrorFlags;

	/*�G���[����������N���A*/
	ClearCommError(rs.hComm,&dwErrorFlags,&ComStat);

	/*��M�o�C�g����Ԃ�*/
	return(ComStat.cbInQue);
}

/*=======================================
		��M�f�[�^�̓ǂݍ���
		n1�o�C�g��s�ɓǂݍ���
 =======================================*/
long rs_read(long n1,unsigned char *s)
{
	unsigned long	n2;	/*���ۂɓǂݍ��񂾐�*/
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
		�f�[�^�̑��M
		n1�o�C�g��s���瑗�M
 ====================================*/
long rs_write(long n1,unsigned char *s)
{
	unsigned long n2;	/*���ۂɑ��M������*/
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
		chan:����� (0=COM1,1=COM2,�`)
		speed:[b/s] (50 �` 57600)
		length:charctor length (5=5bit �` 8=8bit)
		parities:0=non,1=odd,2=even
		stops:stop bits (0,1,2=1,1.5,2)
 ===========================================================*/
long rs_init(char *portName, unsigned long speed,
	unsigned short length, unsigned short parities, unsigned short stops)
{
	char c[10];
	sprintf(c,"\\\\.\\%s",portName);

	/*�ʐM�t�@�C���̃I�[�v��*/
	rs.hComm=CreateFile(c,				/*COM1�`*/
		GENERIC_READ | GENERIC_WRITE,	/*����M*/
		0,								/*�񋤗L���[�h*/
		NULL,							/*��Z�L�����e�B�[���[�h*/
		OPEN_EXISTING,					/*�I�[�v��*/
//		FILE_ATTRIBUTE_NORMAL,			/*��I�[�o�[���b�v�h���[�h*/
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL);							/*�ʐM�ł�NULL*/

	if(rs.hComm==INVALID_HANDLE_VALUE)	{
		printf("%%rs_init CreateFile error\n");
		return(-1);	/*�I�[�v�����s*/
	}

	// ��M�C�x���g����
	ZeroMemory( &rs.ol, sizeof(rs.ol) );  // �I�[�o���b�v�\���̃N���A
	rs.ol.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (rs.ol.hEvent == NULL){
        printf("%%rs_init create event error\n");
        return 1;
    }

	/*�ʐM�}�X�N�ݒ�*/
//	if(!SetCommMask(rs.hComm,0))			/*�C�x���g�Ȃ�*/
	if(!SetCommMask(rs.hComm,EV_RXCHAR))	/*��������M������C�x���g����*/
	{
        printf("%%rs_init SetCommMask error\n");
		rs_end();
		return(-2);						/*�ʐM�}�X�N�ݒ莸�s*/
	}

		/*���o�̓o�b�t�@����ݒ�*/
    if(!SetupComm(rs.hComm,sizeof(g.rcvbuf),sizeof(g.sndbuf))) /*�o�b�t�@��*/
	{
        printf("%%rs_init SetupComm error\n");
		rs_end();
		return(-3);						/*�ݒ莸�s*/
	}
	/*�^�C���A�E�g�ݒ�*/
	/*ReadFile�̓��삷���ɏI������*/
	rs.CommTimeOuts.ReadIntervalTimeout=0xffffffff;
	rs.CommTimeOuts.ReadTotalTimeoutMultiplier=0;
	rs.CommTimeOuts.ReadTotalTimeoutConstant=0;
	/*�P�o�C�g�������݃^�C���A�E�g=20ms*/
	rs.CommTimeOuts.WriteTotalTimeoutMultiplier=20;
	rs.CommTimeOuts.WriteTotalTimeoutConstant=0;
	if(!SetCommTimeouts(rs.hComm,&rs.CommTimeOuts))
	{
		printf("%%rs_init SetupCommTimeouts error\n");
		rs_end();
		return(-4);						/*�ݒ莸�s*/
	}

	/*DCB�\���̂ɒʐM�f�o�C�X�̐ݒ������ǂݍ���*/
	GetCommState(rs.hComm,&rs.siodcb);
	rs.siodcb.DCBlength=sizeof(DCB);
	rs.siodcb.BaudRate=speed;				/*bps*/
	rs.siodcb.fBinary=TRUE;				/*�޲�ذӰ��,EOF�������Ȃ�*/
	rs.siodcb.ByteSize=(unsigned char)length;	/*Bit*/
	rs.siodcb.Parity=(unsigned char)parities;	/*�p���e�B*/
	rs.siodcb.StopBits=(unsigned char)stops;	/*Stop Bit (0,1,2=1,1.5,2)*/
	rs.siodcb.fRtsControl=RTS_CONTROL_DISABLE;	/*RTS�t���[���䂵�Ȃ�*/
	rs.siodcb.fOutxCtsFlow=FALSE;		/*CTS�t���[����Ȃ�*/
	rs.siodcb.fInX=rs.siodcb.fOutX=FALSE;	/*XON/XOFF�t���[����Ȃ�*/
	rs.siodcb.fParity=TRUE;			/*�p���e�B�`�F�b�N�L��*/
	/*DCB�\���̂ɒʐM�f�o�C�X�̐ݒ��������������*/
	if(!SetCommState(rs.hComm,&rs.siodcb))
	{
        printf("%%rs_init SetupCommState error\n");
		rs_end();
		return(-5);					/*�ݒ莸�s*/
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

// ��M�X���b�h
// ��{�I�Ƀ^�[�~�l�[�^�iCRorCRLF�j�̂���ASCII�f�[�^����������i�ؑւ��̓\�[�X�������v�j
// �^�[�~�l�[�^����M������Ars_rcv_proc�����s
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
		// �f�[�^��M�̃C�x���g�҂�
		if (!WaitCommEvent(rs.hComm,&Event,&rs.ol)) {
			if ((err=GetLastError()) == ERROR_IO_PENDING) {
				GetOverlappedResult(rs.hComm,&rs.ol,&Transfer,TRUE);
			}
			else {
				printf("%%rs_thread WaitCommEvent error (%d)\n",err);
				break;
			}
        }
		// g.cmd�𑗐M
		if (strlen(g.cmd)) {
			sprintf(line,"%s\x0D",g.cmd);
			rs_write(strlen(line),(unsigned char*)line);
			SbarPrintf(g.hSbar,2,"%s",g.cmd);
			strcpy(g.cmd,"");
		}

		if (Event&EV_RXCHAR) {	// �C�x���g��EV_RXCHAR�Ȃ��M
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

				// ��M����
				for (i=0; i<(int)dwSize ;i++) {
					g.rbuf[g.rbuf_n++] = g.rcvbuf[i];
//					if (g.rbuf_n>2 && g.rbuf[g.rbuf_n-2]==0x0d && g.rbuf[g.rbuf_n-1]==0x0a) {	// CRLF����
					if (g.rbuf_n>1 && g.rbuf[g.rbuf_n-1]==0x0d) {	// CR����
						g.rbuf_n -= 1;
						//g.rbuf[g.rbuf_n] = 0x00;

						// ��M�����i�^�[�~�l�[�^��M���Ɏ�M�������s���j
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

