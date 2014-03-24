// rclock.cpp: Crclock クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "rclock.h"


//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

Crclock::Crclock()
{

}

Crclock::~Crclock()
{

}

BYTE Crclock::uru(BYTE year)
{
	return !(year%4);
}

int Crclock::days2date(BYTE *mon, BYTE *day, WORD days, BYTE year)
{
	BYTE i;
	
	for (i=0; i<12 ;i++) {
		if (days<=MONTH[i]) {
			*mon = i+1;
			*day = days;
			return 0;
		}
		if (uru(year)&&(i==1))
			days -= 29;
		else
			days -= MONTH[i];
	}
	return 1;
}

WORD Crclock::date2days(BYTE mon, BYTE day, BYTE year)
{
	BYTE i;
	WORD days=0;
	
	for (i=0; i<mon-1 ;i++) {
		if (uru(year)&&(i==1))
			days += 29;
		else
			days += MONTH[i];
	}
	days += day;
	return(days);
}

BYTE Crclock::date2week(WORD year, BYTE mon, BYTE day)
{
    if(mon == 1 || mon == 2 ) {
            year--;
            mon += 12;
    }
    return((year+year/4-year/100+year/400+(13*mon+8)/5+day)%7);
}

BYTE *Crclock::get_charptr(BYTE *lineBuf, BYTE num, BYTE sepa)
{
	BYTE *pos;
	
	num--;
	for (pos=lineBuf ; num && *pos ; pos++)
	  if (*pos == sepa) num--;

	return(pos);
}

BYTE Crclock::val(char c)
{
	switch(c) {
		case '0': return 0;
		case '1': return 1;
		default: return 0;
	}
}

void Crclock::rec_ttable(BYTE tmark)
{
	BYTE i;
	
	if (' '==tmark) return;

	if ((tt.pos>80)||('*'==tmark)) tt.pos = 0;	// clear ttable
	if (func_min_marker(tmark)||!tt.pos) {
		tt.table[tt.pos] = '\0';
		for (i=tt.pos; i<MAX_TTABLE ;i++) tt.table[i] = '\0';
		tt.pos = 0;
		func_decode_time();
		if ('*'==tmark) return;
	}
	tt.table[tt.pos] = tmark;
	tt.pos++;

	tt.tmark = tmark;
	if (func_sync_marker(tmark)) func_decode_time();

	func_show_time();
}

//// JJY time decode ////

void Crclock::jjy_decode(void)
{
	BYTE *p;
	BYTE days;

	p = get_charptr(tt.table,2,'M');
	time.min = val(p[0])*40+val(p[1])*20+val(p[2])*10+val(p[4])*8+val(p[5])*4+val(p[6])*2+val(p[7])*1;
	p = get_charptr(tt.table,3,'M');
	time.hour = val(p[2])*20+val(p[3])*10+val(p[5])*8+val(p[6])*4+val(p[7])*2+val(p[8])*1;
	p = get_charptr(tt.table,4,'M');
	days = val(p[2])*200+val(p[3])*100+val(p[5])*80+val(p[6])*40+val(p[7])*20+val(p[8])*10;
	p = get_charptr(tt.table,5,'M');
	days += val(p[0])*8+val(p[1])*4+val(p[2])*2+val(p[3])*1;
	p = get_charptr(tt.table,6,'M');
	time.year = val(p[1])*80+val(p[2])*40+val(p[3])*20+val(p[4])*10+val(p[5])*8+val(p[6])*4+val(p[7])*2+val(p[8])*1;
	p = get_charptr(tt.table,7,'M');
	time.week = val(p[0])*4+val(p[1])*2+val(p[2])*1;
	days2date(&time.mon,&time.day,days,time.year);
}

BYTE Crclock::jjy_tmark(void)
{
	char tmark;

	if (tt.pcount>=6 && tt.pcount<=44)
		tmark = '0';
	else if (tt.pcount>=45 && tt.pcount<=83)
		tmark = '1';
	else if (tt.pcount>=84 && tt.pcount<=122)
		tmark = 'M';
	else
		tmark = ' ';

	return tmark;
}

BYTE Crclock::jjy_mmarker(BYTE tmark)
{
	return ((tt.tmark=='M') && (tmark=='M'));
}

BYTE Crclock::jjy_sync(BYTE tmark)
{
	return (tmark=='M');
}

void Crclock::jjy_rec(BYTE time_in)
{
	// time code decoder
	if (tt.tcount>127) {
		tt.tcount = 0;
	}
	tt.time_in = tt.time_in<<1;
	if (time_in) tt.time_in |= 0x01;	// TIME_IN
	if ((tt.time_in&0x1F) == 0x0F) {		// rise
		tt.pcount = 1;
	}
	if ((tt.time_in&0x1F) == 0x10) {		// fall
		rec_ttable(func_time_mark());
		tt.pcount = 0;
	}
	if (tt.pcount) tt.pcount++;
	tt.tcount++;
}

