
typedef struct {
	DCB		siodcb;					/*DCB構造体*/
	COMMTIMEOUTS CommTimeOuts;		/*通信タイムアウト構造体*/
	HANDLE	hComm;					/*通信デバイスのハンドル*/
	OVERLAPPED ol;					// 非同期I/O用のオーバーラップ構造体
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
