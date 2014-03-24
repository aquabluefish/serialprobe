#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "skel.h"
#include <tchar.h>
#include <float.h>
#include <process.h>
#include "RsComm.h"

#include <commctrl.h>
#pragma comment(lib, "ComCtl32.lib")

TCHAR szTitle[100] = _T("SerialProbe");
globaldata g;

//////// シリアル通信用ルーチン記述開始 ////////
//  rs_init_proc,rs_end_proc,rs_rcv_procの処理は、rs_threadから呼出される

// 受信スレッドルーチン（本体は、RsComm.cppに記述されている）
extern void rs_thread(void *);

// シリアル通信スレッドrs_threadを開始する
// メインルーチンと並行して、マルチスレッドでrs_threadが実行される
void rs_cmd_proc(char *cmd) {
	str255 line;

	if (strlen(cmd)) sprintf(line,"%s\x0D",cmd);
	rs_write(strlen(line),(unsigned char*)line);
	g.start = clock();
	_beginthread(rs_thread, 0, NULL);
}

// シリアル通信の初期化処理
void rs_init_proc(void)
{
	SbarPrintf(g.hSbar,0,"*connect\n");
	SbarPrintf(g.hSbar,1,"%s",g.dut_port);
	fillrect(g.hpic,0,0,g.picx,g.picy,RGB(0,0,0));
}

// シリアル通信の終了処理
void rs_end_proc(void)
{
	SbarPrintf(g.hSbar,0,"*disconnect\n");
	SbarPrintf(g.hSbar,1,"");
}

void print_pcount(int i)
{
	switch (i) {
	case 0:	DialogPrintf(g.hDlg2,IDC_COUNT0,"%d",g.pcount[i]); break;
	case 1:	DialogPrintf(g.hDlg2,IDC_COUNT1,"%d",g.pcount[i]); break;
	case 2:	DialogPrintf(g.hDlg2,IDC_COUNT2,"%d",g.pcount[i]); break;
	case 3:	DialogPrintf(g.hDlg2,IDC_COUNT3,"%d",g.pcount[i]); break;
	case 4:	DialogPrintf(g.hDlg2,IDC_COUNT4,"%d",g.pcount[i]); break;
	case 5:	DialogPrintf(g.hDlg2,IDC_COUNT5,"%d",g.pcount[i]); break;
	case 6:	DialogPrintf(g.hDlg2,IDC_COUNT6,"%d",g.pcount[i]); break;
	case 7:	DialogPrintf(g.hDlg2,IDC_COUNT7,"%d",g.pcount[i]); break;
	}
}

char print_tcode(int i)
{
	char code;

	if (g.pcount[i]>=6 && g.pcount[i]<=44)
		code = '0';
	else if (g.pcount[i]>=45 && g.pcount[i]<=83)
		code = '1';
	else if (g.pcount[i]>=84 && g.pcount[i]<=122)
		code = 'M';
	else
		code = ' ';
	
	switch (i) {
	case 0:	DialogPrintf(g.hDlg2,IDC_CODE0,"%c",code); break;
	case 1:	DialogPrintf(g.hDlg2,IDC_CODE1,"%c",code); break;
	case 2:	DialogPrintf(g.hDlg2,IDC_CODE2,"%c",code); break;
	case 3:	DialogPrintf(g.hDlg2,IDC_CODE3,"%c",code); break;
	case 4:	DialogPrintf(g.hDlg2,IDC_CODE4,"%c",code); break;
	case 5:	DialogPrintf(g.hDlg2,IDC_CODE5,"%c",code); break;
	case 6:	DialogPrintf(g.hDlg2,IDC_CODE6,"%c",code); break;
	case 7:	DialogPrintf(g.hDlg2,IDC_CODE7,"%c",code); break;
	}

	return code;
}

