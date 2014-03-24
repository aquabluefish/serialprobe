#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include "RsComm.h"
#include "skel.h"

extern globaldata g;
extern void rs_cmd_proc(char *cmd);

// �����񏈗��֐�

int num_item(char *lineBuf)
{
	int	num=0;
	char	*l;

	if (!*lineBuf) return(num);
	if (!strlen(lineBuf)) return(num);
	for (l=lineBuf ; *l ; l++)
	  if (*l == ',') num++;
	num++;
	return(num);
}

char *get_item(char *retBuf, char *lineBuf, int num)
{
	char	*l,*r;

	num--;
	for (l=lineBuf ; num && *l ; l++)
	  if (*l == ',') num--;

	for (r=retBuf; *l != ',' && *l ; r++,l++)
	  *r = *l;

	*r = '\0';
	return(retBuf);
}

char *get_item2(char *retBuf, char *lineBuf, int num, char sepa)
{
	char	*l,*r;

	num--;
	for (l=lineBuf ; num && *l ; l++)
	  if (*l == sepa) num--;

	for (r=retBuf; *l != sepa && *l ; r++,l++)
	  *r = *l;

	*r = '\0';
	return(retBuf);
}

int iitem(char *symBuf, int num)
{
str255	retBuf;

	get_item(retBuf,symBuf,num);
	return (atoi(retBuf));
}

double fitem(char *symBuf, int num)
{
str255	retBuf;

	get_item(retBuf,symBuf,num);
	return (atof(retBuf));
}

// picture�\���p�֐�

void fillrect(HDC hdc, long left, long top, long right, long bottom, COLORREF rgb)
{
	HPEN	hPen,hOldPen;
	HBRUSH	hBrush,hOldBrush;

	hPen = CreatePen( PS_SOLID, 1, rgb );
	if ( hPen != NULL ) {
	   hOldPen = (HPEN) SelectObject( hdc, hPen );
	   hBrush = CreateSolidBrush( rgb );

	   if ( hBrush != NULL ) {
			hOldBrush = (HBRUSH) SelectObject(hdc, hBrush);
			Rectangle(hdc,left,top,right,bottom);

		   SelectObject( hdc, hOldBrush );
		   DeleteObject( hBrush );
	   }

	   SelectObject( hdc, hOldPen );
	   DeleteObject( hPen );
	}
}

// ---- �X�e�[�^�X�o�[��\��t���āA�n���h����Ԃ�
HWND sbar_put(HWND hWnd)
{
	return(
		CreateStatusWindow(
			WS_CHILD | WS_VISIBLE |
			CCS_BOTTOM | SBARS_SIZEGRIP ,
			"sTaTus" , hWnd , 1
		)
	);
}

// ---- �X�e�[�^�X�o�[�̃T�C�Y����
// -- size_list�ŕ����w��i���[�͎w��s�v�j
// -- Ex. sbar_size(hsbar,"10,20",wp,lp);
// -- ��L�̗�ł́A�S����3���ځA��������
// -- 0���ڂ����w��Ȃ��i�S�̂̕��|�����ڂ̕��j�A1���ڂ���10�A2���ڂ���20
void sbar_size(HWND hsbar, char *size_list, WPARAM wp, LPARAM lp)
{
	// ���ӁI�@���̃��[�`����WM_SIZE����Ă΂Ȃ���΂Ȃ�Ȃ��B
	int sbar_size[10],sbar_n,n;

	sbar_n = num_item(size_list)+1;
	sbar_size[sbar_n-1] = LOWORD(lp);	//�E�[
	for (n=sbar_n-1; n>0; n--)
		sbar_size[n-1] = sbar_size[n]-iitem(size_list,n);
	SendMessage(hsbar, SB_SETPARTS, sbar_n, (LPARAM)sbar_size);
	SendMessage(hsbar, WM_SIZE, wp, lp);
}

// ---- �X�e�[�^�X�o�[��n���ځi���[��0���ځj�ɕ�����iline�j��\��
void sbar_show(HWND hsbar, char *line, long n)
{
		SendMessage(hsbar, SB_SETTEXT, 0 | n, (LPARAM)line);
}

void DialogPrintf(HWND hDlg, int ctrlID, LPCSTR form, ...)
{
    va_list	argp;
    str255 line;
    va_start(argp,form);
    vsprintf(line,form,argp);
    va_end(argp);
	SetDlgItemText(hDlg,ctrlID,line);
}

void SbarPrintf(HWND hsbar, long n, LPCSTR form, ...)
{
    va_list	argp;
    str255 line;
    va_start(argp,form);
    vsprintf(line,form,argp);
    va_end(argp);
	sbar_show(hsbar,line,n);
}

