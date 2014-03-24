// skel.h -- プログラムの雛形として作成	100817	by K.I
// serial probe プログラム用に追記      130228  by K.I

#include "resource.h"
#include <time.h>

//      ﾃﾞﾊﾞｯｸﾞ用ﾏｸﾛ定義
#ifndef _H_DEBUG_
#define _H_DEBUG_

#ifdef _DEBUG
    void DebugPrint(const char* str, ...);
    #define TRACE    DebugPrint
#else
    #define TRACE    // _noop
#endif // _DEBUG

#endif // _H_DEBUG_

// 汎用の型定義
typedef char str15[16];
typedef char str31[32];
typedef char str255[256];
typedef char str1023[1024];
typedef char str2047[2048];
typedef char str4095[4096];

#define ID_MYTIMER 777

#define VERSION "SKEL V.100816"

#define POINT_N 15
#define MAGMAX 65000
#define MAGMIN 0
#define COMPMAX 65000

#define SAMPLE_F 5000000

#define TXINTERVAL "2000000,500000,1000000,3000000,4000000,5000000,6000000,8000000,10000000,12000000,14000000,16000000"
#define CENTER_F "31000,15500,1000,5000,10000,20000,30000,30980,30982.5,30985,30987.5,30990,30992.5,30995,30997.5,31000,31002.5,31005,31007.5,31010,31012.5,31015,31017.5,31020,39062.5,40000,40002.5,40005,50000,60000,62000,80000,93000,100000,50,60,12.5"
#define SAMPLE_N "2000000,500000,1000000,3000000,4000000,5000000,6000000,7000000,8000000"
#define BIBUN_N "0,1000000,2000000,3000000,4000000,5000000,6000000,7000000,8000000,0,16777215"
#define DUMMY_F "0,1000,5000,10000,15000,10000,20000,30000,30995,30997.5,31000,31001.25,31002.5,31005,40000,40002.5,40005,50000,50,60"
#define STEP_F "2.5,1,1.25,2,2.5,5,10,20,50,100,200,500,1000,2000,5000,7750,10000,15500,20000,31000,50000,62000,96000"
#define MODE_O "31kHz-11,dummy,square,10-100kHz-19,31kHz-1"
#define IIRPARA "1,2,4,8,16,32,64"
#define ZEROFREQ "0,1000,2000,5000,10000,20000"

// 定数の定義
enum DUT_STATE{DUT_DISCONNECT,DUT_CONNECT,DUT_BUSY};

// グローバル変数用構造体
typedef struct {
	FILE		*fp;
	FILE		*lfp;
	str255		inifile;
	str255		logfile;
	str255		file_name;
	str15		date,time;						// 現在の時刻
	HWND		hMain;							// Mainウィンドウのハンドル
	HINSTANCE	hInst;							// Mainウィンドウのインスタンス
	HMENU		hMenu;							// Menuハンドル
	HWND		hSbar;							// ステータスバーのハンドル
	HWND		hAbout;							// Aboutウィンドウのハンドル
	HWND		hDlg1;							// Mainウィンドウのパネル
	HWND		hDlg2;							// Subウィンドウのパネル
	HWND		hConsole;
	int			dlg1x,dlg1y;					// パネルの大きさ
	int			dlg2x,dlg2y;					// パネルの大きさ
	int			winx, winy;

	str255		dut_port;						// 通信ポート名
	int			dut_speed;						// 通信スピード
	int			dut_state;						// 通信ステータス
	str255		dut_result;						// 通信状態

	int			rs_mode;						// 通信モード（0:Discennect）
	str4095		rcvbuf;							// 受信バッファ
	str1023		sndbuf;							// 送信バッファ
	str4095		rbuf;							// 受信バッファ２
	int			rbuf_n;

	HDC			hpic;	                        // ダブルバッファ
	int			picx,picy;						// バッファの大きさ
	WNDPROC		picproc;						// 元々のpictureコントロールルーチン

	HDC			hpic2;	                        // ダブルバッファ
	int			pic2x,pic2y;					// バッファの大きさ
	WNDPROC		pic2proc;						// 元々のpictureコントロールルーチン

	int			square;							// 矩形近似
	int			iir;
	int			magmax;
	int			compmax;
	short		re[POINT_N],im[POINT_N];
	double		mag[POINT_N],phase[POINT_N];
	str255		cmd;

	int			bibun_n;
	int			sample_n;
	int			txinterval;
	int			iirpara;	// IIRフィルタ係数
	int			zerofreq;	// Dummy Zero Freq.
	double		center_f;
	double		dummy_f;
	double		step_f;
	short		rzero,izero;
	int			repeatzeroF;
	int			dummyzeroF;
	short		dummy_a;
	int			repeatcount;
	clock_t		start;
	int			lastsec;

	str255		line;
	long		lastdata;
	long		tcount;
	long		pcount[8];
	char		time_table[8][256];
	char		last_code[8];
	int			time_pos[8];
	char		tcode_inv;

} globaldata;


// 文字列処理用関数
int num_item(char *lineBuf);
char *get_item(char *retBuf, char *lineBuf, int num);
char *get_item2(char *retBuf, char *lineBuf, int num, char sepa);
int iitem(char *symBuf, int num);
double fitem(char *symBuf, int num);

// Window関連関数
HWND sbar_put(HWND hWnd);
void sbar_size(HWND hsbar, char *size_list, WPARAM wp, LPARAM lp);
void sbar_show(HWND hsbar, char *line, long n);
void DialogPrintf(HWND hDlg, int ctrlID, LPCSTR form, ...);
void SbarPrintf(HWND hsbar, long n, LPCSTR form, ...);
HDC crea_offscreen(HWND hWnd, long bufx, long bufy);
WNDPROC init_pic_subproc(HWND hpict, WNDPROC subproc);
LRESULT CALLBACK picture_subproc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK picture_subproc2(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

void init_combobox(HWND hWnd, int ctrlID, char *list, int index);
void init_checkbox(HWND hWnd, int ctrlID, int check);
int get_combobox(HWND hWnd, int ctrlID, char *list, char *item);
int icombobox(HWND hWnd, int ctrlID, char *list);
double fcombobox(HWND hWnd, int ctrlID, char *list);
int get_checkbox(HWND hWnd, int ctrlID);
int get_menu(HMENU hMenu, int ctrlID);

// 通信用関数
char* com_search(char *comName);
int rs_commu(char *cmd);

// picture表示用関数
void fillrect(HDC hdc, long left, long top, long right, long bottom, COLORREF rgb);

void set_inifile(char *inifile);
void read_inifile(void);
void write_inifile(void);

BOOL openfile_dialog(HWND hWnd, TCHAR *defa_file, TCHAR *title, TCHAR *filter);
BOOL savefile_dialog(HWND hWnd, TCHAR *defa_file, TCHAR *title, TCHAR *defext, TCHAR *filter);