void rec_table(char time_code, int n)
{
	int i;

	if (' '==time_code) return;

	if ((g.last_code[n]=='M') && (time_code=='M')) {
		g.time_table[n][g.time_pos[n]] = '\0';
		for (i=g.time_pos[n]; i<256 ;i++) g.time_table[n][i] = '\0';
		g.time_pos[n] = 0;
	}
	g.time_table[n][g.time_pos[n]] = time_code;
	g.time_pos[n]++;


	if (g.time_pos[n]>70) {
		for (i=0; i<256 ;i++) g.time_table[n][i] = '\0';
		g.time_pos[n] = 0;
	}

	g.last_code[n] = time_code;
}

int val(char c)
{
	switch(c) {
		case '0': return 0;
		case '1': return 1;
		default: return 0;
	}
}

int uru(int year)
{
	return !(year%4);
}

int days2date(int *mon, int *day, int days, int year)
{
	const int month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int i;
	
	for (i=0; i<12 ;i++) {
		if (days<=month[i]) {
			*mon = i+1;
			*day = days;
			return 0;
		}
		if (uru(year)&&(i==1))
			days -= 29;
		else
			days -= month[i];
	}
	return 1;
}

void print_table(int n)
{
	str255 line,p;
	char save;
	int min,hou,dat,yea,wea,mon,day;
	const char weak[7][4] = {"SUN","MON","TUE","WED","THU","FRY","SAT"};

	save = g.time_table[n][g.time_pos[n]];
	g.time_table[n][g.time_pos[n]] = '*';
	switch (n) {
	case 0:	DialogPrintf(g.hDlg2,IDC_TABLE0,"%s",g.time_table[n]); break;
	case 1:	DialogPrintf(g.hDlg2,IDC_TABLE1,"%s",g.time_table[n]); break;
	case 2:	DialogPrintf(g.hDlg2,IDC_TABLE2,"%s",g.time_table[n]); break;
	case 3:	DialogPrintf(g.hDlg2,IDC_TABLE3,"%s",g.time_table[n]); break;
	case 4:	DialogPrintf(g.hDlg2,IDC_TABLE4,"%s",g.time_table[n]); break;
	case 5:	DialogPrintf(g.hDlg2,IDC_TABLE5,"%s",g.time_table[n]); break;
	case 6:	DialogPrintf(g.hDlg2,IDC_TABLE6,"%s",g.time_table[n]); break;
	case 7:	DialogPrintf(g.hDlg2,IDC_TABLE7,"%s",g.time_table[n]); break;
	}
	g.time_table[n][g.time_pos[n]] = save;

	strcpy(line,g.time_table[n]);
	get_item2(p,line,2,'M');
	min = val(p[0])*40+val(p[1])*20+val(p[2])*10+val(p[4])*8+val(p[5])*4+val(p[6])*2+val(p[7])*1;
	get_item2(p,line,3,'M');
	hou = val(p[2])*20+val(p[3])*10+val(p[5])*8+val(p[6])*4+val(p[7])*2+val(p[8])*1;
	get_item2(p,line,4,'M');
	dat = val(p[2])*200+val(p[3])*100+val(p[5])*80+val(p[6])*40+val(p[7])*20+val(p[8])*10;
	get_item2(p,line,5,'M');
	dat += val(p[0])*8+val(p[1])*4+val(p[2])*2+val(p[3])*1;
	get_item2(p,line,6,'M');
	yea = val(p[1])*80+val(p[2])*40+val(p[3])*20+val(p[4])*10+val(p[5])*8+val(p[6])*4+val(p[7])*2+val(p[8])*1;
	get_item2(p,line,7,'M');
	wea = val(p[0])*4+val(p[1])*2+val(p[2])*1;
	days2date(&mon,&day,dat,yea);

	switch (n) {
	case 0: DialogPrintf(g.hDlg2,IDC_TIME0,"%04d/%02d/%02d(%s) %02d:%02d",yea+2000,mon,day,weak[wea],hou,min); break;
	case 1: DialogPrintf(g.hDlg2,IDC_TIME1,"%04d/%02d/%02d(%s) %02d:%02d",yea+2000,mon,day,weak[wea],hou,min); break;
	case 2: DialogPrintf(g.hDlg2,IDC_TIME2,"%04d/%02d/%02d(%s) %02d:%02d",yea+2000,mon,day,weak[wea],hou,min); break;
	case 3: DialogPrintf(g.hDlg2,IDC_TIME3,"%04d/%02d/%02d(%s) %02d:%02d",yea+2000,mon,day,weak[wea],hou,min); break;
	case 4: DialogPrintf(g.hDlg2,IDC_TIME4,"%04d/%02d/%02d(%s) %02d:%02d",yea+2000,mon,day,weak[wea],hou,min); break;
	case 5: DialogPrintf(g.hDlg2,IDC_TIME5,"%04d/%02d/%02d(%s) %02d:%02d",yea+2000,mon,day,weak[wea],hou,min); break;
	case 6: DialogPrintf(g.hDlg2,IDC_TIME6,"%04d/%02d/%02d(%s) %02d:%02d",yea+2000,mon,day,weak[wea],hou,min); break;
	case 7: DialogPrintf(g.hDlg2,IDC_TIME7,"%04d/%02d/%02d(%s) %02d:%02d",yea+2000,mon,day,weak[wea],hou,min); break;
	}
}