// ---- �I�t�X�N���[���i�_�u���o�b�t�@�j����
HDC crea_offscreen(HWND hWnd, long bufx, long bufy)
{
	HDC		hdc;		//tempolary device context
	HDC		hBuffer;
	HBITMAP	hBitmap;

	// -- �I�t�X�N���[���A�r�b�g�}�b�v�𐶐�
	hdc = GetDC(hWnd);
	hBuffer = CreateCompatibleDC(hdc);
	hBitmap = CreateCompatibleBitmap(hdc, bufx, bufy);

	// -- �I�t�X�N���[���Ƀr�b�g�}�b�v��������
	SelectObject(hBuffer , hBitmap);
	SelectObject(hBuffer , GetStockObject(BLACK_PEN));

	// -- �I�t�X�N���[�����N���A����
	PatBlt(hBuffer,0,0,bufx,bufy,BLACKNESS );
	ReleaseDC(hWnd,hdc);

	return(hBuffer);
}

// �s�N�`���R���g���[���̃T�u�N���X��
// picture�R���g���[���̃n���h���ƁA�T�u�N���X�����[�`���̃|�C���^
WNDPROC init_pic_subproc(HWND hpict, WNDPROC subproc)
{
	WNDPROC orgproc;

    // �s�N�`���R���g���[�����[�`���̃A�h���X���擾
    orgproc = (WNDPROC)GetWindowLong(hpict,GWL_WNDPROC);
    // �s�N�`���R���g���[�����[�`���̃A�h���X���T�u�N���X�����[�`���̃A�h���X�ɕύX
    SetWindowLong(hpict,GWL_WNDPROC,(LONG)subproc);
    return (orgproc);
}

// �s�N�`���R���g���[���̃T�u�N���X�����[�`��
// g.picproc = init_pic_subproc(hpicture,picture_subproc);
// ��L�̂悤�ɁA�I���W�i���̃s�N�`���R���g���[���̑����
// ���s����T�u�N���X�����[�`�����w�肷��B
LRESULT CALLBACK picture_subproc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    PAINTSTRUCT ps;
    HDC hdc;

	switch (msg) {
	// WM_PAINT�C�x���g�Ńs�N�`���R���g���[���̓��e��\��
	// �i�P�ɁA�I�t�X�N���[���̓��e���R�s�[���邾���j
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc,0,0,g.picx,g.picy,g.hpic,0,0,SRCCOPY);
        EndPaint(hWnd, &ps);
		break;

	default:
	    break;
	}
	// �I���W�i���̃s�N�`���R���g���[�������s
    return CallWindowProc((WNDPROC)g.picproc,hWnd,msg,wp,lp);
//    return CallWindowProc(g.picproc,hWnd,msg,wp,lp);
}

// DUT�{�[�h�̐ڑ�����Ă���|�[�g����T��
// VCP�̃|�[�g��T���ă|�[�g����Ԃ��B
char* com_search(char *comName) {
	str255 line;
	HKEY hKey;
	TCHAR szName[256];
	DWORD dwIndex,dwType,dwSize,dwNameSize;
	long result;
	char cbuff[] = "HARDWARE\\DEVICEMAP\\SERIALCOMM";

	// ���W�X�g�����Q�Ƃ��āA�|�[�g���ꗗ�𓾂�
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cbuff, 0, KEY_READ, &hKey);
	if (result == ERROR_SUCCESS){
		for (dwIndex = 0;;dwIndex++) {
			dwNameSize = 256;
			result = RegEnumValue(hKey,dwIndex,szName,&dwNameSize,NULL,&dwType,NULL,NULL);

			if(result == ERROR_NO_MORE_ITEMS) break;
			else if (result != ERROR_SUCCESS) break;

			switch (dwType){
			case REG_SZ:
				result = RegQueryValueEx(hKey, szName, NULL, &dwType, NULL, &dwSize);	//�T�C�Y�̂ݎ擾
				result = RegQueryValueEx(hKey, szName, NULL, &dwType, (LPBYTE)comName, &dwSize);

				get_item2(line,szName,3,'\\');

				SbarPrintf(g.hSbar,1,line);
				if (!strncmp(line,"VCP",3)) return(comName);
				else if (!strncmp(line,"Serial2",7)) return(comName);
				else if (!strncmp(line,"Serial3",7)) return(comName);
				else if (!strncmp(line,"Silabser0",9)) return(comName);

				break;
			default:
				break;
			}
		}
		RegCloseKey(hKey);
	}
	strcpy(g.dut_port,"No COM");
	return 0;
}

