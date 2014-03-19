
typedef struct {
	DCB		siodcb;					/*DCB�\����*/
	COMMTIMEOUTS CommTimeOuts;		/*�ʐM�^�C���A�E�g�\����*/
	HANDLE	hComm;					/*�ʐM�f�o�C�X�̃n���h��*/
	OVERLAPPED ol;					// �񓯊�I/O�p�̃I�[�o�[���b�v�\����
} rscommdata;

long rs_check(void);
long rs_read(long,unsigned char *);
long rs_write(long,unsigned char *);
long rs_init(char*,unsigned long,unsigned short,unsigned short,unsigned short);
void rs_end(void);
void modem_status(short *,short *);
void rts_onoff(short);
void dtr_onoff(short);
void break_onoff(short);

void rs_thread(void *);