//// WWVB time decode ////

void Crclock::wwvb_decode(void)
{
	BYTE *p;
	BYTE days;

	p = get_charptr(tt.table,2,'M');
	time.min = val(p[0])*40+val(p[1])*20+val(p[2])*10+val(p[4])*8+val(p[5])*4+val(p[6])*2+val(p[7])*1;
	p = get_charptr(tt.table,3,'M');
	time.hour = val(p[2])*20+val(p[3])*10+val(p[5])*8+val(p[6])*4+val(p[7])*2+val(p[8])*1;
	p = get_charptr(tt.table,4,'M');
	days = val(p[2])*200+val(p[3])*100+val(p[5])*80+val(p[6])*40+val(p[7])*20+val(p[8])*10;
	p = get_charptr(tt.table,5,'M');
	days += val(p[0])*8+val(p[1])*4+val(p[2])*2+val(p[3])*1;
	p = get_charptr(tt.table,6,'M');
	time.year = val(p[5])*80+val(p[6])*40+val(p[7])*20+val(p[8])*10;
	p = get_charptr(tt.table,7,'M');
	time.year += val(p[0])*8+val(p[1])*4+val(p[2])*2+val(p[3])*1;
	days2date(&time.mon,&time.day,days,time.year);
	time.week = date2week(time.year+2000,time.mon,time.day);
}

BYTE Crclock::wwvb_tmark(void)
{
	BYTE tmark;

	if (tt.pcount>=6 && tt.pcount<=44)
		tmark = 'M';
	else if (tt.pcount>=45 && tt.pcount<=83)
		tmark = '1';
	else if (tt.pcount>=84 && tt.pcount<=122)
		tmark = '0';
	else
		tmark = ' ';

	return tmark;
}

BYTE Crclock::wwvb_mmarker(BYTE tmark)
{
	return ((tt.tmark=='M') && (tmark=='M'));
}

BYTE Crclock::wwvb_sync(BYTE tmark)
{
	return (tmark=='M');
}

void Crclock::wwvb_rec(BYTE time_in)
{
	// time code decoder
	if (tt.tcount>127) {
		tt.tcount = 0;
	}
	tt.time_in = tt.time_in<<1;
	if (time_in) tt.time_in |= 0x01;	// TIME_IN
	if ((tt.time_in&0x1F) == 0x0F) {		// rise
		rec_ttable(func_time_mark());
		tt.pcount = 0;
	}
	if ((tt.time_in&0x1F) == 0x10) {		// fall
		tt.pcount = 1;
	}
	if (tt.pcount) tt.pcount++;
	tt.tcount++;
}

//// DCF77 time decode ////

void Crclock::dcf77_decode(void)
{
	BYTE *p;

	p = tt.table;
	time.min = val(p[21])*1+val(p[22])*2+val(p[23])*4+val(p[24])*8+val(p[25])*10+val(p[26])*20+val(p[27])*40;
	time.hour = val(p[29])*1+val(p[30])*2+val(p[31])*4+val(p[32])*8+val(p[33])*10+val(p[34])*20;
	time.day = val(p[36])*1+val(p[37])*2+val(p[38])*4+val(p[39])*8+val(p[40])*10+val(p[41])*20;
	time.week = val(p[42])*1+val(p[43])*2+val(p[44])*4;
	time.mon = val(p[45])*1+val(p[46])*2+val(p[47])*4+val(p[48])*8+val(p[49])*10;
	time.year = val(p[50])*1+val(p[51])*2+val(p[52])*4+val(p[53])*8+val(p[54])*10+val(p[55])*20+val(p[56])*40+val(p[57])*80;
}

BYTE Crclock::dcf77_tmark(void)
{
	BYTE tmark;

	if (tt.pcount>=214 && tt.pcount<=255)
		tmark = 'M';
	else if (tt.pcount>=86 && tt.pcount<=108)
		tmark = '1';
	else if (tt.pcount>=109 && tt.pcount<=132)
		tmark = '0';
	else
		tmark = ' ';

	return tmark;
}

BYTE Crclock::dcf77_mmarker(BYTE tmark)
{
	return (tt.tmark=='M');
}

BYTE Crclock::dcf77_sync(BYTE tmark)
{
	return (tt.pos == 28 || tt.pos == 35 || tt.pos == 423 || tt.pos == 45 || tt.pos == 50 || tt.pos == 58);
}

void Crclock::dcf77_rec(BYTE time_in)
{
	// time code decoder
	if (tt.tcount>127) {
		tt.tcount = 0;
	}
	tt.time_in = tt.time_in<<1;
	if (time_in) tt.time_in |= 0x01;	// TIME_IN
	if ((tt.time_in&0x1F) == 0x0F) {		// rise
		rec_ttable(func_time_mark());
		tt.pcount = 0;
	}
	if ((tt.time_in&0x1F) == 0x10) {		// fall
		tt.pcount = 1;
	}
	if (tt.pcount) tt.pcount++;
	tt.tcount++;
}

