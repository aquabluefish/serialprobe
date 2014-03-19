// rclock.h: Crclock クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>

#if !defined(AFX_RCLOCK_H__B5084A0C_9604_4842_B3E2_494A5A8E5E9C__INCLUDED_)
#define AFX_RCLOCK_H__B5084A0C_9604_4842_B3E2_494A5A8E5E9C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class Crclock  
{
	enum TCODE {TCODE_JJY,TCODE_WWVB,TCODE_MSF,TCODE_DCF77,TCODE_BPC,TCODE_NONE};

	#define MAX_TTABLE 128	
	typedef struct {
		BYTE		tmark;
		BYTE		tcount;
		BYTE		pcount;
		BYTE		table[MAX_TTABLE];
		BYTE		pos;
		BYTE		time_in;
	} time_table;	

	typedef struct {
		BYTE	year;
		BYTE	mon;
		BYTE	day;
		BYTE	hour;
		BYTE	min;
		BYTE	week;	
	} time_data;

public:
	Crclock();
	virtual ~Crclock();

	void select_tcode(BYTE tcode);
	void (*func_rec_timecode)(BYTE time_in);	// record timecode function
	void (*func_show_time)(void);			// show time routine (user define)

	static const BYTE MONTH[12];
	static const char TCODE_SELECT[6][6];
	static const char WEEK[7][4];

private:
	// function pointer
	BYTE (*func_time_mark)(void);			// detect timemark function
	void (*func_decode_time)(void);			// decode time function
	BYTE (*func_min_marker)(BYTE tmark);		// judge minute marker function
	BYTE (*func_sync_marker)(BYTE tmark);	// judge sync marker function

	BYTE uru(BYTE year);
	int days2date(BYTE *mon, BYTE *day, WORD days, BYTE year);
	WORD date2days(BYTE mon, BYTE day, BYTE year);
	BYTE date2week(WORD year, BYTE mon, BYTE day);
	BYTE *get_charptr(BYTE *lineBuf, BYTE num, BYTE sepa);
	BYTE val(char c);
	void rec_ttable(BYTE tmark);
	void jjy_decode(void);
	BYTE jjy_tmark(void);
	BYTE jjy_mmarker(BYTE tmark);
	BYTE jjy_sync(BYTE tmark);
	void jjy_rec(BYTE time_in);
	void wwvb_decode(void);
	BYTE wwvb_tmark(void);
	BYTE wwvb_mmarker(BYTE tmark);
	BYTE wwvb_sync(BYTE tmark);
	void wwvb_rec(BYTE time_in);
	void dcf77_decode(void);
	BYTE dcf77_tmark(void);
	BYTE dcf77_mmarker(BYTE tmark);
	BYTE dcf77_sync(BYTE tmark);
	void dcf77_rec(BYTE time_in);
	void msf_decode(void);
	BYTE msf_tmark(void);
	BYTE msf_mmarker(BYTE tmark);
	BYTE msf_sync(BYTE tmark);
	void msf_rec(BYTE time_in);

	time_table tt;
	time_data time;
	BYTE tcode;
	BYTE tcodex;	// current tcode


};

const BYTE Crclock::MONTH[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
const char Crclock::TCODE_SELECT[6][6] = {"JJY","WWVB","MSF","DCF77","BPC","NONE"};
const char Crclock::WEEK[7][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
		

#endif // !defined(AFX_RCLOCK_H__B5084A0C_9604_4842_B3E2_494A5A8E5E9C__INCLUDED_)