// PC��DUT�ʐM
int rs_commu(char *cmd) {

	// DUT���ڑ����ꂽ�|�[�g�����o
	// �ڑ�����Ă��Ȃ��ꍇ�́A�G���[�\�����Ė߂�
//	if (!com_search(g.dut_port)) {
//		SbarPrintf(g.hSbar,0,"%%DUT not connect\n");
//		return 1;
//	}

	if (g.dut_state == DUT_CONNECT) {	// �ڑ����Ȃ�Ώ������Ȃ�
		rs_cmd_proc(cmd);
	}
	else {
		// �V���A���|�[�g�̏�����
		if (!rs_init(g.dut_port,g.dut_speed,8,0,0)) {
			rs_cmd_proc(cmd);
			g.dut_state = DUT_CONNECT;
		}
		else {
			printf("%%DUT busy\n");
			sprintf(g.dut_result,"%%DUT busy");
			SbarPrintf(g.hSbar,0,"%%DUT busy");
			g.dut_state = DUT_BUSY;
		}
	}
	if (!g.rs_mode) {
		rs_end();
		g.dut_state = DUT_DISCONNECT;
	}
	return 1;
}

void init_combobox(HWND hWnd, int ctrlID, char *list, int index)
{
	int i;
	str255 line;

	for (i=0; i<num_item(list) ;i++)
		SendMessage(GetDlgItem(hWnd,ctrlID),CB_INSERTSTRING,i,(LPARAM)get_item(line,list,i+1));
	SendMessage(GetDlgItem(hWnd,ctrlID),CB_SETCURSEL,index,0); // index��I��
}

void init_checkbox(HWND hWnd, int ctrlID, int check)
{
	SendMessage(GetDlgItem(hWnd,ctrlID), BM_SETCHECK, (WPARAM)check, 0L);
}

int get_combobox(HWND hWnd, int ctrlID, char *list, char *item)
{
	int i;

	i = SendMessage(GetDlgItem(hWnd,ctrlID),CB_GETCURSEL,0,0);
	get_item(item,list,i+1);
	return (i+1);
}

int icombobox(HWND hWnd, int ctrlID, char *list)
{
	str255 item;

	get_combobox(hWnd,ctrlID,list,item);
	return (atoi(item));
}

double fcombobox(HWND hWnd, int ctrlID, char *list)
{
	str255 item;

	get_combobox(hWnd,ctrlID,list,item);
	return (atof(item));
}

int get_checkbox(HWND hWnd, int ctrlID)
{
	return (IsDlgButtonChecked(hWnd,ctrlID)==BST_CHECKED);
}

int get_menu(HMENU hMenu, int ctrlID)
{
	if (GetMenuState(hMenu,ctrlID,MF_BYCOMMAND)&MF_CHECKED) {
		CheckMenuItem(hMenu,ctrlID,MF_UNCHECKED|MF_BYCOMMAND);
		return 0;
	}
	else {
		CheckMenuItem(hMenu,ctrlID,MF_CHECKED|MF_BYCOMMAND);
		return 1;
	}
}

void set_inifile(char *inifile)
{
	str255 dir;

	GetCurrentDirectory(255,dir);	// �J�����g�f�B���N�g����INI�t�@�C���p�X���擾
	wsprintf(g.inifile,"%s\\%s",dir,inifile);
}

void read_inifile(void)
{
	str255 para;

	GetPrivateProfileString("Setting","NAME0","",para,255,g.inifile);
	DialogPrintf(g.hDlg1,IDC_NAME0,"%s",para);
	GetPrivateProfileString("Setting","NAME1","",para,255,g.inifile);
	DialogPrintf(g.hDlg1,IDC_NAME1,"%s",para);
	GetPrivateProfileString("Setting","NAME2","",para,255,g.inifile);
	DialogPrintf(g.hDlg1,IDC_NAME2,"%s",para);
	GetPrivateProfileString("Setting","NAME3","",para,255,g.inifile);
	DialogPrintf(g.hDlg1,IDC_NAME3,"%s",para);
	GetPrivateProfileString("Setting","NAME4","",para,255,g.inifile);
	DialogPrintf(g.hDlg1,IDC_NAME4,"%s",para);
	GetPrivateProfileString("Setting","NAME5","",para,255,g.inifile);
	DialogPrintf(g.hDlg1,IDC_NAME5,"%s",para);
	GetPrivateProfileString("Setting","NAME6","",para,255,g.inifile);
	DialogPrintf(g.hDlg1,IDC_NAME6,"%s",para);
	GetPrivateProfileString("Setting","NAME7","",para,255,g.inifile);
	DialogPrintf(g.hDlg1,IDC_NAME7,"%s",para);

	GetPrivateProfileString("Setting","COMPORT","3",para,255,g.inifile);
	DialogPrintf(g.hDlg1,IDC_COMPORT,"%s",para);
	GetPrivateProfileString("Setting","BAUDRATE","9600",para,255,g.inifile);
	DialogPrintf(g.hDlg1,IDC_BAUDRATE,"%s",para);
}