// シリアル通信受信処理 →ターミネータ（CR）受信する毎に実行される処理
// １秒に128回、つまり約7.8msに１回、16進2byteのデータ＋CRを受信することを想定
//   基本的には、ここで受信したデータの処理を記述する
void rs_rcv_proc(void)
{
	int i;
	long data;
	str255 line;
	int d,ld;
	int xx,yy,zz;
	HPEN hPen,hOldPen;
	clock_t now;
	int nowsec,offset;

	// グラフに1秒毎の縦線を入れるために、時間を取得する
	GetDlgItemText(g.hDlg1,IDC_OFFSET,line,255);
	offset = strtol(line,NULL,10);	// 縦線をいれるタイミングをズラすためのオフセット
	now = clock();
	nowsec = (now-g.start)/1000;
	if (nowsec!=g.lastsec) g.tcount = 0;

	// 受信データをコンソールに表示
	sprintf(line,"%c%c",g.rbuf[0],g.rbuf[1]);
	data = strtol(line,NULL,16);
	printf("%d>>%d>%d\n",now-g.start,g.rbuf_n,data);
	if (g.lfp) {		// logfileに記録
		fprintf(g.lfp,"%02X,%d\n",data,g.tcount);
	}
	xx=g.picx-2;
	yy=10;

	if (g.rbuf_n==2) {
		// グラフ表示（グラフを１ピクセル分左に移動）
		BitBlt(g.hpic,0,0,g.picx-1,g.picy,g.hpic,1,0,SRCCOPY);
		for (i=0; i<8 ;i++) {
			// 1秒毎に縦線を入れる
			if (g.tcount==offset) {
				// 縦線用のPENを準備
				hPen = CreatePen(PS_SOLID, 1, RGB(100,100,100));
				hOldPen = (HPEN)SelectObject(g.hpic, hPen);
				MoveToEx(g.hpic,xx,0,NULL);
				LineTo(g.hpic,xx,g.picy);
				SelectObject(g.hpic, hOldPen);
				DeleteObject(hPen);
			}

			// グラフ用のPENを準備
			hPen = CreatePen(PS_SOLID, 1, RGB(255,0,0));
			hOldPen = (HPEN)SelectObject(g.hpic, hPen);
			d = data & (1<<i);
			ld = g.lastdata & (1<<i);
			if (!g.tcode_inv) {
				if (!ld && d) g.pcount[i] = 0;	// rise
				if (ld && !d) {					// fall
					print_pcount(i);
					rec_table(print_tcode(i),i);
					print_table(i);
				}
				if (d) g.pcount[i]++;
			}
			else {
				if (ld && !d) g.pcount[i] = 0;	// fall
				if (!ld && d) {					// rise
					print_pcount(i);
					rec_table(print_tcode(i),i);
					print_table(i);
				}
				if (!d) g.pcount[i]++;
			}
			// ビット毎に分けて、グラフを描く（ロジアナ風）
			if (ld==d) zz=1; else zz=0;
			if (ld)
				MoveToEx(g.hpic,xx-zz,yy+30*i,NULL);
			else
				MoveToEx(g.hpic,xx-zz,yy+30*i+10,NULL);
			if (d)
				LineTo(g.hpic,xx+zz,yy+30*i);
			else
				LineTo(g.hpic,xx+zz,yy+zz+30*i+10);

			SelectObject(g.hpic, hOldPen);
			DeleteObject(hPen);
			g.lastsec = nowsec;
		}
		// 画面の更新をシステムに知らせる
		InvalidateRect(GetDlgItem(g.hDlg1,IDC_PICTURE), NULL, TRUE);
		UpdateWindow(GetDlgItem(g.hDlg1,IDC_PICTURE));
		g.lastdata = data;
		g.tcount++;
	}
}
//////// シリアル通信用ルーチン終わり（通信しない場合は、ここまでは記述不要）


