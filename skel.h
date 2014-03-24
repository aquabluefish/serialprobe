// skel.h -- �v���O�����̐��`�Ƃ��č쐬	100817	by K.I
// serial probe �v���O�����p�ɒǋL      130228  by K.I

#include "resource.h"
#include <time.h>

//      ���ޯ�ޗpϸے�`
#ifndef _H_DEBUG_
#define _H_DEBUG_

#ifdef _DEBUG
    void DebugPrint(const char* str, ...);
    #define TRACE    DebugPrint
#else
    #define TRACE    // _noop
#endif // _DEBUG

#endif // _H_DEBUG_

// �ėp�̌^��`
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

// �萔�̒�`
enum DUT_STATE{DUT_DISCONNECT,DUT_CONNECT,DUT_BUSY};

// �O���[�o���ϐ��p�\����
typedef struct {
	FILE		*fp;
	FILE		*lfp;
	str255		inifile;
	str255		logfile;
	str255		file_name;
	str15		date,time;						// ���݂̎���
	HWND		hMain;							// Main�E�B���h�E�̃n���h��
	HINSTANCE	hInst;							// Main�E�B���h�E�̃C���X�^���X
	HMENU		hMenu;							// Menu�n���h��
	HWND		hSbar;							// �X�e�[�^�X�o�[�̃n���h��
	HWND		hAbout;							// About�E�B���h�E�̃n���h��
	HWND		hDlg1;							// Main�E�B���h�E�̃p�l��
	HWND		hDlg2;							// Sub�E�B���h�E�̃p�l��
	HWND		hConsole;
	int			dlg1x,dlg1y;					// �p�l���̑傫��
	int			dlg2x,dlg2y;					// �p�l���̑傫��
	int			winx, winy;

	str255		dut_port;						// �ʐM�|�[�g��
	int			dut_speed;						// �ʐM�X�s�[�h
	int			dut_state;						// �ʐM�X�e�[�^�X
	str255		dut_result;						// �ʐM���

	int			rs_mode;						// �ʐM���[�h�i0:Discennect�j
	str4095		rcvbuf;							// ��M�o�b�t�@
	str1023		sndbuf;							// ���M�o�b�t�@
	str4095		rbuf;							// ��M�o�b�t�@�Q
	int			rbuf_n;

	HDC			hpic;	                        // �_�u���o�b�t�@
	int			picx,picy;						// �o�b�t�@�̑傫��
	WNDPROC		picproc;						// ���X��picture�R���g���[�����[�`��

	HDC			hpic2;	                        // �_�u���o�b�t�@
	int			pic2x,pic2y;					// �o�b�t�@�̑傫��
	WNDPROC		pic2proc;						// ���X��picture�R���g���[�����[�`��

	int			square;							// ��`�ߎ�
	int			iir;
	int			magmax;
	int			compmax;
	short		re[POINT_N],im[POINT_N];
	double		mag[POINT_N],phase[POINT_N];
	str255		cmd;

	int			bibun_n;
	int			sample_n;
	int			txinterval;
	int			iirpara;	// IIR�t�B���^�W��
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


// �����񏈗��p�֐�
int num_item(char *lineBuf);
char *get_item(char *retBuf, char *lineBuf, int num);
char *get_item2(char *retBuf, char *lineBuf, int num, char sepa);
int iitem(char *symBuf, int num);
double fitem(char *symBuf, int num);

// Window�֘A�֐�
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

// �ʐM�p�֐�
char* com_search(char *comName);
int rs_commu(char *cmd);

// picture�\���p�֐�
void fillrect(HDC hdc, long left, long top, long right, long bottom, COLORREF rgb);

void set_inifile(char *inifile);
void read_inifile(void);
void write_inifile(void);

BOOL openfile_dialog(HWND hWnd, TCHAR *defa_file, TCHAR *title, TCHAR *filter);
BOOL savefile_dialog(HWND hWnd, TCHAR *defa_file, TCHAR *title, TCHAR *defext, TCHAR *filter);