void write_inifile(void)
{
	str255 para;

	GetDlgItemText(g.hDlg1,IDC_NAME0,para,255);
	WritePrivateProfileString("Setting","NAME0",para,g.inifile);
	GetDlgItemText(g.hDlg1,IDC_NAME1,para,255);
	WritePrivateProfileString("Setting","NAME1",para,g.inifile);
	GetDlgItemText(g.hDlg1,IDC_NAME2,para,255);
	WritePrivateProfileString("Setting","NAME2",para,g.inifile);
	GetDlgItemText(g.hDlg1,IDC_NAME3,para,255);
	WritePrivateProfileString("Setting","NAME3",para,g.inifile);
	GetDlgItemText(g.hDlg1,IDC_NAME4,para,255);
	WritePrivateProfileString("Setting","NAME4",para,g.inifile);
	GetDlgItemText(g.hDlg1,IDC_NAME5,para,255);
	WritePrivateProfileString("Setting","NAME5",para,g.inifile);
	GetDlgItemText(g.hDlg1,IDC_NAME6,para,255);
	WritePrivateProfileString("Setting","NAME6",para,g.inifile);
	GetDlgItemText(g.hDlg1,IDC_NAME7,para,255);
	WritePrivateProfileString("Setting","NAME7",para,g.inifile);

	GetDlgItemText(g.hDlg1,IDC_COMPORT,para,255);
	WritePrivateProfileString("Setting","COMPORT",para,g.inifile);
	GetDlgItemText(g.hDlg1,IDC_BAUDRATE,para,255);
	WritePrivateProfileString("Setting","BAUDRATE",para,g.inifile);
}

BOOL openfile_dialog(HWND hWnd, TCHAR *defa_file, TCHAR *title, TCHAR *filter)
{
        OPENFILENAME OpenFileName;

        OpenFileName.lStructSize       = sizeof(OPENFILENAME);
        OpenFileName.hwndOwner         = hWnd;
        OpenFileName.hInstance         = NULL;
        OpenFileName.lpstrFilter       = filter;

        OpenFileName.lpstrCustomFilter = NULL;
        OpenFileName.nMaxCustFilter    = 0;
        OpenFileName.nFilterIndex      = 0;
        OpenFileName.lpstrFile         = defa_file;
        OpenFileName.nMaxFile          = 511;
        OpenFileName.lpstrFileTitle    = NULL;
        OpenFileName.nMaxFileTitle     = 0;
        OpenFileName.lpstrInitialDir   = NULL;
        OpenFileName.lpstrTitle        = title;
        OpenFileName.nFileOffset       = 0;
        OpenFileName.nFileExtension    = 0;
        OpenFileName.lpstrDefExt       = NULL;
        OpenFileName.lCustData         = NULL;
        OpenFileName.Flags             = OFN_EXPLORER |  OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;

        return GetOpenFileName(&OpenFileName);
}


BOOL savefile_dialog(HWND hWnd, TCHAR *defa_file, TCHAR *title, TCHAR *defext, TCHAR *filter)
{
        OPENFILENAME OpenFileName;

        OpenFileName.lStructSize       = sizeof(OPENFILENAME);
        OpenFileName.hwndOwner         = hWnd;
        OpenFileName.hInstance         = NULL;
        OpenFileName.lpstrFilter       = filter;

        OpenFileName.lpstrCustomFilter = NULL;
        OpenFileName.nMaxCustFilter    = 0;
        OpenFileName.nFilterIndex      = 0;
        OpenFileName.lpstrFile         = defa_file;
        OpenFileName.nMaxFile          = 511;
        OpenFileName.lpstrFileTitle    = NULL;
        OpenFileName.nMaxFileTitle     = 0;
        OpenFileName.lpstrInitialDir   = NULL;
        OpenFileName.lpstrTitle        = title;
        OpenFileName.nFileOffset       = 0;
        OpenFileName.nFileExtension    = 0;
        OpenFileName.lpstrDefExt       = defext;  // �g���q���w�肳��Ȃ��ꍇ�̃f�t�H���g�g���q
        OpenFileName.lCustData         = NULL;
        OpenFileName.Flags             = OFN_OVERWRITEPROMPT;  // �㏑���̊m�F������

        return GetSaveFileName(&OpenFileName);
}

#ifdef _DEBUG

#define WIN32_LEAN_AND_MEAN

/************************
        ���ޯ�ޏo��
************************/
void DebugPrint(const char* str, ...) {
    va_list argp;
    char szBuf[256];

    va_start(argp, str);
    vsprintf(szBuf, str, argp);
    va_end(argp);
    OutputDebugString(szBuf);
}

#endif