// Mainウィンドウ上のパネル処理ルーチン
BOOL CALLBACK Dlg1Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	RECT rt;
	str255 line;
	int i;

	switch (msg) {

	case WM_INITDIALOG:	// 初期設定
		// ピクチャコントロールのサブクラス化
		g.picproc = init_pic_subproc(GetDlgItem(hWnd,IDC_PICTURE),picture_subproc);
		// オフスクリーン生成
		GetClientRect(GetDlgItem(hWnd,IDC_PICTURE),&rt);
		g.picx = rt.right-rt.left;
		g.picy = rt.bottom-rt.top;
		g.hpic = crea_offscreen(hWnd,g.picx,g.picy);

		// いろいろ初期設定
		strcpy(g.cmd,"");
//		init_combobox(hWnd,IDC_TXINTERVAL,TXINTERVAL,0);
		SendMessage(GetDlgItem(hWnd, IDC_OFFSET_C), UDM_SETBUDDY, (WPARAM)GetDlgItem(hWnd, IDC_OFFSET), 0);	// 対応するEditBox指定
		SendMessage(GetDlgItem(hWnd, IDC_OFFSET_C), UDM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(150,0));		// 範囲指定（1～8に設定）
		SendMessage(GetDlgItem(hWnd, IDC_OFFSET_C), UDM_SETPOS, 0, (LPARAM)MAKELONG((short)0, 0));		// 初期値の指定（5に設定）
		g.tcode_inv = 0;

		return TRUE;

	case WM_COMMAND:	// コマンド処理（主にボタン等処理）
		switch (LOWORD(wp)) {

		case IDOK:
			// Connect
			g.rs_mode = 1;
			GetDlgItemText(g.hDlg1,IDC_BAUDRATE,line,255);
			g.dut_speed = strtol(line,NULL,10);;
			GetDlgItemText(g.hDlg1,IDC_COMPORT,line,255);
			strcpy(g.dut_port,line);
			if (g.dut_state!=DUT_CONNECT) rs_commu("");
			return TRUE;

		case IDCANCEL:
			// Disconnect
			g.rs_mode = 0;
			g.dut_state = DUT_DISCONNECT;
			return TRUE;

		case IDC_CLEAR:
			// グラフをクリアする
			for (i=0; i<POINT_N ;i++) {
				g.mag[i] = 0;
				g.phase[i] = 0;
			}
			fillrect(g.hpic,0,0,g.picx,g.picy,RGB(0,0,0));
			InvalidateRect(GetDlgItem(g.hDlg1,IDC_PICTURE), NULL, TRUE);
			UpdateWindow(GetDlgItem(g.hDlg1,IDC_PICTURE));
			return TRUE;

		}
        if (HIWORD(wp)==EN_CHANGE) {
			if (lp==(LPARAM)GetDlgItem(hWnd,IDC_MAGMAX)) {
				g.magmax = GetDlgItemInt(hWnd,IDC_MAGMAX,NULL,FALSE);
			}
			if (lp==(LPARAM)GetDlgItem(hWnd,IDC_COMPMAX)) {
				g.compmax = GetDlgItemInt(hWnd,IDC_COMPMAX,NULL,FALSE);
			}
		}
	}
	return (DefWindowProc(hWnd, msg, wp, lp));	//処理しないものはシステムに渡す
}