//// MSF time decode ////

void Crclock::msf_decode(void)
{
	BYTE *p;

	p = tt.table;
	time.year = val(p[17])*80+val(p[18])*40+val(p[19])*20+val(p[20])*10+val(p[21])*8+val(p[22])*4+val(p[23])*2+val(p[24])*1;
	time.mon = val(p[25])*10+val(p[26])*8+val(p[27])*4+val(p[28])*2+val(p[29])*1;
	time.day = val(p[30])*20+val(p[31])*10+val(p[32])*8+val(p[33])*4+val(p[34])*2+val(p[35])*1;
	time.week = val(p[36])*4+val(p[37])*2+val(p[38])*1;
	time.hour = val(p[39])*20+val(p[40])*10+val(p[41])*8+val(p[42])*4+val(p[43])*2+val(p[44])*1;
	time.min = val(p[45])*40+val(p[46])*20+val(p[47])*10+val(p[48])*8+val(p[49])*4+val(p[50])*2+val(p[51])*1;
}

BYTE Crclock::msf_tmark(void)
{
	BYTE tmark;

	if (tt.pcount>=45 && tt.pcount<=83)
		tmark = 'M';
	else if (tt.pcount>=86 && tt.pcount<=108)
		tmark = '1';
	else if (tt.pcount>=109 && tt.pcount<=132)
		tmark = '0';
	else
		tmark = ' ';

	return tmark;
}

BYTE Crclock::msf_mmarker(BYTE tmark)
{
	return (tmark=='M');
}

BYTE Crclock::msf_sync(BYTE tmark)
{
	return (tt.pos == 25 || tt.pos == 30 || tt.pos == 36 || tt.pos == 39 || tt.pos == 45 || tt.pos == 52);
}

void Crclock::msf_rec(BYTE time_in)
{
	// time code decoder
	if (tt.tcount>127) {
		tt.tcount = 0;
	}
	tt.time_in = tt.time_in<<1;
	if (time_in) tt.time_in |= 0x01;	// TIME_IN
	if ((tt.time_in&0x1F) == 0x0F) {		// rise
		rec_ttable(func_time_mark());
		tt.pcount = 0;
	}
	if ((tt.time_in&0x1F) == 0x10) {		// fall
		tt.pcount = 1;
	}
	if (tt.pcount) tt.pcount++;
	tt.tcount++;
}

//// select time code

void Crclock::select_tcode(BYTE tcode)
{
	if (tcodex==tcode) return;
	
	switch(tcode) {
		
		case TCODE_MSF: {
			void (Crclock::*func_rec_timecode)(BYTE)	= &Crclock::msf_rec;
			BYTE(Crclock::*func_time_mark)(void)		= &Crclock::msf_tmark;
			void (Crclock::*func_decode_time)(void)		= &Crclock::msf_decode;
			BYTE(Crclock::*func_min_marker)(BYTE)		= &Crclock::msf_mmarker;
			BYTE(Crclock::*func_sync_marker)(BYTE)		= &Crclock::msf_sync;
			break;
		}
				
		case TCODE_DCF77: {
			void (Crclock::*func_rec_timecode)(BYTE) = &Crclock::dcf77_rec;
			BYTE(Crclock::*func_time_mark)(void) = &Crclock::dcf77_tmark;
			void (Crclock::*func_decode_time)(void) = &Crclock::dcf77_decode;
			BYTE(Crclock::*func_min_marker)(BYTE) = &Crclock::dcf77_mmarker;
			BYTE(Crclock::*func_sync_marker)(BYTE) = &Crclock::dcf77_sync;
			break;
		}
			
		case TCODE_WWVB: {
			void (Crclock::*func_rec_timecode)(BYTE) = &Crclock::wwvb_rec;
			BYTE(Crclock::*func_time_mark)(void) = &Crclock::wwvb_tmark;
			void (Crclock::*func_decode_time)(void) = &Crclock::wwvb_decode;
			BYTE(Crclock::*func_min_marker)(BYTE) = &Crclock::wwvb_mmarker;
			BYTE(Crclock::*func_sync_marker)(BYTE) = &Crclock::wwvb_sync;
			break;
		}
				
		case TCODE_JJY:
		default: {
			void (Crclock::*func_rec_timecode)(BYTE) = &Crclock::jjy_rec;
			BYTE(Crclock::*func_time_mark)(void) = &Crclock::jjy_tmark;
			void (Crclock::*func_decode_time)(void) = &Crclock::jjy_decode;
			BYTE(Crclock::*func_min_marker)(BYTE) = &Crclock::jjy_mmarker;
			BYTE(Crclock::*func_sync_marker)(BYTE) = &Crclock::jjy_sync;
			break;
		}
	}
	tcodex = tcode;		// current tcode
	rec_ttable('*');	// clear ttable
}