// Subウィンドウ上のパネル処理ルーチン
BOOL CALLBACK Dlg2Proc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{

	switch (msg) {

        case WM_INITDIALOG:
            return TRUE;

		case WM_CLOSE:
			ShowWindow(g.hDlg2,SW_HIDE);
			CheckMenuItem(g.hMenu,IDM_TIME,MF_UNCHECKED|MF_BYCOMMAND);
            return TRUE;
	}
	return FALSE;
}

// Aboutダイアログ表示
LRESULT CALLBACK AboutDlgProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
        case WM_INITDIALOG:
			g.hAbout = hdlg;
			DialogPrintf(hdlg,IDC_VERSION,VERSION);
            return FALSE;
		case WM_LBUTTONDOWN:
			EndDialog(hdlg,IDOK);
			break;
        default:
            return FALSE;
    }
    return TRUE;
}

// Mainウィンドウ処理ルーチン
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	RECT rt,crt,srt;
	LPMINMAXINFO lpmm;

	switch (msg) { 
    case WM_CREATE:
			// Mainウィンドウの初期化処理
			InitCommonControls();

			// パネルダイアログの生成
            g.hDlg1 = CreateDialog(g.hInst,MAKEINTRESOURCE(IDD_DIALOG1),hWnd,(DLGPROC)Dlg1Proc);
			ShowWindow(g.hDlg1, SW_SHOW);
			UpdateWindow(g.hDlg1);

            g.hDlg2 = CreateDialog(g.hInst,MAKEINTRESOURCE(IDD_DIALOG2),hWnd,(DLGPROC)Dlg2Proc);
			ShowWindow(g.hDlg2, SW_HIDE);
			UpdateWindow(g.hDlg2);

			// パネルの大きさを求める
			GetClientRect(g.hDlg1, &rt);
			g.dlg1x = rt.right - rt.left;
			g.dlg1y = rt.bottom - rt.top;

			// ステータスバーを生成
			g.hSbar = sbar_put(hWnd);


			// タイマをセット（500ms）
			SetTimer(hWnd,ID_MYTIMER,500,NULL);

			set_inifile("serialprobe.ini");
			read_inifile();
			g.lfp = NULL;

			SendMessage(hWnd, WM_SIZE, wp, lp);
			break;

	case WM_SIZE:
			// Main Windowの大きさを求める
			GetWindowRect(hWnd, &rt);
			GetClientRect(hWnd, &crt);
			GetClientRect(g.hSbar, &srt);
			g.winx = (rt.right - rt.left) - (crt.right - crt.left) + g.dlg1x;
			g.winy = (rt.bottom - rt.top) - (crt.bottom - crt.top) + (srt.bottom - srt.top) + g.dlg1y;

			// パネルの大きさに合わせて、Mainウィンドウサイズ（横幅のみ）調整
			MoveWindow(hWnd, 0, 0, g.winx, g.winy, TRUE);

			// ステータスバーの大きさを調整
			sbar_size(g.hSbar,"100,100,80,60",wp,lp);
			break;

	case WM_GETMINMAXINFO:
			// ウィンドウサイズの制限
			lpmm = (LPMINMAXINFO)lp;
			if (lpmm) {
				lpmm->ptMinTrackSize.x = g.winx;
				lpmm->ptMinTrackSize.y = g.winy;
				lpmm->ptMaxTrackSize.x = g.winx;
				lpmm->ptMaxTrackSize.y = g.winy;
			}
			break;

	case WM_TIMER:
			// タイマ処理
			if (wp != ID_MYTIMER)
				return(DefWindowProc(hWnd, msg, wp, lp));

			// 時間の読み込み
			SYSTEMTIME st;
			GetLocalTime(&st);
			wsprintf(g.time, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
			wsprintf(g.date, "%d/%02d/%02d",st.wYear,st.wMonth,st.wDay);

			// ステータスバーへの現在時刻表示
			SbarPrintf(g.hSbar,3,"%s",g.date);
			SbarPrintf(g.hSbar,4,"%s",g.time);

			break;

	case WM_COMMAND:
			// コマンド処理（主に、メニュー処理）
			switch (LOWORD(wp)) {
			case IDM_CONSOLE:
				if (get_menu(g.hMenu,IDM_CONSOLE))
					ShowWindow(g.hConsole, SW_SHOW);
				else
					ShowWindow(g.hConsole, SW_HIDE);
				break;

			case IDM_LOGFILE:
				if (get_menu(g.hMenu,IDM_LOGFILE)) {
					strcpy(g.logfile,"undefine");
					if (savefile_dialog(NULL,g.logfile,"LOGファイル設定","log",
						"LOGファイル(*.log)\0*.log\0AllFile(*.*)\0*.*\0\0")) {
						g.lfp = fopen(g.logfile,"a");
						fprintf(g.lfp,"**** serial probe %s %s ****\n",g.date,g.time);
						break;
					}
				}
				CheckMenuItem(g.hMenu,IDM_LOGFILE,MF_UNCHECKED|MF_BYCOMMAND);
				strcpy(g.logfile,"");
				g.lfp = NULL;
				break;

			case IDM_ABOUT:
					DialogBox(g.hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)AboutDlgProc);
					break;

			case IDM_TIME:
				if (get_menu(g.hMenu,IDM_TIME))
					ShowWindow(g.hDlg2, SW_SHOW);
				else
					ShowWindow(g.hDlg2, SW_HIDE);
					break;

			case IDM_TCODEINV:
				if (get_menu(g.hMenu,IDM_TCODEINV))
					g.tcode_inv = 1;
				else
					g.tcode_inv = 0;
					break;

			case IDM_EXIT:
					PostMessage(hWnd,WM_CLOSE,wp,lp);
					if (g.lfp) fclose(g.lfp);
					break;
			}
			break;

	case WM_DESTROY:
			write_inifile();
			DestroyWindow(g.hDlg1);
			DestroyWindow(g.hDlg2);
			PostQuitMessage(0);
			break;

	default:
			return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}

// メインルーチン
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
        MSG msg;
        HWND hWnd;
        WNDCLASSEX wcex;

        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style              = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc        = WndProc;
        wcex.cbClsExtra         = 0;
        wcex.cbWndExtra         = 0;
        wcex.hInstance          = hInstance;
        wcex.hIcon              = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)); // ICON
        wcex.hCursor            = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground      = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszMenuName       = MAKEINTRESOURCE(IDR_MENU1);                      // MENU
        wcex.lpszClassName      = szTitle;
        wcex.hIconSm            = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(NULL)); // small ICON
        if (!RegisterClassEx(&wcex)) return 0;

        hWnd = CreateWindow(szTitle, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                        CW_USEDEFAULT, 0, 480, 480, NULL, NULL, hInstance, NULL);

        if (!hWnd) {
                return 0;
        }
        ShowWindow(hWnd, nCmdShow);

		g.hMain = hWnd;
		g.hInst = hInstance;
		g.hMenu = GetMenu(hWnd);

		// コンソールの生成
		str1023 title;
		AllocConsole();
		::GetConsoleTitle( title, 1023 ) ;
		g.hConsole = FindWindow(NULL,title);
		ShowWindow(g.hConsole, SW_HIDE);
		freopen("CONOUT$", "w", stdout);
		freopen("CONIN$", "r", stdin);
		HMENU hMenu = ::GetSystemMenu(g.hConsole, 0);
		::RemoveMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);

		// メインループ
        while (GetMessage(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        return 0;
}
