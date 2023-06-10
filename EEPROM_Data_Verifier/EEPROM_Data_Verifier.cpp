#include "EEPROM_Data_Verifier.h"
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <direct.h>
#include <io.h>
#include <afx.h>

using namespace std;

char D[524288][2] = { 0};
bool useData[524288], spec_Bin_Ready=false;
unsigned char DecData[524288], dflt_Data,spec_Data[524288];
int LSC[25][33][4], MTK_LSC[15][15][4], LSC_LSI1[25][33][4], LSC_LSI2[11][13][4];
int PDgainLeft[30][30], PDgainRight[30][30],shift_Data[2][21], Cross_DW_before[2][21], Cross_DW_after[2][14];
int GainMostBrightLeft[15][17], GainMostBrightRight[15][17];
int DCC[15][17], DCC2[15][17], checkSum_addr[30][4],SR_Spec[2][2], Gmap_Item3[12], PD_Item3[14];
int AF_Data[6][3],SFR_Data[50],LCC_CrossTalk[3],LSC_Item3[10],Gyro_offset_spec[2];
char chk[2], Fuse_ID[30];  int fuse_ID_Length = 16;
string global_STR, shift_Item[4], cross_Item[3], akm_cross_Item[3], AF_Item[6][5],AF_info_Item[10][4],SFR_Item[4][4],ZOOM_Item[3][3],Magnification[9];
string PDAF_info_Item[18][3],Gmap_Item[12][3],PD_Item[18][3], OIS_info_Item[30][4],OIS_data_Item[8][3], AA_Item[18],sData_Item[20][6];
string Fix_Data_Item[10][3];
int HL = 0, checkDivisor = 0, customer_Data_END = 0, customer_end = 0, first_Pixel = 0, mode = 0, OK=0, NG=0;
float QSC_Data[4][4][12][16] = { 0 };
int selection = 0,reserved_check = 1,duplicate_value_NG_ratio=50, SFR_Format=0;
extern void EEPMap_Out();
extern void set_Map_inf(string info, string ref, int addr, data_Type t);
int DataFormat = 0;
int EEP_Size = 16384;

typedef struct {
	string hex = "0";
	int dec = 0;
}Panel_Data;
Panel_Data Flag[2], noData;

typedef struct {
	string item_name = "";
	int hash[1010] = {0};
}Value_Hs;
Value_Hs value_Hash[80];

typedef struct {
	string info = "", ref=" ";
	int addr = 0;
	data_Type t;
}Map_Data;

vector<Map_Data> Map;

int modelSelect = 1;
//string EEPROM_Map = ".\\Setting\\EEPROM_Map_V1_797.ini";
string EEPROM_Map;

string src;
//ofstream fout(".\\MemoryParseData.txt");
ofstream  fout;
//ofstream dump_result(".\\dump_result.txt");
ofstream dump_result;

typedef struct {
	string item[5] = { "","","","","" };
	int flag = 0, start = 0, end = 0, checksum = 0, Except_S=0, Except_E=0;
}checkSum_Format;
checkSum_Format checkSum[40];

typedef struct {
	string item[3] = { "","","" };
	unsigned int addr = 0,end=0, spec;
}info_Format;
info_Format infoData[40],QR_Data,Fuse_Data,Fuse_Data2,QC_LSC_Data[8], MTK_LSC_Data[2],Seg_Data[20],Same_Item[6];

typedef struct {
	string item[2] = { "","" };
}dual_info_Format;
dual_info_Format AEC_Data[12], History_Data[12], QSC_Item;

typedef struct {
	string item[10] = { "","","","","","","","","",""};
	int year = 0, month = 0, Day = 0;
}date_Format;
date_Format product_Date;

typedef struct {
	string name = "";
	unsigned int code = 0, spec_Min = 0, spec_Max = 1023;
	bool HL = true;
}afCode_Format;
afCode_Format afCode[10];


typedef struct {
	string fuse_ID = "";
	unsigned int MTK_LSC = 0;
	unsigned int Same[6] = { 0 };
}Hash_Number;
Hash_Number current_Hash;
vector<Hash_Number> dump_Hash;

oppo_AWB_Format OPPO_AWB[3],MTK_AWB[3],LSI_AWB[3];
vivo_AWB_Format VIVO_AWB_Data[2], XIAOMI_AWB_Data;

int checkFF(int x) {
	if (x == 255)
		return 0;
	return x;
}

void map_Push(unsigned int addr, string s, string ref, data_Type t) {
	if (addr > 0||(addr == 0&&s.length()>1)){
		Map_Data temp;
		temp.addr = addr;
		temp.info = s;
		temp.ref = ref;
		temp.t = t;
		useData[addr] = 1;
		Map.push_back(temp);
	}
}

unsigned long long unstringHex2int(string str) {
	unsigned int s = 0;
	for (int i = 0; i < str.length(); i++) {
		s = s * 16;
		if (str[i] >= 'a'&&str[i]<='f')
			s += str[i] - 'a' + 10;
		else if (str[i] >= 'A'&&str[i] <= 'F')
			s += str[i] - 'A' + 10;
		else if (str[i] >= '0'&&str[i] <= '9')
			s += str[i] - '0';
		else if (str[i] == 'x' || str[i] == 'X')
			s = 0;
		else s /= 16;
	}
	return s;
}


int marking_Hex2int(string str , string item, string ref, data_Type t) {
	unsigned int s = unstringHex2int(str);
	map_Push(s,item, ref, t);
	useData[s] = 1;
	return s;
}

int hex2Dec(int x) {
	int s = 0;
	for (int i = 0; i < 2; i++) {
		s = s * 16;
		if (D[x][i] >= 'a')
			s += D[x][i] - 'a' + 10;
		else if (D[x][i] >= 'A')
			s += D[x][i] - 'A' + 10;
		else
			s += D[x][i] - '0';
	}
	return s;
}

string GetASCII(int x, int y) {
	string s = "";
	for (int i = 0; i < y; i++) {
		char a = hex2Dec(x + i);
		s += a;
	}
	return s;
}

void getHex(unsigned int tmp) {
	tmp = tmp % 256;
	int a = tmp / 16;
	int b = tmp % 16;
	if (a < 10)
		chk[0] = '0' + a;
	else
		chk[0] = 'A' + a - 10;

	if (b < 10)
		chk[1] = '0' + b;
	else
		chk[1] = 'A' + b - 10;
}


unsigned char getHexValue(char x) {
	unsigned char s = 0;
	if (x >= 'a')
		s = x - 'a' + 10;
	else if (x >= 'A')
		s = x - 'A' + 10;
	else
		s = x - '0';
	return s;
}


unsigned char getUchar() {
	if (src.length() < 2)
		return 0;
	unsigned char s = getHexValue(src[0]) * 16 + getHexValue(src[1]);
	return s;
}


QString address2hex(int x) {

	QString s = "0x";
	char c[4] = { 0 };
	int a = x;
	for (int i = 0; i < 4; i++) {
		a = x % 16;
		if (a > 9)
			c[3 - i] = 'A' + a - 10;
		else
			c[3 - i] = '0' + a;
		x /= 16;
	}

	for (int i = 0; i < 4; i++)
		s.append(c[i]);

	return s;
}


LPTSTR lptstr2int(int v) {
	CString ctmp;
	ctmp.Format(_T("%d"), v);
	return ctmp.AllocSysString();
}

int lptstrHex2int(LPTSTR v) {

	int sum = 0;
	for (int i = 0; i < lstrlen(v); i++) {
		sum = sum * 16 + (v[i] - '0');
	}
	return sum;
}

float lptstr2flt(LPTSTR v) {

	float sum = 0;
	for (int i = 0; i < lstrlen(v); i++) {
		sum = sum * 16 + (v[i] - '0');
	}
	return sum;
}

#define SEC_CRC_ID 0xEDB88320
unsigned long Get_SEC_CRC32(unsigned char* ucBuffer, unsigned int unStartAddress, unsigned long ulSizeInBytes)
{
	unsigned char* Buffer = (unsigned char*)(ucBuffer + unStartAddress);
	unsigned long Length = ulSizeInBytes;

	unsigned long CRC = 0;
	unsigned long table[256];
	unsigned long i, j, k;
	unsigned long id = SEC_CRC_ID;

	//Make Table
	for (i = 0; i < 256; ++i) {
		k = i;
		for (j = 0; j < 8; ++j) {
			if (k & 1) k = (k >> 1) ^ id;
			else k >>= 1;
		}
		table[i] = k;
	}

	//Calculate CRC
	CRC = ~CRC;
	while (Length--)
		CRC = table[(CRC ^ *(Buffer++)) & 0xFF] ^ (CRC >> 8);

	CRC = ~CRC;
	return CRC;
}

void save_EEPROM_Setting() {
	WritePrivateProfileString(TEXT("Default_Setting"), TEXT("Model_Select"), lptstr2int(modelSelect), TEXT(".\\Setting\\EEPROM_Tool_Setting.ini"));
}


void EEPROM_Data_Verifier::parameterDisplay() {

	if ((selection & 1) >0) {
		ui.info_date->setChecked(true);
	}
	else {
		ui.info_date->setChecked(false);
	}
	if ((selection & 2) >0) {
		ui.info_QR->setChecked(true);
	}
	else {
		ui.info_QR->setChecked(false);
	}
	if ((selection & 4) >0) {
		ui.info_fuse->setChecked(true);
	}
	else {
		ui.info_fuse->setChecked(false);
	}
	if ((selection & 8) >0) {
		ui.gyro_dec->setChecked(true);
	}
	else {
		ui.gyro_float->setChecked(true);
	}
	if ((selection & 16) >0) {
		ui.sr_hl->setChecked(true);
	}
	else {
		ui.sr_hex->setChecked(true);
	}

	if ((selection & 32) >0) {
		ui.shift_HL->setChecked(true);
	}
	else {
		ui.shift_LH->setChecked(true);
	}

	//if ((selection & 64) >0) {
	//	ui.SFR_HEX->setChecked(true);
	//}
	//else {
	//	ui.SFR_DEC->setChecked(true);
	//}
	if ((selection & 128) >0) {
		ui.checkBox_ZOOM->setChecked(true);
	}
	else {
		ui.checkBox_ZOOM->setChecked(false);
	}
	if ((selection & 256) >0) {
		ui.gyro_int->setChecked(true);
	}
	else {
		
	}
	if ((selection & 512) >0) {
		ui.QC->setChecked(true);
	}
	else {
		ui.QC->setChecked(false);
	}
	if ((selection & 1024) >0) {
		ui.MTK->setChecked(true);
	}
	else {
		ui.MTK->setChecked(false);
	}
	if ((selection & 2048) >0) {
		ui.LSI->setChecked(true);
	}
	else {
		ui.LSI->setChecked(false);
	}
	if ((selection & 4096) >0) {
		ui.QSC3->setChecked(true);
	}
	else {
		ui.QSC4->setChecked(true);
	}
	if ((selection & 8192) >0) {
		ui.info_fuse_2->setChecked(true);
	}
	else {
		ui.info_fuse_2->setChecked(false);
	}
	if ((selection & 0x4000) >0) {
		ui.sr100->setChecked(true);
	}
	else {	
	}
	if ((selection & 0x8000) >0) {
		ui.PD_Offset_LH->setChecked(true);
	}
	else {
		ui.PD_Offset_HL->setChecked(true);
	}
	if ((selection & 0x10000) >0) {
		ui.PD_offet_new->setChecked(true);
	}
	else {
		ui.PD_offet_new->setChecked(false);
	}

	//////////////////////////////////first_Pixel
	if (first_Pixel == 0) {
		ui.R->setChecked(true);
	}
	else if (first_Pixel == 1) {
		ui.Gr->setChecked(true);
	}
	else if (first_Pixel == 2) {
		ui.Gb->setChecked(true);
	}
	else if (first_Pixel == 3) {
		ui.B->setChecked(true);
	}

	//////////////////////////////////DataFormat
	if ((DataFormat %10) == 1) {
		ui.oppo->setChecked(true);
	}
	else if ((DataFormat % 10) == 2) {
		ui.vivo->setChecked(true);
	}
	else if ((DataFormat % 10) == 3) {
		ui.xiaomi->setChecked(true);
	}
	else if ((DataFormat % 10) == 4) {
		ui.sony->setChecked(true);
	}
	else if ((DataFormat % 10) == 5) {
		ui.SAMSUNG->setChecked(true);
	}
	else if ((DataFormat % 10) == 6) {
		ui.CS_other->setChecked(true);
	}

	ui.SFR_Format->setCurrentIndex(SFR_Format);

	////////////////////fix_data_check
	ui.fix11->setText(Fix_Data_Item[0][0].c_str());
	ui.fix12->setText(Fix_Data_Item[0][1].c_str());
	ui.fix13->setText(Fix_Data_Item[0][2].c_str());

	ui.fix21->setText(Fix_Data_Item[1][0].c_str());
	ui.fix22->setText(Fix_Data_Item[1][1].c_str());
	ui.fix23->setText(Fix_Data_Item[1][2].c_str());

	ui.fix31->setText(Fix_Data_Item[2][0].c_str());
	ui.fix32->setText(Fix_Data_Item[2][1].c_str());
	ui.fix33->setText(Fix_Data_Item[2][2].c_str());

	ui.fix41->setText(Fix_Data_Item[3][0].c_str());
	ui.fix42->setText(Fix_Data_Item[3][1].c_str());
	ui.fix43->setText(Fix_Data_Item[3][2].c_str());

	ui.fix51->setText(Fix_Data_Item[4][0].c_str());
	ui.fix52->setText(Fix_Data_Item[4][1].c_str());
	ui.fix53->setText(Fix_Data_Item[4][2].c_str());

	ui.fix61->setText(Fix_Data_Item[5][0].c_str());
	ui.fix62->setText(Fix_Data_Item[5][1].c_str());
	ui.fix63->setText(Fix_Data_Item[5][2].c_str());

	ui.fix71->setText(Fix_Data_Item[6][0].c_str());
	ui.fix72->setText(Fix_Data_Item[6][1].c_str());
	ui.fix73->setText(Fix_Data_Item[6][2].c_str());

	ui.fix81->setText(Fix_Data_Item[7][0].c_str());
	ui.fix82->setText(Fix_Data_Item[7][1].c_str());
	ui.fix83->setText(Fix_Data_Item[7][2].c_str());

	ui.fix91->setText(Fix_Data_Item[8][0].c_str());
	ui.fix92->setText(Fix_Data_Item[8][1].c_str());
	ui.fix93->setText(Fix_Data_Item[8][2].c_str());

	ui.fix101->setText(Fix_Data_Item[9][0].c_str());
	ui.fix102->setText(Fix_Data_Item[9][1].c_str());
	ui.fix103->setText(Fix_Data_Item[9][2].c_str());

	////////////////////Zoom AF Formula
	ui.ZOOM11->setText(ZOOM_Item[0][0].c_str());
	ui.ZOOM12->setText(ZOOM_Item[0][1].c_str());
	ui.ZOOM13->setText(ZOOM_Item[0][2].c_str());

	ui.ZOOM21->setText(ZOOM_Item[1][0].c_str());
	ui.ZOOM22->setText(ZOOM_Item[1][1].c_str());
	ui.ZOOM23->setText(ZOOM_Item[1][2].c_str());

	ui.ZOOM31->setText(ZOOM_Item[2][0].c_str());
	ui.ZOOM32->setText(ZOOM_Item[2][1].c_str());
	ui.ZOOM33->setText(ZOOM_Item[2][2].c_str());

	ui.Magnification1->setText(Magnification[0].c_str());
	ui.Magnification2->setText(Magnification[1].c_str());
	ui.Magnification3->setText(Magnification[2].c_str());
	ui.Magnification4->setText(Magnification[3].c_str());
	ui.Magnification5->setText(Magnification[4].c_str());
	ui.Magnification6->setText(Magnification[5].c_str());
	ui.Magnification7->setText(Magnification[6].c_str());
	ui.Magnification8->setText(Magnification[7].c_str());

	////////////////////history_Date
	ui.history11->setText(History_Data[0].item[0].c_str());
	ui.history12->setText(History_Data[0].item[1].c_str());

	ui.history21->setText(History_Data[1].item[0].c_str());
	ui.history22->setText(History_Data[1].item[1].c_str());

	ui.history31->setText(History_Data[2].item[0].c_str());
	ui.history32->setText(History_Data[2].item[1].c_str());

	ui.history41->setText(History_Data[3].item[0].c_str());
	ui.history42->setText(History_Data[3].item[1].c_str());

	ui.history51->setText(History_Data[4].item[0].c_str());
	ui.history52->setText(History_Data[4].item[1].c_str());

	ui.history51->setText(History_Data[4].item[0].c_str());
	ui.history52->setText(History_Data[4].item[1].c_str());

	ui.history61->setText(History_Data[5].item[0].c_str());
	ui.history62->setText(History_Data[5].item[1].c_str());

	ui.history71->setText(History_Data[6].item[0].c_str());
	ui.history72->setText(History_Data[6].item[1].c_str());

	ui.history81->setText(History_Data[7].item[0].c_str());
	ui.history82->setText(History_Data[7].item[1].c_str());

	ui.history91->setText(History_Data[8].item[0].c_str());
	ui.history92->setText(History_Data[8].item[1].c_str());

	ui.history101->setText(History_Data[9].item[0].c_str());
	ui.history102->setText(History_Data[9].item[1].c_str());

	///////////////////AEC_data
	ui.AEC11->setText(AEC_Data[0].item[0].c_str());
	ui.AEC12->setText(AEC_Data[0].item[1].c_str());

	ui.AEC21->setText(AEC_Data[1].item[0].c_str());
	ui.AEC22->setText(AEC_Data[1].item[1].c_str());

	ui.AEC31->setText(AEC_Data[2].item[0].c_str());
	ui.AEC32->setText(AEC_Data[2].item[1].c_str());
	
	ui.AEC41->setText(AEC_Data[3].item[0].c_str());
	ui.AEC42->setText(AEC_Data[3].item[1].c_str());

	ui.AEC51->setText(AEC_Data[4].item[0].c_str());
	ui.AEC52->setText(AEC_Data[4].item[1].c_str());

	ui.AEC61->setText(AEC_Data[5].item[0].c_str());
	ui.AEC62->setText(AEC_Data[5].item[1].c_str());

	ui.AEC71->setText(AEC_Data[6].item[0].c_str());
	ui.AEC72->setText(AEC_Data[6].item[1].c_str());

	ui.AEC81->setText(AEC_Data[7].item[0].c_str());
	ui.AEC82->setText(AEC_Data[7].item[1].c_str());

	ui.AEC91->setText(AEC_Data[8].item[0].c_str());
	ui.AEC92->setText(AEC_Data[8].item[1].c_str());

	ui.AEC101->setText(AEC_Data[9].item[0].c_str());
	ui.AEC102->setText(AEC_Data[9].item[1].c_str());

	ui.AEC111->setText(AEC_Data[10].item[0].c_str());
	ui.AEC112->setText(AEC_Data[10].item[1].c_str());

	ui.AEC121->setText(AEC_Data[11].item[0].c_str());
	ui.AEC122->setText(AEC_Data[11].item[1].c_str());

	//////////////////////QSC

	ui.QSC11->setText(QSC_Item.item[0].c_str());
	ui.QSC12->setText(QSC_Item.item[1].c_str());

	///////////////////Seg_data
	ui.seg11->setText(Seg_Data[0].item[0].c_str());
	ui.seg12->setText(Seg_Data[0].item[1].c_str());
	ui.seg13->setText(Seg_Data[0].item[2].c_str());

	ui.seg21->setText(Seg_Data[1].item[0].c_str());
	ui.seg22->setText(Seg_Data[1].item[1].c_str());
	ui.seg23->setText(Seg_Data[1].item[2].c_str());

	ui.seg31->setText(Seg_Data[2].item[0].c_str());
	ui.seg32->setText(Seg_Data[2].item[1].c_str());
	ui.seg33->setText(Seg_Data[2].item[2].c_str());

	ui.seg41->setText(Seg_Data[3].item[0].c_str());
	ui.seg42->setText(Seg_Data[3].item[1].c_str());
	ui.seg43->setText(Seg_Data[3].item[2].c_str());

	ui.seg51->setText(Seg_Data[4].item[0].c_str());
	ui.seg52->setText(Seg_Data[4].item[1].c_str());
	ui.seg53->setText(Seg_Data[4].item[2].c_str());

	ui.seg61->setText(Seg_Data[5].item[0].c_str());
	ui.seg62->setText(Seg_Data[5].item[1].c_str());
	ui.seg63->setText(Seg_Data[5].item[2].c_str());

	ui.seg71->setText(Seg_Data[6].item[0].c_str());
	ui.seg72->setText(Seg_Data[6].item[1].c_str());
	ui.seg73->setText(Seg_Data[6].item[2].c_str());

	ui.seg81->setText(Seg_Data[7].item[0].c_str());
	ui.seg82->setText(Seg_Data[7].item[1].c_str());
	ui.seg83->setText(Seg_Data[7].item[2].c_str());

	ui.seg91->setText(Seg_Data[8].item[0].c_str());
	ui.seg92->setText(Seg_Data[8].item[1].c_str());
	ui.seg93->setText(Seg_Data[8].item[2].c_str());

	ui.seg101->setText(Seg_Data[9].item[0].c_str());
	ui.seg102->setText(Seg_Data[9].item[1].c_str());
	ui.seg103->setText(Seg_Data[9].item[2].c_str());

	ui.seg111->setText(Seg_Data[10].item[0].c_str());
	ui.seg112->setText(Seg_Data[10].item[1].c_str());
	ui.seg113->setText(Seg_Data[10].item[2].c_str());

	ui.seg121->setText(Seg_Data[11].item[0].c_str());
	ui.seg122->setText(Seg_Data[11].item[1].c_str());
	ui.seg123->setText(Seg_Data[11].item[2].c_str());

	ui.seg131->setText(Seg_Data[12].item[0].c_str());
	ui.seg132->setText(Seg_Data[12].item[1].c_str());
	ui.seg133->setText(Seg_Data[12].item[2].c_str());

	ui.seg141->setText(Seg_Data[13].item[0].c_str());
	ui.seg142->setText(Seg_Data[13].item[1].c_str());
	ui.seg143->setText(Seg_Data[13].item[2].c_str());

	ui.seg151->setText(Seg_Data[14].item[0].c_str());
	ui.seg152->setText(Seg_Data[14].item[1].c_str());
	ui.seg153->setText(Seg_Data[14].item[2].c_str());

	ui.seg161->setText(Seg_Data[15].item[0].c_str());
	ui.seg162->setText(Seg_Data[15].item[1].c_str());
	ui.seg163->setText(Seg_Data[15].item[2].c_str());

	ui.seg171->setText(Seg_Data[16].item[0].c_str());
	ui.seg172->setText(Seg_Data[16].item[1].c_str());
	ui.seg173->setText(Seg_Data[16].item[2].c_str());

	ui.seg181->setText(Seg_Data[17].item[0].c_str());
	ui.seg182->setText(Seg_Data[17].item[1].c_str());
	ui.seg183->setText(Seg_Data[17].item[2].c_str());

	ui.seg191->setText(Seg_Data[18].item[0].c_str());
	ui.seg192->setText(Seg_Data[18].item[1].c_str());
	ui.seg193->setText(Seg_Data[18].item[2].c_str());

	ui.seg201->setText(Seg_Data[19].item[0].c_str());
	ui.seg202->setText(Seg_Data[19].item[1].c_str());
	ui.seg203->setText(Seg_Data[19].item[2].c_str());

	///////////////////AA_data
	ui.AA_data1->setText(AA_Item[0].c_str());
	ui.AA_data2->setText(AA_Item[1].c_str());
	ui.AA_data3->setText(AA_Item[2].c_str());
	ui.AA_data4->setText(AA_Item[3].c_str());
	ui.AA_data5->setText(AA_Item[4].c_str());
	ui.AA_data6->setText(AA_Item[5].c_str());
	ui.AA_data7->setText(AA_Item[6].c_str());
	ui.AA_data8->setText(AA_Item[7].c_str());
	ui.AA_data9->setText(AA_Item[8].c_str());
	ui.AA_data10->setText(AA_Item[9].c_str());
	ui.AA_data11->setText(AA_Item[10].c_str());
	ui.AA_data12->setText(AA_Item[11].c_str());

	///////////////////S_data
	ui.sData_item11->setText(sData_Item[0][0].c_str());
	ui.sData_item12->setText(sData_Item[0][1].c_str());
	ui.sData_item13->setText(sData_Item[0][2].c_str());
	ui.sData_item14->setText(sData_Item[0][3].c_str());
	ui.sData_item15->setText(sData_Item[0][4].c_str());
	ui.sData_item16->setText(sData_Item[0][5].c_str());

	ui.sData_item21->setText(sData_Item[1][0].c_str());
	ui.sData_item22->setText(sData_Item[1][1].c_str());
	ui.sData_item23->setText(sData_Item[1][2].c_str());
	ui.sData_item24->setText(sData_Item[1][3].c_str());
	ui.sData_item25->setText(sData_Item[1][4].c_str());
	ui.sData_item26->setText(sData_Item[1][5].c_str());

	ui.sData_item31->setText(sData_Item[2][0].c_str());
	ui.sData_item32->setText(sData_Item[2][1].c_str());
	ui.sData_item33->setText(sData_Item[2][2].c_str());
	ui.sData_item34->setText(sData_Item[2][3].c_str());
	ui.sData_item35->setText(sData_Item[2][4].c_str());
	ui.sData_item36->setText(sData_Item[2][5].c_str());

	ui.sData_item41->setText(sData_Item[3][0].c_str());
	ui.sData_item42->setText(sData_Item[3][1].c_str());
	ui.sData_item43->setText(sData_Item[3][2].c_str());
	ui.sData_item44->setText(sData_Item[3][3].c_str());
	ui.sData_item45->setText(sData_Item[3][4].c_str());
	ui.sData_item46->setText(sData_Item[3][5].c_str());

	ui.sData_item51->setText(sData_Item[4][0].c_str());
	ui.sData_item52->setText(sData_Item[4][1].c_str());
	ui.sData_item53->setText(sData_Item[4][2].c_str());
	ui.sData_item54->setText(sData_Item[4][3].c_str());
	ui.sData_item55->setText(sData_Item[4][4].c_str());
	ui.sData_item56->setText(sData_Item[4][5].c_str());

	ui.sData_item61->setText(sData_Item[5][0].c_str());
	ui.sData_item62->setText(sData_Item[5][1].c_str());
	ui.sData_item63->setText(sData_Item[5][2].c_str());
	ui.sData_item64->setText(sData_Item[5][3].c_str());
	ui.sData_item65->setText(sData_Item[5][4].c_str());
	ui.sData_item66->setText(sData_Item[5][5].c_str());

	ui.sData_item71->setText(sData_Item[6][0].c_str());
	ui.sData_item72->setText(sData_Item[6][1].c_str());
	ui.sData_item73->setText(sData_Item[6][2].c_str());
	ui.sData_item74->setText(sData_Item[6][3].c_str());
	ui.sData_item75->setText(sData_Item[6][4].c_str());
	ui.sData_item76->setText(sData_Item[6][5].c_str());

	ui.sData_item81->setText(sData_Item[7][0].c_str());
	ui.sData_item82->setText(sData_Item[7][1].c_str());
	ui.sData_item83->setText(sData_Item[7][2].c_str());
	ui.sData_item84->setText(sData_Item[7][3].c_str());
	ui.sData_item85->setText(sData_Item[7][4].c_str());
	ui.sData_item86->setText(sData_Item[7][5].c_str());

	ui.sData_item91->setText(sData_Item[8][0].c_str());
	ui.sData_item92->setText(sData_Item[8][1].c_str());
	ui.sData_item93->setText(sData_Item[8][2].c_str());
	ui.sData_item94->setText(sData_Item[8][3].c_str());
	ui.sData_item95->setText(sData_Item[8][4].c_str());
	ui.sData_item96->setText(sData_Item[8][5].c_str());

	ui.sData_item101->setText(sData_Item[9][0].c_str());
	ui.sData_item102->setText(sData_Item[9][1].c_str());
	ui.sData_item103->setText(sData_Item[9][2].c_str());
	ui.sData_item104->setText(sData_Item[9][3].c_str());
	ui.sData_item105->setText(sData_Item[9][4].c_str());
	ui.sData_item106->setText(sData_Item[9][5].c_str());

	ui.sData_item111->setText(sData_Item[10][0].c_str());
	ui.sData_item112->setText(sData_Item[10][1].c_str());
	ui.sData_item113->setText(sData_Item[10][2].c_str());
	ui.sData_item114->setText(sData_Item[10][3].c_str());
	ui.sData_item115->setText(sData_Item[10][4].c_str());
	ui.sData_item116->setText(sData_Item[10][5].c_str());

	ui.sData_item121->setText(sData_Item[11][0].c_str());
	ui.sData_item122->setText(sData_Item[11][1].c_str());
	ui.sData_item123->setText(sData_Item[11][2].c_str());
	ui.sData_item124->setText(sData_Item[11][3].c_str());
	ui.sData_item125->setText(sData_Item[11][4].c_str());
	ui.sData_item126->setText(sData_Item[11][5].c_str());

	ui.sData_item131->setText(sData_Item[12][0].c_str());
	ui.sData_item132->setText(sData_Item[12][1].c_str());
	ui.sData_item133->setText(sData_Item[12][2].c_str());
	ui.sData_item134->setText(sData_Item[12][3].c_str());
	ui.sData_item135->setText(sData_Item[12][4].c_str());
	ui.sData_item136->setText(sData_Item[12][5].c_str());

	ui.sData_item141->setText(sData_Item[13][0].c_str());
	ui.sData_item142->setText(sData_Item[13][1].c_str());
	ui.sData_item143->setText(sData_Item[13][2].c_str());
	ui.sData_item144->setText(sData_Item[13][3].c_str());
	ui.sData_item145->setText(sData_Item[13][4].c_str());
	ui.sData_item146->setText(sData_Item[13][5].c_str());

	ui.sData_item151->setText(sData_Item[14][0].c_str());
	ui.sData_item152->setText(sData_Item[14][1].c_str());
	ui.sData_item153->setText(sData_Item[14][2].c_str());
	ui.sData_item154->setText(sData_Item[14][3].c_str());
	ui.sData_item155->setText(sData_Item[14][4].c_str());
	ui.sData_item156->setText(sData_Item[14][5].c_str());

	ui.sData_item161->setText(sData_Item[15][0].c_str());
	ui.sData_item162->setText(sData_Item[15][1].c_str());
	ui.sData_item163->setText(sData_Item[15][2].c_str());
	ui.sData_item164->setText(sData_Item[15][3].c_str());
	ui.sData_item165->setText(sData_Item[15][4].c_str());
	ui.sData_item166->setText(sData_Item[15][5].c_str());

	ui.sData_item171->setText(sData_Item[16][0].c_str());
	ui.sData_item172->setText(sData_Item[16][1].c_str());
	ui.sData_item173->setText(sData_Item[16][2].c_str());
	ui.sData_item174->setText(sData_Item[16][3].c_str());
	ui.sData_item175->setText(sData_Item[16][4].c_str());
	ui.sData_item176->setText(sData_Item[16][5].c_str());

	ui.sData_item181->setText(sData_Item[17][0].c_str());
	ui.sData_item182->setText(sData_Item[17][1].c_str());
	ui.sData_item183->setText(sData_Item[17][2].c_str());
	ui.sData_item184->setText(sData_Item[17][3].c_str());
	ui.sData_item185->setText(sData_Item[17][4].c_str());
	ui.sData_item186->setText(sData_Item[17][5].c_str());

	///////////////////OIS_data
	ui.OIS_data11->setText(OIS_data_Item[0][0].c_str());
	ui.OIS_data12->setText(OIS_data_Item[0][1].c_str());
	ui.OIS_data13->setText(OIS_data_Item[0][2].c_str());
	ui.OIS_data21->setText(OIS_data_Item[1][0].c_str());
	ui.OIS_data22->setText(OIS_data_Item[1][1].c_str());
	ui.OIS_data23->setText(OIS_data_Item[1][2].c_str());
	ui.OIS_data31->setText(OIS_data_Item[2][0].c_str());
	ui.OIS_data32->setText(OIS_data_Item[2][1].c_str());
	ui.OIS_data33->setText(OIS_data_Item[2][2].c_str());
	ui.OIS_data41->setText(OIS_data_Item[3][0].c_str());
	ui.OIS_data42->setText(OIS_data_Item[3][1].c_str());
	ui.OIS_data43->setText(OIS_data_Item[3][2].c_str());
	ui.OIS_data51->setText(OIS_data_Item[4][0].c_str());
	ui.OIS_data52->setText(OIS_data_Item[4][1].c_str());
	ui.OIS_data61->setText(OIS_data_Item[5][0].c_str());
	ui.OIS_data62->setText(OIS_data_Item[5][1].c_str());
	ui.OIS_data71->setText(OIS_data_Item[6][0].c_str());
	ui.OIS_data72->setText(OIS_data_Item[6][1].c_str());
	ui.OIS_data81->setText(OIS_data_Item[7][0].c_str());
	ui.OIS_data82->setText(OIS_data_Item[7][1].c_str());

	///////////////////OIS_info
	ui.OIS_info11->setText(OIS_info_Item[0][0].c_str());
	ui.OIS_info12->setText(OIS_info_Item[0][1].c_str());
	ui.OIS_info13->setText(OIS_info_Item[0][2].c_str());
	ui.OIS_info14->setText(OIS_info_Item[0][3].c_str());
	ui.OIS_info21->setText(OIS_info_Item[1][0].c_str());
	ui.OIS_info22->setText(OIS_info_Item[1][1].c_str());
	ui.OIS_info23->setText(OIS_info_Item[1][2].c_str());
	ui.OIS_info24->setText(OIS_info_Item[1][3].c_str());
	ui.OIS_info31->setText(OIS_info_Item[2][0].c_str());
	ui.OIS_info32->setText(OIS_info_Item[2][1].c_str());
	ui.OIS_info33->setText(OIS_info_Item[2][2].c_str());
	ui.OIS_info34->setText(OIS_info_Item[2][3].c_str());
	ui.OIS_info41->setText(OIS_info_Item[3][0].c_str());
	ui.OIS_info42->setText(OIS_info_Item[3][1].c_str());
	ui.OIS_info43->setText(OIS_info_Item[3][2].c_str());
	ui.OIS_info44->setText(OIS_info_Item[3][3].c_str());
	ui.OIS_info51->setText(OIS_info_Item[4][0].c_str());
	ui.OIS_info52->setText(OIS_info_Item[4][1].c_str());
	ui.OIS_info53->setText(OIS_info_Item[4][2].c_str());
	ui.OIS_info54->setText(OIS_info_Item[4][3].c_str());
	ui.OIS_info61->setText(OIS_info_Item[5][0].c_str());
	ui.OIS_info62->setText(OIS_info_Item[5][1].c_str());
	ui.OIS_info63->setText(OIS_info_Item[5][2].c_str());
	ui.OIS_info64->setText(OIS_info_Item[5][3].c_str());
	ui.OIS_info71->setText(OIS_info_Item[6][0].c_str());
	ui.OIS_info72->setText(OIS_info_Item[6][1].c_str());
	ui.OIS_info73->setText(OIS_info_Item[6][2].c_str());
	ui.OIS_info74->setText(OIS_info_Item[6][3].c_str());
	ui.OIS_info81->setText(OIS_info_Item[7][0].c_str());
	ui.OIS_info82->setText(OIS_info_Item[7][1].c_str());
	ui.OIS_info83->setText(OIS_info_Item[7][2].c_str());
	ui.OIS_info84->setText(OIS_info_Item[7][3].c_str());
	ui.OIS_info91->setText(OIS_info_Item[8][0].c_str());
	ui.OIS_info92->setText(OIS_info_Item[8][1].c_str());
	ui.OIS_info93->setText(OIS_info_Item[8][2].c_str());
	ui.OIS_info94->setText(OIS_info_Item[8][3].c_str());
	ui.OIS_info101->setText(OIS_info_Item[9][0].c_str());
	ui.OIS_info102->setText(OIS_info_Item[9][1].c_str());
	ui.OIS_info103->setText(OIS_info_Item[9][2].c_str());
	ui.OIS_info104->setText(OIS_info_Item[9][3].c_str());
	ui.OIS_info111->setText(OIS_info_Item[10][0].c_str());
	ui.OIS_info112->setText(OIS_info_Item[10][1].c_str());
	ui.OIS_info113->setText(OIS_info_Item[10][2].c_str());
	ui.OIS_info114->setText(OIS_info_Item[10][3].c_str());
	ui.OIS_info121->setText(OIS_info_Item[11][0].c_str());
	ui.OIS_info122->setText(OIS_info_Item[11][1].c_str());
	ui.OIS_info123->setText(OIS_info_Item[11][2].c_str());
	ui.OIS_info124->setText(OIS_info_Item[11][3].c_str());
	ui.OIS_info131->setText(OIS_info_Item[12][0].c_str());
	ui.OIS_info132->setText(OIS_info_Item[12][1].c_str());
	ui.OIS_info133->setText(OIS_info_Item[12][2].c_str());
	ui.OIS_info134->setText(OIS_info_Item[12][3].c_str());
	ui.OIS_info141->setText(OIS_info_Item[13][0].c_str());
	ui.OIS_info142->setText(OIS_info_Item[13][1].c_str());
	ui.OIS_info143->setText(OIS_info_Item[13][2].c_str());
	ui.OIS_info144->setText(OIS_info_Item[13][3].c_str());
	ui.OIS_info151->setText(OIS_info_Item[14][0].c_str());
	ui.OIS_info152->setText(OIS_info_Item[14][1].c_str());
	ui.OIS_info153->setText(OIS_info_Item[14][2].c_str());
	ui.OIS_info154->setText(OIS_info_Item[14][3].c_str());
	ui.OIS_info161->setText(OIS_info_Item[15][0].c_str());
	ui.OIS_info162->setText(OIS_info_Item[15][1].c_str());
	ui.OIS_info163->setText(OIS_info_Item[15][2].c_str());
	ui.OIS_info164->setText(OIS_info_Item[15][3].c_str());
	ui.OIS_info171->setText(OIS_info_Item[16][0].c_str());
	ui.OIS_info172->setText(OIS_info_Item[16][1].c_str());
	ui.OIS_info173->setText(OIS_info_Item[16][2].c_str());
	ui.OIS_info174->setText(OIS_info_Item[16][3].c_str());
	ui.OIS_info181->setText(OIS_info_Item[17][0].c_str());
	ui.OIS_info182->setText(OIS_info_Item[17][1].c_str());
	ui.OIS_info183->setText(OIS_info_Item[17][2].c_str());
	ui.OIS_info184->setText(OIS_info_Item[17][3].c_str());
	ui.OIS_info191->setText(OIS_info_Item[18][0].c_str());
	ui.OIS_info192->setText(OIS_info_Item[18][1].c_str());
	ui.OIS_info193->setText(OIS_info_Item[18][2].c_str());
	ui.OIS_info194->setText(OIS_info_Item[18][3].c_str());
	ui.OIS_info201->setText(OIS_info_Item[19][0].c_str());
	ui.OIS_info202->setText(OIS_info_Item[19][1].c_str());
	ui.OIS_info203->setText(OIS_info_Item[19][2].c_str());
	ui.OIS_info204->setText(OIS_info_Item[19][3].c_str());
	ui.OIS_info211->setText(OIS_info_Item[20][0].c_str());
	ui.OIS_info212->setText(OIS_info_Item[20][1].c_str());
	ui.OIS_info213->setText(OIS_info_Item[20][2].c_str());
	ui.OIS_info214->setText(OIS_info_Item[20][3].c_str());
	ui.OIS_info221->setText(OIS_info_Item[21][0].c_str());
	ui.OIS_info222->setText(OIS_info_Item[21][1].c_str());
	ui.OIS_info223->setText(OIS_info_Item[21][2].c_str());
	ui.OIS_info224->setText(OIS_info_Item[21][3].c_str());
	ui.OIS_info231->setText(OIS_info_Item[22][0].c_str());
	ui.OIS_info232->setText(OIS_info_Item[22][1].c_str());
	ui.OIS_info233->setText(OIS_info_Item[22][2].c_str());
	ui.OIS_info234->setText(OIS_info_Item[22][3].c_str());
	ui.OIS_info241->setText(OIS_info_Item[23][0].c_str());
	ui.OIS_info242->setText(OIS_info_Item[23][1].c_str());
	ui.OIS_info243->setText(OIS_info_Item[23][2].c_str());
	ui.OIS_info244->setText(OIS_info_Item[23][3].c_str());

	//////////////PD data info
	ui.DCC11->setText(PD_Item[0][0].c_str());
	ui.DCC12->setText(PD_Item[0][1].c_str());
	ui.DCC13->setCurrentIndex(PD_Item3[0]);

	ui.DCC21->setText(PD_Item[1][0].c_str());
	ui.DCC22->setText(PD_Item[1][1].c_str());
	ui.DCC23->setCurrentIndex(PD_Item3[1]);

	ui.DCC31->setText(PD_Item[2][0].c_str());
	ui.DCC32->setText(PD_Item[2][1].c_str());
	ui.DCC33->setCurrentIndex(PD_Item3[2]);

	ui.DCC41->setText(PD_Item[3][0].c_str());
	ui.DCC42->setText(PD_Item[3][1].c_str());
	ui.DCC43->setCurrentIndex(PD_Item3[3]);

	ui.DCC51->setText(PD_Item[4][0].c_str());
	ui.DCC52->setText(PD_Item[4][1].c_str());
	ui.DCC53->setCurrentIndex(PD_Item3[4]);

	ui.DCC61->setText(PD_Item[5][0].c_str());
	ui.DCC62->setText(PD_Item[5][1].c_str());
	ui.DCC63->setCurrentIndex(PD_Item3[5]);

	ui.DCC71->setText(PD_Item[6][0].c_str());
	ui.DCC72->setText(PD_Item[6][1].c_str());
	ui.DCC73->setCurrentIndex(PD_Item3[6]);

	ui.DCC81->setText(PD_Item[7][0].c_str());
	ui.DCC82->setText(PD_Item[7][1].c_str());
	ui.DCC83->setCurrentIndex(PD_Item3[7]);

	ui.DCC91->setText(PD_Item[8][0].c_str());
	ui.DCC92->setText(PD_Item[8][1].c_str());
	ui.DCC93->setCurrentIndex(PD_Item3[8]);

	ui.DCC101->setText(PD_Item[9][0].c_str());
	ui.DCC102->setText(PD_Item[9][1].c_str());
	ui.DCC103->setCurrentIndex(PD_Item3[9]);

	ui.DCC111->setText(PD_Item[10][0].c_str());
	ui.DCC112->setText(PD_Item[10][1].c_str());
	ui.DCC113->setCurrentIndex(PD_Item3[10]);

	ui.DCC121->setText(PD_Item[11][0].c_str());
	ui.DCC122->setText(PD_Item[11][1].c_str());
	ui.DCC123->setCurrentIndex(PD_Item3[11]);

	ui.DCC131->setText(PD_Item[12][0].c_str());
	ui.DCC132->setText(PD_Item[12][1].c_str());
	ui.DCC133->setCurrentIndex(PD_Item3[12]);

	ui.DCC141->setText(PD_Item[13][0].c_str());
	ui.DCC142->setText(PD_Item[13][1].c_str());
	ui.DCC143->setCurrentIndex(PD_Item3[13]);

	////////Gmap addr setting
	ui.Gmap11->setText(Gmap_Item[0][0].c_str());
	ui.Gmap12->setText(Gmap_Item[0][1].c_str());
	ui.Gmap13->setText(Gmap_Item[0][2].c_str());
 	ui.Gmap14->setCurrentIndex(Gmap_Item3[0]);

	ui.Gmap21->setText(Gmap_Item[1][0].c_str());
	ui.Gmap22->setText(Gmap_Item[1][1].c_str());
	ui.Gmap23->setText(Gmap_Item[1][2].c_str());
	ui.Gmap24->setCurrentIndex(Gmap_Item3[1]);

	ui.Gmap31->setText(Gmap_Item[2][0].c_str());
	ui.Gmap32->setText(Gmap_Item[2][1].c_str());
	ui.Gmap33->setText(Gmap_Item[2][2].c_str());
	ui.Gmap34->setCurrentIndex(Gmap_Item3[2]);

	ui.Gmap41->setText(Gmap_Item[3][0].c_str());
	ui.Gmap42->setText(Gmap_Item[3][1].c_str());
	ui.Gmap43->setText(Gmap_Item[3][2].c_str());
	ui.Gmap44->setCurrentIndex(Gmap_Item3[3]);

	ui.Gmap51->setText(Gmap_Item[4][0].c_str());
	ui.Gmap52->setText(Gmap_Item[4][1].c_str());
	ui.Gmap53->setText(Gmap_Item[4][2].c_str());
	ui.Gmap54->setCurrentIndex(Gmap_Item3[4]);

	ui.Gmap61->setText(Gmap_Item[5][0].c_str());
	ui.Gmap62->setText(Gmap_Item[5][1].c_str());
	ui.Gmap63->setText(Gmap_Item[5][2].c_str());
	ui.Gmap64->setCurrentIndex(Gmap_Item3[5]);

	ui.Gmap71->setText(Gmap_Item[6][0].c_str());
	ui.Gmap72->setText(Gmap_Item[6][1].c_str());
	ui.Gmap73->setText(Gmap_Item[6][2].c_str());
	ui.Gmap74->setCurrentIndex(Gmap_Item3[6]);

	ui.Gmap81->setText(Gmap_Item[7][0].c_str());
	ui.Gmap82->setText(Gmap_Item[7][1].c_str());
	ui.Gmap83->setText(Gmap_Item[7][2].c_str());
	ui.Gmap84->setCurrentIndex(Gmap_Item3[7]);

	ui.Gmap91->setText(Gmap_Item[8][0].c_str());
	ui.Gmap92->setText(Gmap_Item[8][1].c_str());
	ui.Gmap93->setText(Gmap_Item[8][2].c_str());
	ui.Gmap94->setCurrentIndex(Gmap_Item3[8]);

	ui.Gmap101->setText(Gmap_Item[9][0].c_str());
	ui.Gmap102->setText(Gmap_Item[9][1].c_str());
	ui.Gmap103->setText(Gmap_Item[9][2].c_str());
	ui.Gmap104->setCurrentIndex(Gmap_Item3[9]);

	//////////////////PDAF INFO
	ui.PDAF_info11->setText(PDAF_info_Item[0][0].c_str());
	ui.PDAF_info12->setText(PDAF_info_Item[0][1].c_str());
	ui.PDAF_info13->setText(PDAF_info_Item[0][2].c_str());

	ui.PDAF_info21->setText(PDAF_info_Item[1][0].c_str());
	ui.PDAF_info22->setText(PDAF_info_Item[1][1].c_str());
	ui.PDAF_info23->setText(PDAF_info_Item[1][2].c_str());

	ui.PDAF_info31->setText(PDAF_info_Item[2][0].c_str());
	ui.PDAF_info32->setText(PDAF_info_Item[2][1].c_str());
	ui.PDAF_info33->setText(PDAF_info_Item[2][2].c_str());

	ui.PDAF_info41->setText(PDAF_info_Item[3][0].c_str());
	ui.PDAF_info42->setText(PDAF_info_Item[3][1].c_str());
	ui.PDAF_info43->setText(PDAF_info_Item[3][2].c_str());

	ui.PDAF_info51->setText(PDAF_info_Item[4][0].c_str());
	ui.PDAF_info52->setText(PDAF_info_Item[4][1].c_str());
	ui.PDAF_info53->setText(PDAF_info_Item[4][2].c_str());

	ui.PDAF_info61->setText(PDAF_info_Item[5][0].c_str());
	ui.PDAF_info62->setText(PDAF_info_Item[5][1].c_str());
	ui.PDAF_info63->setText(PDAF_info_Item[5][2].c_str());

	ui.PDAF_info71->setText(PDAF_info_Item[6][0].c_str());
	ui.PDAF_info72->setText(PDAF_info_Item[6][1].c_str());
	ui.PDAF_info73->setText(PDAF_info_Item[6][2].c_str());

	ui.PDAF_info81->setText(PDAF_info_Item[7][0].c_str());
	ui.PDAF_info82->setText(PDAF_info_Item[7][1].c_str());
	ui.PDAF_info83->setText(PDAF_info_Item[7][2].c_str());

	ui.PDAF_info91->setText(PDAF_info_Item[8][0].c_str());
	ui.PDAF_info92->setText(PDAF_info_Item[8][1].c_str());
	ui.PDAF_info93->setText(PDAF_info_Item[8][2].c_str());

	ui.PDAF_info101->setText(PDAF_info_Item[9][0].c_str());
	ui.PDAF_info102->setText(PDAF_info_Item[9][1].c_str());
	ui.PDAF_info103->setText(PDAF_info_Item[9][2].c_str());

	ui.PDAF_info111->setText(PDAF_info_Item[10][0].c_str());
	ui.PDAF_info112->setText(PDAF_info_Item[10][1].c_str());
	ui.PDAF_info113->setText(PDAF_info_Item[10][2].c_str());

	ui.PDAF_info121->setText(PDAF_info_Item[11][0].c_str());
	ui.PDAF_info122->setText(PDAF_info_Item[11][1].c_str());
	ui.PDAF_info123->setText(PDAF_info_Item[11][2].c_str());

	ui.PDAF_info131->setText(PDAF_info_Item[12][0].c_str());
	ui.PDAF_info132->setText(PDAF_info_Item[12][1].c_str());
	ui.PDAF_info133->setText(PDAF_info_Item[12][2].c_str());

	ui.PDAF_info141->setText(PDAF_info_Item[13][0].c_str());
	ui.PDAF_info142->setText(PDAF_info_Item[13][1].c_str());
	ui.PDAF_info143->setText(PDAF_info_Item[13][2].c_str());

	ui.PDAF_info151->setText(PDAF_info_Item[14][0].c_str());
	ui.PDAF_info152->setText(PDAF_info_Item[14][1].c_str());
	ui.PDAF_info153->setText(PDAF_info_Item[14][2].c_str());

	ui.PDAF_info161->setText(PDAF_info_Item[15][0].c_str());
	ui.PDAF_info162->setText(PDAF_info_Item[15][1].c_str());
	ui.PDAF_info163->setText(PDAF_info_Item[15][2].c_str());

	//////////////////////////////////////////////////////////
	ui.QR_start->setText(QR_Data.item[0].c_str());
	ui.QR_end->setText(QR_Data.item[1].c_str());
	ui.QR_spec->setText(QR_Data.item[2].c_str());

	ui.fuse_start->setText(Fuse_Data.item[0].c_str());
	ui.fuse_end->setText(Fuse_Data.item[1].c_str());
	ui.fuse_spec->setText(Fuse_Data.item[2].c_str());

	ui.fuse_start_2->setText(Fuse_Data2.item[0].c_str());
	ui.fuse_end_2->setText(Fuse_Data2.item[1].c_str());
	ui.fuse_spec_2->setText(Fuse_Data2.item[2].c_str());

	ui.lsc11->setText(QC_LSC_Data[0].item[0].c_str());
	ui.lsc12->setText(QC_LSC_Data[0].item[1].c_str());
	ui.lsc13->setText(QC_LSC_Data[0].item[2].c_str());
	ui.lsc14->setCurrentIndex(LSC_Item3[0]);

	ui.lsc21->setText(QC_LSC_Data[1].item[0].c_str());
	ui.lsc22->setText(QC_LSC_Data[1].item[1].c_str());
	ui.lsc23->setText(QC_LSC_Data[1].item[2].c_str());
	ui.lsc24->setCurrentIndex(LSC_Item3[1]);

	ui.lsc31->setText(QC_LSC_Data[2].item[0].c_str());
	ui.lsc32->setText(QC_LSC_Data[2].item[1].c_str());
	ui.lsc33->setText(QC_LSC_Data[2].item[2].c_str());
	ui.lsc34->setCurrentIndex(LSC_Item3[2]);

	ui.lsc41->setText(QC_LSC_Data[3].item[0].c_str());
	ui.lsc42->setText(QC_LSC_Data[3].item[1].c_str());
	ui.lsc43->setText(QC_LSC_Data[3].item[2].c_str());
	ui.lsc44->setCurrentIndex(LSC_Item3[3]);

	ui.lsc51->setText(QC_LSC_Data[4].item[0].c_str());
	ui.lsc52->setText(QC_LSC_Data[4].item[1].c_str());
	ui.lsc53->setText(QC_LSC_Data[4].item[2].c_str());
	ui.lsc54->setCurrentIndex(LSC_Item3[4]);

	ui.lsc61->setText(QC_LSC_Data[5].item[0].c_str());
	ui.lsc62->setText(QC_LSC_Data[5].item[1].c_str());
	ui.lsc63->setText(QC_LSC_Data[5].item[2].c_str());
	ui.lsc64->setCurrentIndex(LSC_Item3[5]);

	ui.lsc71->setText(QC_LSC_Data[6].item[0].c_str());
	ui.lsc72->setText(QC_LSC_Data[6].item[1].c_str());
	ui.lsc73->setText(QC_LSC_Data[6].item[2].c_str());
	ui.lsc74->setCurrentIndex(LSC_Item3[6]);

	ui.lsc81->setText(QC_LSC_Data[7].item[0].c_str());
	ui.lsc82->setText(QC_LSC_Data[7].item[1].c_str());
	ui.lsc83->setText(QC_LSC_Data[7].item[2].c_str());
	ui.lsc84->setCurrentIndex(LSC_Item3[7]);

	ui.MTK_LSC11->setText(MTK_LSC_Data[0].item[0].c_str());
	ui.MTK_LSC12->setText(MTK_LSC_Data[0].item[1].c_str());

	ui.MTK_LSC21->setText(MTK_LSC_Data[1].item[0].c_str());
	ui.MTK_LSC22->setText(MTK_LSC_Data[1].item[1].c_str());

	ui.shift_start->setText(shift_Item[0].c_str());
	ui.shift_byte->setText(shift_Item[1].c_str());
	ui.shift_point->setText(shift_Item[2].c_str());
	ui.shift_start_2->setText(shift_Item[3].c_str());

	ui.cross1->setText(cross_Item[0].c_str());
	ui.cross2->setText(cross_Item[1].c_str());
	ui.cross3->setText(cross_Item[2].c_str());

	ui.akm_cross1->setText(akm_cross_Item[0].c_str());
	ui.akm_cross2->setText(akm_cross_Item[1].c_str());
	ui.akm_cross3->setText(akm_cross_Item[2].c_str());

	ui.AF_name1->setText(AF_Item[0][0].c_str());
	ui.AF_start1->setText(AF_Item[0][1].c_str());
	ui.af_min1->setText(AF_Item[0][2].c_str());
	ui.af_max1->setText(AF_Item[0][3].c_str());

	ui.AF_name2->setText(AF_Item[1][0].c_str());
	ui.AF_start2->setText(AF_Item[1][1].c_str());
	ui.af_min2->setText(AF_Item[1][2].c_str());
	ui.af_max2->setText(AF_Item[1][3].c_str());

	ui.AF_name3->setText(AF_Item[2][0].c_str());
	ui.AF_start3->setText(AF_Item[2][1].c_str());
	ui.af_min3->setText(AF_Item[2][2].c_str());
	ui.af_max3->setText(AF_Item[2][3].c_str());

	ui.AF_name4->setText(AF_Item[3][0].c_str());
	ui.AF_start4->setText(AF_Item[3][1].c_str());
	ui.af_min4->setText(AF_Item[3][2].c_str());
	ui.af_max4->setText(AF_Item[3][3].c_str());

	ui.AF_name5->setText(AF_Item[4][0].c_str());
	ui.AF_start5->setText(AF_Item[4][1].c_str());
	ui.af_min5->setText(AF_Item[4][2].c_str());
	ui.af_max5->setText(AF_Item[4][3].c_str());

	ui.AF_name6->setText(AF_Item[5][0].c_str());
	ui.AF_start6->setText(AF_Item[5][1].c_str());
	ui.af_min6->setText(AF_Item[5][2].c_str());
	ui.af_max6->setText(AF_Item[5][3].c_str());

	ui.AF_info11->setText(AF_info_Item[0][0].c_str());
	ui.AF_info12->setText(AF_info_Item[0][1].c_str());
	ui.AF_info13->setText(AF_info_Item[0][2].c_str());
	ui.AF_info14->setText(AF_info_Item[0][3].c_str());

	ui.AF_info21->setText(AF_info_Item[1][0].c_str());
	ui.AF_info22->setText(AF_info_Item[1][1].c_str());
	ui.AF_info23->setText(AF_info_Item[1][2].c_str());
	ui.AF_info24->setText(AF_info_Item[1][3].c_str());

	ui.AF_info31->setText(AF_info_Item[2][0].c_str());
	ui.AF_info32->setText(AF_info_Item[2][1].c_str());
	ui.AF_info33->setText(AF_info_Item[2][2].c_str());
	ui.AF_info34->setText(AF_info_Item[2][3].c_str());

	ui.AF_info41->setText(AF_info_Item[3][0].c_str());
	ui.AF_info42->setText(AF_info_Item[3][1].c_str());
	ui.AF_info43->setText(AF_info_Item[3][2].c_str());
	ui.AF_info44->setText(AF_info_Item[3][3].c_str());

	ui.AF_info51->setText(AF_info_Item[4][0].c_str());
	ui.AF_info52->setText(AF_info_Item[4][1].c_str());
	ui.AF_info53->setText(AF_info_Item[4][2].c_str());
	ui.AF_info54->setText(AF_info_Item[4][3].c_str());

	ui.AF_info61->setText(AF_info_Item[5][0].c_str());
	ui.AF_info62->setText(AF_info_Item[5][1].c_str());
	ui.AF_info63->setText(AF_info_Item[5][2].c_str());
	ui.AF_info64->setText(AF_info_Item[5][3].c_str());

	ui.AF_info71->setText(AF_info_Item[6][0].c_str());
	ui.AF_info72->setText(AF_info_Item[6][1].c_str());
	ui.AF_info73->setText(AF_info_Item[6][2].c_str());
	ui.AF_info74->setText(AF_info_Item[6][3].c_str());

	ui.AF_info81->setText(AF_info_Item[7][0].c_str());
	ui.AF_info82->setText(AF_info_Item[7][1].c_str());
	ui.AF_info83->setText(AF_info_Item[7][2].c_str());
	ui.AF_info84->setText(AF_info_Item[7][3].c_str());

	ui.AF_info91->setText(AF_info_Item[8][0].c_str());
	ui.AF_info92->setText(AF_info_Item[8][1].c_str());
	ui.AF_info93->setText(AF_info_Item[8][2].c_str());
	ui.AF_info94->setText(AF_info_Item[8][3].c_str());

	ui.AF_info91->setText(AF_info_Item[8][0].c_str());
	ui.AF_info92->setText(AF_info_Item[8][1].c_str());
	ui.AF_info93->setText(AF_info_Item[8][2].c_str());
	ui.AF_info94->setText(AF_info_Item[8][3].c_str());

	ui.AF_info101->setText(AF_info_Item[9][0].c_str());
	ui.AF_info102->setText(AF_info_Item[9][1].c_str());
	ui.AF_info103->setText(AF_info_Item[9][2].c_str());
	ui.AF_info104->setText(AF_info_Item[9][3].c_str());

	ui.SFR_name1->setText(SFR_Item[0][0].c_str());
	ui.SFR_grade1->setText(SFR_Item[0][1].c_str());
	ui.SFR_start1->setText(SFR_Item[0][2].c_str());
	ui.SFR_cnt1->setText(SFR_Item[0][3].c_str());

	ui.SFR_name2->setText(SFR_Item[1][0].c_str());
	ui.SFR_grade2->setText(SFR_Item[1][1].c_str());
	ui.SFR_start2->setText(SFR_Item[1][2].c_str());
	ui.SFR_cnt2->setText(SFR_Item[1][3].c_str());

	ui.SFR_name3->setText(SFR_Item[2][0].c_str());
	ui.SFR_grade3->setText(SFR_Item[2][1].c_str());
	ui.SFR_start3->setText(SFR_Item[2][2].c_str());
	ui.SFR_cnt3->setText(SFR_Item[2][3].c_str());

	ui.SFR_name4->setText(SFR_Item[3][0].c_str());
	ui.SFR_grade4->setText(SFR_Item[3][1].c_str());
	ui.SFR_start4->setText(SFR_Item[3][2].c_str());
	ui.SFR_cnt4->setText(SFR_Item[3][3].c_str());

	ui.checksum11->setText(checkSum[0].item[0].c_str());
	ui.checksum12->setText(checkSum[0].item[1].c_str());
	ui.checksum13->setText(checkSum[0].item[2].c_str());
	ui.checksum14->setText(checkSum[0].item[3].c_str());
	ui.checksum15->setText(checkSum[0].item[4].c_str());

	ui.checksum21->setText(checkSum[1].item[0].c_str());
	ui.checksum22->setText(checkSum[1].item[1].c_str());
	ui.checksum23->setText(checkSum[1].item[2].c_str());
	ui.checksum24->setText(checkSum[1].item[3].c_str());
	ui.checksum25->setText(checkSum[1].item[4].c_str());

	ui.checksum31->setText(checkSum[2].item[0].c_str());
	ui.checksum32->setText(checkSum[2].item[1].c_str());
	ui.checksum33->setText(checkSum[2].item[2].c_str());
	ui.checksum34->setText(checkSum[2].item[3].c_str());
	ui.checksum35->setText(checkSum[2].item[4].c_str());

	ui.checksum41->setText(checkSum[3].item[0].c_str());
	ui.checksum42->setText(checkSum[3].item[1].c_str());
	ui.checksum43->setText(checkSum[3].item[2].c_str());
	ui.checksum44->setText(checkSum[3].item[3].c_str());
	ui.checksum45->setText(checkSum[3].item[4].c_str());

	ui.checksum51->setText(checkSum[4].item[0].c_str());
	ui.checksum52->setText(checkSum[4].item[1].c_str());
	ui.checksum53->setText(checkSum[4].item[2].c_str());
	ui.checksum54->setText(checkSum[4].item[3].c_str());
	ui.checksum55->setText(checkSum[4].item[4].c_str());

	ui.checksum61->setText(checkSum[5].item[0].c_str());
	ui.checksum62->setText(checkSum[5].item[1].c_str());
	ui.checksum63->setText(checkSum[5].item[2].c_str());
	ui.checksum64->setText(checkSum[5].item[3].c_str());
	ui.checksum65->setText(checkSum[5].item[4].c_str());

	ui.checksum71->setText(checkSum[6].item[0].c_str());
	ui.checksum72->setText(checkSum[6].item[1].c_str());
	ui.checksum73->setText(checkSum[6].item[2].c_str());
	ui.checksum74->setText(checkSum[6].item[3].c_str());
	ui.checksum75->setText(checkSum[6].item[4].c_str());

	ui.checksum81->setText(checkSum[7].item[0].c_str());
	ui.checksum82->setText(checkSum[7].item[1].c_str());
	ui.checksum83->setText(checkSum[7].item[2].c_str());
	ui.checksum84->setText(checkSum[7].item[3].c_str());
	ui.checksum85->setText(checkSum[7].item[4].c_str());

	ui.checksum91->setText(checkSum[8].item[0].c_str());
	ui.checksum92->setText(checkSum[8].item[1].c_str());
	ui.checksum93->setText(checkSum[8].item[2].c_str());
	ui.checksum94->setText(checkSum[8].item[3].c_str());
	ui.checksum95->setText(checkSum[8].item[4].c_str());

	ui.checksum101->setText(checkSum[9].item[0].c_str());
	ui.checksum102->setText(checkSum[9].item[1].c_str());
	ui.checksum103->setText(checkSum[9].item[2].c_str());
	ui.checksum104->setText(checkSum[9].item[3].c_str());
	ui.checksum105->setText(checkSum[9].item[4].c_str());

	ui.checksum111->setText(checkSum[10].item[0].c_str());
	ui.checksum112->setText(checkSum[10].item[1].c_str());
	ui.checksum113->setText(checkSum[10].item[2].c_str());
	ui.checksum114->setText(checkSum[10].item[3].c_str());
	ui.checksum115->setText(checkSum[10].item[4].c_str());

	ui.checksum121->setText(checkSum[11].item[0].c_str());
	ui.checksum122->setText(checkSum[11].item[1].c_str());
	ui.checksum123->setText(checkSum[11].item[2].c_str());
	ui.checksum124->setText(checkSum[11].item[3].c_str());
	ui.checksum125->setText(checkSum[11].item[4].c_str());

	ui.checksum131->setText(checkSum[12].item[0].c_str());
	ui.checksum132->setText(checkSum[12].item[1].c_str());
	ui.checksum133->setText(checkSum[12].item[2].c_str());
	ui.checksum134->setText(checkSum[12].item[3].c_str());
	ui.checksum135->setText(checkSum[12].item[4].c_str());

	ui.checksum141->setText(checkSum[13].item[0].c_str());
	ui.checksum142->setText(checkSum[13].item[1].c_str());
	ui.checksum143->setText(checkSum[13].item[2].c_str());
	ui.checksum144->setText(checkSum[13].item[3].c_str());
	ui.checksum145->setText(checkSum[13].item[4].c_str());

	ui.checksum151->setText(checkSum[14].item[0].c_str());
	ui.checksum152->setText(checkSum[14].item[1].c_str());
	ui.checksum153->setText(checkSum[14].item[2].c_str());
	ui.checksum154->setText(checkSum[14].item[3].c_str());
	ui.checksum155->setText(checkSum[14].item[4].c_str());

	ui.checksum161->setText(checkSum[15].item[0].c_str());
	ui.checksum162->setText(checkSum[15].item[1].c_str());
	ui.checksum163->setText(checkSum[15].item[2].c_str());
	ui.checksum164->setText(checkSum[15].item[3].c_str());
	ui.checksum165->setText(checkSum[15].item[4].c_str());

	ui.checksum171->setText(checkSum[16].item[0].c_str());
	ui.checksum172->setText(checkSum[16].item[1].c_str());
	ui.checksum173->setText(checkSum[16].item[2].c_str());
	ui.checksum174->setText(checkSum[16].item[3].c_str());
	ui.checksum175->setText(checkSum[16].item[4].c_str());

	ui.checksum181->setText(checkSum[17].item[0].c_str());
	ui.checksum182->setText(checkSum[17].item[1].c_str());
	ui.checksum183->setText(checkSum[17].item[2].c_str());
	ui.checksum184->setText(checkSum[17].item[3].c_str());
	ui.checksum185->setText(checkSum[17].item[4].c_str());

	ui.checksum191->setText(checkSum[18].item[0].c_str());
	ui.checksum192->setText(checkSum[18].item[1].c_str());
	ui.checksum193->setText(checkSum[18].item[2].c_str());
	ui.checksum194->setText(checkSum[18].item[3].c_str());
	ui.checksum195->setText(checkSum[18].item[4].c_str());

	ui.checksum201->setText(checkSum[19].item[0].c_str());
	ui.checksum202->setText(checkSum[19].item[1].c_str());
	ui.checksum203->setText(checkSum[19].item[2].c_str());
	ui.checksum204->setText(checkSum[19].item[3].c_str());
	ui.checksum205->setText(checkSum[19].item[4].c_str());

	ui.checksum211->setText(checkSum[20].item[0].c_str());
	ui.checksum212->setText(checkSum[20].item[1].c_str());
	ui.checksum213->setText(checkSum[20].item[2].c_str());
	ui.checksum214->setText(checkSum[20].item[3].c_str());
	ui.checksum215->setText(checkSum[20].item[4].c_str());

	ui.checksum221->setText(checkSum[21].item[0].c_str());
	ui.checksum222->setText(checkSum[21].item[1].c_str());
	ui.checksum223->setText(checkSum[21].item[2].c_str());
	ui.checksum224->setText(checkSum[21].item[3].c_str());
	ui.checksum225->setText(checkSum[21].item[4].c_str());

	ui.checksum231->setText(checkSum[22].item[0].c_str());
	ui.checksum232->setText(checkSum[22].item[1].c_str());
	ui.checksum233->setText(checkSum[22].item[2].c_str());
	ui.checksum234->setText(checkSum[22].item[3].c_str());
	ui.checksum235->setText(checkSum[22].item[4].c_str());

	ui.checksum241->setText(checkSum[23].item[0].c_str());
	ui.checksum242->setText(checkSum[23].item[1].c_str());
	ui.checksum243->setText(checkSum[23].item[2].c_str());
	ui.checksum244->setText(checkSum[23].item[3].c_str());
	ui.checksum245->setText(checkSum[23].item[4].c_str());

	ui.checksum251->setText(checkSum[24].item[0].c_str());
	ui.checksum252->setText(checkSum[24].item[1].c_str());
	ui.checksum253->setText(checkSum[24].item[2].c_str());
	ui.checksum254->setText(checkSum[24].item[3].c_str());
	ui.checksum255->setText(checkSum[24].item[4].c_str());

	ui.checksum261->setText(checkSum[25].item[0].c_str());
	ui.checksum262->setText(checkSum[25].item[1].c_str());
	ui.checksum263->setText(checkSum[25].item[2].c_str());
	ui.checksum264->setText(checkSum[25].item[3].c_str());
	ui.checksum265->setText(checkSum[25].item[4].c_str());

	ui.checksum271->setText(checkSum[26].item[0].c_str());
	ui.checksum272->setText(checkSum[26].item[1].c_str());
	ui.checksum273->setText(checkSum[26].item[2].c_str());
	ui.checksum274->setText(checkSum[26].item[3].c_str());
	ui.checksum275->setText(checkSum[26].item[4].c_str());

	ui.checksum281->setText(checkSum[27].item[0].c_str());
	ui.checksum282->setText(checkSum[27].item[1].c_str());
	ui.checksum283->setText(checkSum[27].item[2].c_str());
	ui.checksum284->setText(checkSum[27].item[3].c_str());
	ui.checksum285->setText(checkSum[27].item[4].c_str());

	ui.checksum291->setText(checkSum[28].item[0].c_str());
	ui.checksum292->setText(checkSum[28].item[1].c_str());
	ui.checksum293->setText(checkSum[28].item[2].c_str());
	ui.checksum294->setText(checkSum[28].item[3].c_str());
	ui.checksum295->setText(checkSum[28].item[4].c_str());

	ui.checksum301->setText(checkSum[29].item[0].c_str());
	ui.checksum302->setText(checkSum[29].item[1].c_str());
	ui.checksum303->setText(checkSum[29].item[2].c_str());
	ui.checksum304->setText(checkSum[29].item[3].c_str());
	ui.checksum305->setText(checkSum[29].item[4].c_str());

	ui.checksum311->setText(checkSum[30].item[0].c_str());
	ui.checksum312->setText(checkSum[30].item[1].c_str());
	ui.checksum313->setText(checkSum[30].item[2].c_str());
	ui.checksum314->setText(checkSum[30].item[3].c_str());
	ui.checksum315->setText(checkSum[30].item[4].c_str());

	ui.checksum321->setText(checkSum[31].item[0].c_str());
	ui.checksum322->setText(checkSum[31].item[1].c_str());
	ui.checksum323->setText(checkSum[31].item[2].c_str());
	ui.checksum324->setText(checkSum[31].item[3].c_str());
	ui.checksum325->setText(checkSum[31].item[4].c_str());

	ui.checksum331->setText(checkSum[32].item[0].c_str());
	ui.checksum332->setText(checkSum[32].item[1].c_str());
	ui.checksum333->setText(checkSum[32].item[2].c_str());
	ui.checksum334->setText(checkSum[32].item[3].c_str());
	ui.checksum335->setText(checkSum[32].item[4].c_str());

	ui.checksum341->setText(checkSum[33].item[0].c_str());
	ui.checksum342->setText(checkSum[33].item[1].c_str());
	ui.checksum343->setText(checkSum[33].item[2].c_str());
	ui.checksum344->setText(checkSum[33].item[3].c_str());
	ui.checksum345->setText(checkSum[33].item[4].c_str());

	ui.checksum351->setText(checkSum[34].item[0].c_str());
	ui.checksum352->setText(checkSum[34].item[1].c_str());
	ui.checksum353->setText(checkSum[34].item[2].c_str());
	ui.checksum354->setText(checkSum[34].item[3].c_str());
	ui.checksum355->setText(checkSum[34].item[4].c_str());

	ui.checksum361->setText(checkSum[35].item[0].c_str());
	ui.checksum362->setText(checkSum[35].item[1].c_str());
	ui.checksum363->setText(checkSum[35].item[2].c_str());
	ui.checksum364->setText(checkSum[35].item[3].c_str());
	ui.checksum365->setText(checkSum[35].item[4].c_str());

	ui.checksum371->setText(checkSum[36].item[0].c_str());
	ui.checksum372->setText(checkSum[36].item[1].c_str());
	ui.checksum373->setText(checkSum[36].item[2].c_str());
	ui.checksum374->setText(checkSum[36].item[3].c_str());
	ui.checksum375->setText(checkSum[36].item[4].c_str());

	ui.checksum381->setText(checkSum[37].item[0].c_str());
	ui.checksum382->setText(checkSum[37].item[1].c_str());
	ui.checksum383->setText(checkSum[37].item[2].c_str());
	ui.checksum384->setText(checkSum[37].item[3].c_str());
	ui.checksum385->setText(checkSum[37].item[4].c_str());

	ui.checksum391->setText(checkSum[38].item[0].c_str());
	ui.checksum392->setText(checkSum[38].item[1].c_str());
	ui.checksum393->setText(checkSum[38].item[2].c_str());
	ui.checksum394->setText(checkSum[38].item[3].c_str());
	ui.checksum395->setText(checkSum[38].item[4].c_str());

	ui.checksum401->setText(checkSum[39].item[0].c_str());
	ui.checksum402->setText(checkSum[39].item[1].c_str());
	ui.checksum403->setText(checkSum[39].item[2].c_str());
	ui.checksum404->setText(checkSum[39].item[3].c_str());
	ui.checksum405->setText(checkSum[39].item[4].c_str());

	if (checkDivisor == 0)ui.radioButton_255->setChecked(true);
	else if (checkDivisor == 1)ui.radioButton_255_1->setChecked(true);
	else if (checkDivisor == 2)ui.radioButton_256->setChecked(true);
	else if (checkDivisor == 4)ui.radioButton_CRC32->setChecked(true);
	else if (checkDivisor == 8)ui.radioButton_FFFF->setChecked(true);

	if (HL&1)ui.radioButton_HL->setChecked(true);
	else ui.radioButton_LH->setChecked(true);

	if (HL & 2)ui.OIS_HL->setChecked(true);
	else ui.OIS_LH->setChecked(true);

	if (HL & 4)ui.SR_LH->setChecked(true);
	else ui.SR_HL->setChecked(true);

	if (HL & 8)ui.PD_Offset_LH->setChecked(true);
	else ui.PD_Offset_HL->setChecked(true);

	ui.info11->setText(infoData[0].item[0].c_str());
	ui.info12->setText(infoData[0].item[1].c_str());
	ui.info13->setText(infoData[0].item[2].c_str());

	ui.info21->setText(infoData[1].item[0].c_str());
	ui.info22->setText(infoData[1].item[1].c_str());
	ui.info23->setText(infoData[1].item[2].c_str());

	ui.info31->setText(infoData[2].item[0].c_str());
	ui.info32->setText(infoData[2].item[1].c_str());
	ui.info33->setText(infoData[2].item[2].c_str());

	ui.info41->setText(infoData[3].item[0].c_str());
	ui.info42->setText(infoData[3].item[1].c_str());
	ui.info43->setText(infoData[3].item[2].c_str());

	ui.info51->setText(infoData[4].item[0].c_str());
	ui.info52->setText(infoData[4].item[1].c_str());
	ui.info53->setText(infoData[4].item[2].c_str());

	ui.info61->setText(infoData[5].item[0].c_str());
	ui.info62->setText(infoData[5].item[1].c_str());
	ui.info63->setText(infoData[5].item[2].c_str());

	ui.info71->setText(infoData[6].item[0].c_str());
	ui.info72->setText(infoData[6].item[1].c_str());
	ui.info73->setText(infoData[6].item[2].c_str());

	ui.info81->setText(infoData[7].item[0].c_str());
	ui.info82->setText(infoData[7].item[1].c_str());
	ui.info83->setText(infoData[7].item[2].c_str());

	ui.info91->setText(infoData[8].item[0].c_str());
	ui.info92->setText(infoData[8].item[1].c_str());
	ui.info93->setText(infoData[8].item[2].c_str());

	ui.info101->setText(infoData[9].item[0].c_str());
	ui.info102->setText(infoData[9].item[1].c_str());
	ui.info103->setText(infoData[9].item[2].c_str());

	ui.info111->setText(infoData[10].item[0].c_str());
	ui.info112->setText(infoData[10].item[1].c_str());
	ui.info113->setText(infoData[10].item[2].c_str());

	ui.info121->setText(infoData[11].item[0].c_str());
	ui.info122->setText(infoData[11].item[1].c_str());
	ui.info123->setText(infoData[11].item[2].c_str());

	ui.info131->setText(infoData[12].item[0].c_str());
	ui.info132->setText(infoData[12].item[1].c_str());
	ui.info133->setText(infoData[12].item[2].c_str());

	ui.info141->setText(infoData[13].item[0].c_str());
	ui.info142->setText(infoData[13].item[1].c_str());
	ui.info143->setText(infoData[13].item[2].c_str());

	ui.info151->setText(infoData[14].item[0].c_str());
	ui.info152->setText(infoData[14].item[1].c_str());
	ui.info153->setText(infoData[14].item[2].c_str());

	ui.info161->setText(infoData[15].item[0].c_str());
	ui.info162->setText(infoData[15].item[1].c_str());
	ui.info163->setText(infoData[15].item[2].c_str());

	ui.info171->setText(infoData[16].item[0].c_str());
	ui.info172->setText(infoData[16].item[1].c_str());
	ui.info173->setText(infoData[16].item[2].c_str());

	ui.info181->setText(infoData[17].item[0].c_str());
	ui.info182->setText(infoData[17].item[1].c_str());
	ui.info183->setText(infoData[17].item[2].c_str());

	ui.info191->setText(infoData[18].item[0].c_str());
	ui.info192->setText(infoData[18].item[1].c_str());
	ui.info193->setText(infoData[18].item[2].c_str());

	ui.info201->setText(infoData[19].item[0].c_str());
	ui.info202->setText(infoData[19].item[1].c_str());
	ui.info203->setText(infoData[19].item[2].c_str());

	ui.info211->setText(infoData[20].item[0].c_str());
	ui.info212->setText(infoData[20].item[1].c_str());
	ui.info213->setText(infoData[20].item[2].c_str());

	ui.info221->setText(infoData[21].item[0].c_str());
	ui.info222->setText(infoData[21].item[1].c_str());
	ui.info223->setText(infoData[21].item[2].c_str());

	ui.info231->setText(infoData[22].item[0].c_str());
	ui.info232->setText(infoData[22].item[1].c_str());
	ui.info233->setText(infoData[22].item[2].c_str());

	ui.info241->setText(infoData[23].item[0].c_str());
	ui.info242->setText(infoData[23].item[1].c_str());
	ui.info243->setText(infoData[23].item[2].c_str());

	ui.info251->setText(infoData[24].item[0].c_str());
	ui.info252->setText(infoData[24].item[1].c_str());
	ui.info253->setText(infoData[24].item[2].c_str());

	ui.info261->setText(infoData[25].item[0].c_str());
	ui.info262->setText(infoData[25].item[1].c_str());
	ui.info263->setText(infoData[25].item[2].c_str());

	ui.info271->setText(infoData[26].item[0].c_str());
	ui.info272->setText(infoData[26].item[1].c_str());
	ui.info273->setText(infoData[26].item[2].c_str());

	ui.info281->setText(infoData[27].item[0].c_str());
	ui.info282->setText(infoData[27].item[1].c_str());
	ui.info283->setText(infoData[27].item[2].c_str());

	ui.info291->setText(infoData[28].item[0].c_str());
	ui.info292->setText(infoData[28].item[1].c_str());
	ui.info293->setText(infoData[28].item[2].c_str());

	ui.info301->setText(infoData[29].item[0].c_str());
	ui.info302->setText(infoData[29].item[1].c_str());
	ui.info303->setText(infoData[29].item[2].c_str());

	ui.info311->setText(infoData[30].item[0].c_str());
	ui.info312->setText(infoData[30].item[1].c_str());
	ui.info313->setText(infoData[30].item[2].c_str());

	ui.info321->setText(infoData[31].item[0].c_str());
	ui.info322->setText(infoData[31].item[1].c_str());
	ui.info323->setText(infoData[31].item[2].c_str());

	ui.year_H->setText(product_Date.item[0].c_str());
	ui.year_L->setText(product_Date.item[1].c_str());
	ui.month->setText(product_Date.item[2].c_str());
	ui.day->setText(product_Date.item[3].c_str());
	ui.hour->setText(product_Date.item[4].c_str());
	ui.minute->setText(product_Date.item[5].c_str());
	ui.second->setText(product_Date.item[6].c_str());
	ui.factory->setText(product_Date.item[7].c_str());
	ui.line->setText(product_Date.item[8].c_str());

	ui.same11->setText(Same_Item[0].item[0].c_str());
	ui.same12->setText(Same_Item[0].item[1].c_str());
	ui.same13->setText(Same_Item[0].item[2].c_str());

	ui.same21->setText(Same_Item[1].item[0].c_str());
	ui.same22->setText(Same_Item[1].item[1].c_str());
	ui.same23->setText(Same_Item[1].item[2].c_str());

	ui.same31->setText(Same_Item[2].item[0].c_str());
	ui.same32->setText(Same_Item[2].item[1].c_str());
	ui.same33->setText(Same_Item[2].item[2].c_str());

	ui.same41->setText(Same_Item[3].item[0].c_str());
	ui.same42->setText(Same_Item[3].item[1].c_str());
	ui.same43->setText(Same_Item[3].item[2].c_str());

	ui.same51->setText(Same_Item[4].item[0].c_str());
	ui.same52->setText(Same_Item[4].item[1].c_str());
	ui.same53->setText(Same_Item[4].item[2].c_str());

	ui.same61->setText(Same_Item[5].item[0].c_str());
	ui.same62->setText(Same_Item[5].item[1].c_str());
	ui.same63->setText(Same_Item[5].item[2].c_str());

}


void EEPROM_Data_Verifier::load_EEPROM_Address() {

	for (int i = 0; i < 80; i++) {
		memset(value_Hash[i].hash, 0, sizeof(value_Hash[i].hash));
		value_Hash[i].item_name = "";
	}
	set_ini_Path(EEPROM_Map);
	spec_Bin_Ready=read_Spec_Bin();

	TCHAR lpTexts[256]; int temp = 0; string str;
	USES_CONVERSION;
	for (int i = 0; i < 40; i++)
		for (int k = 0; k < 5; k++) {
		string item = "checkSum" + to_string(i+1) + to_string(k+1);		 
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		checkSum[i].item[k]  = CT2A(lpTexts);
	}

	for (int i = 0; i < 40; i++) {
		if (checkSum[i].item[0].length()>1) {
			if (i < 38) {
				checkSum[i].flag = unstringHex2int(checkSum[i].item[1]);
				if (checkSum[i].item[1].length() < 1)
					checkSum[i].flag = -1;
				checkSum[i].start = unstringHex2int(checkSum[i].item[2]);
				checkSum[i].end = unstringHex2int(checkSum[i].item[3]);
				checkSum[i].checksum = unstringHex2int(checkSum[i].item[4]);
			}
			else {
				checkSum[i].start = unstringHex2int(checkSum[i].item[0]);
				checkSum[i].end = unstringHex2int(checkSum[i].item[1]);
				checkSum[i].Except_S = unstringHex2int(checkSum[i].item[2]);
				checkSum[i].Except_E = unstringHex2int(checkSum[i].item[3]);
				checkSum[i].checksum = unstringHex2int(checkSum[i].item[4]);

			}
		}
	}
	for (int i = 0; i < 32; i++)
		for (int k = 0; k < 3; k++) {
			string item = "info" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			infoData[i].item[k] = CT2A(lpTexts);
		}
	
	for (int i = 0; i < 6; i++) {
		string item = "AF_name" + to_string(i + 1);
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		AF_Item[i][0] = CT2A(lpTexts);
		item = "AF_start" + to_string(i + 1);
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		AF_Item[i][1] = CT2A(lpTexts);
		item = "af_min" + to_string(i + 1);
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		AF_Item[i][2] = CT2A(lpTexts);
		item = "af_max" + to_string(i + 1);
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		AF_Item[i][3] = CT2A(lpTexts);

		AF_Data[i][1] = atoi(AF_Item[i][2].c_str());
		AF_Data[i][2] = atoi(AF_Item[i][3].c_str());
	}

	for (int i = 0; i < 10; i++)
		for (int k = 0; k < 4; k++) {
			string item = "AF_info" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 255, CA2CT(EEPROM_Map.c_str()));
			AF_info_Item[i][k] = CT2A(lpTexts);
		}

	for (int i = 0; i < 4; i++) {
		string item = "SFR_name" + to_string(i + 1);
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		SFR_Item[i][0] = CT2A(lpTexts);
		item = "SFR_grade" + to_string(i + 1);
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		SFR_Item[i][1] = CT2A(lpTexts);
		item = "SFR_start" + to_string(i + 1);
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		SFR_Item[i][2] = CT2A(lpTexts);
		item = "SFR_cnt" + to_string(i + 1);
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		SFR_Item[i][3] = CT2A(lpTexts);
	}

	for (int i = 0; i < 16; i++) 
		for (int k = 0; k < 3; k++) {
			string item = "PDAF_info" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			PDAF_info_Item[i][k] = CT2A(lpTexts);
	}

	for (int i = 0; i < 10; i++)
		for (int k = 0; k < 3; k++) {
			string item = "Fix_Data" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			Fix_Data_Item[i][k] = CT2A(lpTexts);
		}

	for (int i = 0; i < 10; i++) 
		for (int k = 0; k < 4; k++) {
			string item = "Gmap_Item" + to_string(i + 1) + to_string(k + 1);
			if (k < 3) {
				GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
				Gmap_Item[i][k] = CT2A(lpTexts);
			}
			else {
				Gmap_Item3[i]= GetPrivateProfileInt(_T("EEPROM_Address"), CA2CT(item.c_str()), 0, CA2CT(EEPROM_Map.c_str()));
			}
		}
	
	for (int i = 0; i < 14; i++)
		for (int k = 0; k < 3; k++) {
			string item = "PD_Item" + to_string(i + 1) + to_string(k + 1);
			if (k < 2) {
				GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
				PD_Item[i][k] = CT2A(lpTexts);
			}
			else {
				PD_Item3[i] = GetPrivateProfileInt(_T("EEPROM_Address"), CA2CT(item.c_str()), 0, CA2CT(EEPROM_Map.c_str()));
			}
		}

	for (int i = 0; i < 10; i++)
		for (int k = 0; k < 2; k++) {
			string item = "History_info" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			History_Data[i].item[k] = CT2A(lpTexts);
		}

	for (int i = 0; i < 24; i++)
		for (int k = 0; k < 4; k++) {
			string item = "OIS_info" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			OIS_info_Item[i][k] = CT2A(lpTexts);
		}

	for (int i = 0; i < 8; i++)
		for (int k = 0; k < 3; k++) {
			string item = "OIS_data" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			OIS_data_Item[i][k] = CT2A(lpTexts);
		}

	for (int i = 0; i < 12; i++) {
		string item = "AA_Item" + to_string(i + 1);
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		AA_Item[i] = CT2A(lpTexts);

	}

	for (int i = 0; i < 18; i++)
		for (int k = 0; k < 6; k++) {
			string item = "Value_data" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			sData_Item[i][k] = CT2A(lpTexts);
		}

	for (int i = 0; i < 20; i++)
		for (int k = 0; k < 3; k++) {
			string item = "Seg_info" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			Seg_Data[i].item[k] = CT2A(lpTexts);
		}

	for (int i = 0; i < 12; i++)
		for (int k = 0; k < 2; k++) {
			string item = "AEC_info" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			AEC_Data[i].item[k] = CT2A(lpTexts);
		}

	for (int i = 0; i < 1; i++)
		for (int k = 0; k < 2; k++) {
			string item = "QSC_info" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			QSC_Item.item[k] = CT2A(lpTexts);
		}

	for (int i = 0; i < 8; i++)
		for (int k = 0; k < 4; k++) {
			string item = "LSC_info" + to_string(i + 1) + to_string(k + 1);
			if (k < 3) {
				GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
				QC_LSC_Data[i].item[k] = CT2A(lpTexts);
			}
			else {
				LSC_Item3[i] = GetPrivateProfileInt(_T("EEPROM_Address"), CA2CT(item.c_str()), 0, CA2CT(EEPROM_Map.c_str()));
			}
		}

	for (int i = 0; i < 3; i++)
		for (int k = 0; k < 3; k++) {
			string item = "ZOOM_data" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			ZOOM_Item[i][k] = CT2A(lpTexts);
		}

	for (int k = 0; k < 8; k++) {
		string item = "Magnification_info" + to_string(k + 1);
		GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		Magnification[k] = CT2A(lpTexts);
	}

	for (int i = 0; i < 6; i++) {
		for (int k = 0; k < 3; k++) {
			string item = "Same_info" + to_string(i + 1) + to_string(k + 1);
			GetPrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
			Same_Item[i].item[k] = CT2A(lpTexts);
		}
		Same_Item[i].addr = unstringHex2int(Same_Item[i].item[1]);
		Same_Item[i].end = unstringHex2int(Same_Item[i].item[2]);
	}
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Year_H"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	product_Date.item[0]= CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Year_L"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	product_Date.item[1] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Month"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	product_Date.item[2] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Day"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	product_Date.item[3] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Hour"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	product_Date.item[4] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Minute"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	product_Date.item[5] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Second"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	product_Date.item[6] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Factory"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	product_Date.item[7] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Line"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	product_Date.item[8] = CT2A(lpTexts);

	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("QR_start"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	QR_Data.item[0] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("QR_end"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	QR_Data.item[1] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("QR_spec"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	QR_Data.item[2] = CT2A(lpTexts);

	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_start"), TEXT(""), lpTexts, 256, CA2CT(EEPROM_Map.c_str()));
	Fuse_Data.item[0] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_end"), TEXT(""), lpTexts, 256, CA2CT(EEPROM_Map.c_str()));
	Fuse_Data.item[1] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_spec"), TEXT(""), lpTexts, 256, CA2CT(EEPROM_Map.c_str()));
	Fuse_Data.item[2] = CT2A(lpTexts);

	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_start2"), TEXT(""), lpTexts, 256, CA2CT(EEPROM_Map.c_str()));
	Fuse_Data2.item[0] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_end2"), TEXT(""), lpTexts, 256, CA2CT(EEPROM_Map.c_str()));
	Fuse_Data2.item[1] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_spec2"), TEXT(""), lpTexts, 256, CA2CT(EEPROM_Map.c_str()));
	Fuse_Data2.item[2] = CT2A(lpTexts);

	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("MTK_LSC_Data11"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	MTK_LSC_Data[0].item[0] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("MTK_LSC_Data12"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	MTK_LSC_Data[0].item[1] = CT2A(lpTexts);

	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("MTK_LSC_Data21"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	MTK_LSC_Data[1].item[0] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("MTK_LSC_Data22"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	MTK_LSC_Data[1].item[1] = CT2A(lpTexts);

	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("shift_start"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	shift_Item[0] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("shift_byte"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	shift_Item[1] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("shift_point"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	shift_Item[2] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("shift_start_2"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	shift_Item[3] = CT2A(lpTexts);

	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("cross1"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	cross_Item[0] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("cross2"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	cross_Item[1] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("cross3"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	cross_Item[2] = CT2A(lpTexts);

	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("akm_cross1"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	akm_cross_Item[0] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("akm_cross2"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	akm_cross_Item[1] = CT2A(lpTexts);
	GetPrivateProfileString(TEXT("EEPROM_Address"), TEXT("akm_cross3"), TEXT(""), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	akm_cross_Item[2] = CT2A(lpTexts);


	/////////////////////////////////////////////////////////////////////////////////////////
	GetPrivateProfileString(TEXT("EEPROM_Set"), TEXT("validFlag"), TEXT("01"), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	Flag[0].dec = lptstrHex2int(lpTexts);
	str= CT2A(lpTexts);
	ui.flag_valid->setText(str.c_str());

	GetPrivateProfileString(TEXT("EEPROM_Set"), TEXT("invalidFlag"), TEXT("00"), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	Flag[1].dec = lptstrHex2int(lpTexts);
	str = CT2A(lpTexts);
	ui.flag_invalid->setText(str.c_str());

	GetPrivateProfileString(TEXT("EEPROM_Set"), TEXT("noData"), TEXT("FF"), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
	str = CT2A(lpTexts);
	dflt_Data=noData.dec = unstringHex2int(str);
	ui.noData->setText(str.c_str());

	SFR_Format=GetPrivateProfileInt(_T("EEPROM_Address"), TEXT("SFR_Format"), 0, CA2CT(EEPROM_Map.c_str()));

	HL = GetPrivateProfileInt(_T("EEPROM_Set"), TEXT("HL"), HL, CA2CT(EEPROM_Map.c_str()));
	checkDivisor = GetPrivateProfileInt(_T("EEPROM_Set"), TEXT("checkDivisor"), checkDivisor, CA2CT(EEPROM_Map.c_str()));

	DataFormat  = GetPrivateProfileInt(_T("EEPROM_Set"), TEXT("DataFormat"), DataFormat, CA2CT(EEPROM_Map.c_str()));
	selection = GetPrivateProfileInt(_T("EEPROM_Set"), TEXT("selection"), selection, CA2CT(EEPROM_Map.c_str()));
	first_Pixel = GetPrivateProfileInt(_T("EEPROM_Set"), TEXT("first_Pixel"), 0, CA2CT(EEPROM_Map.c_str()));
	duplicate_value_NG_ratio=GetPrivateProfileInt(_T("Spec_Set"), TEXT("duplicate_value_NG_ratio"), 50, CA2CT(EEPROM_Map.c_str()));

	LCC_CrossTalk[0] = GetPrivateProfileInt(_T("Spec_Set"), TEXT("LCC_CrossTalk_noMove"), 4, CA2CT(EEPROM_Map.c_str()));
	LCC_CrossTalk[1] = GetPrivateProfileInt(_T("Spec_Set"), TEXT("LCC_CrossTalk_MoveMin"), 12, CA2CT(EEPROM_Map.c_str()));
	LCC_CrossTalk[2] = GetPrivateProfileInt(_T("Spec_Set"), TEXT("LCC_CrossTalk_MoveMax"), 35, CA2CT(EEPROM_Map.c_str()));

	SR_Spec[0][0] = GetPrivateProfileInt(_T("Spec_Set"), TEXT("SR1_X_Spec"), 18, CA2CT(EEPROM_Map.c_str()));
	SR_Spec[0][1] = GetPrivateProfileInt(_T("Spec_Set"), TEXT("SR1_Y_Spec"), 18, CA2CT(EEPROM_Map.c_str()));

	SR_Spec[1][0] = GetPrivateProfileInt(_T("Spec_Set"), TEXT("SR2_X_Spec"), 18, CA2CT(EEPROM_Map.c_str()));
	SR_Spec[1][1] = GetPrivateProfileInt(_T("Spec_Set"), TEXT("SR2_Y_Spec"), 18, CA2CT(EEPROM_Map.c_str()));

	Gyro_offset_spec[0] = GetPrivateProfileInt(_T("Spec_Set"), TEXT("Gyro_Offset_Spec_X"), 3000, CA2CT(EEPROM_Map.c_str()));
	Gyro_offset_spec[1] = GetPrivateProfileInt(_T("Spec_Set"), TEXT("Gyro_Offset_Spec_Y"), 3000, CA2CT(EEPROM_Map.c_str()));

	int save_OnOff = GetPrivateProfileInt(_T("EEPROM_Set"), TEXT("save_OnOff"), 1, CA2CT(EEPROM_Map.c_str()));
	if (save_OnOff == 0) {
		ui.pushButton_setsave->setEnabled(false);
	}

	reserved_check = GetPrivateProfileInt(_T("EEPROM_Set"), TEXT("reserved_check"), 1, CA2CT(EEPROM_Map.c_str()));

	parameterDisplay();
}

 
void EEPROM_Data_Verifier::load_Panel() {

	modelSelect = ui.model->document()->toPlainText().toInt();
	SFR_Format = ui.SFR_Format->currentIndex();
	 
	if (ui.info_date->isChecked()) {
		selection |= 1;
	}
	else {
		selection &= 0xFFFFFFFF - 1;
	}
	if (ui.info_QR->isChecked()) {
		selection |= 2;
	}
	else {
		selection &= 0xFFFFFFFF - 2;
	}
	if (ui.info_fuse->isChecked()) {
		selection |= 4;
	}
	else {
		selection &= 0xFFFFFFFF - 4;
	}
	if (ui.gyro_dec->isChecked()) {
		selection |= 8;
	}
	else {
		selection &= 0xFFFFFFFF - 8;
	}
	if (ui.sr_hl->isChecked()) {
		selection |= 16;
	}
	else {
		selection &= 0xFFFFFFFF - 16;
	}
	if (ui.shift_HL->isChecked()) {
		selection |= 32;
	}
	else {
		selection &= 0xFFFFFFFF - 32;;
	}
	//if (ui.SFR_HEX->isChecked()) {
	//	selection |= 64;
	//}
	//else {
	//	selection &= 0xFFFFFFFF - 64;
	//}
	if (ui.checkBox_ZOOM->isChecked()) {
		selection |= 128;
	}
	else {
		selection &= 0xFFFFFFFF - 128;
	}
	if (ui.gyro_int->isChecked()) {
		selection |= 256;
	}
	else {
		selection &= 0xFFFFFFFF - 256;
	}
	if (ui.QC->isChecked()) {
		selection |= 512;
	}
	else {
		selection &= 0xFFFFFFFF - 512;
	}
	if (ui.MTK->isChecked()) {
		selection |= 1024;
	}
	else {
		selection &= 0xFFFFFFFF - 1024;
	}
	if (ui.LSI->isChecked()) {
		selection |= 2048;
	}
	else {
		selection &= 0xFFFFFFFF - 2048;
	}
	if (ui.QSC3->isChecked()) {
		selection |= 4096;
	}
	else {
		selection &= 0xFFFFFFFF - 4096;
	}
	if (ui.info_fuse_2->isChecked()) {
		selection |= 8192;
	}
	else {
		selection &= 0xFFFFFFFF - 8192;
	}
	if (ui.sr100->isChecked()) {
		selection |= 0x4000;
	}
	else {
		selection &= 0xFFFFFFFF - 0x4000;
	}
	if (ui.PD_Offset_LH->isChecked()) {
		selection |= 0x8000;
	}
	else {
		selection &= 0xFFFFFFFF - 0x8000;
	}
	if (ui.PD_offet_new->isChecked()) {
		selection |= 0x10000;
	} 
	else {
		selection &= 0xFFFFFFFF - 0x10000;
	}

	///////////////////////////////////DataFormat
	if (ui.oppo->isChecked()) {
		DataFormat = DataFormat / 10 + 1;
	}
	else if (ui.vivo->isChecked()) {
		DataFormat = DataFormat / 10 + 2;
	}
	else if (ui.xiaomi->isChecked()) {
		DataFormat = DataFormat / 10 + 3; 
	}
	else if (ui.sony->isChecked()) {
		DataFormat = DataFormat / 10 + 4;
	}
	else if (ui.SAMSUNG->isChecked()) {
		DataFormat = DataFormat / 10 + 5;
	}
	else if (ui.CS_other->isChecked()) {
		DataFormat = DataFormat / 10 + 6;
	}

	if (ui.R->isChecked()) {
		first_Pixel = 0;
	}
	else if (ui.Gr->isChecked()) {
		first_Pixel = 1;
	}
	else if (ui.Gb->isChecked()) {
		first_Pixel = 2;
	}
	else if (ui.B->isChecked()) {
		first_Pixel = 3;
	}

	if (ui.radioButton_HL->isChecked()) {
		HL |= 1;
	}else {
		HL &= 0xFFFFFFFF - 1;
	}

	if (ui.OIS_HL->isChecked()) {
		HL |= 2;
	}
	else {
		HL &= 0xFFFFFFFF - 2;
	}

	if (ui.SR_LH->isChecked()) {
		HL |= 4;
	}
	else {
		HL &= 0xFFFFFFFF - 4;
	}
	if (ui.PD_Offset_LH->isChecked()) {
		HL |= 8;
	}
	else {
		HL &= 0xFFFFFFFF - 8;
	}

	///////////////////Same_Item
	Same_Item[0].item[0] = string((const char *)ui.same11->document()->toPlainText().toLocal8Bit());
	Same_Item[0].item[1] = string((const char *)ui.same12->document()->toPlainText().toLocal8Bit());
	Same_Item[0].item[2] = string((const char *)ui.same13->document()->toPlainText().toLocal8Bit());

	Same_Item[1].item[0] = string((const char *)ui.same21->document()->toPlainText().toLocal8Bit());
	Same_Item[1].item[1] = string((const char *)ui.same22->document()->toPlainText().toLocal8Bit());
	Same_Item[1].item[2] = string((const char *)ui.same23->document()->toPlainText().toLocal8Bit());

	Same_Item[2].item[0] = string((const char *)ui.same31->document()->toPlainText().toLocal8Bit());
	Same_Item[2].item[1] = string((const char *)ui.same32->document()->toPlainText().toLocal8Bit());
	Same_Item[2].item[2] = string((const char *)ui.same33->document()->toPlainText().toLocal8Bit());

	Same_Item[3].item[0] = string((const char *)ui.same41->document()->toPlainText().toLocal8Bit());
	Same_Item[3].item[1] = string((const char *)ui.same42->document()->toPlainText().toLocal8Bit());
	Same_Item[3].item[2] = string((const char *)ui.same43->document()->toPlainText().toLocal8Bit());

	Same_Item[4].item[0] = string((const char *)ui.same51->document()->toPlainText().toLocal8Bit());
	Same_Item[4].item[1] = string((const char *)ui.same52->document()->toPlainText().toLocal8Bit());
	Same_Item[4].item[2] = string((const char *)ui.same53->document()->toPlainText().toLocal8Bit());

	Same_Item[5].item[0] = string((const char *)ui.same61->document()->toPlainText().toLocal8Bit());
	Same_Item[5].item[1] = string((const char *)ui.same62->document()->toPlainText().toLocal8Bit());
	Same_Item[5].item[2] = string((const char *)ui.same63->document()->toPlainText().toLocal8Bit());

	///////////////////ZOOM_data
	ZOOM_Item[0][0] = string((const char *)ui.ZOOM11->document()->toPlainText().toLocal8Bit());
	ZOOM_Item[0][1] = string((const char *)ui.ZOOM12->document()->toPlainText().toLocal8Bit());
	ZOOM_Item[0][2] = string((const char *)ui.ZOOM13->document()->toPlainText().toLocal8Bit());

	ZOOM_Item[1][0] = string((const char *)ui.ZOOM21->document()->toPlainText().toLocal8Bit());
	ZOOM_Item[1][1] = string((const char *)ui.ZOOM22->document()->toPlainText().toLocal8Bit());
	ZOOM_Item[1][2] = string((const char *)ui.ZOOM23->document()->toPlainText().toLocal8Bit());

	ZOOM_Item[2][0] = string((const char *)ui.ZOOM31->document()->toPlainText().toLocal8Bit());
	ZOOM_Item[2][1] = string((const char *)ui.ZOOM32->document()->toPlainText().toLocal8Bit());
	ZOOM_Item[2][2] = string((const char *)ui.ZOOM33->document()->toPlainText().toLocal8Bit());

	Magnification[0] = string((const char *)ui.Magnification1->document()->toPlainText().toLocal8Bit());
	Magnification[1] = string((const char *)ui.Magnification2->document()->toPlainText().toLocal8Bit());
	Magnification[2] = string((const char *)ui.Magnification3->document()->toPlainText().toLocal8Bit());
	Magnification[3] = string((const char *)ui.Magnification4->document()->toPlainText().toLocal8Bit());
	Magnification[4] = string((const char *)ui.Magnification5->document()->toPlainText().toLocal8Bit());
	Magnification[5] = string((const char *)ui.Magnification6->document()->toPlainText().toLocal8Bit());
	Magnification[6] = string((const char *)ui.Magnification7->document()->toPlainText().toLocal8Bit());
	Magnification[7] = string((const char *)ui.Magnification8->document()->toPlainText().toLocal8Bit());

	///////////////////History_data
	History_Data[0].item[0] = string((const char *)ui.history11->document()->toPlainText().toLocal8Bit());
	History_Data[0].item[1] = string((const char *)ui.history12->document()->toPlainText().toLocal8Bit());

	History_Data[1].item[0] = string((const char *)ui.history21->document()->toPlainText().toLocal8Bit());
	History_Data[1].item[1] = string((const char *)ui.history22->document()->toPlainText().toLocal8Bit());

	History_Data[2].item[0] = string((const char *)ui.history31->document()->toPlainText().toLocal8Bit());
	History_Data[2].item[1] = string((const char *)ui.history32->document()->toPlainText().toLocal8Bit());

	History_Data[3].item[0] = string((const char *)ui.history41->document()->toPlainText().toLocal8Bit());
	History_Data[3].item[1] = string((const char *)ui.history42->document()->toPlainText().toLocal8Bit());

	History_Data[4].item[0] = string((const char *)ui.history51->document()->toPlainText().toLocal8Bit());
	History_Data[4].item[1] = string((const char *)ui.history52->document()->toPlainText().toLocal8Bit());

	History_Data[5].item[0] = string((const char *)ui.history61->document()->toPlainText().toLocal8Bit());
	History_Data[5].item[1] = string((const char *)ui.history62->document()->toPlainText().toLocal8Bit());

	History_Data[6].item[0] = string((const char *)ui.history71->document()->toPlainText().toLocal8Bit());
	History_Data[6].item[1] = string((const char *)ui.history72->document()->toPlainText().toLocal8Bit());

	History_Data[7].item[0] = string((const char *)ui.history81->document()->toPlainText().toLocal8Bit());
	History_Data[7].item[1] = string((const char *)ui.history82->document()->toPlainText().toLocal8Bit());

	History_Data[8].item[0] = string((const char *)ui.history91->document()->toPlainText().toLocal8Bit());
	History_Data[8].item[1] = string((const char *)ui.history92->document()->toPlainText().toLocal8Bit());

	History_Data[9].item[0] = string((const char *)ui.history101->document()->toPlainText().toLocal8Bit());
	History_Data[9].item[1] = string((const char *)ui.history102->document()->toPlainText().toLocal8Bit());


	///////////////////AEC_data

	AEC_Data[0].item[0] = string((const char *)ui.AEC11->document()->toPlainText().toLocal8Bit());
	AEC_Data[0].item[1] = string((const char *)ui.AEC12->document()->toPlainText().toLocal8Bit());

	AEC_Data[1].item[0] = string((const char *)ui.AEC21->document()->toPlainText().toLocal8Bit());
	AEC_Data[1].item[1] = string((const char *)ui.AEC22->document()->toPlainText().toLocal8Bit());

	AEC_Data[2].item[0] = string((const char *)ui.AEC31->document()->toPlainText().toLocal8Bit());
	AEC_Data[2].item[1] = string((const char *)ui.AEC32->document()->toPlainText().toLocal8Bit());

	AEC_Data[3].item[0] = string((const char *)ui.AEC41->document()->toPlainText().toLocal8Bit());
	AEC_Data[3].item[1] = string((const char *)ui.AEC42->document()->toPlainText().toLocal8Bit());

	AEC_Data[4].item[0] = string((const char *)ui.AEC51->document()->toPlainText().toLocal8Bit());
	AEC_Data[4].item[1] = string((const char *)ui.AEC52->document()->toPlainText().toLocal8Bit());

	AEC_Data[5].item[0] = string((const char *)ui.AEC61->document()->toPlainText().toLocal8Bit());
	AEC_Data[5].item[1] = string((const char *)ui.AEC62->document()->toPlainText().toLocal8Bit());

	AEC_Data[6].item[0] = string((const char *)ui.AEC71->document()->toPlainText().toLocal8Bit());
	AEC_Data[6].item[1] = string((const char *)ui.AEC72->document()->toPlainText().toLocal8Bit());

	AEC_Data[7].item[0] = string((const char *)ui.AEC81->document()->toPlainText().toLocal8Bit());
	AEC_Data[7].item[1] = string((const char *)ui.AEC82->document()->toPlainText().toLocal8Bit());

	AEC_Data[8].item[0] = string((const char *)ui.AEC91->document()->toPlainText().toLocal8Bit());
	AEC_Data[8].item[1] = string((const char *)ui.AEC92->document()->toPlainText().toLocal8Bit());

	AEC_Data[9].item[0] = string((const char *)ui.AEC101->document()->toPlainText().toLocal8Bit());
	AEC_Data[9].item[1] = string((const char *)ui.AEC102->document()->toPlainText().toLocal8Bit());

	AEC_Data[10].item[0] = string((const char *)ui.AEC111->document()->toPlainText().toLocal8Bit());
	AEC_Data[10].item[1] = string((const char *)ui.AEC112->document()->toPlainText().toLocal8Bit());

	AEC_Data[11].item[0] = string((const char *)ui.AEC121->document()->toPlainText().toLocal8Bit());
	AEC_Data[11].item[1] = string((const char *)ui.AEC122->document()->toPlainText().toLocal8Bit());

	///////////////////QSC_Item
	QSC_Item.item[0] = string((const char *)ui.QSC11->document()->toPlainText().toLocal8Bit());
	QSC_Item.item[1] = string((const char *)ui.QSC12->document()->toPlainText().toLocal8Bit());

	///////////////////Seg_data
	Seg_Data[0].item[0] = string((const char *)ui.seg11->document()->toPlainText().toLocal8Bit());
	Seg_Data[0].item[1] = string((const char *)ui.seg12->document()->toPlainText().toLocal8Bit());
	Seg_Data[0].item[2] = string((const char *)ui.seg13->document()->toPlainText().toLocal8Bit());

	Seg_Data[1].item[0] = string((const char *)ui.seg21->document()->toPlainText().toLocal8Bit());
	Seg_Data[1].item[1] = string((const char *)ui.seg22->document()->toPlainText().toLocal8Bit());
	Seg_Data[1].item[2] = string((const char *)ui.seg23->document()->toPlainText().toLocal8Bit());

	Seg_Data[2].item[0] = string((const char *)ui.seg31->document()->toPlainText().toLocal8Bit());
	Seg_Data[2].item[1] = string((const char *)ui.seg32->document()->toPlainText().toLocal8Bit());
	Seg_Data[2].item[2] = string((const char *)ui.seg33->document()->toPlainText().toLocal8Bit());

	Seg_Data[3].item[0] = string((const char *)ui.seg41->document()->toPlainText().toLocal8Bit());
	Seg_Data[3].item[1] = string((const char *)ui.seg42->document()->toPlainText().toLocal8Bit());
	Seg_Data[3].item[2] = string((const char *)ui.seg43->document()->toPlainText().toLocal8Bit());

	Seg_Data[4].item[0] = string((const char *)ui.seg51->document()->toPlainText().toLocal8Bit());
	Seg_Data[4].item[1] = string((const char *)ui.seg52->document()->toPlainText().toLocal8Bit());
	Seg_Data[4].item[2] = string((const char *)ui.seg53->document()->toPlainText().toLocal8Bit());

	Seg_Data[5].item[0] = string((const char *)ui.seg61->document()->toPlainText().toLocal8Bit());
	Seg_Data[5].item[1] = string((const char *)ui.seg62->document()->toPlainText().toLocal8Bit());
	Seg_Data[5].item[2] = string((const char *)ui.seg63->document()->toPlainText().toLocal8Bit());

	Seg_Data[6].item[0] = string((const char *)ui.seg71->document()->toPlainText().toLocal8Bit());
	Seg_Data[6].item[1] = string((const char *)ui.seg72->document()->toPlainText().toLocal8Bit());
	Seg_Data[6].item[2] = string((const char *)ui.seg73->document()->toPlainText().toLocal8Bit());

	Seg_Data[7].item[0] = string((const char *)ui.seg81->document()->toPlainText().toLocal8Bit());
	Seg_Data[7].item[1] = string((const char *)ui.seg82->document()->toPlainText().toLocal8Bit());
	Seg_Data[7].item[2] = string((const char *)ui.seg83->document()->toPlainText().toLocal8Bit());

	Seg_Data[8].item[0] = string((const char *)ui.seg91->document()->toPlainText().toLocal8Bit());
	Seg_Data[8].item[1] = string((const char *)ui.seg92->document()->toPlainText().toLocal8Bit());
	Seg_Data[8].item[2] = string((const char *)ui.seg93->document()->toPlainText().toLocal8Bit());

	Seg_Data[9].item[0] = string((const char *)ui.seg101->document()->toPlainText().toLocal8Bit());
	Seg_Data[9].item[1] = string((const char *)ui.seg102->document()->toPlainText().toLocal8Bit());
	Seg_Data[9].item[2] = string((const char *)ui.seg103->document()->toPlainText().toLocal8Bit());

	Seg_Data[10].item[0] = string((const char *)ui.seg111->document()->toPlainText().toLocal8Bit());
	Seg_Data[10].item[1] = string((const char *)ui.seg112->document()->toPlainText().toLocal8Bit());
	Seg_Data[10].item[2] = string((const char *)ui.seg113->document()->toPlainText().toLocal8Bit());

	Seg_Data[11].item[0] = string((const char *)ui.seg121->document()->toPlainText().toLocal8Bit());
	Seg_Data[11].item[1] = string((const char *)ui.seg122->document()->toPlainText().toLocal8Bit());
	Seg_Data[11].item[2] = string((const char *)ui.seg123->document()->toPlainText().toLocal8Bit());

	Seg_Data[12].item[0] = string((const char *)ui.seg131->document()->toPlainText().toLocal8Bit());
	Seg_Data[12].item[1] = string((const char *)ui.seg132->document()->toPlainText().toLocal8Bit());
	Seg_Data[12].item[2] = string((const char *)ui.seg133->document()->toPlainText().toLocal8Bit());

	Seg_Data[13].item[0] = string((const char *)ui.seg141->document()->toPlainText().toLocal8Bit());
	Seg_Data[13].item[1] = string((const char *)ui.seg142->document()->toPlainText().toLocal8Bit());
	Seg_Data[13].item[2] = string((const char *)ui.seg143->document()->toPlainText().toLocal8Bit());

	Seg_Data[14].item[0] = string((const char *)ui.seg151->document()->toPlainText().toLocal8Bit());
	Seg_Data[14].item[1] = string((const char *)ui.seg152->document()->toPlainText().toLocal8Bit());
	Seg_Data[14].item[2] = string((const char *)ui.seg153->document()->toPlainText().toLocal8Bit());

	Seg_Data[15].item[0] = string((const char *)ui.seg161->document()->toPlainText().toLocal8Bit());
	Seg_Data[15].item[1] = string((const char *)ui.seg162->document()->toPlainText().toLocal8Bit());
	Seg_Data[15].item[2] = string((const char *)ui.seg163->document()->toPlainText().toLocal8Bit());

	Seg_Data[16].item[0] = string((const char *)ui.seg171->document()->toPlainText().toLocal8Bit());
	Seg_Data[16].item[1] = string((const char *)ui.seg172->document()->toPlainText().toLocal8Bit());
	Seg_Data[16].item[2] = string((const char *)ui.seg173->document()->toPlainText().toLocal8Bit());

	Seg_Data[17].item[0] = string((const char *)ui.seg181->document()->toPlainText().toLocal8Bit());
	Seg_Data[17].item[1] = string((const char *)ui.seg182->document()->toPlainText().toLocal8Bit());
	Seg_Data[17].item[2] = string((const char *)ui.seg183->document()->toPlainText().toLocal8Bit());

	Seg_Data[18].item[0] = string((const char *)ui.seg191->document()->toPlainText().toLocal8Bit());
	Seg_Data[18].item[1] = string((const char *)ui.seg192->document()->toPlainText().toLocal8Bit());
	Seg_Data[18].item[2] = string((const char *)ui.seg193->document()->toPlainText().toLocal8Bit());

	Seg_Data[19].item[0] = string((const char *)ui.seg201->document()->toPlainText().toLocal8Bit());
	Seg_Data[19].item[1] = string((const char *)ui.seg202->document()->toPlainText().toLocal8Bit());
	Seg_Data[19].item[2] = string((const char *)ui.seg203->document()->toPlainText().toLocal8Bit());

	///////////////////AA_data
	AA_Item[0] = string((const char *)ui.AA_data1->document()->toPlainText().toLocal8Bit());
	AA_Item[1] = string((const char *)ui.AA_data2->document()->toPlainText().toLocal8Bit());
	AA_Item[2] = string((const char *)ui.AA_data3->document()->toPlainText().toLocal8Bit());
	AA_Item[3] = string((const char *)ui.AA_data4->document()->toPlainText().toLocal8Bit());
	AA_Item[4] = string((const char *)ui.AA_data5->document()->toPlainText().toLocal8Bit());
	AA_Item[5] = string((const char *)ui.AA_data6->document()->toPlainText().toLocal8Bit());
	AA_Item[6] = string((const char *)ui.AA_data7->document()->toPlainText().toLocal8Bit());
	AA_Item[7] = string((const char *)ui.AA_data8->document()->toPlainText().toLocal8Bit());
	AA_Item[8] = string((const char *)ui.AA_data9->document()->toPlainText().toLocal8Bit());
	AA_Item[9] = string((const char *)ui.AA_data10->document()->toPlainText().toLocal8Bit());
	AA_Item[10] = string((const char *)ui.AA_data11->document()->toPlainText().toLocal8Bit());
	AA_Item[11] = string((const char *)ui.AA_data12->document()->toPlainText().toLocal8Bit());

	///////////////////Value_data
	sData_Item[0][0] = string((const char *)ui.sData_item11->document()->toPlainText().toLocal8Bit());
	sData_Item[0][1] = string((const char *)ui.sData_item12->document()->toPlainText().toLocal8Bit());
	sData_Item[0][2] = string((const char *)ui.sData_item13->document()->toPlainText().toLocal8Bit());
	sData_Item[0][3] = string((const char *)ui.sData_item14->document()->toPlainText().toLocal8Bit());
	sData_Item[0][4] = string((const char *)ui.sData_item15->document()->toPlainText().toLocal8Bit());
	sData_Item[0][5] = string((const char *)ui.sData_item16->document()->toPlainText().toLocal8Bit());

	sData_Item[1][0] = string((const char *)ui.sData_item21->document()->toPlainText().toLocal8Bit());
	sData_Item[1][1] = string((const char *)ui.sData_item22->document()->toPlainText().toLocal8Bit());
	sData_Item[1][2] = string((const char *)ui.sData_item23->document()->toPlainText().toLocal8Bit());
	sData_Item[1][3] = string((const char *)ui.sData_item24->document()->toPlainText().toLocal8Bit());
	sData_Item[1][4] = string((const char *)ui.sData_item25->document()->toPlainText().toLocal8Bit());
	sData_Item[1][5] = string((const char *)ui.sData_item26->document()->toPlainText().toLocal8Bit());

	sData_Item[2][0] = string((const char *)ui.sData_item31->document()->toPlainText().toLocal8Bit());
	sData_Item[2][1] = string((const char *)ui.sData_item32->document()->toPlainText().toLocal8Bit());
	sData_Item[2][2] = string((const char *)ui.sData_item33->document()->toPlainText().toLocal8Bit());
	sData_Item[2][3] = string((const char *)ui.sData_item34->document()->toPlainText().toLocal8Bit());
	sData_Item[2][4] = string((const char *)ui.sData_item35->document()->toPlainText().toLocal8Bit());
	sData_Item[2][5] = string((const char *)ui.sData_item36->document()->toPlainText().toLocal8Bit());

	sData_Item[3][0] = string((const char *)ui.sData_item41->document()->toPlainText().toLocal8Bit());
	sData_Item[3][1] = string((const char *)ui.sData_item42->document()->toPlainText().toLocal8Bit());
	sData_Item[3][2] = string((const char *)ui.sData_item43->document()->toPlainText().toLocal8Bit());
	sData_Item[3][3] = string((const char *)ui.sData_item44->document()->toPlainText().toLocal8Bit());
	sData_Item[3][4] = string((const char *)ui.sData_item45->document()->toPlainText().toLocal8Bit());
	sData_Item[3][5] = string((const char *)ui.sData_item46->document()->toPlainText().toLocal8Bit());

	sData_Item[4][0] = string((const char *)ui.sData_item51->document()->toPlainText().toLocal8Bit());
	sData_Item[4][1] = string((const char *)ui.sData_item52->document()->toPlainText().toLocal8Bit());
	sData_Item[4][2] = string((const char *)ui.sData_item53->document()->toPlainText().toLocal8Bit());
	sData_Item[4][3] = string((const char *)ui.sData_item54->document()->toPlainText().toLocal8Bit());
	sData_Item[4][4] = string((const char *)ui.sData_item55->document()->toPlainText().toLocal8Bit());
	sData_Item[4][5] = string((const char *)ui.sData_item56->document()->toPlainText().toLocal8Bit());

	sData_Item[5][0] = string((const char *)ui.sData_item61->document()->toPlainText().toLocal8Bit());
	sData_Item[5][1] = string((const char *)ui.sData_item62->document()->toPlainText().toLocal8Bit());
	sData_Item[5][2] = string((const char *)ui.sData_item63->document()->toPlainText().toLocal8Bit());
	sData_Item[5][3] = string((const char *)ui.sData_item64->document()->toPlainText().toLocal8Bit());
	sData_Item[5][4] = string((const char *)ui.sData_item65->document()->toPlainText().toLocal8Bit());
	sData_Item[5][5] = string((const char *)ui.sData_item66->document()->toPlainText().toLocal8Bit());

	sData_Item[6][0] = string((const char *)ui.sData_item71->document()->toPlainText().toLocal8Bit());
	sData_Item[6][1] = string((const char *)ui.sData_item72->document()->toPlainText().toLocal8Bit());
	sData_Item[6][2] = string((const char *)ui.sData_item73->document()->toPlainText().toLocal8Bit());
	sData_Item[6][3] = string((const char *)ui.sData_item74->document()->toPlainText().toLocal8Bit());
	sData_Item[6][4] = string((const char *)ui.sData_item75->document()->toPlainText().toLocal8Bit());
	sData_Item[6][5] = string((const char *)ui.sData_item76->document()->toPlainText().toLocal8Bit());

	sData_Item[7][0] = string((const char *)ui.sData_item81->document()->toPlainText().toLocal8Bit());
	sData_Item[7][1] = string((const char *)ui.sData_item82->document()->toPlainText().toLocal8Bit());
	sData_Item[7][2] = string((const char *)ui.sData_item83->document()->toPlainText().toLocal8Bit());
	sData_Item[7][3] = string((const char *)ui.sData_item84->document()->toPlainText().toLocal8Bit());
	sData_Item[7][4] = string((const char *)ui.sData_item85->document()->toPlainText().toLocal8Bit());
	sData_Item[7][5] = string((const char *)ui.sData_item86->document()->toPlainText().toLocal8Bit());

	sData_Item[8][0] = string((const char *)ui.sData_item91->document()->toPlainText().toLocal8Bit());
	sData_Item[8][1] = string((const char *)ui.sData_item92->document()->toPlainText().toLocal8Bit());
	sData_Item[8][2] = string((const char *)ui.sData_item93->document()->toPlainText().toLocal8Bit());
	sData_Item[8][3] = string((const char *)ui.sData_item94->document()->toPlainText().toLocal8Bit());
	sData_Item[8][4] = string((const char *)ui.sData_item95->document()->toPlainText().toLocal8Bit());
	sData_Item[8][5] = string((const char *)ui.sData_item96->document()->toPlainText().toLocal8Bit());

	sData_Item[9][0] = string((const char *)ui.sData_item101->document()->toPlainText().toLocal8Bit());
	sData_Item[9][1] = string((const char *)ui.sData_item102->document()->toPlainText().toLocal8Bit());
	sData_Item[9][2] = string((const char *)ui.sData_item103->document()->toPlainText().toLocal8Bit());
	sData_Item[9][3] = string((const char *)ui.sData_item104->document()->toPlainText().toLocal8Bit());
	sData_Item[9][4] = string((const char *)ui.sData_item105->document()->toPlainText().toLocal8Bit());
	sData_Item[9][5] = string((const char *)ui.sData_item106->document()->toPlainText().toLocal8Bit());

	sData_Item[10][0] = string((const char *)ui.sData_item111->document()->toPlainText().toLocal8Bit());
	sData_Item[10][1] = string((const char *)ui.sData_item112->document()->toPlainText().toLocal8Bit());
	sData_Item[10][2] = string((const char *)ui.sData_item113->document()->toPlainText().toLocal8Bit());
	sData_Item[10][3] = string((const char *)ui.sData_item114->document()->toPlainText().toLocal8Bit());
	sData_Item[10][4] = string((const char *)ui.sData_item115->document()->toPlainText().toLocal8Bit());
	sData_Item[10][5] = string((const char *)ui.sData_item116->document()->toPlainText().toLocal8Bit());

	sData_Item[11][0] = string((const char *)ui.sData_item121->document()->toPlainText().toLocal8Bit());
	sData_Item[11][1] = string((const char *)ui.sData_item122->document()->toPlainText().toLocal8Bit());
	sData_Item[11][2] = string((const char *)ui.sData_item123->document()->toPlainText().toLocal8Bit());
	sData_Item[11][3] = string((const char *)ui.sData_item124->document()->toPlainText().toLocal8Bit());
	sData_Item[11][4] = string((const char *)ui.sData_item125->document()->toPlainText().toLocal8Bit());
	sData_Item[11][5] = string((const char *)ui.sData_item126->document()->toPlainText().toLocal8Bit());

	sData_Item[12][0] = string((const char *)ui.sData_item131->document()->toPlainText().toLocal8Bit());
	sData_Item[12][1] = string((const char *)ui.sData_item132->document()->toPlainText().toLocal8Bit());
	sData_Item[12][2] = string((const char *)ui.sData_item133->document()->toPlainText().toLocal8Bit());
	sData_Item[12][3] = string((const char *)ui.sData_item134->document()->toPlainText().toLocal8Bit());
	sData_Item[12][4] = string((const char *)ui.sData_item135->document()->toPlainText().toLocal8Bit());
	sData_Item[12][5] = string((const char *)ui.sData_item136->document()->toPlainText().toLocal8Bit());

	sData_Item[13][0] = string((const char *)ui.sData_item141->document()->toPlainText().toLocal8Bit());
	sData_Item[13][1] = string((const char *)ui.sData_item142->document()->toPlainText().toLocal8Bit());
	sData_Item[13][2] = string((const char *)ui.sData_item143->document()->toPlainText().toLocal8Bit());
	sData_Item[13][3] = string((const char *)ui.sData_item144->document()->toPlainText().toLocal8Bit());
	sData_Item[13][4] = string((const char *)ui.sData_item145->document()->toPlainText().toLocal8Bit());
	sData_Item[13][5] = string((const char *)ui.sData_item146->document()->toPlainText().toLocal8Bit());

	sData_Item[14][0] = string((const char *)ui.sData_item151->document()->toPlainText().toLocal8Bit());
	sData_Item[14][1] = string((const char *)ui.sData_item152->document()->toPlainText().toLocal8Bit());
	sData_Item[14][2] = string((const char *)ui.sData_item153->document()->toPlainText().toLocal8Bit());
	sData_Item[14][3] = string((const char *)ui.sData_item154->document()->toPlainText().toLocal8Bit());
	sData_Item[14][4] = string((const char *)ui.sData_item155->document()->toPlainText().toLocal8Bit());
	sData_Item[14][5] = string((const char *)ui.sData_item156->document()->toPlainText().toLocal8Bit());

	sData_Item[15][0] = string((const char *)ui.sData_item161->document()->toPlainText().toLocal8Bit());
	sData_Item[15][1] = string((const char *)ui.sData_item162->document()->toPlainText().toLocal8Bit());
	sData_Item[15][2] = string((const char *)ui.sData_item163->document()->toPlainText().toLocal8Bit());
	sData_Item[15][3] = string((const char *)ui.sData_item164->document()->toPlainText().toLocal8Bit());
	sData_Item[15][4] = string((const char *)ui.sData_item165->document()->toPlainText().toLocal8Bit());
	sData_Item[15][5] = string((const char *)ui.sData_item166->document()->toPlainText().toLocal8Bit());

	sData_Item[16][0] = string((const char *)ui.sData_item171->document()->toPlainText().toLocal8Bit());
	sData_Item[16][1] = string((const char *)ui.sData_item172->document()->toPlainText().toLocal8Bit());
	sData_Item[16][2] = string((const char *)ui.sData_item173->document()->toPlainText().toLocal8Bit());
	sData_Item[16][3] = string((const char *)ui.sData_item174->document()->toPlainText().toLocal8Bit());
	sData_Item[16][4] = string((const char *)ui.sData_item175->document()->toPlainText().toLocal8Bit());
	sData_Item[16][5] = string((const char *)ui.sData_item176->document()->toPlainText().toLocal8Bit());

	sData_Item[17][0] = string((const char *)ui.sData_item181->document()->toPlainText().toLocal8Bit());
	sData_Item[17][1] = string((const char *)ui.sData_item182->document()->toPlainText().toLocal8Bit());
	sData_Item[17][2] = string((const char *)ui.sData_item183->document()->toPlainText().toLocal8Bit());
	sData_Item[17][3] = string((const char *)ui.sData_item184->document()->toPlainText().toLocal8Bit());
	sData_Item[17][4] = string((const char *)ui.sData_item185->document()->toPlainText().toLocal8Bit());
	sData_Item[17][5] = string((const char *)ui.sData_item186->document()->toPlainText().toLocal8Bit());

	///////////////////OIS_data
	OIS_data_Item[0][0] = string((const char *)ui.OIS_data11->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[0][1] = string((const char *)ui.OIS_data12->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[0][2] = string((const char *)ui.OIS_data13->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[1][0] = string((const char *)ui.OIS_data21->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[1][1] = string((const char *)ui.OIS_data22->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[1][2] = string((const char *)ui.OIS_data23->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[2][0] = string((const char *)ui.OIS_data31->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[2][1] = string((const char *)ui.OIS_data32->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[2][2] = string((const char *)ui.OIS_data33->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[3][0] = string((const char *)ui.OIS_data41->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[3][1] = string((const char *)ui.OIS_data42->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[3][2] = string((const char *)ui.OIS_data43->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[4][0] = string((const char *)ui.OIS_data51->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[4][1] = string((const char *)ui.OIS_data52->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[5][0] = string((const char *)ui.OIS_data61->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[5][1] = string((const char *)ui.OIS_data62->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[6][0] = string((const char *)ui.OIS_data71->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[6][1] = string((const char *)ui.OIS_data72->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[7][0] = string((const char *)ui.OIS_data81->document()->toPlainText().toLocal8Bit());
	OIS_data_Item[7][1] = string((const char *)ui.OIS_data82->document()->toPlainText().toLocal8Bit());

	///////////////////Fix_Value
	Fix_Data_Item[0][0] = string((const char *)ui.fix11->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[0][1] = string((const char *)ui.fix12->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[0][2] = string((const char *)ui.fix13->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[1][0] = string((const char *)ui.fix21->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[1][1] = string((const char *)ui.fix22->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[1][2] = string((const char *)ui.fix23->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[2][0] = string((const char *)ui.fix31->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[2][1] = string((const char *)ui.fix32->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[2][2] = string((const char *)ui.fix33->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[3][0] = string((const char *)ui.fix41->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[3][1] = string((const char *)ui.fix42->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[3][2] = string((const char *)ui.fix43->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[4][0] = string((const char *)ui.fix51->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[4][1] = string((const char *)ui.fix52->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[4][2] = string((const char *)ui.fix53->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[5][0] = string((const char *)ui.fix61->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[5][1] = string((const char *)ui.fix62->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[5][2] = string((const char *)ui.fix63->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[6][0] = string((const char *)ui.fix71->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[6][1] = string((const char *)ui.fix72->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[6][2] = string((const char *)ui.fix73->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[7][0] = string((const char *)ui.fix81->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[7][1] = string((const char *)ui.fix82->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[7][2] = string((const char *)ui.fix83->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[8][0] = string((const char *)ui.fix91->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[8][1] = string((const char *)ui.fix92->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[8][2] = string((const char *)ui.fix93->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[9][0] = string((const char *)ui.fix101->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[9][1] = string((const char *)ui.fix102->document()->toPlainText().toLocal8Bit());
	Fix_Data_Item[9][2] = string((const char *)ui.fix103->document()->toPlainText().toLocal8Bit());

	///////////////////OIS_info
	OIS_info_Item[0][0] = string((const char *)ui.OIS_info11->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[0][1] = string((const char *)ui.OIS_info12->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[0][2] = string((const char *)ui.OIS_info13->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[0][3] = string((const char *)ui.OIS_info14->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[1][0] = string((const char *)ui.OIS_info21->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[1][1] = string((const char *)ui.OIS_info22->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[1][2] = string((const char *)ui.OIS_info23->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[1][3] = string((const char *)ui.OIS_info24->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[2][0] = string((const char *)ui.OIS_info31->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[2][1] = string((const char *)ui.OIS_info32->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[2][2] = string((const char *)ui.OIS_info33->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[2][3] = string((const char *)ui.OIS_info34->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[3][0] = string((const char *)ui.OIS_info41->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[3][1] = string((const char *)ui.OIS_info42->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[3][2] = string((const char *)ui.OIS_info43->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[3][3] = string((const char *)ui.OIS_info44->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[4][0] = string((const char *)ui.OIS_info51->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[4][1] = string((const char *)ui.OIS_info52->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[4][2] = string((const char *)ui.OIS_info53->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[4][3] = string((const char *)ui.OIS_info54->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[5][0] = string((const char *)ui.OIS_info61->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[5][1] = string((const char *)ui.OIS_info62->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[5][2] = string((const char *)ui.OIS_info63->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[5][3] = string((const char *)ui.OIS_info64->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[6][0] = string((const char *)ui.OIS_info71->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[6][1] = string((const char *)ui.OIS_info72->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[6][2] = string((const char *)ui.OIS_info73->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[6][3] = string((const char *)ui.OIS_info74->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[7][0] = string((const char *)ui.OIS_info81->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[7][1] = string((const char *)ui.OIS_info82->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[7][2] = string((const char *)ui.OIS_info83->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[7][3] = string((const char *)ui.OIS_info84->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[8][0] = string((const char *)ui.OIS_info91->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[8][1] = string((const char *)ui.OIS_info92->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[8][2] = string((const char *)ui.OIS_info93->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[8][3] = string((const char *)ui.OIS_info94->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[9][0] = string((const char *)ui.OIS_info101->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[9][1] = string((const char *)ui.OIS_info102->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[9][2] = string((const char *)ui.OIS_info103->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[9][3] = string((const char *)ui.OIS_info104->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[10][0] = string((const char *)ui.OIS_info111->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[10][1] = string((const char *)ui.OIS_info112->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[10][2] = string((const char *)ui.OIS_info113->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[10][3] = string((const char *)ui.OIS_info114->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[11][0] = string((const char *)ui.OIS_info121->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[11][1] = string((const char *)ui.OIS_info122->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[11][2] = string((const char *)ui.OIS_info123->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[11][3] = string((const char *)ui.OIS_info124->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[12][0] = string((const char *)ui.OIS_info131->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[12][1] = string((const char *)ui.OIS_info132->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[12][2] = string((const char *)ui.OIS_info133->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[12][3] = string((const char *)ui.OIS_info134->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[13][0] = string((const char *)ui.OIS_info141->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[13][1] = string((const char *)ui.OIS_info142->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[13][2] = string((const char *)ui.OIS_info143->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[13][3] = string((const char *)ui.OIS_info144->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[14][0] = string((const char *)ui.OIS_info151->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[14][1] = string((const char *)ui.OIS_info152->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[14][2] = string((const char *)ui.OIS_info153->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[14][3] = string((const char *)ui.OIS_info154->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[15][0] = string((const char *)ui.OIS_info161->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[15][1] = string((const char *)ui.OIS_info162->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[15][2] = string((const char *)ui.OIS_info163->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[15][3] = string((const char *)ui.OIS_info164->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[16][0] = string((const char *)ui.OIS_info171->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[16][1] = string((const char *)ui.OIS_info172->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[16][2] = string((const char *)ui.OIS_info173->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[16][3] = string((const char *)ui.OIS_info174->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[17][0] = string((const char *)ui.OIS_info181->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[17][1] = string((const char *)ui.OIS_info182->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[17][2] = string((const char *)ui.OIS_info183->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[17][3] = string((const char *)ui.OIS_info184->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[18][0] = string((const char *)ui.OIS_info191->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[18][1] = string((const char *)ui.OIS_info192->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[18][2] = string((const char *)ui.OIS_info193->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[18][3] = string((const char *)ui.OIS_info194->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[19][0] = string((const char *)ui.OIS_info201->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[19][1] = string((const char *)ui.OIS_info202->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[19][2] = string((const char *)ui.OIS_info203->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[19][3] = string((const char *)ui.OIS_info204->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[20][0] = string((const char *)ui.OIS_info211->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[20][1] = string((const char *)ui.OIS_info212->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[20][2] = string((const char *)ui.OIS_info213->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[20][3] = string((const char *)ui.OIS_info214->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[21][0] = string((const char *)ui.OIS_info221->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[21][1] = string((const char *)ui.OIS_info222->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[21][2] = string((const char *)ui.OIS_info223->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[21][3] = string((const char *)ui.OIS_info224->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[22][0] = string((const char *)ui.OIS_info231->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[22][1] = string((const char *)ui.OIS_info232->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[22][2] = string((const char *)ui.OIS_info233->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[22][3] = string((const char *)ui.OIS_info234->document()->toPlainText().toLocal8Bit());

	OIS_info_Item[23][0] = string((const char *)ui.OIS_info241->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[23][1] = string((const char *)ui.OIS_info242->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[23][2] = string((const char *)ui.OIS_info243->document()->toPlainText().toLocal8Bit());
	OIS_info_Item[23][3] = string((const char *)ui.OIS_info244->document()->toPlainText().toLocal8Bit());

	///////////////////DCC_Data
	PD_Item[0][0] = string((const char *)ui.DCC11->document()->toPlainText().toLocal8Bit());
	PD_Item[0][1] = string((const char *)ui.DCC12->document()->toPlainText().toLocal8Bit());
//	PD_Item[0][2] = string((const char *)ui.DCC13->document()->toPlainText().toLocal8Bit());
	PD_Item3[0] = ui.DCC13->currentIndex();
	PD_Item[1][0] = string((const char *)ui.DCC21->document()->toPlainText().toLocal8Bit());
	PD_Item[1][1] = string((const char *)ui.DCC22->document()->toPlainText().toLocal8Bit());
//	PD_Item[1][2] = string((const char *)ui.DCC23->document()->toPlainText().toLocal8Bit());
	PD_Item3[1] = ui.DCC23->currentIndex();
	PD_Item[2][0] = string((const char *)ui.DCC31->document()->toPlainText().toLocal8Bit());
	PD_Item[2][1] = string((const char *)ui.DCC32->document()->toPlainText().toLocal8Bit());
//	PD_Item[2][2] = string((const char *)ui.DCC33->document()->toPlainText().toLocal8Bit());
	PD_Item3[2] = ui.DCC33->currentIndex();
	PD_Item[3][0] = string((const char *)ui.DCC41->document()->toPlainText().toLocal8Bit());
	PD_Item[3][1] = string((const char *)ui.DCC42->document()->toPlainText().toLocal8Bit());
//	PD_Item[3][2] = string((const char *)ui.DCC43->document()->toPlainText().toLocal8Bit());
	PD_Item3[3] = ui.DCC43->currentIndex();
	PD_Item[4][0] = string((const char *)ui.DCC51->document()->toPlainText().toLocal8Bit());
	PD_Item[4][1] = string((const char *)ui.DCC52->document()->toPlainText().toLocal8Bit());
//	PD_Item[4][2] = string((const char *)ui.DCC53->document()->toPlainText().toLocal8Bit());
	PD_Item3[4] = ui.DCC53->currentIndex();
	PD_Item[5][0] = string((const char *)ui.DCC61->document()->toPlainText().toLocal8Bit());
	PD_Item[5][1] = string((const char *)ui.DCC62->document()->toPlainText().toLocal8Bit());
//	PD_Item[5][2] = string((const char *)ui.DCC63->document()->toPlainText().toLocal8Bit());
	PD_Item3[5] = ui.DCC63->currentIndex();
	PD_Item[6][0] = string((const char *)ui.DCC71->document()->toPlainText().toLocal8Bit());
	PD_Item[6][1] = string((const char *)ui.DCC72->document()->toPlainText().toLocal8Bit());
//	PD_Item[6][2] = string((const char *)ui.DCC73->document()->toPlainText().toLocal8Bit());
	PD_Item3[6] = ui.DCC73->currentIndex();
	PD_Item[7][0] = string((const char *)ui.DCC81->document()->toPlainText().toLocal8Bit());
	PD_Item[7][1] = string((const char *)ui.DCC82->document()->toPlainText().toLocal8Bit());
//	PD_Item[7][2] = string((const char *)ui.DCC83->document()->toPlainText().toLocal8Bit());
	PD_Item3[7] = ui.DCC83->currentIndex();
	PD_Item[8][0] = string((const char *)ui.DCC91->document()->toPlainText().toLocal8Bit());
	PD_Item[8][1] = string((const char *)ui.DCC92->document()->toPlainText().toLocal8Bit());
//	PD_Item[8][2] = string((const char *)ui.DCC93->document()->toPlainText().toLocal8Bit());
	PD_Item3[8] = ui.DCC93->currentIndex();
	PD_Item[9][0] = string((const char *)ui.DCC101->document()->toPlainText().toLocal8Bit());
	PD_Item[9][1] = string((const char *)ui.DCC102->document()->toPlainText().toLocal8Bit());
//	PD_Item[9][2] = string((const char *)ui.DCC103->document()->toPlainText().toLocal8Bit());
	PD_Item3[9] = ui.DCC103->currentIndex();
	PD_Item[10][0] = string((const char *)ui.DCC111->document()->toPlainText().toLocal8Bit());
	PD_Item[10][1] = string((const char *)ui.DCC112->document()->toPlainText().toLocal8Bit());
//	PD_Item[10][2] = string((const char *)ui.DCC113->document()->toPlainText().toLocal8Bit());
	PD_Item3[10] = ui.DCC113->currentIndex();
	PD_Item[11][0] = string((const char *)ui.DCC121->document()->toPlainText().toLocal8Bit());
	PD_Item[11][1] = string((const char *)ui.DCC122->document()->toPlainText().toLocal8Bit());
//	PD_Item[11][2] = string((const char *)ui.DCC123->document()->toPlainText().toLocal8Bit());
	PD_Item3[11] = ui.DCC123->currentIndex();
	PD_Item[12][0] = string((const char *)ui.DCC131->document()->toPlainText().toLocal8Bit());
	PD_Item[12][1] = string((const char *)ui.DCC132->document()->toPlainText().toLocal8Bit());
//	PD_Item[12][2] = string((const char *)ui.DCC133->document()->toPlainText().toLocal8Bit());
	PD_Item3[12] = ui.DCC133->currentIndex();
	PD_Item[13][0] = string((const char *)ui.DCC141->document()->toPlainText().toLocal8Bit());
	PD_Item[13][1] = string((const char *)ui.DCC142->document()->toPlainText().toLocal8Bit());
//	PD_Item[13][2] = string((const char *)ui.DCC143->document()->toPlainText().toLocal8Bit());
	PD_Item3[13] = ui.DCC143->currentIndex();

	///////////////////GMap_data
	Gmap_Item[0][0] = string((const char *)ui.Gmap11->document()->toPlainText().toLocal8Bit());
	Gmap_Item[0][1] = string((const char *)ui.Gmap12->document()->toPlainText().toLocal8Bit());
	Gmap_Item[0][2] = string((const char *)ui.Gmap13->document()->toPlainText().toLocal8Bit());
	Gmap_Item3[0] = ui.Gmap14->currentIndex();

	Gmap_Item[1][0] = string((const char *)ui.Gmap21->document()->toPlainText().toLocal8Bit());
	Gmap_Item[1][1] = string((const char *)ui.Gmap22->document()->toPlainText().toLocal8Bit());
	Gmap_Item[1][2] = string((const char *)ui.Gmap23->document()->toPlainText().toLocal8Bit());
	Gmap_Item3[1] = ui.Gmap24->currentIndex();

	Gmap_Item[2][0] = string((const char *)ui.Gmap31->document()->toPlainText().toLocal8Bit());
	Gmap_Item[2][1] = string((const char *)ui.Gmap32->document()->toPlainText().toLocal8Bit());
	Gmap_Item[2][2] = string((const char *)ui.Gmap33->document()->toPlainText().toLocal8Bit());
	Gmap_Item3[2] = ui.Gmap34->currentIndex();

	Gmap_Item[3][0] = string((const char *)ui.Gmap41->document()->toPlainText().toLocal8Bit());
	Gmap_Item[3][1] = string((const char *)ui.Gmap42->document()->toPlainText().toLocal8Bit());
	Gmap_Item[3][2] = string((const char *)ui.Gmap43->document()->toPlainText().toLocal8Bit());
	Gmap_Item3[3] = ui.Gmap44->currentIndex();

	Gmap_Item[4][0] = string((const char *)ui.Gmap51->document()->toPlainText().toLocal8Bit());
	Gmap_Item[4][1] = string((const char *)ui.Gmap52->document()->toPlainText().toLocal8Bit());
	Gmap_Item[4][2] = string((const char *)ui.Gmap53->document()->toPlainText().toLocal8Bit());
	Gmap_Item3[4] = ui.Gmap54->currentIndex();

	Gmap_Item[5][0] = string((const char *)ui.Gmap61->document()->toPlainText().toLocal8Bit());
	Gmap_Item[5][1] = string((const char *)ui.Gmap62->document()->toPlainText().toLocal8Bit());
	Gmap_Item[5][2] = string((const char *)ui.Gmap63->document()->toPlainText().toLocal8Bit());
	Gmap_Item3[5] = ui.Gmap64->currentIndex();

	Gmap_Item[6][0] = string((const char *)ui.Gmap71->document()->toPlainText().toLocal8Bit());
	Gmap_Item[6][1] = string((const char *)ui.Gmap72->document()->toPlainText().toLocal8Bit());
	Gmap_Item[6][2] = string((const char *)ui.Gmap73->document()->toPlainText().toLocal8Bit());
	Gmap_Item3[6] = ui.Gmap74->currentIndex();

	Gmap_Item[7][0] = string((const char *)ui.Gmap81->document()->toPlainText().toLocal8Bit());
	Gmap_Item[7][1] = string((const char *)ui.Gmap82->document()->toPlainText().toLocal8Bit());
	Gmap_Item[7][2] = string((const char *)ui.Gmap83->document()->toPlainText().toLocal8Bit());
	Gmap_Item3[7] = ui.Gmap84->currentIndex();

	Gmap_Item[8][0] = string((const char *)ui.Gmap91->document()->toPlainText().toLocal8Bit());
	Gmap_Item[8][1] = string((const char *)ui.Gmap92->document()->toPlainText().toLocal8Bit());
	Gmap_Item[8][2] = string((const char *)ui.Gmap93->document()->toPlainText().toLocal8Bit());
	Gmap_Item3[8] = ui.Gmap94->currentIndex();

	Gmap_Item[9][0] = string((const char *)ui.Gmap101->document()->toPlainText().toLocal8Bit());
	Gmap_Item[9][1] = string((const char *)ui.Gmap102->document()->toPlainText().toLocal8Bit());
	Gmap_Item[9][2] = string((const char *)ui.Gmap103->document()->toPlainText().toLocal8Bit());
	Gmap_Item3[9] = ui.Gmap104->currentIndex();

	///////////////////PDAF_info
	PDAF_info_Item[0][0] = string((const char *)ui.PDAF_info11->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[0][1] = string((const char *)ui.PDAF_info12->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[0][2] = string((const char *)ui.PDAF_info13->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[1][0] = string((const char *)ui.PDAF_info21->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[1][1] = string((const char *)ui.PDAF_info22->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[1][2] = string((const char *)ui.PDAF_info23->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[2][0] = string((const char *)ui.PDAF_info31->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[2][1] = string((const char *)ui.PDAF_info32->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[2][2] = string((const char *)ui.PDAF_info33->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[3][0] = string((const char *)ui.PDAF_info41->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[3][1] = string((const char *)ui.PDAF_info42->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[3][2] = string((const char *)ui.PDAF_info43->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[4][0] = string((const char *)ui.PDAF_info51->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[4][1] = string((const char *)ui.PDAF_info52->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[4][2] = string((const char *)ui.PDAF_info53->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[5][0] = string((const char *)ui.PDAF_info61->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[5][1] = string((const char *)ui.PDAF_info62->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[5][2] = string((const char *)ui.PDAF_info63->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[6][0] = string((const char *)ui.PDAF_info71->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[6][1] = string((const char *)ui.PDAF_info72->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[6][2] = string((const char *)ui.PDAF_info73->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[7][0] = string((const char *)ui.PDAF_info81->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[7][1] = string((const char *)ui.PDAF_info82->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[7][2] = string((const char *)ui.PDAF_info83->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[8][0] = string((const char *)ui.PDAF_info91->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[8][1] = string((const char *)ui.PDAF_info92->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[8][2] = string((const char *)ui.PDAF_info93->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[9][0] = string((const char *)ui.PDAF_info101->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[9][1] = string((const char *)ui.PDAF_info102->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[9][2] = string((const char *)ui.PDAF_info103->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[10][0] = string((const char *)ui.PDAF_info111->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[10][1] = string((const char *)ui.PDAF_info112->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[10][2] = string((const char *)ui.PDAF_info113->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[11][0] = string((const char *)ui.PDAF_info121->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[11][1] = string((const char *)ui.PDAF_info122->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[11][2] = string((const char *)ui.PDAF_info123->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[12][0] = string((const char *)ui.PDAF_info131->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[12][1] = string((const char *)ui.PDAF_info132->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[12][2] = string((const char *)ui.PDAF_info133->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[13][0] = string((const char *)ui.PDAF_info141->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[13][1] = string((const char *)ui.PDAF_info142->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[13][2] = string((const char *)ui.PDAF_info143->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[14][0] = string((const char *)ui.PDAF_info151->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[14][1] = string((const char *)ui.PDAF_info152->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[14][2] = string((const char *)ui.PDAF_info153->document()->toPlainText().toLocal8Bit());

	PDAF_info_Item[15][0] = string((const char *)ui.PDAF_info161->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[15][1] = string((const char *)ui.PDAF_info162->document()->toPlainText().toLocal8Bit());
	PDAF_info_Item[15][2] = string((const char *)ui.PDAF_info163->document()->toPlainText().toLocal8Bit());

	///////////////////  INFO data
	QR_Data.item[0] = string((const char *)ui.QR_start->document()->toPlainText().toLocal8Bit());
	QR_Data.item[1] = string((const char *)ui.QR_end->document()->toPlainText().toLocal8Bit());
	QR_Data.item[2] = string((const char *)ui.QR_spec->document()->toPlainText().toLocal8Bit());

	Fuse_Data.item[0] = string((const char *)ui.fuse_start->document()->toPlainText().toLocal8Bit());
	Fuse_Data.item[1] = string((const char *)ui.fuse_end->document()->toPlainText().toLocal8Bit());
	Fuse_Data.item[2] = string((const char *)ui.fuse_spec->document()->toPlainText().toLocal8Bit());

	Fuse_Data2.item[0] = string((const char *)ui.fuse_start_2->document()->toPlainText().toLocal8Bit());
	Fuse_Data2.item[1] = string((const char *)ui.fuse_end_2->document()->toPlainText().toLocal8Bit());
	Fuse_Data2.item[2] = string((const char *)ui.fuse_spec_2->document()->toPlainText().toLocal8Bit()); 

	QC_LSC_Data[0].item[0] = string((const char *)ui.lsc11->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[0].item[1] = string((const char *)ui.lsc12->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[0].item[2] = string((const char *)ui.lsc13->document()->toPlainText().toLocal8Bit());
	LSC_Item3[0] = ui.lsc14->currentIndex();

	QC_LSC_Data[1].item[0] = string((const char *)ui.lsc21->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[1].item[1] = string((const char *)ui.lsc22->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[1].item[2] = string((const char *)ui.lsc23->document()->toPlainText().toLocal8Bit());
	LSC_Item3[1] = ui.lsc24->currentIndex();

	QC_LSC_Data[2].item[0] = string((const char *)ui.lsc31->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[2].item[1] = string((const char *)ui.lsc32->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[2].item[2] = string((const char *)ui.lsc33->document()->toPlainText().toLocal8Bit());
	LSC_Item3[2] = ui.lsc34->currentIndex();

	QC_LSC_Data[3].item[0] = string((const char *)ui.lsc41->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[3].item[1] = string((const char *)ui.lsc42->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[3].item[2] = string((const char *)ui.lsc43->document()->toPlainText().toLocal8Bit());
	LSC_Item3[3] = ui.lsc44->currentIndex();

	QC_LSC_Data[4].item[0] = string((const char *)ui.lsc51->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[4].item[1] = string((const char *)ui.lsc52->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[4].item[2] = string((const char *)ui.lsc53->document()->toPlainText().toLocal8Bit());
	LSC_Item3[4] = ui.lsc54->currentIndex();

	QC_LSC_Data[5].item[0] = string((const char *)ui.lsc61->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[5].item[1] = string((const char *)ui.lsc62->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[5].item[2] = string((const char *)ui.lsc63->document()->toPlainText().toLocal8Bit());
	LSC_Item3[5] = ui.lsc64->currentIndex();

	QC_LSC_Data[6].item[0] = string((const char *)ui.lsc71->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[6].item[1] = string((const char *)ui.lsc72->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[6].item[2] = string((const char *)ui.lsc73->document()->toPlainText().toLocal8Bit());
	LSC_Item3[6] = ui.lsc74->currentIndex();

	QC_LSC_Data[7].item[0] = string((const char *)ui.lsc81->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[7].item[1] = string((const char *)ui.lsc82->document()->toPlainText().toLocal8Bit());
	QC_LSC_Data[7].item[2] = string((const char *)ui.lsc83->document()->toPlainText().toLocal8Bit());
	LSC_Item3[7] = ui.lsc84->currentIndex();
 
	MTK_LSC_Data[0].item[0] = string((const char *)ui.MTK_LSC11->document()->toPlainText().toLocal8Bit());
	MTK_LSC_Data[0].item[1] = string((const char *)ui.MTK_LSC12->document()->toPlainText().toLocal8Bit());

	MTK_LSC_Data[1].item[0] = string((const char *)ui.MTK_LSC21->document()->toPlainText().toLocal8Bit());
	MTK_LSC_Data[1].item[1] = string((const char *)ui.MTK_LSC22->document()->toPlainText().toLocal8Bit());

	shift_Item[0] = string((const char *)ui.shift_start->document()->toPlainText().toLocal8Bit());
	shift_Item[1] = string((const char *)ui.shift_byte->document()->toPlainText().toLocal8Bit());
	shift_Item[2] = string((const char *)ui.shift_point->document()->toPlainText().toLocal8Bit());
	shift_Item[3] = string((const char *)ui.shift_start_2->document()->toPlainText().toLocal8Bit());

	cross_Item[0] = string((const char *)ui.cross1->document()->toPlainText().toLocal8Bit());
	cross_Item[1] = string((const char *)ui.cross2->document()->toPlainText().toLocal8Bit());
	cross_Item[2] = string((const char *)ui.cross3->document()->toPlainText().toLocal8Bit());

	akm_cross_Item[0] = string((const char *)ui.akm_cross1->document()->toPlainText().toLocal8Bit());
	akm_cross_Item[1] = string((const char *)ui.akm_cross2->document()->toPlainText().toLocal8Bit());
	akm_cross_Item[2] = string((const char *)ui.akm_cross3->document()->toPlainText().toLocal8Bit());

	AF_Item[0][0] = string((const char *)ui.AF_name1->document()->toPlainText().toLocal8Bit());
	AF_Item[0][1] = string((const char *)ui.AF_start1->document()->toPlainText().toLocal8Bit());
	AF_Item[0][2] = string((const char *)ui.af_min1->document()->toPlainText().toLocal8Bit());
	AF_Item[0][3] = string((const char *)ui.af_max1->document()->toPlainText().toLocal8Bit());

	AF_Item[1][0] = string((const char *)ui.AF_name2->document()->toPlainText().toLocal8Bit());
	AF_Item[1][1] = string((const char *)ui.AF_start2->document()->toPlainText().toLocal8Bit());
	AF_Item[1][2] = string((const char *)ui.af_min2->document()->toPlainText().toLocal8Bit());
	AF_Item[1][3] = string((const char *)ui.af_max2->document()->toPlainText().toLocal8Bit());

	AF_Item[2][0] = string((const char *)ui.AF_name3->document()->toPlainText().toLocal8Bit());
	AF_Item[2][1] = string((const char *)ui.AF_start3->document()->toPlainText().toLocal8Bit());
	AF_Item[2][2] = string((const char *)ui.af_min3->document()->toPlainText().toLocal8Bit());
	AF_Item[2][3] = string((const char *)ui.af_max3->document()->toPlainText().toLocal8Bit());

	AF_Item[3][0] = string((const char *)ui.AF_name4->document()->toPlainText().toLocal8Bit());
	AF_Item[3][1] = string((const char *)ui.AF_start4->document()->toPlainText().toLocal8Bit());
	AF_Item[3][2] = string((const char *)ui.af_min4->document()->toPlainText().toLocal8Bit());
	AF_Item[3][3] = string((const char *)ui.af_max4->document()->toPlainText().toLocal8Bit());

	AF_Item[4][0] = string((const char *)ui.AF_name5->document()->toPlainText().toLocal8Bit());
	AF_Item[4][1] = string((const char *)ui.AF_start5->document()->toPlainText().toLocal8Bit());
	AF_Item[4][2] = string((const char *)ui.af_min5->document()->toPlainText().toLocal8Bit());
	AF_Item[4][3] = string((const char *)ui.af_max5->document()->toPlainText().toLocal8Bit());

	AF_Item[5][0] = string((const char *)ui.AF_name6->document()->toPlainText().toLocal8Bit());
	AF_Item[5][1] = string((const char *)ui.AF_start6->document()->toPlainText().toLocal8Bit());
	AF_Item[5][2] = string((const char *)ui.af_min6->document()->toPlainText().toLocal8Bit());
	AF_Item[5][3] = string((const char *)ui.af_max6->document()->toPlainText().toLocal8Bit());

	AF_info_Item[0][0] = string((const char *)ui.AF_info11->document()->toPlainText().toLocal8Bit());
	AF_info_Item[0][1] = string((const char *)ui.AF_info12->document()->toPlainText().toLocal8Bit());
	AF_info_Item[0][2] = string((const char *)ui.AF_info13->document()->toPlainText().toLocal8Bit());
	AF_info_Item[0][3] = string((const char *)ui.AF_info14->document()->toPlainText().toLocal8Bit());

	AF_info_Item[1][0] = string((const char *)ui.AF_info21->document()->toPlainText().toLocal8Bit());
	AF_info_Item[1][1] = string((const char *)ui.AF_info22->document()->toPlainText().toLocal8Bit());
	AF_info_Item[1][2] = string((const char *)ui.AF_info23->document()->toPlainText().toLocal8Bit());
	AF_info_Item[1][3] = string((const char *)ui.AF_info24->document()->toPlainText().toLocal8Bit());

	AF_info_Item[2][0] = string((const char *)ui.AF_info31->document()->toPlainText().toLocal8Bit());
	AF_info_Item[2][1] = string((const char *)ui.AF_info32->document()->toPlainText().toLocal8Bit());
	AF_info_Item[2][2] = string((const char *)ui.AF_info33->document()->toPlainText().toLocal8Bit());
	AF_info_Item[2][3] = string((const char *)ui.AF_info34->document()->toPlainText().toLocal8Bit());

	AF_info_Item[3][0] = string((const char *)ui.AF_info41->document()->toPlainText().toLocal8Bit());
	AF_info_Item[3][1] = string((const char *)ui.AF_info42->document()->toPlainText().toLocal8Bit());
	AF_info_Item[3][2] = string((const char *)ui.AF_info43->document()->toPlainText().toLocal8Bit());
	AF_info_Item[3][3] = string((const char *)ui.AF_info44->document()->toPlainText().toLocal8Bit());

	AF_info_Item[4][0] = string((const char *)ui.AF_info51->document()->toPlainText().toLocal8Bit());
	AF_info_Item[4][1] = string((const char *)ui.AF_info52->document()->toPlainText().toLocal8Bit());
	AF_info_Item[4][2] = string((const char *)ui.AF_info53->document()->toPlainText().toLocal8Bit());
	AF_info_Item[4][3] = string((const char *)ui.AF_info54->document()->toPlainText().toLocal8Bit());

	AF_info_Item[5][0] = string((const char *)ui.AF_info61->document()->toPlainText().toLocal8Bit());
	AF_info_Item[5][1] = string((const char *)ui.AF_info62->document()->toPlainText().toLocal8Bit());
	AF_info_Item[5][2] = string((const char *)ui.AF_info63->document()->toPlainText().toLocal8Bit());
	AF_info_Item[5][3] = string((const char *)ui.AF_info64->document()->toPlainText().toLocal8Bit());

	AF_info_Item[6][0] = string((const char *)ui.AF_info71->document()->toPlainText().toLocal8Bit());
	AF_info_Item[6][1] = string((const char *)ui.AF_info72->document()->toPlainText().toLocal8Bit());
	AF_info_Item[6][2] = string((const char *)ui.AF_info73->document()->toPlainText().toLocal8Bit());
	AF_info_Item[6][3] = string((const char *)ui.AF_info74->document()->toPlainText().toLocal8Bit());

	AF_info_Item[7][0] = string((const char *)ui.AF_info81->document()->toPlainText().toLocal8Bit());
	AF_info_Item[7][1] = string((const char *)ui.AF_info82->document()->toPlainText().toLocal8Bit());
	AF_info_Item[7][2] = string((const char *)ui.AF_info83->document()->toPlainText().toLocal8Bit());
	AF_info_Item[7][3] = string((const char *)ui.AF_info84->document()->toPlainText().toLocal8Bit());

	AF_info_Item[8][0] = string((const char *)ui.AF_info91->document()->toPlainText().toLocal8Bit());
	AF_info_Item[8][1] = string((const char *)ui.AF_info92->document()->toPlainText().toLocal8Bit());
	AF_info_Item[8][2] = string((const char *)ui.AF_info93->document()->toPlainText().toLocal8Bit());
	AF_info_Item[8][3] = string((const char *)ui.AF_info94->document()->toPlainText().toLocal8Bit());

	AF_info_Item[9][0] = string((const char *)ui.AF_info101->document()->toPlainText().toLocal8Bit());
	AF_info_Item[9][1] = string((const char *)ui.AF_info102->document()->toPlainText().toLocal8Bit());
	AF_info_Item[9][2] = string((const char *)ui.AF_info103->document()->toPlainText().toLocal8Bit());
	AF_info_Item[9][3] = string((const char *)ui.AF_info104->document()->toPlainText().toLocal8Bit());

	SFR_Item[0][0]=string((const char *)ui.SFR_name1->document()->toPlainText().toLocal8Bit());
	SFR_Item[0][1] = string((const char *)ui.SFR_grade1->document()->toPlainText().toLocal8Bit());
	SFR_Item[0][2] = string((const char *)ui.SFR_start1->document()->toPlainText().toLocal8Bit());
	SFR_Item[0][3] = string((const char *)ui.SFR_cnt1->document()->toPlainText().toLocal8Bit());

	SFR_Item[1][0] = string((const char *)ui.SFR_name2->document()->toPlainText().toLocal8Bit());
	SFR_Item[1][1] = string((const char *)ui.SFR_grade2->document()->toPlainText().toLocal8Bit());
	SFR_Item[1][2] = string((const char *)ui.SFR_start2->document()->toPlainText().toLocal8Bit());
	SFR_Item[1][3] = string((const char *)ui.SFR_cnt2->document()->toPlainText().toLocal8Bit());

	SFR_Item[2][0] = string((const char *)ui.SFR_name3->document()->toPlainText().toLocal8Bit());
	SFR_Item[2][1] = string((const char *)ui.SFR_grade3->document()->toPlainText().toLocal8Bit());
	SFR_Item[2][2] = string((const char *)ui.SFR_start3->document()->toPlainText().toLocal8Bit());
	SFR_Item[2][3] = string((const char *)ui.SFR_cnt3->document()->toPlainText().toLocal8Bit());

	SFR_Item[3][0] = string((const char *)ui.SFR_name4->document()->toPlainText().toLocal8Bit());
	SFR_Item[3][1] = string((const char *)ui.SFR_grade4->document()->toPlainText().toLocal8Bit());
	SFR_Item[3][2] = string((const char *)ui.SFR_start4->document()->toPlainText().toLocal8Bit());
	SFR_Item[3][3] = string((const char *)ui.SFR_cnt4->document()->toPlainText().toLocal8Bit());


	/////////////////////////////// Checksum panel
	checkSum[0].item[0] = string((const char *)ui.checksum11->document()->toPlainText().toLocal8Bit());
	checkSum[0].item[1] = string((const char *)ui.checksum12->document()->toPlainText().toLocal8Bit());
	checkSum[0].item[2] = string((const char *)ui.checksum13->document()->toPlainText().toLocal8Bit());
	checkSum[0].item[3] = string((const char *)ui.checksum14->document()->toPlainText().toLocal8Bit());
	checkSum[0].item[4] = string((const char *)ui.checksum15->document()->toPlainText().toLocal8Bit());

	checkSum[1].item[0] = string((const char *)ui.checksum21->document()->toPlainText().toLocal8Bit());
	checkSum[1].item[1] = string((const char *)ui.checksum22->document()->toPlainText().toLocal8Bit());
	checkSum[1].item[2] = string((const char *)ui.checksum23->document()->toPlainText().toLocal8Bit());
	checkSum[1].item[3] = string((const char *)ui.checksum24->document()->toPlainText().toLocal8Bit());
	checkSum[1].item[4] = string((const char *)ui.checksum25->document()->toPlainText().toLocal8Bit());

	checkSum[2].item[0] = string((const char *)ui.checksum31->document()->toPlainText().toLocal8Bit());
	checkSum[2].item[1] = string((const char *)ui.checksum32->document()->toPlainText().toLocal8Bit());
	checkSum[2].item[2] = string((const char *)ui.checksum33->document()->toPlainText().toLocal8Bit());
	checkSum[2].item[3] = string((const char *)ui.checksum34->document()->toPlainText().toLocal8Bit());
	checkSum[2].item[4] = string((const char *)ui.checksum35->document()->toPlainText().toLocal8Bit());

	checkSum[3].item[0] = string((const char *)ui.checksum41->document()->toPlainText().toLocal8Bit());
	checkSum[3].item[1] = string((const char *)ui.checksum42->document()->toPlainText().toLocal8Bit());
	checkSum[3].item[2] = string((const char *)ui.checksum43->document()->toPlainText().toLocal8Bit());
	checkSum[3].item[3] = string((const char *)ui.checksum44->document()->toPlainText().toLocal8Bit());
	checkSum[3].item[4] = string((const char *)ui.checksum45->document()->toPlainText().toLocal8Bit());

	checkSum[4].item[0] = string((const char *)ui.checksum51->document()->toPlainText().toLocal8Bit());
	checkSum[4].item[1] = string((const char *)ui.checksum52->document()->toPlainText().toLocal8Bit());
	checkSum[4].item[2] = string((const char *)ui.checksum53->document()->toPlainText().toLocal8Bit());
	checkSum[4].item[3] = string((const char *)ui.checksum54->document()->toPlainText().toLocal8Bit());
	checkSum[4].item[4] = string((const char *)ui.checksum55->document()->toPlainText().toLocal8Bit());

	checkSum[5].item[0] = string((const char *)ui.checksum61->document()->toPlainText().toLocal8Bit());
	checkSum[5].item[1] = string((const char *)ui.checksum62->document()->toPlainText().toLocal8Bit());
	checkSum[5].item[2] = string((const char *)ui.checksum63->document()->toPlainText().toLocal8Bit());
	checkSum[5].item[3] = string((const char *)ui.checksum64->document()->toPlainText().toLocal8Bit());
	checkSum[5].item[4] = string((const char *)ui.checksum65->document()->toPlainText().toLocal8Bit());

	checkSum[6].item[0] = string((const char *)ui.checksum71->document()->toPlainText().toLocal8Bit());
	checkSum[6].item[1] = string((const char *)ui.checksum72->document()->toPlainText().toLocal8Bit());
	checkSum[6].item[2] = string((const char *)ui.checksum73->document()->toPlainText().toLocal8Bit());
	checkSum[6].item[3] = string((const char *)ui.checksum74->document()->toPlainText().toLocal8Bit());
	checkSum[6].item[4] = string((const char *)ui.checksum75->document()->toPlainText().toLocal8Bit());

	checkSum[7].item[0] = string((const char *)ui.checksum81->document()->toPlainText().toLocal8Bit());
	checkSum[7].item[1] = string((const char *)ui.checksum82->document()->toPlainText().toLocal8Bit());
	checkSum[7].item[2] = string((const char *)ui.checksum83->document()->toPlainText().toLocal8Bit());
	checkSum[7].item[3] = string((const char *)ui.checksum84->document()->toPlainText().toLocal8Bit());
	checkSum[7].item[4] = string((const char *)ui.checksum85->document()->toPlainText().toLocal8Bit());

	checkSum[8].item[0] = string((const char *)ui.checksum91->document()->toPlainText().toLocal8Bit());
	checkSum[8].item[1] = string((const char *)ui.checksum92->document()->toPlainText().toLocal8Bit());
	checkSum[8].item[2] = string((const char *)ui.checksum93->document()->toPlainText().toLocal8Bit());
	checkSum[8].item[3] = string((const char *)ui.checksum94->document()->toPlainText().toLocal8Bit());
	checkSum[8].item[4] = string((const char *)ui.checksum95->document()->toPlainText().toLocal8Bit());

	checkSum[9].item[0] = string((const char *)ui.checksum101->document()->toPlainText().toLocal8Bit());
	checkSum[9].item[1] = string((const char *)ui.checksum102->document()->toPlainText().toLocal8Bit());
	checkSum[9].item[2] = string((const char *)ui.checksum103->document()->toPlainText().toLocal8Bit());
	checkSum[9].item[3] = string((const char *)ui.checksum104->document()->toPlainText().toLocal8Bit());
	checkSum[9].item[4] = string((const char *)ui.checksum105->document()->toPlainText().toLocal8Bit());

	checkSum[10].item[0] = string((const char *)ui.checksum111->document()->toPlainText().toLocal8Bit());
	checkSum[10].item[1] = string((const char *)ui.checksum112->document()->toPlainText().toLocal8Bit());
	checkSum[10].item[2] = string((const char *)ui.checksum113->document()->toPlainText().toLocal8Bit());
	checkSum[10].item[3] = string((const char *)ui.checksum114->document()->toPlainText().toLocal8Bit());
	checkSum[10].item[4] = string((const char *)ui.checksum115->document()->toPlainText().toLocal8Bit());

	checkSum[11].item[0] = string((const char *)ui.checksum121->document()->toPlainText().toLocal8Bit());
	checkSum[11].item[1] = string((const char *)ui.checksum122->document()->toPlainText().toLocal8Bit());
	checkSum[11].item[2] = string((const char *)ui.checksum123->document()->toPlainText().toLocal8Bit());
	checkSum[11].item[3] = string((const char *)ui.checksum124->document()->toPlainText().toLocal8Bit());
	checkSum[11].item[4] = string((const char *)ui.checksum125->document()->toPlainText().toLocal8Bit());

	checkSum[12].item[0] = string((const char *)ui.checksum131->document()->toPlainText().toLocal8Bit());
	checkSum[12].item[1] = string((const char *)ui.checksum132->document()->toPlainText().toLocal8Bit());
	checkSum[12].item[2] = string((const char *)ui.checksum133->document()->toPlainText().toLocal8Bit());
	checkSum[12].item[3] = string((const char *)ui.checksum134->document()->toPlainText().toLocal8Bit());
	checkSum[12].item[4] = string((const char *)ui.checksum135->document()->toPlainText().toLocal8Bit());

	checkSum[13].item[0] = string((const char *)ui.checksum141->document()->toPlainText().toLocal8Bit());
	checkSum[13].item[1] = string((const char *)ui.checksum142->document()->toPlainText().toLocal8Bit());
	checkSum[13].item[2] = string((const char *)ui.checksum143->document()->toPlainText().toLocal8Bit());
	checkSum[13].item[3] = string((const char *)ui.checksum144->document()->toPlainText().toLocal8Bit());
	checkSum[13].item[4] = string((const char *)ui.checksum145->document()->toPlainText().toLocal8Bit());

	checkSum[14].item[0] = string((const char *)ui.checksum151->document()->toPlainText().toLocal8Bit());
	checkSum[14].item[1] = string((const char *)ui.checksum152->document()->toPlainText().toLocal8Bit());
	checkSum[14].item[2] = string((const char *)ui.checksum153->document()->toPlainText().toLocal8Bit());
	checkSum[14].item[3] = string((const char *)ui.checksum154->document()->toPlainText().toLocal8Bit());
	checkSum[14].item[4] = string((const char *)ui.checksum155->document()->toPlainText().toLocal8Bit());

	checkSum[15].item[0] = string((const char *)ui.checksum161->document()->toPlainText().toLocal8Bit());
	checkSum[15].item[1] = string((const char *)ui.checksum162->document()->toPlainText().toLocal8Bit());
	checkSum[15].item[2] = string((const char *)ui.checksum163->document()->toPlainText().toLocal8Bit());
	checkSum[15].item[3] = string((const char *)ui.checksum164->document()->toPlainText().toLocal8Bit());
	checkSum[15].item[4] = string((const char *)ui.checksum165->document()->toPlainText().toLocal8Bit());

	checkSum[16].item[0] = string((const char *)ui.checksum171->document()->toPlainText().toLocal8Bit());
	checkSum[16].item[1] = string((const char *)ui.checksum172->document()->toPlainText().toLocal8Bit());
	checkSum[16].item[2] = string((const char *)ui.checksum173->document()->toPlainText().toLocal8Bit());
	checkSum[16].item[3] = string((const char *)ui.checksum174->document()->toPlainText().toLocal8Bit());
	checkSum[16].item[4] = string((const char *)ui.checksum175->document()->toPlainText().toLocal8Bit());

	checkSum[17].item[0] = string((const char *)ui.checksum181->document()->toPlainText().toLocal8Bit());
	checkSum[17].item[1] = string((const char *)ui.checksum182->document()->toPlainText().toLocal8Bit());
	checkSum[17].item[2] = string((const char *)ui.checksum183->document()->toPlainText().toLocal8Bit());
	checkSum[17].item[3] = string((const char *)ui.checksum184->document()->toPlainText().toLocal8Bit());
	checkSum[17].item[4] = string((const char *)ui.checksum185->document()->toPlainText().toLocal8Bit());

	checkSum[18].item[0] = string((const char *)ui.checksum191->document()->toPlainText().toLocal8Bit());
	checkSum[18].item[1] = string((const char *)ui.checksum192->document()->toPlainText().toLocal8Bit());
	checkSum[18].item[2] = string((const char *)ui.checksum193->document()->toPlainText().toLocal8Bit());
	checkSum[18].item[3] = string((const char *)ui.checksum194->document()->toPlainText().toLocal8Bit());
	checkSum[18].item[4] = string((const char *)ui.checksum195->document()->toPlainText().toLocal8Bit());

	checkSum[19].item[0] = string((const char *)ui.checksum201->document()->toPlainText().toLocal8Bit());
	checkSum[19].item[1] = string((const char *)ui.checksum202->document()->toPlainText().toLocal8Bit());
	checkSum[19].item[2] = string((const char *)ui.checksum203->document()->toPlainText().toLocal8Bit());
	checkSum[19].item[3] = string((const char *)ui.checksum204->document()->toPlainText().toLocal8Bit());
	checkSum[19].item[4] = string((const char *)ui.checksum205->document()->toPlainText().toLocal8Bit());

	checkSum[20].item[0] = string((const char *)ui.checksum211->document()->toPlainText().toLocal8Bit());
	checkSum[20].item[1] = string((const char *)ui.checksum212->document()->toPlainText().toLocal8Bit());
	checkSum[20].item[2] = string((const char *)ui.checksum213->document()->toPlainText().toLocal8Bit());
	checkSum[20].item[3] = string((const char *)ui.checksum214->document()->toPlainText().toLocal8Bit());
	checkSum[20].item[4] = string((const char *)ui.checksum215->document()->toPlainText().toLocal8Bit());

	checkSum[21].item[0] = string((const char *)ui.checksum221->document()->toPlainText().toLocal8Bit());
	checkSum[21].item[1] = string((const char *)ui.checksum222->document()->toPlainText().toLocal8Bit());
	checkSum[21].item[2] = string((const char *)ui.checksum223->document()->toPlainText().toLocal8Bit());
	checkSum[21].item[3] = string((const char *)ui.checksum224->document()->toPlainText().toLocal8Bit());
	checkSum[21].item[4] = string((const char *)ui.checksum225->document()->toPlainText().toLocal8Bit());

	checkSum[22].item[0] = string((const char *)ui.checksum231->document()->toPlainText().toLocal8Bit());
	checkSum[22].item[1] = string((const char *)ui.checksum232->document()->toPlainText().toLocal8Bit());
	checkSum[22].item[2] = string((const char *)ui.checksum233->document()->toPlainText().toLocal8Bit());
	checkSum[22].item[3] = string((const char *)ui.checksum234->document()->toPlainText().toLocal8Bit());
	checkSum[22].item[4] = string((const char *)ui.checksum235->document()->toPlainText().toLocal8Bit());

	checkSum[23].item[0] = string((const char *)ui.checksum241->document()->toPlainText().toLocal8Bit());
	checkSum[23].item[1] = string((const char *)ui.checksum242->document()->toPlainText().toLocal8Bit());
	checkSum[23].item[2] = string((const char *)ui.checksum243->document()->toPlainText().toLocal8Bit());
	checkSum[23].item[3] = string((const char *)ui.checksum244->document()->toPlainText().toLocal8Bit());
	checkSum[23].item[4] = string((const char *)ui.checksum245->document()->toPlainText().toLocal8Bit());

	checkSum[24].item[0] = string((const char *)ui.checksum251->document()->toPlainText().toLocal8Bit());
	checkSum[24].item[1] = string((const char *)ui.checksum252->document()->toPlainText().toLocal8Bit());
	checkSum[24].item[2] = string((const char *)ui.checksum253->document()->toPlainText().toLocal8Bit());
	checkSum[24].item[3] = string((const char *)ui.checksum254->document()->toPlainText().toLocal8Bit());
	checkSum[24].item[4] = string((const char *)ui.checksum255->document()->toPlainText().toLocal8Bit());

	checkSum[25].item[0] = string((const char *)ui.checksum261->document()->toPlainText().toLocal8Bit());
	checkSum[25].item[1] = string((const char *)ui.checksum262->document()->toPlainText().toLocal8Bit());
	checkSum[25].item[2] = string((const char *)ui.checksum263->document()->toPlainText().toLocal8Bit());
	checkSum[25].item[3] = string((const char *)ui.checksum264->document()->toPlainText().toLocal8Bit());
	checkSum[25].item[4] = string((const char *)ui.checksum265->document()->toPlainText().toLocal8Bit());

	checkSum[26].item[0] = string((const char *)ui.checksum271->document()->toPlainText().toLocal8Bit());
	checkSum[26].item[1] = string((const char *)ui.checksum272->document()->toPlainText().toLocal8Bit());
	checkSum[26].item[2] = string((const char *)ui.checksum273->document()->toPlainText().toLocal8Bit());
	checkSum[26].item[3] = string((const char *)ui.checksum274->document()->toPlainText().toLocal8Bit());
	checkSum[26].item[4] = string((const char *)ui.checksum275->document()->toPlainText().toLocal8Bit());

	checkSum[27].item[0] = string((const char *)ui.checksum281->document()->toPlainText().toLocal8Bit());
	checkSum[27].item[1] = string((const char *)ui.checksum282->document()->toPlainText().toLocal8Bit());
	checkSum[27].item[2] = string((const char *)ui.checksum283->document()->toPlainText().toLocal8Bit());
	checkSum[27].item[3] = string((const char *)ui.checksum284->document()->toPlainText().toLocal8Bit());
	checkSum[27].item[4] = string((const char *)ui.checksum285->document()->toPlainText().toLocal8Bit());

	checkSum[28].item[0] = string((const char *)ui.checksum291->document()->toPlainText().toLocal8Bit());
	checkSum[28].item[1] = string((const char *)ui.checksum292->document()->toPlainText().toLocal8Bit());
	checkSum[28].item[2] = string((const char *)ui.checksum293->document()->toPlainText().toLocal8Bit());
	checkSum[28].item[3] = string((const char *)ui.checksum294->document()->toPlainText().toLocal8Bit());
	checkSum[28].item[4] = string((const char *)ui.checksum295->document()->toPlainText().toLocal8Bit());

	checkSum[29].item[0] = string((const char *)ui.checksum301->document()->toPlainText().toLocal8Bit());
	checkSum[29].item[1] = string((const char *)ui.checksum302->document()->toPlainText().toLocal8Bit());
	checkSum[29].item[2] = string((const char *)ui.checksum303->document()->toPlainText().toLocal8Bit());
	checkSum[29].item[3] = string((const char *)ui.checksum304->document()->toPlainText().toLocal8Bit());
	checkSum[29].item[4] = string((const char *)ui.checksum305->document()->toPlainText().toLocal8Bit());

	checkSum[30].item[0] = string((const char *)ui.checksum311->document()->toPlainText().toLocal8Bit());
	checkSum[30].item[1] = string((const char *)ui.checksum312->document()->toPlainText().toLocal8Bit());
	checkSum[30].item[2] = string((const char *)ui.checksum313->document()->toPlainText().toLocal8Bit());
	checkSum[30].item[3] = string((const char *)ui.checksum314->document()->toPlainText().toLocal8Bit());
	checkSum[30].item[4] = string((const char *)ui.checksum315->document()->toPlainText().toLocal8Bit());

	checkSum[31].item[0] = string((const char *)ui.checksum321->document()->toPlainText().toLocal8Bit());
	checkSum[31].item[1] = string((const char *)ui.checksum322->document()->toPlainText().toLocal8Bit());
	checkSum[31].item[2] = string((const char *)ui.checksum323->document()->toPlainText().toLocal8Bit());
	checkSum[31].item[3] = string((const char *)ui.checksum324->document()->toPlainText().toLocal8Bit());
	checkSum[31].item[4] = string((const char *)ui.checksum325->document()->toPlainText().toLocal8Bit());

	checkSum[32].item[0] = string((const char *)ui.checksum331->document()->toPlainText().toLocal8Bit());
	checkSum[32].item[1] = string((const char *)ui.checksum332->document()->toPlainText().toLocal8Bit());
	checkSum[32].item[2] = string((const char *)ui.checksum333->document()->toPlainText().toLocal8Bit());
	checkSum[32].item[3] = string((const char *)ui.checksum334->document()->toPlainText().toLocal8Bit());
	checkSum[32].item[4] = string((const char *)ui.checksum335->document()->toPlainText().toLocal8Bit());

	checkSum[33].item[0] = string((const char *)ui.checksum341->document()->toPlainText().toLocal8Bit());
	checkSum[33].item[1] = string((const char *)ui.checksum342->document()->toPlainText().toLocal8Bit());
	checkSum[33].item[2] = string((const char *)ui.checksum343->document()->toPlainText().toLocal8Bit());
	checkSum[33].item[3] = string((const char *)ui.checksum344->document()->toPlainText().toLocal8Bit());
	checkSum[33].item[4] = string((const char *)ui.checksum345->document()->toPlainText().toLocal8Bit());

	checkSum[34].item[0] = string((const char *)ui.checksum351->document()->toPlainText().toLocal8Bit());
	checkSum[34].item[1] = string((const char *)ui.checksum352->document()->toPlainText().toLocal8Bit());
	checkSum[34].item[2] = string((const char *)ui.checksum353->document()->toPlainText().toLocal8Bit());
	checkSum[34].item[3] = string((const char *)ui.checksum354->document()->toPlainText().toLocal8Bit());
	checkSum[34].item[4] = string((const char *)ui.checksum355->document()->toPlainText().toLocal8Bit());

	checkSum[35].item[0] = string((const char *)ui.checksum361->document()->toPlainText().toLocal8Bit());
	checkSum[35].item[1] = string((const char *)ui.checksum362->document()->toPlainText().toLocal8Bit());
	checkSum[35].item[2] = string((const char *)ui.checksum363->document()->toPlainText().toLocal8Bit());
	checkSum[35].item[3] = string((const char *)ui.checksum364->document()->toPlainText().toLocal8Bit());
	checkSum[35].item[4] = string((const char *)ui.checksum365->document()->toPlainText().toLocal8Bit());

	checkSum[36].item[0] = string((const char *)ui.checksum371->document()->toPlainText().toLocal8Bit());
	checkSum[36].item[1] = string((const char *)ui.checksum372->document()->toPlainText().toLocal8Bit());
	checkSum[36].item[2] = string((const char *)ui.checksum373->document()->toPlainText().toLocal8Bit());
	checkSum[36].item[3] = string((const char *)ui.checksum374->document()->toPlainText().toLocal8Bit());
	checkSum[36].item[4] = string((const char *)ui.checksum375->document()->toPlainText().toLocal8Bit());

	checkSum[37].item[0] = string((const char *)ui.checksum381->document()->toPlainText().toLocal8Bit());
	checkSum[37].item[1] = string((const char *)ui.checksum382->document()->toPlainText().toLocal8Bit());
	checkSum[37].item[2] = string((const char *)ui.checksum383->document()->toPlainText().toLocal8Bit());
	checkSum[37].item[3] = string((const char *)ui.checksum384->document()->toPlainText().toLocal8Bit());
	checkSum[37].item[4] = string((const char *)ui.checksum385->document()->toPlainText().toLocal8Bit());

	checkSum[38].item[0] = string((const char *)ui.checksum391->document()->toPlainText().toLocal8Bit());
	checkSum[38].item[1] = string((const char *)ui.checksum392->document()->toPlainText().toLocal8Bit());
	checkSum[38].item[2] = string((const char *)ui.checksum393->document()->toPlainText().toLocal8Bit());
	checkSum[38].item[3] = string((const char *)ui.checksum394->document()->toPlainText().toLocal8Bit());
	checkSum[38].item[4] = string((const char *)ui.checksum395->document()->toPlainText().toLocal8Bit());

	checkSum[39].item[0] = string((const char *)ui.checksum401->document()->toPlainText().toLocal8Bit());
	checkSum[39].item[1] = string((const char *)ui.checksum402->document()->toPlainText().toLocal8Bit());
	checkSum[39].item[2] = string((const char *)ui.checksum403->document()->toPlainText().toLocal8Bit());
	checkSum[39].item[3] = string((const char *)ui.checksum404->document()->toPlainText().toLocal8Bit());
	checkSum[39].item[4] = string((const char *)ui.checksum405->document()->toPlainText().toLocal8Bit());


	noData.hex= string((const char *)ui.noData->document()->toPlainText().toLocal8Bit());
	Flag[0].hex = string((const char *)ui.flag_valid->document()->toPlainText().toLocal8Bit());
	Flag[1].hex = string((const char *)ui.flag_invalid->document()->toPlainText().toLocal8Bit());

	infoData[0].item[0] = string((const char *)ui.info11->document()->toPlainText().toLocal8Bit());
	infoData[0].item[1] = string((const char *)ui.info12->document()->toPlainText().toLocal8Bit());
	infoData[0].item[2] = string((const char *)ui.info13->document()->toPlainText().toLocal8Bit());

	infoData[1].item[0] = string((const char *)ui.info21->document()->toPlainText().toLocal8Bit());
	infoData[1].item[1] = string((const char *)ui.info22->document()->toPlainText().toLocal8Bit());
	infoData[1].item[2] = string((const char *)ui.info23->document()->toPlainText().toLocal8Bit());

	infoData[2].item[0] = string((const char *)ui.info31->document()->toPlainText().toLocal8Bit());
	infoData[2].item[1] = string((const char *)ui.info32->document()->toPlainText().toLocal8Bit());
	infoData[2].item[2] = string((const char *)ui.info33->document()->toPlainText().toLocal8Bit());

	infoData[3].item[0] = string((const char *)ui.info41->document()->toPlainText().toLocal8Bit());
	infoData[3].item[1] = string((const char *)ui.info42->document()->toPlainText().toLocal8Bit());
	infoData[3].item[2] = string((const char *)ui.info43->document()->toPlainText().toLocal8Bit());

	infoData[4].item[0] = string((const char *)ui.info51->document()->toPlainText().toLocal8Bit());
	infoData[4].item[1] = string((const char *)ui.info52->document()->toPlainText().toLocal8Bit());
	infoData[4].item[2] = string((const char *)ui.info53->document()->toPlainText().toLocal8Bit());

	infoData[5].item[0] = string((const char *)ui.info61->document()->toPlainText().toLocal8Bit());
	infoData[5].item[1] = string((const char *)ui.info62->document()->toPlainText().toLocal8Bit());
	infoData[5].item[2] = string((const char *)ui.info63->document()->toPlainText().toLocal8Bit());

	infoData[6].item[0] = string((const char *)ui.info71->document()->toPlainText().toLocal8Bit());
	infoData[6].item[1] = string((const char *)ui.info72->document()->toPlainText().toLocal8Bit());
	infoData[6].item[2] = string((const char *)ui.info73->document()->toPlainText().toLocal8Bit());

	infoData[7].item[0] = string((const char *)ui.info81->document()->toPlainText().toLocal8Bit());
	infoData[7].item[1] = string((const char *)ui.info82->document()->toPlainText().toLocal8Bit());
	infoData[7].item[2] = string((const char *)ui.info83->document()->toPlainText().toLocal8Bit());

	infoData[8].item[0] = string((const char *)ui.info91->document()->toPlainText().toLocal8Bit());
	infoData[8].item[1] = string((const char *)ui.info92->document()->toPlainText().toLocal8Bit());
	infoData[8].item[2] = string((const char *)ui.info93->document()->toPlainText().toLocal8Bit());

	infoData[9].item[0] = string((const char *)ui.info101->document()->toPlainText().toLocal8Bit());
	infoData[9].item[1] = string((const char *)ui.info102->document()->toPlainText().toLocal8Bit());
	infoData[9].item[2] = string((const char *)ui.info103->document()->toPlainText().toLocal8Bit());

	infoData[10].item[0] = string((const char *)ui.info111->document()->toPlainText().toLocal8Bit());
	infoData[10].item[1] = string((const char *)ui.info112->document()->toPlainText().toLocal8Bit());
	infoData[10].item[2] = string((const char *)ui.info113->document()->toPlainText().toLocal8Bit());

	infoData[11].item[0] = string((const char *)ui.info121->document()->toPlainText().toLocal8Bit());
	infoData[11].item[1] = string((const char *)ui.info122->document()->toPlainText().toLocal8Bit());
	infoData[11].item[2] = string((const char *)ui.info123->document()->toPlainText().toLocal8Bit());

	infoData[12].item[0] = string((const char *)ui.info131->document()->toPlainText().toLocal8Bit());
	infoData[12].item[1] = string((const char *)ui.info132->document()->toPlainText().toLocal8Bit());
	infoData[12].item[2] = string((const char *)ui.info133->document()->toPlainText().toLocal8Bit());

	infoData[13].item[0] = string((const char *)ui.info141->document()->toPlainText().toLocal8Bit());
	infoData[13].item[1] = string((const char *)ui.info142->document()->toPlainText().toLocal8Bit());
	infoData[13].item[2] = string((const char *)ui.info143->document()->toPlainText().toLocal8Bit());

	infoData[14].item[0] = string((const char *)ui.info151->document()->toPlainText().toLocal8Bit());
	infoData[14].item[1] = string((const char *)ui.info152->document()->toPlainText().toLocal8Bit());
	infoData[14].item[2] = string((const char *)ui.info153->document()->toPlainText().toLocal8Bit());

	infoData[15].item[0] = string((const char *)ui.info161->document()->toPlainText().toLocal8Bit());
	infoData[15].item[1] = string((const char *)ui.info162->document()->toPlainText().toLocal8Bit());
	infoData[15].item[2] = string((const char *)ui.info163->document()->toPlainText().toLocal8Bit());

	infoData[16].item[0] = string((const char *)ui.info171->document()->toPlainText().toLocal8Bit());
	infoData[16].item[1] = string((const char *)ui.info172->document()->toPlainText().toLocal8Bit());
	infoData[16].item[2] = string((const char *)ui.info173->document()->toPlainText().toLocal8Bit());

	infoData[17].item[0] = string((const char *)ui.info181->document()->toPlainText().toLocal8Bit());
	infoData[17].item[1] = string((const char *)ui.info182->document()->toPlainText().toLocal8Bit());
	infoData[17].item[2] = string((const char *)ui.info183->document()->toPlainText().toLocal8Bit());

	infoData[18].item[0] = string((const char *)ui.info191->document()->toPlainText().toLocal8Bit());
	infoData[18].item[1] = string((const char *)ui.info192->document()->toPlainText().toLocal8Bit());
	infoData[18].item[2] = string((const char *)ui.info193->document()->toPlainText().toLocal8Bit());

	infoData[19].item[0] = string((const char *)ui.info201->document()->toPlainText().toLocal8Bit());
	infoData[19].item[1] = string((const char *)ui.info202->document()->toPlainText().toLocal8Bit());
	infoData[19].item[2] = string((const char *)ui.info203->document()->toPlainText().toLocal8Bit());

	infoData[20].item[0] = string((const char *)ui.info211->document()->toPlainText().toLocal8Bit());
	infoData[20].item[1] = string((const char *)ui.info212->document()->toPlainText().toLocal8Bit());
	infoData[20].item[2] = string((const char *)ui.info213->document()->toPlainText().toLocal8Bit());

	infoData[21].item[0] = string((const char *)ui.info221->document()->toPlainText().toLocal8Bit());
	infoData[21].item[1] = string((const char *)ui.info222->document()->toPlainText().toLocal8Bit());
	infoData[21].item[2] = string((const char *)ui.info223->document()->toPlainText().toLocal8Bit());

	infoData[22].item[0] = string((const char *)ui.info231->document()->toPlainText().toLocal8Bit());
	infoData[22].item[1] = string((const char *)ui.info232->document()->toPlainText().toLocal8Bit());
	infoData[22].item[2] = string((const char *)ui.info233->document()->toPlainText().toLocal8Bit());

	infoData[23].item[0] = string((const char *)ui.info241->document()->toPlainText().toLocal8Bit());
	infoData[23].item[1] = string((const char *)ui.info242->document()->toPlainText().toLocal8Bit());
	infoData[23].item[2] = string((const char *)ui.info243->document()->toPlainText().toLocal8Bit());

	infoData[24].item[0] = string((const char *)ui.info251->document()->toPlainText().toLocal8Bit());
	infoData[24].item[1] = string((const char *)ui.info252->document()->toPlainText().toLocal8Bit());
	infoData[24].item[2] = string((const char *)ui.info253->document()->toPlainText().toLocal8Bit());

	infoData[25].item[0] = string((const char *)ui.info261->document()->toPlainText().toLocal8Bit());
	infoData[25].item[1] = string((const char *)ui.info262->document()->toPlainText().toLocal8Bit());
	infoData[25].item[2] = string((const char *)ui.info263->document()->toPlainText().toLocal8Bit());

	infoData[26].item[0] = string((const char *)ui.info271->document()->toPlainText().toLocal8Bit());
	infoData[26].item[1] = string((const char *)ui.info272->document()->toPlainText().toLocal8Bit());
	infoData[26].item[2] = string((const char *)ui.info273->document()->toPlainText().toLocal8Bit());

	infoData[27].item[0] = string((const char *)ui.info281->document()->toPlainText().toLocal8Bit());
	infoData[27].item[1] = string((const char *)ui.info282->document()->toPlainText().toLocal8Bit());
	infoData[27].item[2] = string((const char *)ui.info283->document()->toPlainText().toLocal8Bit());

	infoData[28].item[0] = string((const char *)ui.info291->document()->toPlainText().toLocal8Bit());
	infoData[28].item[1] = string((const char *)ui.info292->document()->toPlainText().toLocal8Bit());
	infoData[28].item[2] = string((const char *)ui.info293->document()->toPlainText().toLocal8Bit());

	infoData[29].item[0] = string((const char *)ui.info301->document()->toPlainText().toLocal8Bit());
	infoData[29].item[1] = string((const char *)ui.info302->document()->toPlainText().toLocal8Bit());
	infoData[29].item[2] = string((const char *)ui.info303->document()->toPlainText().toLocal8Bit());

	infoData[30].item[0] = string((const char *)ui.info311->document()->toPlainText().toLocal8Bit());
	infoData[30].item[1] = string((const char *)ui.info312->document()->toPlainText().toLocal8Bit());
	infoData[30].item[2] = string((const char *)ui.info313->document()->toPlainText().toLocal8Bit());

	infoData[31].item[0] = string((const char *)ui.info321->document()->toPlainText().toLocal8Bit());
	infoData[31].item[1] = string((const char *)ui.info322->document()->toPlainText().toLocal8Bit());
	infoData[31].item[2] = string((const char *)ui.info323->document()->toPlainText().toLocal8Bit());

	product_Date.item[0] = string((const char *)ui.year_H->document()->toPlainText().toLocal8Bit());
	product_Date.item[1] = string((const char *)ui.year_L->document()->toPlainText().toLocal8Bit());
	product_Date.item[2] = string((const char *)ui.month->document()->toPlainText().toLocal8Bit());
	product_Date.item[3] = string((const char *)ui.day->document()->toPlainText().toLocal8Bit());
	product_Date.item[4] = string((const char *)ui.hour->document()->toPlainText().toLocal8Bit());
	product_Date.item[5] = string((const char *)ui.minute->document()->toPlainText().toLocal8Bit());
	product_Date.item[6] = string((const char *)ui.second->document()->toPlainText().toLocal8Bit());
	product_Date.item[7] = string((const char *)ui.factory->document()->toPlainText().toLocal8Bit());
	product_Date.item[8] = string((const char *)ui.line->document()->toPlainText().toLocal8Bit());

}


void EEPROM_Data_Verifier::save_EEPROM_Address() {

	load_Panel();
	TCHAR lpTexts[20];
	USES_CONVERSION;
	for (int i = 0; i < 40; i++)
		for (int k = 0; k < 5; k++) {
			string item = "checkSum" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(checkSum[i].item[k].c_str()), CA2CT(EEPROM_Map.c_str()));	
		}

	for (int i = 0; i < 32; i++)
		for (int k = 0; k < 3; k++) {
			string item = "info" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(infoData[i].item[k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 6; i++) {
		string item = "AF_name" + to_string(i + 1);
		WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(AF_Item[i][0].c_str()), CA2CT(EEPROM_Map.c_str()));
		item = "AF_start" + to_string(i + 1);
		WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(AF_Item[i][1].c_str()), CA2CT(EEPROM_Map.c_str()));
		item = "af_min" + to_string(i + 1);
		WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(AF_Item[i][2].c_str()), CA2CT(EEPROM_Map.c_str()));
		item = "af_max" + to_string(i + 1);
		WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(AF_Item[i][3].c_str()), CA2CT(EEPROM_Map.c_str()));
	}

	for (int i = 0; i < 10; i++)
		for (int k = 0; k < 4; k++) {
			string item = "AF_info" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(AF_info_Item[i][k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 4; i++) {
		string item = "SFR_name" + to_string(i + 1);
		WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(SFR_Item[i][0].c_str()), CA2CT(EEPROM_Map.c_str()));
		SFR_Item[i][0] = CT2A(lpTexts);
		item = "SFR_grade" + to_string(i + 1);
		WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(SFR_Item[i][1].c_str()), CA2CT(EEPROM_Map.c_str()));
		SFR_Item[i][1] = CT2A(lpTexts);
		item = "SFR_start" + to_string(i + 1);
		WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(SFR_Item[i][2].c_str()), CA2CT(EEPROM_Map.c_str()));
		SFR_Item[i][2] = CT2A(lpTexts);
		item = "SFR_cnt" + to_string(i + 1);
		WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(SFR_Item[i][3].c_str()), CA2CT(EEPROM_Map.c_str()));
		SFR_Item[i][2] = CT2A(lpTexts);
	}

	for (int i = 0; i < 16; i++)
		for (int k = 0; k < 3; k++) {
			string item = "PDAF_info" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(PDAF_info_Item[i][k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 10; i++)
		for (int k = 0; k < 3; k++) {
			string item = "Fix_Data" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(Fix_Data_Item[i][k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 10; i++)
		for (int k = 0; k < 4; k++) {
			string item = "Gmap_Item" + to_string(i + 1) + to_string(k + 1);
			if (k < 3) {
				WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(Gmap_Item[i][k].c_str()), CA2CT(EEPROM_Map.c_str()));
			}else {
				WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(to_string(Gmap_Item3[i]).c_str()), CA2CT(EEPROM_Map.c_str()));
			}
		}

	for (int i = 0; i < 14; i++)
		for (int k = 0; k < 3; k++) {
			string item = "PD_Item" + to_string(i + 1) + to_string(k + 1);
			if (k < 2) {
				WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(PD_Item[i][k].c_str()), CA2CT(EEPROM_Map.c_str()));
			}	else {
				WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(to_string(PD_Item3[i]).c_str()), CA2CT(EEPROM_Map.c_str()));
			}
		}

	for (int i = 0; i < 10; i++)
		for (int k = 0; k < 2; k++) {
			string item = "History_info" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(History_Data[i].item[k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 24; i++)
		for (int k = 0; k < 4; k++) {
			string item = "OIS_info" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(OIS_info_Item[i][k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 8; i++)
		for (int k = 0; k < 3; k++){
			string item = "OIS_data" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(OIS_data_Item[i][k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 12; i++) {
		string item = "AA_Item" + to_string(i + 1); 
		WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(AA_Item[i].c_str()), CA2CT(EEPROM_Map.c_str()));
	}

	for (int i = 0; i < 18; i++)
		for (int k = 0; k < 6; k++) {
			string item = "Value_data" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(sData_Item[i][k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 20; i++)
		for (int k = 0; k < 3; k++) {
			string item = "Seg_info" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(Seg_Data[i].item [k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 12; i++)
		for (int k = 0; k < 2; k++) {
			string item = "AEC_info" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(AEC_Data[i].item[k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 1; i++)
		for (int k = 0; k < 2; k++) {
			string item = "QSC_info" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(QSC_Item.item[k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}

	for (int i = 0; i < 8; i++)
		for (int k = 0; k < 4; k++) {
			string item = "LSC_info" + to_string(i + 1) + to_string(k + 1);
			if (k < 3) {
				WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(QC_LSC_Data[i].item[k].c_str()), CA2CT(EEPROM_Map.c_str()));
			}else {
				WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(to_string(LSC_Item3[i]).c_str()), CA2CT(EEPROM_Map.c_str()));
			}
		}

	for (int i = 0; i < 3; i++)
		for (int k = 0; k < 3; k++) {
			string item = "ZOOM_data" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(ZOOM_Item[i][k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}
	for (int k = 0; k < 8; k++) {
		string item = "Magnification_info" + to_string(k + 1);
		WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(Magnification[k].c_str()), CA2CT(EEPROM_Map.c_str()));
	}

	for (int i = 0; i < 6; i++) {
		for (int k = 0; k < 3; k++) {
			string item = "Same_info" + to_string(i + 1) + to_string(k + 1);
			WritePrivateProfileString(TEXT("EEPROM_Address"), CA2CT(item.c_str()), CA2CT(Same_Item[i].item[k].c_str()), CA2CT(EEPROM_Map.c_str()));
		}
	}

	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("SFR_Format"), CA2CT(to_string(SFR_Format).c_str()), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Year_H"), CA2CT(product_Date.item[0].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Year_L"), CA2CT(product_Date.item[1].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Month"), CA2CT(product_Date.item[2].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Day"), CA2CT(product_Date.item[3].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Hour"), CA2CT(product_Date.item[4].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Minute"), CA2CT(product_Date.item[5].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Second"), CA2CT(product_Date.item[6].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Factory"), CA2CT(product_Date.item[7].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Line"), CA2CT(product_Date.item[8].c_str()), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("QR_start"), CA2CT(QR_Data.item[0].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("QR_end"), CA2CT(QR_Data.item[1].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("QR_spec"), CA2CT(QR_Data.item[2].c_str()), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_start"), CA2CT(Fuse_Data.item[0].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_end"), CA2CT(Fuse_Data.item[1].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_spec"), CA2CT(Fuse_Data.item[2].c_str()), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_start2"), CA2CT(Fuse_Data2.item[0].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_end2"), CA2CT(Fuse_Data2.item[1].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("Fuse_spec2"), CA2CT(Fuse_Data2.item[2].c_str()), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("MTK_LSC_Data11"), CA2CT(MTK_LSC_Data[0].item[0].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("MTK_LSC_Data12"), CA2CT(MTK_LSC_Data[0].item[1].c_str()), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("MTK_LSC_Data21"), CA2CT(MTK_LSC_Data[1].item[0].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("MTK_LSC_Data22"), CA2CT(MTK_LSC_Data[1].item[1].c_str()), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("shift_start"), CA2CT(shift_Item[0].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("shift_byte"), CA2CT(shift_Item[1].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("shift_point"), CA2CT(shift_Item[2].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("shift_start_2"), CA2CT(shift_Item[3].c_str()), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("cross1"), CA2CT(cross_Item[0].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("cross2"), CA2CT(cross_Item[1].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("cross3"), CA2CT(cross_Item[2].c_str()), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("akm_cross1"), CA2CT(akm_cross_Item[0].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("akm_cross2"), CA2CT(akm_cross_Item[1].c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Address"), TEXT("akm_cross3"), CA2CT(akm_cross_Item[2].c_str()), CA2CT(EEPROM_Map.c_str()));

	/////////////////////////////////EEPROM_Set
	WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("noData"), CA2CT(noData.hex.c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("validFlag"), CA2CT(Flag[0].hex.c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("invalidFlag"), CA2CT(Flag[1].hex.c_str()), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("HL"), CA2CT(to_string(HL).c_str()), CA2CT(EEPROM_Map.c_str()));

	if (ui.radioButton_255->isChecked())
		WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("checkDivisor"), TEXT("0"), CA2CT(EEPROM_Map.c_str()));
	else if (ui.radioButton_255_1->isChecked())
		WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("checkDivisor"), TEXT("1"), CA2CT(EEPROM_Map.c_str()));
	else if (ui.radioButton_256->isChecked())
		WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("checkDivisor"), TEXT("2"), CA2CT(EEPROM_Map.c_str()));
	else if (ui.radioButton_CRC32->isChecked())
		WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("checkDivisor"), TEXT("4"), CA2CT(EEPROM_Map.c_str()));
	else if (ui.radioButton_FFFF->isChecked())
		WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("checkDivisor"), TEXT("8"), CA2CT(EEPROM_Map.c_str()));

	WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("selection"), CA2CT(to_string(selection).c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("DataFormat"), CA2CT(to_string(DataFormat).c_str()), CA2CT(EEPROM_Map.c_str()));
	WritePrivateProfileString(TEXT("EEPROM_Set"), TEXT("first_Pixel"), CA2CT(to_string(first_Pixel).c_str()), CA2CT(EEPROM_Map.c_str()));

	save_EEPROM_Setting();
}


EEPROM_Data_Verifier::EEPROM_Data_Verifier(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.output->insertPlainText("Please Select Model: \n");
	ifstream in(".\\Setting\\EEPROM_Tool_Setting.ini");
	ostringstream outStr;
	outStr << in.rdbuf();
	string outContent = outStr.str();
	ui.output->insertPlainText(outContent.c_str());

	modelSelect = GetPrivateProfileInt(_T("Default_Setting"), TEXT("Model_Select"), 1, TEXT(".\\Setting\\EEPROM_Tool_Setting.ini"));
	ui.model->setText(QString::number(modelSelect, 10));

	selectModel();
	load_EEPROM_Address();

}


void EEPROM_Data_Verifier::DisplayOutput() {

	ui.output->clear();
	ui.output->setFontPointSize(9);
	ifstream in(".\\MemoryParseData.txt");
	ostringstream outStr;
	outStr << in.rdbuf();
	string outContent = outStr.str();
	ui.output->insertPlainText(outContent.c_str());
}


int char_Out(int e) {
	int tmp = DecData[e];
	if (tmp > 0x7F)
		tmp = tmp - 0x100;
	return tmp;
}

short short_Out(int e, bool H_L) {
	int tmp = 0;
	if (H_L) {
		tmp = DecData[e] * 256 + DecData[e + 1];
	}
	else {
		tmp = DecData[e + 1] * 256 + DecData[e];
	}
	if (tmp > 0x7FFF)
		tmp = tmp - 65536;

	return tmp ;

}

int ushort_Out(int e, bool H_L) {
	int tmp = 0;
	if (H_L) {
		tmp = DecData[e] * 256 + DecData[e + 1];
	}
	else {
		tmp = DecData[e + 1] * 256 + DecData[e];
	}
	return tmp;
}

float gyro_Out(int e, bool H_L) {

	unsigned int tmp = 0;
	if (H_L) {
		for (int i = 0; i < 4; i++) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
	}
	else {
		for (int i = 3; i >= 0; i--) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
	}
	return (float)tmp / 0x80000000;
}

float Dcm_out2(int e, bool H_L) {

	unsigned int tmp = 0;
	if (H_L) {
		for (int i = 0; i < 2; i++) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
	}
	else {
		for (int i = 1; i >= 0; i--) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
	}
	return (float)tmp / 0x8000;
}

float SR_Out_HL(int e, bool H_L) {

	float tmp = 0;
	if (H_L) {
		tmp = DecData[e] + DecData[e+1] / 100.0;
	}
	else {
		tmp = DecData[e+1] + DecData[e] / 100.0;
	}
	return tmp;
}

float SR_Out_Hex(int e, bool H_L) {

	float tmp = 0;
	if (H_L) {
		tmp = (DecData[e]*256 + DecData[e + 1]) / 10.0;
	}
	else {
		tmp = (DecData[e + 1]*256 + DecData[e]) / 10.0;
	}
	return tmp;
}

float SR_Out100(int e, bool H_L) {

	float tmp = 0;
	if (H_L) {
		tmp = (DecData[e] * 256 + DecData[e + 1]) / 100.0;
	}
	else {
		tmp = (DecData[e + 1] * 256 + DecData[e]) / 100.0;
	}
	return tmp;
}

int int_Out(int e, bool H_L) {

	unsigned int tmp = 0;

	if (H_L) {
		for (int i = 0; i < 4; i++) {
			tmp *= 256;
			tmp += DecData[e + i];
			useData[e + i] = 1;
		}
	}
	else {
		for (int i = 3; i >= 0; i--) {
			tmp *= 256;
			tmp += DecData[e + i];
			useData[e + i] = 1;
		}
	}
	int* fp = (int*)&tmp;
	return *fp;
}


unsigned int uint_Out(int e, bool H_L) {

	unsigned int tmp = 0;
	if (H_L) {
		for (int i = 0; i < 4; i++) {
			tmp *= 256;
			tmp += DecData[e + i];
			useData[e + i] = 1;
		}
	}
	else {
		for (int i = 3; i >= 0; i--) {
			tmp *= 256;
			tmp += DecData[e + i];
			useData[e + i] = 1;
		}
	}
	return tmp;
}


float flt_Out(int e, bool H_L) {

	unsigned int tmp = 0;
	float* fp = (float*)&tmp;
	if (H_L) {
		for (int i = 0; i < 4; i++) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
	}
	else {
		for (int i = 3; i >= 0; i--) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
	}
	return *fp ;
}

float flt_Out2301(int e, bool H_L) {

	unsigned int tmp = 0;
	float* fp = (float*)&tmp;
	if (H_L) {
		tmp = (DecData[e + 1]<<24)+(DecData[e] << 16)+(DecData[e+3] << 8) + DecData[e+2];	
	}
	else {
		tmp = (DecData[e + 2] << 24) + (DecData[e+3] << 16) + (DecData[e + 0] << 8) + DecData[e + 1];
	}
	return *fp;
}

double dbl_Out(int e, bool H_L) {

	unsigned long long tmp = 0;
	double* fp = (double*)&tmp;
	if (H_L) {
		for (int i = 0; i < 8; i++) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
	}
	else {
		for (int i = 7; i >= 0; i--) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
	}
	return *fp;
}

UINT uByte_3(int e, bool H_L) {

	unsigned long tmp = 0;
	if (H_L) {
		for (int i = 0; i < 3; i++) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
	}
	else {
		for (int i = 2; i >= 0; i--) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
	}
	return tmp;
}


bool str_cmp(const std::string& str1, const std::string& str2) {

	bool ret = true;
	int x = 0;

	while (x < str1.length() && x < str1.length()) {
		if (str1[x] != ' '&&str1[x] != '	'&&str2[x] != ' '&&str2[x] != '	') {
			if (str1[x] != str2[x]) {
				return false;
			}
		}
		x++;
	}
	return ret;

}


void EEPROM_Data_Verifier::selectModel() {
	//Load EEPROM_Map.ini
	modelSelect = ui.model->document()->toPlainText().toInt();

	TCHAR lpTexts[50];
	string temp_model = to_string(modelSelect);
	GetPrivateProfileString(TEXT("Model_List"), CA2CT(temp_model.c_str()), TEXT("EEPROM_Map_V1_797.ini"), lpTexts, 50, TEXT(".\\Setting\\EEPROM_Tool_Setting.ini"));
	EEPROM_Map = CT2A(lpTexts);

	string str1 = ".\\Setting\\";
	EEPROM_Map = str1 + EEPROM_Map;
	//	save_EEPROM_Setting();
}


int EEPROM_Data_Verifier::LSC_Parse(int start, int group) {

	int e = start, ret=0;
	int W = 17, H = 13;
	if (e > 0) {
		if (LSC_Item3[group]==1){

			for (int i = 0; i < H; i++)
				for (int j = 0; j < W; j++) {
					for (int k = 0; k < 4; k++) {
						LSC[i][j][k] = DecData[e + k];
						LSC[i][j][k] += 256 * ((DecData[e + 4] >> (6 - 2 * k)) & 3);
						useData[e + k] = 1;
					}
					useData[e + 5] = 1;
					e += 5;			
					if ((i * 17 + j + 1) % 51 == 0) {
						e++;
					}
				}
		}
		else if (LSC_Item3[group] == 0) {

			for (int i = 0; i < H; i++)
				for (int j = 0; j < W; j++)
					for (int k = 0; k < 4; k++) {

						if (HL&1)
							LSC[i][j][k] = 256 * DecData[e] + DecData[e + 1];
						else
							LSC[i][j][k] = 256 * DecData[e + 1] + DecData[e];

						useData[e + 1] = useData[e] = 1;
						e += 2;
					}
		}
		else if (LSC_Item3[group] == 2) {
			int offset = atoi(QC_LSC_Data[group].item[2].c_str());
			W = 13, H = 9 ;
			for (int k = 0; k < 3; k++)
				for (int i = 0; i < H; i++)
					for (int j = 0; j < W; j++){
						LSC[i][j][k] = DecData[e];
						useData[e] = 1;
						e += 1;
					}
			e = start + offset;

			for (int i = 0; i < H; i++)
				for (int j = 0; j < W; j++) {
					LSC[i][j][3] = DecData[e];
					useData[e] = 1;
					e += 1;
				}
		}
		else if (LSC_Item3[group] == 4) {
			int offset = atoi(QC_LSC_Data[group].item[2].c_str());
			e += offset;
			W = 33, H = 25;
			for (int i = 0; i < H; i++)
				for (int j = 0; j < W; j++) {
					LSC[i][j][0] = DecData[e] << 4 + DecData[e + 1] >> 4;
					LSC[i][j][1] = (DecData[e + 1] & 0xF) << 8 + DecData[e + 2];
					LSC[i][j][2] = DecData[e + 3] << 4 + DecData[e + 4] >> 4;
					LSC[i][j][3] = (DecData[e + 4] & 0xF) << 8 + DecData[e + 5];
					for (int k = 0; k < 6; k++){
						useData[e + k] = 1;
					}
					e += 6;
				}
		}
		else if (LSC_Item3[group] == 5) {
			for (int k = 0; k < 4; k++)
				for (int i = 0; i < H; i++)
					for (int j = 0; j < W; j++) {

						if (HL & 1)
							LSC[i][j][k] = 256 * DecData[e] + DecData[e + 1];
						else
							LSC[i][j][k] = 256 * DecData[e + 1] + DecData[e];

						useData[e + 1] = useData[e] = 1;
						e += 2;
					}
		}

		if (mode == 0) {
			fout << "~~~Red Channel LSC:" << endl;
			for (int i = 0; i < H; i++) {
				for (int j = 0; j < W; j++) {
					fout << LSC[i][j][0] << "	";
				}
				fout << endl;
			}

			fout << "~~~Gr Channel LSC:" << endl;
			for (int i = 0; i < H; i++) {
				for (int j = 0; j < W; j++) {
					fout << LSC[i][j][1] << "	";
				}
				fout << endl;
			}

			fout << "~~~Gb Channel LSC:" << endl;
			for (int i = 0; i < H; i++) {
				for (int j = 0; j < W; j++) {
					fout << LSC[i][j][2] << "	";
				}
				fout << endl;
			}

			fout << "~~~Blue Channel LSC:" << endl;
			for (int i = 0; i < H; i++) {
				for (int j = 0; j < W; j++) {
					fout << LSC[i][j][3] << "	";
				}
				fout << endl;
			}
			fout << endl;
		}

		if (LSC[H/2][W/2][0]>0 && LSC[H/2][W/2][0]<0xF000) {
			ret = QC_LSC_FP_Check(LSC, LSC_Item3[group]);
			if (ret != 0) {
				string s = "LSC"+to_string(group+1)+" FP NG, please check FP_log File!\n";
				ui.log->insertPlainText(s.c_str());
			}
		}
		else {
			ui.log->insertPlainText("LSC Data is invalid!\n");
			ret |= 1;
		}

		fout << endl;
	}

	return ret;
}


int EEPROM_Data_Verifier::MTK_LSC_Parse(int start) {

	int e = start+68, ret=0;
	unsigned long long sumXsum = 0,FFcnt = 0, Zerocnt = 0;

	if (e > 0) {
		for (int i = 0; i < 15; i++){
			for (int j = 0; j < 15; j++) {
				for (int k = 0; k < 4; k++) {

					MTK_LSC[i][j][k] = 256 * DecData[e + 1] + DecData[e];
					sumXsum = (sumXsum * 65536 + MTK_LSC[i][j][k]) % 4000000007;
					useData[e + 1] = useData[e] = 1;
					e += 2;

					if (DecData[k] == 255&& DecData[k+1] == 255) {
						FFcnt+=2;
						if (FFcnt == 8) {
							ret |= 1;
						}
					}
					else {
						FFcnt = 0;
					}
					if (DecData[k] == 0 && DecData[k + 1] == 0) {
						Zerocnt+=2;
						if (FFcnt == 16) {
							ret |= 1;
						}
					}
					else {
						Zerocnt = 0;
					}
				}
			}
		}

		current_Hash.MTK_LSC = sumXsum;

		if ((ret |= 1) > 0)
			current_Hash.MTK_LSC = 0;
	
		if (mode == 0){
			fout << "-------MTK LSC CAL Data------" << endl;
			fout << "~~~CH0 Channel MTK_LSC:" << endl;
			for (int i = 0; i < 15; i++) {
				for (int j = 0; j < 15; j++) {
					fout << MTK_LSC[i][j][0] << "	";
				}
				fout << endl;
			}

			fout << "~~~CH1 Channel MTK_LSC:" << endl;
			for (int i = 0; i < 15; i++) {
				for (int j = 0; j < 15; j++) {
					fout << MTK_LSC[i][j][1] << "	";
				}
				fout << endl;
			}

			fout << "~~~CH2 Channel MTK_LSC:" << endl;
			for (int i = 0; i < 15; i++) {
				for (int j = 0; j < 15; j++) {
					fout << MTK_LSC[i][j][2] << "	";
				}
				fout << endl;
			}

			fout << "~~~CH3 Channel MTK_LSC:" << endl;
			for (int i = 0; i < 15; i++) {
				for (int j = 0; j < 15; j++) {
					fout << MTK_LSC[i][j][3] << "	";
				}
				fout << endl;
			}
			fout << endl;
		}

		if (MTK_LSC[7][7][0]>0 && MTK_LSC[7][7][0]<66000) {
			ret = MTK_LSC_FP_Check(MTK_LSC, first_Pixel);
			if (ret != 0) {
				ui.log->insertPlainText("MTK LSC FP NG, please check FP_log File!\n");
			}
		}
		else {
			ui.log->insertPlainText("MTK LSC Data is invalid!\n");
			ret |= 1;
		}
		fout << endl;
	}
	return ret;
}


void lscLSI_Parse(int S, int E) {

	if (S > 0) {
		int e = S + 1;		unsigned int tmp = 0;		float* fp = (float*)&tmp;

		fout << "~~~~~~~~~~ LSI LSC ~~~~~~~~~~:	" << endl;
		fout << "LSI LSC Version:	" << DecData[e] * 256 + DecData[e + 1] << endl;
		e += 2;
		fout << "Grid width for gridBugger[0]:	" << (int)DecData[e++] << endl;
		fout << "Grid height for gridBugger[0]:	" << (int)DecData[e++] << endl;
		fout << "Grid width for gridBugger[1]:	" << (int)DecData[e++] << endl;
		fout << "Grid height for gridBugger[1]:	" << (int)DecData[e++] << endl;
		fout << "Bit Length for Grid[0]:	" << (int)DecData[e++] << endl;
		fout << "Bit Length for Grid[1]:	" << (int)DecData[e++] << endl;
		fout << "Orientation(Mirror_flip:	0x" << D[e][0] << D[e][1] << D[e + 1][0] << D[e + 1][1] << endl;
		e += 2;
		fout << "Center position X:	" << DecData[e] * 256 + DecData[e + 1] << endl;
		e += 2;
		fout << "Center position Y:	" << DecData[e] * 256 + DecData[e + 1] << endl;
		e += 2;
		fout << "Scale coefficient:	" << DecData[e] * 256 + DecData[e + 1] << endl;
		e += 2;
		tmp = 0;
		for (int i = 0; i < 4; i++) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
		fout << "Polynomial coefficient A:	" << *fp << endl;
		e += 4;
		tmp = 0;
		for (int i = 0; i < 4; i++) {
			tmp *= 256;
			tmp += DecData[e + i];
		}
		fout << "Polynomial coefficient B:	" << *fp << endl;
		e += 4;

		for (int i = 0; i < 25; i++)
			for (int j = 0; j < 33; j++) {
				LSC_LSI1[i][j][0] = DecData[e] * 16 + (DecData[e + 1] >> 4);
				LSC_LSI1[i][j][1] = (DecData[e + 1] & 0x0F) * 256 + DecData[e + 2];
				LSC_LSI1[i][j][2] = DecData[e + 3] * 16 + (DecData[e + 4] >> 4);
				LSC_LSI1[i][j][3] = (DecData[e + 4] & 0x0F) * 256 + DecData[e + 5];
				e += 6;
			}

		fout << "LSI lsc buffer[0]:	" << endl;

		for (int k = 0; k < 4; k++) {
			for (int i = 0; i < 25; i++) {
				for (int j = 0; j < 33; j++) {
					fout << LSC_LSI1[i][j][k] << "	";
				}
				fout << endl;
			}
			fout << endl;
		}

		e += 2;
		for (int i = 0; i < 11; i++)
			for (int j = 0; j < 13; j++) {
				LSC_LSI2[i][j][0] = DecData[e] * 16 + (DecData[e + 1] >> 4);
				LSC_LSI2[i][j][1] = (DecData[e + 1] & 0x0F) * 256 + DecData[e + 2];
				LSC_LSI2[i][j][2] = DecData[e + 3] * 16 + (DecData[e + 4] >> 4);
				LSC_LSI2[i][j][3] = (DecData[e + 4] & 0x0F) * 256 + DecData[e + 5];
				e += 6;
			}

		fout << "LSI lsc buffer[1]:	" << endl;

		for (int k = 0; k < 4; k++) {
			for (int i = 0; i < 11; i++) {
				for (int j = 0; j < 13; j++) {
					fout << LSC_LSI2[i][j][k] << "	";
				}
				fout << endl;
			}
			fout << endl;
		}
	}

}


void lsc_MTK_Parse(int S, int E) {

	if (S > 0) {
		int e = S + 68;		unsigned int tmp = 0;		float* fp = (float*)&tmp;

		fout << "~~~~~~~~~~ MTK LSC ~~~~~~~~~~:	" << endl;
		/*for (int i = 0; i < 4; i++) {
		tmp *= 256;
		tmp += DecData[e + i];
		}
		fout << "Polynomial coefficient A:	" << *fp << endl;
		e += 4;
		tmp = 0;
		for (int i = 0; i < 4; i++) {
		tmp *= 256;
		tmp += DecData[e + i];
		}
		fout << "Polynomial coefficient B:	" << *fp << endl;
		e += 4;
		*/
		for (int i = 0; i < 15; i++)
			for (int j = 0; j < 15; j++)
				for (int k = 0; k < 4; k++) {
					LSC_LSI1[i][j][k] = DecData[e] + DecData[e + 1] * 256;
					e += 2;
				}

		for (int k = 0; k < 4; k++) {
			for (int i = 0; i < 15; i++) {
				for (int j = 0; j < 15; j++) {
					fout << LSC_LSI1[i][j][k] << "	";
				}
				fout << endl;
			}
			fout << endl;
		}

	}
}


void EEPROM_Data_Verifier::on_pushButton_clear_clicked()
{
	ui.pushButton_parser->setEnabled(true);
	ui.pushButton_golden->setEnabled(true);

	memset(DecData, 0, sizeof(DecData));
	memset(D, 0, sizeof(D));
	ui.input->document()->clear();
	ui.output->clear();
	ui.log->clear();
	ui.OK_cnt->setText("0");
	ui.NG_cnt->setText("0");

}


int EEPROM_Data_Verifier::CheckSum_Check() {

	int ret = 0;
	customer_end = 0;
	
	fout << "-------Check Sum Compare------" << endl;
	fout << "(ChkSum)	(EEP_D)	(Calc.)	(Flag)	" << endl;
	customer_Data_END = 0;

	for (int i = 0; i < 40; i++) {
		unsigned long long tmp = 0, d = 0, temp_cksm=0;
		if (checkSum[i].end > 0) {
			if(checkSum[i].item[1].length()>2&&i<38)
				map_Push(checkSum[i].flag, "Flag of " + checkSum[i].item[0], " ", Flag1);
			if (i < 38) {
				map_Push(checkSum[i].checksum, "CheckSum of " + checkSum[i].item[0], " ", CheckSum1);
				for (int k = checkSum[i].start; k <= checkSum[i].end; k++) {
					tmp += DecData[k];
					useData[k] = 1;
				}
				fout << checkSum[i].item[0] << "	";

			}
			else {
				map_Push(checkSum[i].checksum, "Total CheckSum", " ", CheckSum1);
				for (int k = checkSum[i].start; k < checkSum[i].Except_S; k++) {
					tmp += DecData[k];
					useData[k] = 1;
				}
				for (int k = checkSum[i].Except_E+1; k <= checkSum[i].end; k++) {
					tmp += DecData[k];
					useData[k] = 1;
				}
				fout << "Tsum_" <<i-37<< "	";
			}
  			if (checkDivisor == 0)
				d = tmp % 255;
			else if (checkDivisor == 1)
				d = tmp % 255 + 1;
			else if (checkDivisor == 2)
				d = tmp % 0x10000;
			else if (checkDivisor == 4)
				d = Get_SEC_CRC32(DecData, checkSum[i].start, checkSum[i].end- checkSum[i].start+1);
			else if (checkDivisor == 8)
				d = tmp % 0xFFFF+1;

			getHex(d);
			string chk_str = "";
		
			if (checkDivisor < 2) {
				fout << D[checkSum[i].checksum][0] << D[checkSum[i].checksum][1] << "	";
				chk_str += chk[0];
				chk_str += chk[1];	
				fout << chk_str << "	";
				temp_cksm = DecData[checkSum[i].checksum];
			}
			else if (checkDivisor == 2|| checkDivisor == 8) {
				temp_cksm = ushort_Out(checkSum[i].checksum, HL&1);
				fout << D[checkSum[i].checksum][0] << D[checkSum[i].checksum][1] << D[checkSum[i].checksum+1][0] << D[checkSum[i].checksum+1][1] <<"	";
				getHex(d / 256);
				chk_str += chk[0];
				chk_str += chk[1];
				getHex(d % 256);
				chk_str += chk[0];
				chk_str += chk[1];
				fout << chk_str << "	";
			}
			else if (checkDivisor == 4) {
				temp_cksm = uint_Out(checkSum[i].checksum, HL & 1);
				fout << D[checkSum[i].checksum+3][0] << D[checkSum[i].checksum+3][1] << D[checkSum[i].checksum + 2][0] << D[checkSum[i].checksum + 2][1];
				fout << D[checkSum[i].checksum+1][0] << D[checkSum[i].checksum+1][1] << D[checkSum[i].checksum][0] << D[checkSum[i].checksum][1] << "	";
				getHex(d / 0xFFFFFF);
				chk_str += chk[0];
				chk_str += chk[1];
				getHex((d&0xFFFFFF)/ 0xFFFF);
				chk_str += chk[0];
				chk_str += chk[1];
				getHex((d & 0xFFFF) / 0xFF);
				chk_str += chk[0];
				chk_str += chk[1];
				getHex((d & 0xFF));
				chk_str += chk[0];
				chk_str += chk[1];
				fout << chk_str << "	";
			}

			if (customer_Data_END < checkSum[i].checksum)
				customer_Data_END = checkSum[i].checksum;

			if (checkSum[i].checksum>customer_end)
				customer_end = checkSum[i].checksum;

			if (d != temp_cksm) {

				QString strDisplay = checkSum[i].item[0].c_str();
				strDisplay = strDisplay + " CheckSum in ";
				strDisplay += address2hex(checkSum[i].checksum);
				strDisplay += " NG: 0x";
				strDisplay.append(D[checkSum[i].checksum][0]);
				strDisplay.append(D[checkSum[i].checksum][1]);
				if (checkDivisor == 2|| checkDivisor == 8) {
					strDisplay.append(D[checkSum[i+1].checksum][0]);
					strDisplay.append(D[checkSum[i+1].checksum][1]);
				}
				strDisplay += ", Calc is 0x";
				strDisplay.append(chk_str.c_str());
				strDisplay += '\n';
					ui.log->insertPlainText(strDisplay);
				ret |= 1;
			}

			if (i < 38&&checkSum[i].flag >= 0){
				fout << D[checkSum[i].flag][0] << D[checkSum[i].flag][1] << "	";
				if (Flag[0].dec != DecData[checkSum[i].flag]) {
					QString strDisplay = checkSum[i].item[0].c_str();
					strDisplay = strDisplay + " Flag NG\n ";
					ui.log->insertPlainText(strDisplay);
					ret |= 2;
				}
			}
			fout << endl;
		}		
	}
	fout << endl;
	return ret;
}


int EEPROM_Data_Verifier::drift_Parse() {
	//AF Drift Compensation Data:
	int e = unstringHex2int(shift_Item[0]), ret = 0;
	string str;

	bool drift_HL = selection & 32;

	if (e > 0) {
		ret = 0;
		if (mode == 0)
			fout << "--------OIS Shift Compensation Data-------" << endl;

		int step = unstringHex2int(shift_Item[1]);
		int point = unstringHex2int(shift_Item[2]);
		int Y_start = unstringHex2int(shift_Item[3]);
		if (!drift_HL) {
			for (int i = 0; i < point; i++) {
				if(step==4)
					shift_Data[0][i] = int_Out(e, 0);
				if (step == 2)
					shift_Data[0][i] = short_Out(e, 0);
				if (step == 1)
					shift_Data[0][i] = char_Out(e);

				if (mode == 0) {
					fout << "X_Position_" << to_string(i) << ":	";
					//shift_Data[0][i] = int_Out(e, HL&1);
					fout << shift_Data[0][i] << endl;
					str = "X_Position_" + to_string(i);
					map_Push(e, str + " [0]_L", " ", info1);
					map_Push(e + 1, str + " [1]_L", " ", info1);
					map_Push(e + 2, str + " [2]_H", " ", info1);
					map_Push(e + 3, str + " [3]_H", " ", info1);
				}
				e += step;
			}
			fout << endl;
			e = Y_start;
			for (int i = 0; i < point; i++) {
				if (step == 4)
					shift_Data[1][i] = int_Out(e, 0);
				if (step == 2)
					shift_Data[0][i] = short_Out(e, 0);
				if (step == 1)
					shift_Data[0][i] = char_Out(e);
				if (mode == 0) {
					fout << "Y_Position_" << to_string(i) << ":	";
					//shift_Data[1][i] = int_Out(e, HL&1);
					fout << shift_Data[1][i] << endl;
					str = "Y_Position_" + to_string(i);
					map_Push(e, str + " [0]_L", " ", info1);
					map_Push(e + 1, str + " [1]_L", " ", info1);
					map_Push(e + 2, str + " [2]_H", " ", info1);
					map_Push(e + 3, str + " [3]_H", " ", info1);
				}
				e += step;
			}
		}
		else {
			if (step == 4){
				e += 2;
				for (int i = 0; i < point; i++) {
					shift_Data[0][i] = int_Out(e, 1);
					if (mode == 0) {
						fout << "X_Position_" << to_string(i) << ":	";
						fout << shift_Data[0][i] << endl;
						str = "X_Position_" + to_string(i);
						map_Push(e, str + " [3]_H", " ", info1);
						map_Push(e + 1, str + " [2]_H", " ", info1);
						map_Push(e + 2, str + " [1]_L", " ", info1);
						map_Push(e + 3, str + " [0]_L", " ", info1);
					}
					e += step;
				}
				fout << endl;
				e = Y_start;
				for (int i = 0; i < point; i++) {
					shift_Data[0][i] = int_Out(e, 1);
					if (mode == 0) {
						fout << "Y_Position_" << to_string(i) << ":	";
						fout << shift_Data[1][i] << endl;
						str = "Y_Position_" + to_string(i);
						map_Push(e, str + " [3]_H", " ", info1);
						map_Push(e + 1, str + " [2]_H", " ", info1);
						map_Push(e + 2, str + " [1]_L", " ", info1);
						map_Push(e + 3, str + " [0]_L", " ", info1);
					}
					e += step;
				}
			}
			else if (step == 2) {
				for (int i = 0; i < point; i++) {
					shift_Data[0][i] = short_Out(e, 1);
					if (mode == 0) {
						fout << "X_Position_" << to_string(i) << ":	";
						fout << shift_Data[0][i] << endl;
						str = "X_Position_" + to_string(i);
						map_Push(e, str + " _H", " ", info1);
						map_Push(e + 1, str + " _L", " ", info1);
					}
					e += step;
				}
				fout << endl;
				e = Y_start;
				for (int i = 0; i < point; i++) {
					shift_Data[1][i] = short_Out(e, 1);
					if (mode == 0){
						fout << "Y_Position_" << to_string(i) << ":	";
						fout << shift_Data[1][i] << endl;
						str = "Y_Position_" + to_string(i);
						map_Push(e, str + "_H", " ", info1);
						map_Push(e + 1, str + " _L", " ", info1);
					}
					e += step;
				}
			}
			else if (step == 1) {
				for (int i = 0; i < point; i++) {
					shift_Data[0][i] = char_Out(e);
					if (mode == 0) {
						fout << "X_Position_" << to_string(i) << ":	";
						fout << shift_Data[0][i] << endl;
						str = "X_Position_" + to_string(i);
						map_Push(e, str, " ", info1);
					}
					e += step;
				}
				fout << endl;

				e = Y_start;

				for (int i = 0; i < point; i++) {
					shift_Data[1][i] = char_Out(e);
					if (mode == 0) {
						fout << "Y_Position_" << to_string(i) << ":	";
						fout << shift_Data[1][i] << endl;
						str = "Y_Position_" + to_string(i);
						map_Push(e, str , " ", info1);
					}
					e += step;
				}
			}
		}
		ret = drift_FP_Check(shift_Data, point, step);
		if (ret > 0) {
			ui.log->insertPlainText("drift cal data FP NG!\n");
		}
		fout << endl;
	}
	return ret;
}


int EEPROM_Data_Verifier::cross_Parse() {

	int e = unstringHex2int(cross_Item[0]);
	int e1 = unstringHex2int(cross_Item[1]),ret=0;
	int Point = unstringHex2int(cross_Item[2]);
	string str;

	if (e > 0) {
		ret = 0;
		if (mode == 0)
			fout << "--------OIS Corss talk Data-------" << endl;

		if (ui.Cross_AW->isChecked()) {
			for (int i = 0; i < Point; i++) {			
				float cData = flt_Out(e, 0);
				e += 4;
				if (mode == 0)
					fout << "Linearity_X_K" << i <<":	"<< cData << endl;

			}
			for (int i = 0; i < Point; i++) {
				float cData = flt_Out(e, 0);
				e += 4;
				if (mode == 0)
					fout << "Linearity_X_B" << i << ":	" << cData << endl;
			}
			for (int i = 0; i < Point; i++) {
				float cData = flt_Out(e1, 0);
				e1 += 4;
				if (mode == 0)
					fout << "Linearity_Y_K" << i << ":	" << cData << endl;

			}
			for (int i = 0; i < Point; i++) {
				float cData = flt_Out(e1, 0);
				e1 += 4;
				if (mode == 0)
					fout << "Linearity_Y_B" << i << ":	" << cData << endl;
			}
		}
		else {
			//int X = unstringHex2int(cross_Item[0]);
			//int Y = unstringHex2int(cross_Item[1]);
			if (HL & 1 == 0) {
				for (int i = 0; i < Point; i++) {

					Cross_DW_before[0][i] = short_Out(e, 0);
					if (mode == 0) {
						fout << "Crosstalk position_XX" << to_string(i) << ":	";
						fout << Cross_DW_before[0][i] << endl;
						str = "XonXmove" + to_string(i);
						map_Push(e, str + " _L", " ", info1);
						map_Push(e + 1, str + " _H", " ", info1);
					}
					e += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 0; i < Point; i++) {
					if (mode == 0) {
						Cross_DW_before[1][i] = short_Out(e, 0);
						fout << "Crosstalk position_YX" << to_string(i) << ":	";
						fout << Cross_DW_before[1][i] << endl;
						str = "YonXmove" + to_string(i);
						map_Push(e, str + " _L", " ", info1);
						map_Push(e + 1, str + " _H", " ", info1);
					}
					e += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 7; i < Point * 2; i++) {
					Cross_DW_before[0][i] = short_Out(e, 0);
					if (mode == 0) {
						fout << "Crosstalk position_XY" << to_string(i) << ":	";
						fout << Cross_DW_before[0][i] << endl;
						str = "XonYmove" + to_string(i);
						map_Push(e, str + " _L", " ", info1);
						map_Push(e + 1, str + " _H", " ", info1);
					}
					e += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 0; i < Point; i++) {
					Cross_DW_before[1][i] = short_Out(e, 0);
					if (mode == 0) {
						fout << "Crosstalk position_YY" << to_string(i) << ":	";
						fout << Cross_DW_before[1][i] << endl;
						str = "YonYmove" + to_string(i);
						map_Push(e, str + " _L", " ", info1);
						map_Push(e + 1, str + " _H", " ", info1);
					}
					e += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 0; i < Point; i++) {
					Cross_DW_after[0][i] = short_Out(e1, 0);
					if (mode == 0) {
						fout << "Crosstalk position_XX" << to_string(i) << ":	";
						fout << Cross_DW_after[0][i] << endl;
						str = "XonXmove" + to_string(i);
						map_Push(e1, str + " _L", " ", info1);
						map_Push(e1 + 1, str + " _H", " ", info1);
					}
					e1 += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 0; i < Point; i++) {
					Cross_DW_after[1][i] = short_Out(e1, 0);
					if (mode == 0) {
						fout << "Crosstalk position_YX" << to_string(i) << ":	";
						fout << Cross_DW_after[1][i] << endl;
						str = "XonXmove" + to_string(i);
						map_Push(e1, str + " _L", " ", info1);
						map_Push(e1 + 1, str + " _H", " ", info1);
					}
					e1 += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 7; i < Point * 2; i++) {
					Cross_DW_after[0][i] = short_Out(e1, 0);
					if (mode == 0) {
						fout << "Crosstalk position_XY" << to_string(i) << ":	";
						fout << Cross_DW_after[0][i] << endl;
						str = "XonXmove" + to_string(i);
						map_Push(e1, str + " _L", " ", info1);
						map_Push(e1 + 1, str + " _H", " ", info1);
					}
					e1 += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 7; i < Point * 2; i++) {
					Cross_DW_after[1][i] = short_Out(e1, 0);
					if (mode == 0) {
						fout << "Crosstalk position_YY" << to_string(i) << ":	";
						fout << Cross_DW_after[1][i] << endl;
						str = "XonXmove" + to_string(i);
						map_Push(e1, str + " _L", " ", info1);
						map_Push(e1 + 1, str + " _H", " ", info1);
					}
					e1 += 2;
				}
				if (mode == 0)
					fout << endl;

			}
			else {
				for (int i = 0; i < Point; i++) {
					Cross_DW_before[0][i] = short_Out(e, 1);
					if (mode == 0) {
						fout << "Crosstalk position_XX" << to_string(i) << ":	";
						fout << Cross_DW_before[0][i] << endl;
						str = "XonXmove" + to_string(i);
						map_Push(e, str + " _H", " ", info1);
						map_Push(e + 1, str + " _L", " ", info1);
					}
					e += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 0; i < Point; i++) {
					Cross_DW_before[1][i] = short_Out(e, 1);
					if (mode == 0) {
						fout << "Crosstalk position_YX" << to_string(i) << ":	";
						fout << Cross_DW_before[1][i] << endl;
						str = "YonXmove" + to_string(i);
						map_Push(e, str + " _H", " ", info1);
						map_Push(e + 1, str + " _L", " ", info1);
					}
					e += 2;
				}

				if (mode == 0)
					fout << endl;
				for (int i = 7; i < Point * 2; i++) {
					Cross_DW_before[0][i] = short_Out(e, 1);
					if (mode == 0) {
						fout << "Crosstalk position_XY" << to_string(i) << ":	";
						fout << Cross_DW_before[0][i] << endl;
						str = "XonYmove" + to_string(i);
						map_Push(e, str + " _H", " ", info1);
						map_Push(e + 1, str + " _L", " ", info1);
					}
					e += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 7; i < Point * 2; i++) {
					Cross_DW_before[1][i] = short_Out(e, 1);
					if (mode == 0) {
						fout << "Crosstalk position_YY" << to_string(i) << ":	";
						fout << Cross_DW_before[1][i] << endl;
						str = "Cross_DW_before" + to_string(i);
						map_Push(e, str + " _H", " ", info1);
						map_Push(e + 1, str + " _L", " ", info1);
					}
					e += 2;
				}
				if (mode == 0)
					fout << endl;

				for (int i = 0; i < Point * 2; i++) {
					Cross_DW_after[0][i] = short_Out(e1, 1);
					if (mode == 0) {
						fout << "Crosstalk position_XX" << to_string(i) << ":	";
						fout << Cross_DW_after[0][i] << endl;
						str = "XonXmove" + to_string(i);
						map_Push(e1, str + " _H", " ", info1);
						map_Push(e1 + 1, str + " _L", " ", info1);
					}
					e1 += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 0; i < Point; i++) {
					Cross_DW_after[1][i] = short_Out(e1, 1);
					if (mode == 0) {
						fout << "Crosstalk position_YX" << to_string(i) << ":	";
						fout << Cross_DW_after[1][i] << endl;
						str = "XonXmove" + to_string(i);
						map_Push(e1, str + " _H", " ", info1);
						map_Push(e1 + 1, str + " _L", " ", info1);
					}
					e1 += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 7; i < Point * 2; i++) {
					Cross_DW_after[0][i] = short_Out(e1, 1);
					if (mode == 0) {
						fout << "Crosstalk position_XY" << to_string(i) << ":	";
						fout << Cross_DW_after[0][i] << endl;
						str = "XonXmove" + to_string(i);
						map_Push(e1, str + " _H", " ", info1);
						map_Push(e1 + 1, str + " _L", " ", info1);
					}
					e1 += 2;
				}
				if (mode == 0)
					fout << endl;
				for (int i = 7; i < Point * 2; i++) {
					Cross_DW_after[1][i] = short_Out(e1, 1);
					if (mode == 0) {
						fout << "Crosstalk position_YY" << to_string(i) << ":	";
						fout << Cross_DW_after[1][i] << endl;
						str = "XonXmove" + to_string(i);
						map_Push(e1, str + " _H", " ", info1);
						map_Push(e1 + 1, str + " _L", " ", info1);
					}
					e1 += 2;
				}
				if (mode == 0)
					fout << endl;
			}
			if (mode == 0)
				fout << endl;

			for (int i = 0; i < 13; i++) {

				if (i < 6) {
					int x_diff = abs(Cross_DW_after[0][i] - Cross_DW_after[0][i + 1]);
					if (x_diff < LCC_CrossTalk[1] || x_diff>LCC_CrossTalk[2]) {
						ret |= 1;
					}
					int y_diff = abs(Cross_DW_after[1][i] - Cross_DW_after[1][i + 1]);
					if (y_diff > LCC_CrossTalk[0]) {
						ret |= 1;
					}
					x_diff = abs(Cross_DW_before[0][i] - Cross_DW_before[0][i + 1]);
					if (x_diff < LCC_CrossTalk[1] || x_diff>LCC_CrossTalk[2]) {
						ret |= 2;
					}
					y_diff = abs(Cross_DW_before[1][i] - Cross_DW_before[1][i + 1]);
					if (y_diff > LCC_CrossTalk[0]) {
						ret |= 2;
					}
				}
				else if (i > 6) {

					int x_diff = abs(Cross_DW_after[0][i] - Cross_DW_after[0][i + 1]);
					if (x_diff > LCC_CrossTalk[0]) {
						ret |= 1;
					}
					int y_diff = abs(Cross_DW_after[1][i] - Cross_DW_after[1][i + 1]);
					if (y_diff < LCC_CrossTalk[1] || y_diff>LCC_CrossTalk[2]) {
						ret |= 1;
					}
					x_diff = abs(Cross_DW_before[0][i] - Cross_DW_before[0][i + 1]);
					if (x_diff > LCC_CrossTalk[0]) {
						ret |= 2;
					}
					y_diff = abs(Cross_DW_before[1][i] - Cross_DW_before[1][i + 1]);
					if (y_diff<LCC_CrossTalk[1] || y_diff>LCC_CrossTalk[2]) {
						ret |= 2;
					}
				}
			}

			if (ret > 0) {
				if (mode == 0) {
					ui.log->insertPlainText("Crosstalk Data FP NG!\n");
				}
				else {
					for (int i = 0; i < 13; i++) {
						fout << "Crosstalk Cal Point_" << i << " X, Y=	" << Cross_DW_before[0][i] << "	,	" << Cross_DW_before[1][i] << endl;
					}
					fout << endl;
					for (int i = 0; i < 13; i++) {
						fout << "Crosstalk verify Point_" << i << " X, Y=	" << Cross_DW_after[0][i] << "	,	" << Cross_DW_before[1][i] << endl;
					}

				}
			}
		}
	}else {
		e = unstringHex2int(akm_cross_Item[0]);
		e1 = unstringHex2int(akm_cross_Item[1]);
		if (e > 0) {
			for (int i = 0; i < 21; i++) {
				Cross_DW_before[0][i] = char_Out(e);
				fout << "Crosstalk_X" << i << "	" << Cross_DW_before[0][i] << endl;
				if (i > 0) {
					if (abs(Cross_DW_before[0][i] - Cross_DW_before[0][i - 1])>LCC_CrossTalk[2])
						ret |= 1;
				}
				e++;
			}
		}
		if (e1 > 0) {
			for (int i = 0; i < 21; i++) {
				Cross_DW_before[1][i] = char_Out(e1);
				fout << "Crosstalk_Y" << i << "	" << Cross_DW_before[1][i] << endl;
				if (i > 0) {
					if (abs(Cross_DW_before[1][i] - Cross_DW_before[1][i - 1]) > LCC_CrossTalk[2])
						ret |= 1;
				}
				e1++;
			}
		}

		if (ret > 0) {
			ui.log->insertPlainText("Crosstalk Data FP NG!\n");
		}
	}

	return ret;
}


int EEPROM_Data_Verifier::PDAF_Parse() {

	int ret = 0;
	unsigned int spec;
	for (int i = 0; i < 16; i++)
		if (PDAF_info_Item[i][1].length()>1) {

			if (i == 0 && mode == 0) {
				fout << "-------PDAF Info Data Compare------" << endl;
				fout << "(Item)	(Data)	(Spec)" << endl;
			}
			fout << PDAF_info_Item[i][0] << ":	";
			unsigned int addr = unstringHex2int(PDAF_info_Item[i][1]);
			unsigned int d = 0;
			if (HL & 1 == 1) {
				string str = " ";
				if (PDAF_info_Item[i][2].length() > 1)
					str = PDAF_info_Item[i][2].substr(0, 2);
				map_Push(addr, PDAF_info_Item[i][0] + "_H", str, info1);
				str = " ";
				if (PDAF_info_Item[i][2].length() >= 4)
					str = PDAF_info_Item[i][2].substr(2, 2);
				map_Push(addr + 1, PDAF_info_Item[i][0] + "_L", str, info1);
				fout << D[addr][0] << D[addr][1] << D[addr + 1][0] << D[addr + 1][1];
				d = DecData[addr + 1] + DecData[addr] * 256;
			}
			else {
				string str = " ";
				if (PDAF_info_Item[i][2].length() >= 4)
					str = PDAF_info_Item[i][2].substr(2, 2);
				map_Push(addr, PDAF_info_Item[i][0] + "_L", str, info1);
				str = " ";
				if (PDAF_info_Item[i][2].length() > 1)
					str = PDAF_info_Item[i][2].substr(0, 2);
				map_Push(addr + 1, PDAF_info_Item[i][0] + "_H", str, info1);
				fout << D[addr + 1][0] << D[addr + 1][1] << D[addr][0] << D[addr][1];
				d = DecData[addr] + DecData[addr + 1] * 256;
			}
			fout << "	" << PDAF_info_Item[i][2] << endl;
			spec = unstringHex2int(PDAF_info_Item[i][2]);
			if (d != spec&&PDAF_info_Item[i][2].length() > 1) {
				string s = PDAF_info_Item[i][0] + " Data in 0x" + PDAF_info_Item[i][1] + ": " + to_string(d) + " != " + PDAF_info_Item[i][2] + '\n';
				ui.log->insertPlainText(s.c_str());
				ret |= 1;
			}
		}
	fout << endl;

	for (int k = 0; k < 10; k++)
		if (Gmap_Item[k][1].length()>1) {
			unsigned int e = marking_Hex2int(Gmap_Item[k][1], Gmap_Item[k][0], "", QC_GainMap);
			int W = 17, H = 13, offset = 0;
			if (Gmap_Item3[k] == 1 || Gmap_Item3[k] == 3) {
				W = 16, H = 12, offset = atoi(Gmap_Item[k][2].c_str());
			}

			/*
			if (Gmap_Item[k][2].length() > 0) {
				offset = atoi(Gmap_Item[k][2].c_str());

				for (int i = 0; i < offset; i += 2) {
					int d = 0;
					string str = " ";
					if (PDAF_info_Item[i][2].length() >= 4)
						str = PDAF_info_Item[i][2].substr(2, 2);
					map_Push(e + i, PDAF_info_Item[i][0] + "_L", str, info1);
					str = " ";
					if (PDAF_info_Item[i][2].length() > 1)
						str = PDAF_info_Item[i][2].substr(0, 2);
					map_Push(e + i + 1, PDAF_info_Item[i][0] + "_H", str, info1);
					fout << PDAF_info_Item[i][0] << ":	";
					fout << D[e + i + 1][0] << D[e + i + 1][1] << D[e + i][0] << D[e + i][1];
					d = DecData[e + i] + DecData[e + i + 1] * 256;
					fout << "	" << PDAF_info_Item[i][2] << endl;
					spec = unstringHex2int(PDAF_info_Item[i][2]);
					if (d != spec&&PDAF_info_Item[i][2].length() > 1) {
						string s = PDAF_info_Item[i][0] + " Data in 0x" + PDAF_info_Item[i][1] + ": " + to_string(d) + " != " + PDAF_info_Item[i][2] + '\n';
						ui.log->insertPlainText(s.c_str());
						ret |= 2;
					}
				}

				e += offset;
			}
			*/

			if (Gmap_Item3[k] != 2 && Gmap_Item3[k] != 4) {
				for (int i = 0; i < H; i++)
					for (int j = 0; j < W; j++) {
						useData[e] = 1;
						useData[e + 1] = 1;
						if (Gmap_Item3[k] != 3) {
							if (HL & 1 == 1)
								PDgainLeft[i][j] = DecData[e] * 256 + DecData[e + 1];
							else
								PDgainLeft[i][j] = DecData[e] + DecData[e + 1] * 256;
							e += 2;
						}
						else {
							PDgainLeft[i][j] = DecData[e];
							e++;
						}
					}

				e += offset;
				for (int i = 0; i < H; i++)
					for (int j = 0; j < W; j++) {
						useData[e] = 1;
						useData[e + 1] = 1;
						if (Gmap_Item3[k] != 3) {

							if (HL&1)
								PDgainRight[i][j] = DecData[e] * 256 + DecData[e + 1];
							else
								PDgainRight[i][j] = DecData[e] + DecData[e + 1] * 256;
							e += 2;
						}
						else {
							PDgainRight[i][j] = DecData[e];
							e++;
						}
					}

				if (mode == 0) {
					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " Left Gain map:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							fout << PDgainLeft[i][j] << "	";
						}
						fout << endl;
					}

					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " Right Gain map:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							fout << PDgainRight[i][j] << "	";
						}
						fout << endl;
					}
					fout << endl;
				}
			}		
			else if (Gmap_Item3[k] == 2) {
				W = 20, H = 4;
				unsigned int eStart = e, e1 = e;
				e = eStart + 16;
				e1 = eStart + 176;
				
				for (int i = 0; i < H; i++)
					for (int j = 0; j < W; j++) {
						useData[e] = useData[e + 1] = 1;
						useData[e1] = useData[e1 + 1] = 1;
						PDgainLeft[i][j] = DecData[e]  + DecData[e + 1] * 256;
						PDgainRight[i][j] = DecData[e1]  + DecData[e1 + 1] * 256;
						e += 2; e1 += 2;
					}
				if (mode == 0) {
					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " MTK Proc1 LR Ratio:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							float xx = PDgainLeft[i][j] / 1024.0;
							if (xx<0.5 || xx>1.5) { ret |= 128; }
							fout << xx << "	";
						}
						fout << endl;
					}

					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " MTK Proc1 UD Ratio:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							float xx = PDgainRight[i][j] / 1024.0;
							if (xx<0.5 || xx>1.5) { ret |= 128; }
							fout << xx << "	";
						}
						fout << endl;
					}
					fout << endl;
				}

				e = eStart + 336;
				e1 = eStart + 336 + 160;

				for (int i = 0; i < H; i++)
					for (int j = 0; j < W; j++) {
						useData[e] = useData[e + 1] = 1;
						useData[e1] = useData[e1 + 1] = 1;
						PDgainLeft[i][j] = DecData[e] + DecData[e + 1] * 256;
						PDgainRight[i][j] = DecData[e1] + DecData[e1 + 1] * 256;
						e += 2; e1 += 2;
					}
				if (mode == 0) {
					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " MTK Proc1 Left:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							fout << PDgainLeft[i][j] << "	";
						}
						fout << endl;
					}

					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " MTK Proc1 Right:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							fout << PDgainRight[i][j] << "	";
						}
						fout << endl;
					}
					fout << endl;
				}

				if (GainMap_FP_Check(PDgainLeft, PDgainRight, Gmap_Item3[k]) > 0) {
					string s = Gmap_Item[k][0] + " GainMap in 0x" + Gmap_Item[k][1] + " is NG!\n ";
					ui.log->insertPlainText(s.c_str());
					ret |= 4;
				}

				e = eStart + 336 +320;
				e1 = eStart + 336 + 480;
				for (int j = 0; j < W; j++)
					for (int i = 0; i < H; i++) {
						useData[e] = useData[e + 1] = 1;
						useData[e1] = useData[e1 + 1] = 1;
						PDgainLeft[i][j] = DecData[e] + DecData[e + 1] * 256;
						PDgainRight[i][j] = DecData[e1] + DecData[e1 + 1] * 256;
						e += 2; e1 += 2;
					}
				if (mode == 0) {
					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " MTK Proc1 UP:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							fout << PDgainLeft[i][j] << "	";
						}
						fout << endl;
					}

					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " MTK Proc1 Down:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {			
							fout << PDgainRight[i][j] << "	";
						}
						fout << endl;
					}
					fout << endl;
				}
				if (GainMap_FP_Check(PDgainLeft, PDgainRight, Gmap_Item3[k]) > 0) {
					string s = Gmap_Item[k][0] + " GainMap in 0x" + Gmap_Item[k][1] + " is NG!\n ";
					ui.log->insertPlainText(s.c_str());
					ret |= 4;
				}
			}
			else if (Gmap_Item3[k] == 4) {
				W = 20, H = 4;
				unsigned int eStart = e;
				e = eStart + 16;

				for (int i = 0; i < H; i++)
					for (int j = 0; j < W; j++) {
						useData[e] = 1;
						useData[e + 1] = 1;
						PDgainLeft[i][j] = DecData[e] + DecData[e + 1] * 256;
					
						e += 2; 
					}
				if (mode == 0) {
					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " MTK Proc1 LR Ratio:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							float xx = PDgainLeft[i][j] / 1024.0;
							if (xx<0.5 || xx>1.5) {
								ret |= 128;} 
							fout << xx << "	";
						}
						fout << endl;
					}
					fout << endl;
				}
				int e1 = e + 160;
				for (int i = 0; i < H; i++)
					for (int j = 0; j < W; j++) {
						useData[e] = useData[e + 1] = 1;
						useData[e1] = useData[e1 + 1] = 1;
						PDgainLeft[i][j] = DecData[e] + DecData[e + 1] * 256;
						PDgainRight[i][j] = DecData[e1] + DecData[e1 + 1] * 256;
						e += 2; e1 += 2;
					}

				if (mode == 0) {
					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " MTK Proc1 Left:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							fout << PDgainLeft[i][j] << "	";
						}
						fout << endl;
					}

					fout << "~~~~~~~~~~~" << Gmap_Item[k][0] << " MTK Proc1 Right:" << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							fout << PDgainRight[i][j] << "	";
						}
						fout << endl;
					}
					fout << endl;
				}
				if (GainMap_FP_Check(PDgainLeft, PDgainRight, Gmap_Item3[k]) > 0) {
					string s = Gmap_Item[k][0] + " GainMap in 0x" + Gmap_Item[k][1] + " is NG!\n ";
					ui.log->insertPlainText(s.c_str());
					ret |= 4;
				}
			}
		}
	
	for (int k = 0; k < 14; k++)
		if (PD_Item[k][1].length()>1) {
			unsigned int e = marking_Hex2int(PD_Item[k][1], PD_Item[k][0], "", QC_DCC);
			int W = 8, H = 6, offset = 0;
			if (PD_Item3[k] == 1) {
				W = 16, H = 12;
			}

			if (k < 10) {
				if (PD_Item3[k] < 2) {
					if (mode == 0)
						fout << "~~~~~~~~~~~" << PD_Item[k][0] << " Map:" << endl;
					e += offset;
					for (int i = 0; i < H; i++)
						for (int j = 0; j < W; j++) {
							useData[e] = 1;
							useData[e + 1] = 1;
							if (HL&1)
								DCC[i][j] = 256 * DecData[e] + DecData[e + 1];
							else						
								DCC[i][j] = 256 * DecData[e + 1] + DecData[e];

							e += 2;
						}

					if (mode == 0) {
						for (int i = 0; i < H; i++) {
							for (int j = 0; j < W; j++) {
								fout << DCC[i][j] << "	";
							}
							fout << endl;
						}
						fout << endl;
					}

					if (DCC_FP_Check(DCC, PD_Item3[k]) > 0) {
						string s = PD_Item[k][0] + " in 0x" + PD_Item[k][1] + " is NG!\n ";
						ui.log->insertPlainText(s.c_str());
						ret |= 16;
					}
				}

				if (PD_Item3[k] == 2) {
					W = 8, H = 8; 
					e += 16;
					int MTK_PD_Data[64][10] = { 0 }, MTK_PD_Data2[64][10] = { 0 }, e2 = e+=128;
					for (int i = 0; i <H; i++)
						for (int j = 0; j < W; j++) {
							useData[e] = useData[e+1] =1;
							useData[e2] = useData[e2 + 1] = 1;
							DCC[i][j] = 256 * DecData[e + 1] + DecData[e];
							DCC2[i][j] = 256 * DecData[e2 + 1] + DecData[e2];
							e += 2; e2 += 2;
						}

					e += 24; e2 = e + 640;
					for (int i = 0; i <64; i++)
						for (int j = 0; j < 10; j++) {
							useData[e] = useData[e2] = 1;
							MTK_PD_Data[i][j] = DecData[e];
							MTK_PD_Data2[i][j] = DecData[e2];
							e ++; e2 ++;
						}

					if (DCC_FP_Check(DCC, PD_Item3[k]) > 0) {
						string s = PD_Item[k][0] + " in 0x" + PD_Item[k][1] + " is NG!\n ";
						ui.log->insertPlainText(s.c_str());
						ret |= 16;
					}

					if (DCC_FP_Check(DCC2, PD_Item3[k]) > 0) {
						string s = PD_Item[k][0] + " in 0x" + PD_Item[k][1] + " is NG!\n ";
						ui.log->insertPlainText(s.c_str());
						ret |= 16;
					}

					if (mode == 0) {
						fout << "~~~~~~~~~~~" << PD_Item[k][0] << " MTK Proc2 LR DCC:" << endl;
						for (int i = 0; i < H; i++) {
							for (int j = 0; j < W; j++) {
								fout << DCC[i][j] << "	";
							}
							fout << endl;
						}

						fout << "~~~~~~~~~~~" << PD_Item[k][0] << " MTK Proc2 UD DCC:" << endl;
						for (int i = 0; i < H; i++) {
							for (int j = 0; j < W; j++) {
								fout << DCC2[i][j] << "	";
							}
							fout << endl;
						}
						fout << endl;

						fout << "~~~~~~~~~~~" << PD_Item[k][0] << " MTK LR PD_Data:" << endl;
						for (int i = 0; i < 64; i++) {
							for (int j = 0; j < 10; j++) {
								fout << MTK_PD_Data[i][j] << "	";
							}
							fout << endl;
						}

						fout << "~~~~~~~~~~~" << PD_Item[k][0] << " MTK UD PD_Data::" << endl;
						for (int i = 0; i < 64; i++) {
							for (int j = 0; j < 10; j++) {
								fout << MTK_PD_Data2[i][j] << "	";
							}
							fout << endl;
						}
						fout << endl;
					}		
				}

				if (PD_Item3[k] == 3) {
					W = 8, H = 8;
					e += 16;
					int MTK_PD_Data[64][10] = { 0 };
					for (int i = 0; i <H; i++)
						for (int j = 0; j < W; j++) {
							useData[e] = useData[e + 1] = 1;
							DCC[i][j] = 256 * DecData[e + 1] + DecData[e];
							e += 2; 
						}

					if (DCC_FP_Check(DCC, PD_Item3[k]) > 0) {
						string s = PD_Item[k][0] + " in 0x" + PD_Item[k][1] + " is NG!\n ";
						ui.log->insertPlainText(s.c_str());
						ret |= 16;
					}

					e += 22; 
					for (int i = 0; i <64; i++)
						for (int j = 0; j < 10; j++) {
							useData[e] = 1;
							MTK_PD_Data[i][j] = DecData[e];				
							e++; 
						}

					if (mode == 0) {
						fout << "~~~~~~~~~~~" << PD_Item[k][0] << " MTK Proc2 LR DCC:" << endl;
						for (int i = 0; i < H; i++) {
							for (int j = 0; j < W; j++) {
								fout << DCC[i][j] << "	";
							}
							fout << endl;
						}
						fout << "~~~~~~~~~~~" << PD_Item[k][0] << " MTK LR PD_Data:" << endl;
						for (int i = 0; i < 64; i++) {
							for (int j = 0; j < 10; j++) {
								fout << MTK_PD_Data[i][j] << "	";
							}
							fout << endl;
						}
					}
				}
			}

			/////// QC PD offset
			if (PD_Item3[k] == 4 || k>9) {
				fout << "~~~~~~~~~~~" << PD_Item[k][0] << "(QC PD offset)_Data:" << endl;
				float F_DCC[6][8] = { 0 };
				for (int i = 0; i <H; i++)
					for (int j = 0; j < W; j++) {

						for (int a = 0; a < 4; a++)
							useData[e + a] = 1;

						F_DCC[i][j] = flt_Out(e, ui.PD_Offset_HL->isChecked());
						if(ui.PD_offet_new->isChecked())
							F_DCC[i][j] = flt_Out2301(e, ui.PD_Offset_HL->isChecked());

						if (F_DCC[i][j]>5|| F_DCC[i][j] < -5) {
							ret |= 32;
						}
						e += 4;
					}

				if (mode == 0||(ret&32)>0) {
					for (int i = 0; i < 6; i++) {
						for (int j = 0; j < 8; j++) {
							fout << F_DCC[i][j] << "	";
						}
						fout << endl;
					}
					fout << endl;
					if ((ret & 32) > 0) {
						string s = PD_Item[k][0] + " in 0x" + PD_Item[k][1] + " is NG!\n ";
						ui.log->insertPlainText(s.c_str());
					}
				}
			}
		}
	return ret;
}


int EEPROM_Data_Verifier::QSC_Parse() {

	int ret = 0; unsigned int spec, offset = 96;

	if (QSC_Item.item[1].length()>1) {

		unsigned int e = marking_Hex2int(QSC_Item.item[1], QSC_Item.item[0], "", QSC_Type);

		if (DecData[e] == 0xFF && DecData[e + 1] == 0xFF && DecData[e + 1] == 0xFF) {
			return 1;
		}
		if (DecData[e] == 0 && DecData[e + 1] == 0 && DecData[e + 1] == 0) {
			return 1;
		}
		if (ui.QSC3->isChecked()) {
			for (int i = 0; i < 12; i++)
				for (int j = 0; j < 16; j++)
					for (int k = 0; k < 4; k++) {

						QSC_Data[k][0][i][j] = ((DecData[e] >> 2) + offset) / 128.0;
						QSC_Data[k][1][i][j] = ((DecData[e] & 0x3) * 16 + (DecData[e + 1] >> 4) + offset) / 128.0;
						QSC_Data[k][2][i][j] = ((DecData[e + 1] & 0xF) * 4 + (DecData[e + 1] >> 6) + offset) / 128.0;
						QSC_Data[k][3][i][j] = ((DecData[e + 2] & 0x3F) + offset) / 128.0;

						useData[e] = 1;
						useData[e + 1] = 1;
						useData[e + 2] = 1;
						e += 3;
					}
		}
		else if (ui.QSC4->isChecked()) {

			for (int i = 0; i < 12; i++)
				for (int j = 0; j < 16; j++)
					for (int k = 0; k < 4; k++) 
						for (int a = 0; a < 4; a++) {
							QSC_Data[k][a][i][j] = (DecData[e] + offset) / 128.0;
							useData[e] = 1;
							e ++;
						}
		}

		if (mode == 0) {
			fout << "~~~~~~~~~~~" << "QSC Data:" << endl;
			for (int a = 0; a < 4; a++){
				for (int k = 0; k < 4; k++) {
					fout << "CH" << a << "_"<< k << "Table" << endl;
					for (int i = 0; i < 12; i++) {
						for (int j = 0; j < 16; j++)
						{
							fout << QSC_Data[a][k][i][j] << "	";
						}
						fout << endl;
					}
				}
				fout << endl;
			}
		}

		if (QSC_Check(QSC_Data) > 0) {
			string s = QSC_Item.item[0] + " in 0x" + QSC_Item.item[1] + " is NG!\n ";
			ui.log->insertPlainText(s.c_str());
			ret |= 4;
		}
	}

	return ret;
}


int EEPMap_Data_add(int size, string addr, string item, string ref, data_Type t) {

	int e = unstringHex2int(addr);
	if (size == 2){
		if (HL&1 == 1) {
			string str = " ";
			if (ref.length() > 1)
				str = ref.substr(0, 2);
			map_Push(e, item + "_H", ref, info1);
			str = " ";
			if (ref.length() == 4)
				str = ref.substr(2, 2);
			map_Push(e + 1, item + "_L", ref, info1);
		}
		else {
			string str = " ";
			if (ref.length() == 4)
				str = ref.substr(2, 2);
			map_Push(e, item + "_L", ref, info1);
			str = " ";
			if (ref.length() > 1)
				str = ref.substr(0, 2);
			map_Push(e + 1, item + "_H", ref, info1);
		}
	}
	if (size == 4) {

	}
	return e;
}


int EEPROM_Data_Verifier::af_Parse() {

	fout << "-------AF CAL Data------" << endl;
	int ret = 0;	
	for (int i = 0; i < 6; i++) {
		if (AF_Item[i][1].length()>1) {
			
			value_Hash[i].item_name = AF_Item[i][0];

			int e = EEPMap_Data_add(2,AF_Item[i][1], AF_Item[i][0]," ", AF_Code);
			if (HL&1) {
				AF_Data[i][0] = DecData[e] * 256 + DecData[e+1];
			}
			else {
				AF_Data[i][0] = DecData[e+1] * 256 + DecData[e ];
			}
			value_Hash[i].hash[AF_Data[i][0]%1009]++;
			fout << AF_Item[i][0] << ":	" << AF_Data[i][0] << endl;
		}
	}
	ret = AF_FP_Check(AF_Data);
	if (ret > 0)
		ui.log->insertPlainText("AF Code Spec Check NG!\n");

	fout << endl;

	///////////////////// ZOOM Type data check
	int p = unstringHex2int(Magnification[0]);

	if (p > 4) {

		for (int i = 0; i < p; i++) {
			int e = unstringHex2int(ZOOM_Item[0][2]);
			char n = 'A' + i;
			string s = ZOOM_Item[0][0] + " " + n;
			fout << s << ":	";
			fout << dbl_Out(e + 8 * i, HL&1) << endl;

			for (int k = 0; k < 8; k++) {
				s += "_[" + to_string(k) + "]";
				map_Push(e + k, s, "", Formula_Code);
			}

		}
		for (int m = 1; m < 3; m++){

			int e = unstringHex2int(ZOOM_Item[m][1]);
			for (int k = 0; k < p; k++) {
				string s = ZOOM_Item[m][0] + " position_" +    Magnification[k + 1];
				fout << s << "x:	";
				fout << DecData[e + k * 2 + 1] * 256 + DecData[e + k * 2] << endl;
				map_Push(e + k * 2, s + "_L", "", AF_Code);
				map_Push(e + k * 2 + 1, s + "_H", "", AF_Code);

			}

			for (int i = 0; i < p; i++) {
				e = unstringHex2int(ZOOM_Item[m][2]);
				char n = 'A' + i;
				string s = ZOOM_Item[m][0] + " " + n;
				fout << s << ":	";
				fout << dbl_Out(e + 8 * i, HL&1) << endl;

				for (int k = 0; k < 8; k++) {
					s += "_[" + to_string(k) + "]";
					map_Push(e + k, s, "", Formula_Code);
				}
			}
		}
	}


	int SFR_ret = 0;
	///////////////////// SFR data check
	fout << "-------SFR Test Data------" << endl;
	for (int i = 0; i < 4; i++) {

		if (SFR_Item[i][2].length()>1) {

			fout << SFR_Item[i][0];
			if (SFR_Item[i][1].length()>1) {
				int d = unstringHex2int(SFR_Item[i][1]);
				fout << " Grade:	" << D[d][0] << D[d][1];
			}
			if (mode == 0)
				fout << endl;
			int d = unstringHex2int(SFR_Item[i][2]);
			int cnt = atoi(SFR_Item[i][3].c_str());
			int f = unstringHex2int(SFR_Item[i][1]);

			for (int k = 0; k < cnt; k++) {
				if (SFR_Format == 0) {
					SFR_Data[k] = (int)(D[d + k][0] - '0') * 10 + (D[d + k][1] - '0');
					if (mode == 0)
						fout << "0." << D[d + k][0] << D[d + k][1] << "	";
				}
				else if (SFR_Format == 1) {
					SFR_Data[k] = DecData[d + k];
					if (mode == 0)
						fout << (float)SFR_Data[k] / 100.0 << "	";
				}
				else if (SFR_Format == 2) {
					SFR_Data[k] = DecData[d + 2 * k] * 256 + DecData[d + 2 * k + 1];
					if (mode == 0)
						fout << (float)SFR_Data[k] / 10000.0 << "	";
					SFR_Data[k] = SFR_Data[k] / 100;
				}
				else if (SFR_Format == 3) {
					SFR_Data[k] = DecData[d + 2 * k] * 256 + DecData[d + 2 * k + 1];
					if (mode == 0)
						fout << SFR_Data[k] << "	";
				}
				else if (SFR_Format == 4) {
					if((HL&1)==0)
						SFR_Data[k] = DecData[d + 2 * k]  + DecData[d + 2 * k + 1]*256;
					else SFR_Data[k] = DecData[d + 2 * k] * 256 + DecData[d + 2 * k + 1] * 256;
					if (mode == 0)
						fout << (float)SFR_Data[k] / 1000.0 << "	";
					SFR_Data[k] = SFR_Data[k] / 10;
				}
			}
			SFR_ret += SFR_FP_Check(SFR_Data, cnt, i + 1, SFR_Format);
			if (mode == 0)
				fout << endl;
		}
	}
	if (SFR_ret > 0)
		ui.log->insertPlainText("SFR Spec Check NG!\n");

	ret |= SFR_ret;
	fout << endl;

	if (mode == 0)
		for (int i = 0; i < 12; i++) {

			int e = unstringHex2int(AA_Item[i]);
			if (e > 0) {
				if (i == 0)
					fout << "~~~~~~~~~~~AA Info~~~~~~~~~ " << endl;

				switch (i) {
				case 0:
					fout << "AA Flag: " << (int)DecData[e] << endl;
					map_Push(e, "AA Flag", "", info1);
					break;
				case 1:
					fout << "Equip Type: " << D[e][0] << D[e][1] << endl;
					map_Push(e, "Equip Type", "", info1);
					break;
				case 2:
					fout << "Equip No: " << (int)DecData[e] << endl;
					map_Push(e, "Equip No", "", info1);
					break;
				case 3:
					fout << "Port No: " << (int)DecData[e] << endl;
					map_Push(e, "Port No", "", info1);
					break;
				case 4:
					fout << "Date: " << (int)DecData[e] << "-" << (int)DecData[e + 1] << "-" << (int)DecData[e + 2] << " " << (int)DecData[e + 3] << ":" << (int)DecData[e + 4] << ":" << (int)DecData[e + 5] << endl;
					map_Push(e, "Manufactured year", "", info1);
					map_Push(e + 1, "Manufactured month", "", info1);
					map_Push(e + 2, "Manufactured day", "", info1);
					map_Push(e + 3, "Manufactured hour", "", info1);
					map_Push(e + 4, "Manufactured Minute", "", info1);
					map_Push(e + 5, "Manufactured second", "", info1);
					break;
				case 5:
					fout << "OC_X: " << (int)DecData[e] * 256 + DecData[e + 1] << endl;
					map_Push(e, "OC X_H", "", info1);
					map_Push(e, "OC X_L", "", info1);
					break;
				case 6:
					fout << "OC_Y: " << (int)DecData[e] * 256 + DecData[e + 1] << endl;
					map_Push(e, "OC Y_H", "", info1);
					map_Push(e, "OC Y_L", "", info1);
					break;
				case 7:
					fout << "Tilt_X: " << flt_Out(e, false) << endl;
					map_Push(e, "AA X tilt (Degree)_1", "", info1);
					map_Push(e + 1, "AA X tilt (Degree)_2", "", info1);
					map_Push(e + 2, "AA X tilt (Degree)_3", "", info1);
					map_Push(e + 3, "AA X tilt (Degree)_4", "", info1);
					break;
				case 8:
					fout << "Tilt_Y: " << flt_Out(e, false) << endl;
					map_Push(e, "AA Y tilt (Degree)_1", "", info1);
					map_Push(e + 1, "AA Y tilt (Degree)_2", "", info1);
					map_Push(e + 2, "AA Y tilt (Degree)_3", "", info1);
					map_Push(e + 3, "AA Y tilt (Degree)_4", "", info1);
					break;
				case 9:
					fout << "AF Code: " << (int)DecData[e] * 256 + DecData[e + 1] << endl;
					map_Push(e, "AA AF Code_H", "", info1);
					map_Push(e + 1, "AA AF Code_L", "", info1);
					break;
				case 10:
					fout << "AA BV: " << (int)DecData[e] << endl;
					map_Push(e, "AA BV_H", "", info1);
					map_Push(e + 1, "AA BV_L", "", info1);
					break;
				case 11:
					fout << "CPS Code: " << (int)DecData[e] * 100 + DecData[e + 1] << endl;
					map_Push(e, "CPS Code_H", "", info1);
					map_Push(e + 1, "CPS Code_L", "", info1);
					break;

				default:
					break;
				}
			}
		}
	
	fout << endl;
	return ret;
}


void EEPROM_Data_Verifier::Oppo_AWB_Parse(int group) {

	TCHAR lpTexts[10]; int temp = 0, L, H;
	string s, color = "5100K", item;

	if (group == 0)
		color = "5100K";
	else if (group == 1)
		color = "4000K";
	else if (group == 2)
		color = "3100K";

	fout << "-------" << color << " AWB Data------" << endl;
	// AWB 
	item = "AWB R_avg_L_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	L = marking_Hex2int(s, item, "", QC_AWB);
	item = "AWB R_avg_H_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", QC_AWB);
	OPPO_AWB[group].AWB[0] = DecData[H] * 256 + DecData[L];
	if (H == 0 || L == 0){
		OPPO_AWB[group].AWB[0] = 0;
	}
	else {
		fout << "AWB R_avg_" << color << " :	" << OPPO_AWB[group].AWB[0] << endl;
	}
	item = "AWB Gr_avg_L_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	L = marking_Hex2int(s, item, "", QC_AWB);
	item = "AWB Gr_avg_H_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", QC_AWB);
	OPPO_AWB[group].AWB[1] = DecData[H] * 256 + DecData[L];

	if (H == 0 || L == 0) {
		OPPO_AWB[group].AWB[1] = 0;
	}
	else {
		fout << "AWB Gr_avg_" << color << " :	" << OPPO_AWB[group].AWB[1] << endl;
	}
	
	item = "AWB Gb_avg_L_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	L = marking_Hex2int(s, item, "", QC_AWB);
	item = "AWB Gb_avg_H_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", QC_AWB);
	OPPO_AWB[group].AWB[2] = DecData[H] * 256 + DecData[L];
	
	if (H == 0 || L == 0) {
		OPPO_AWB[group].AWB[2] = 0;
	}
	else {
		fout << "AWB Gb_avg_" << color << " :	" << OPPO_AWB[group].AWB[2] << endl;
	}

	item = "AWB B_avg_L_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	L = marking_Hex2int(s, item, "", QC_AWB);
	item = "AWB B_avg_H_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", QC_AWB);
	OPPO_AWB[group].AWB[3] = DecData[H] * 256 + DecData[L];
	
	if (H == 0 || L == 0) {
		OPPO_AWB[group].AWB[3] = 0;
	}
	else {
		fout << "AWB B_avg_" << color << " :	" << OPPO_AWB[group].AWB[3] << endl;
	}

	////////////////////////////////////////////////////////////////////// Golden
	item = "Golden R_avg_L_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	L = marking_Hex2int(s, item, "", QC_AWB);
	item = "Golden R_avg_H_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", QC_AWB);
	OPPO_AWB[group].Golden [0] = DecData[H] * 256 + DecData[L];
	
	if (H == 0 || L == 0) {
		OPPO_AWB[group].Golden[0] = 0;
	}
	else {
		fout << "Golden R_avg " << color << " :	" << OPPO_AWB[group].Golden[0] << endl;
	}

	item = "Golden Gr_avg_L_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	L = marking_Hex2int(s, item, "", QC_AWB);
	item = "Golden Gr_avg_H_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", QC_AWB);
	OPPO_AWB[group].Golden[1] = DecData[H] * 256 + DecData[L];
	
	if (H == 0 || L == 0) {
		OPPO_AWB[group].Golden[1] = 0;
	}
	else {
		fout << "Golden Gr_avg " << color << " :	" << OPPO_AWB[group].Golden[1] << endl;
	}

	item = "Golden Gb_avg_L_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	L = marking_Hex2int(s, item, "", QC_AWB);
	item = "Golden Gb_avg_H_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", QC_AWB);
	OPPO_AWB[group].Golden[2] = DecData[H] * 256 + DecData[L];
	
	if (H == 0 || L == 0) {
		OPPO_AWB[group].Golden[2] = 0;
	}
	else {
		fout << "Golden Gb_avg " << color << " :	" << OPPO_AWB[group].Golden[2] << endl;
	}

	item = "Golden B_avg_L_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	L = marking_Hex2int(s, item, "", QC_AWB);
	item = "Golden B_avg_H_" + color;
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", QC_AWB);
	OPPO_AWB[group].Golden[3] = DecData[H] * 256 + DecData[L];

	if (H == 0 || L == 0) {
		OPPO_AWB[group].Golden[3] = 0;
	}
	else {
		fout << "Golden B_avg " << color << " :	" << OPPO_AWB[group].Golden[3] << endl;
	}

	/////////////////////////////////////////////////////////////////////////////// light coef
	item = color+" light source RG calibration_L";
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	L = marking_Hex2int(s, item, "", QC_AWB);
	item = color + " light source RG calibration_H";
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", QC_AWB);
	OPPO_AWB[group].Light[0] = DecData[H] * 256 + DecData[L];
	
	if (H == 0 || L == 0) {
		OPPO_AWB[group].Light[0] = 0;
	}
	else {
		fout << color << " light source RG calibration" << " :	" << OPPO_AWB[group].Light[0] << endl;
	}

	item = color + " light source BG calibration_L";
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	L = marking_Hex2int(s, item, "", QC_AWB);
	item = color + " light source BG calibration_H";
	GetPrivateProfileString(TEXT("OPPO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", QC_AWB);
	OPPO_AWB[group].Light[1] = DecData[H] * 256 + DecData[L];
	
	if (H == 0 || L == 0) {
		OPPO_AWB[group].Light[1] = 0;
	}
	else {
		fout << color << " light source BG calibration" << " :	" << OPPO_AWB[group].Light[1] << endl;
	}

	fout << endl;

}


void EEPROM_Data_Verifier::VIVO_AWB_Parse(int group) {

	TCHAR lpTexts[10]; int temp = 0, L, H;
	int ret = 0;
	string s, color = "AWB_5100K_", item;

	if (group == 1)
		color = "AWB_3100K_";

	fout << "-------" << color << "Data------" << endl;

	/////////////////////////////////// AWB 
	item = color + "Gain R/Gr";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H+1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].AWB[0] = DecData[H] * 256 + DecData[H+1];
	fout << item<<" :	" << VIVO_AWB_Data[group].AWB[0] << endl;

	item = color + "Gain B/Gr";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].AWB[1] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].AWB[1] << endl;

	item = color + "Gain Gr/Gb";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].AWB[2] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].AWB[2] << endl;

	/////////////////////////////////// Golden
	item = color + "Golden Gain R/Gr";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Golden[0] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Golden[0] << endl;

	item = color + "Golden Gain B/Gr";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Golden[1] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Golden[1] << endl;

	item = color + "Golden Gain Gr/Gb";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Golden[2] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Golden[2] << endl;

	//////////////////////////////////////// light coef
	item = color + "Light Source R/G";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " Calibration H", "", VIVO_AWB);
	map_Push(H + 1, item + "Calibration L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Light[0] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Light[0] << endl;

	item = color + "Light Source B/G";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " Calibration H", "", VIVO_AWB);
	map_Push(H + 1, item + "Calibration L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Light[1] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Light[1] << endl;

	//item = color + "Light Source Gr/Gb";
	//GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	//s = CT2A(lpTexts);
	//H = marking_Hex2int(s, item + " Calibration H", "", VIVO_AWB);
	//map_Push(H + 1, item + "Calibration L", "", VIVO_AWB);
	//VIVO_AWB_Data[group].Light[2] = DecData[H] * 256 + DecData[H + 1];
	//fout << item << " :	" << VIVO_AWB_Data[group].Light[2] << endl;

	fout << endl;

}


void EEPROM_Data_Verifier::SONY_AWB_Parse(int group) {

	TCHAR lpTexts[10]; int temp = 0, L, H;
	int ret = 0;
	string s, color = "High_Color", item;

	if (group == 1)
		color = "Low_Color";

	fout << "-------SONY AWB " << color << "Data------" << endl;

	/////////////////////////////////// AWB 
	item = "SONY_rg_"+color;
	GetPrivateProfileString(TEXT("SONY"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", VIVO_AWB);
	VIVO_AWB_Data[group].AWB[0] = uint_Out(H,HL);
	fout << item << " :	" << (float)VIVO_AWB_Data[group].AWB[0] / (1 << 24) << endl;

	item = "SONY_bg_" + color;
	GetPrivateProfileString(TEXT("SONY"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", VIVO_AWB);
	VIVO_AWB_Data[group].AWB[1] = uint_Out(H, HL);
	fout << item << " :	" << (float)VIVO_AWB_Data[group].AWB[1] / (1 << 24) << endl;

	item = "SONY_gbgr_" + color;
	GetPrivateProfileString(TEXT("SONY"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item , "", VIVO_AWB);
	VIVO_AWB_Data[group].AWB[2] = uint_Out(H, HL);
	fout << item << " :	" << (float)VIVO_AWB_Data[group].AWB[2] / (1 << 24) << endl;

	fout << endl;

}


void EEPROM_Data_Verifier::vivo_MTK_AWB_Parse(int group) {

	TCHAR lpTexts[10]; int temp = 0, L, H;
	string s, color = "MTK_AWB_5100K_", item;
	fout << "-------MTK_" << color << " AWB Data------" << endl;
	if (group == 0)
		color = "MTK_AWB_5100K_";
	else if (group == 1)
		color = "MTK_AWB_3100K_";

	/////////////////////////////////// AWB 
	item = color + "Gain R/Gr";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].AWB[0] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].AWB[0] << endl;

	item = color + "Gain B/Gr";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].AWB[1] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].AWB[1] << endl;

	item = color + "Gain Gr/Gb";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].AWB[2] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].AWB[2] << endl;

	/////////////////////////////////// Golden
	item = color + "Golden Gain R/Gr";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Golden[0] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Golden[0] << endl;

	item = color + "Golden Gain B/Gr";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Golden[1] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Golden[1] << endl;

	item = color + "Golden Gain Gr/Gb";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Golden[2] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Golden[2] << endl;

	//////////////////////////////////////// light coef
	item = color + "Light Source R/G";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " Calibration H", "", VIVO_AWB);
	map_Push(H + 1, item + "Calibration L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Light[0] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Light[0] << endl;

	item = color + "Light Source B/G";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " Calibration H", "", VIVO_AWB);
	map_Push(H + 1, item + "Calibration L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Light[1] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Light[1] << endl;

	item = color + "Light Source Gr/Gb";
	GetPrivateProfileString(TEXT("VIVO"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " Calibration H", "", VIVO_AWB);
	map_Push(H + 1, item + "Calibration L", "", VIVO_AWB);
	VIVO_AWB_Data[group].Light[2] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << VIVO_AWB_Data[group].Light[2] << endl;

	fout << endl;

}


void EEPROM_Data_Verifier::MTK_AWB_Parse(int group) {

	TCHAR lpTexts[10]; int temp = 0, L, H;
	string s, color = "MTK_AWB_5100K_", item;
	fout << "-------MTK_" << color << " AWB Data------" << endl;
	if (group == 0)
		color = "MTK_AWB_5100K";

	/////////////////////////////////// AWB 
	item = color + "_R";
	GetPrivateProfileString(TEXT("MTK"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", MTK_AWB_Type);
	map_Push(H + 1, item + " L", "", MTK_AWB_Type);
	MTK_AWB[group].AWB[0] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << MTK_AWB[group].AWB[0] << endl;

	value_Hash[8].item_name = item;
	value_Hash[8].hash[MTK_AWB[group].AWB[0] % 1009]++;

	item = color + "_Gr";
	GetPrivateProfileString(TEXT("MTK"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", MTK_AWB_Type);
	map_Push(H + 1, item + " L", "", MTK_AWB_Type);
	MTK_AWB[group].AWB[1] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << MTK_AWB[group].AWB[1] << endl;

	value_Hash[9].item_name = item;
	value_Hash[9].hash[MTK_AWB[group].AWB[1] % 1009]++;

	item = color + "_Gb";
	GetPrivateProfileString(TEXT("MTK"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", MTK_AWB_Type);
	map_Push(H + 1, item + " L", "", MTK_AWB_Type);
	MTK_AWB[group].AWB[2] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << MTK_AWB[group].AWB[2] << endl;

	value_Hash[10].item_name = item;
	value_Hash[10].hash[MTK_AWB[group].AWB[2] % 1009]++;

	item = color + "_B";
	GetPrivateProfileString(TEXT("MTK"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", MTK_AWB_Type);
	map_Push(H + 1, item + " L", "", MTK_AWB_Type);
	MTK_AWB[group].AWB[3] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << MTK_AWB[group].AWB[3] << endl;

	value_Hash[11].item_name = item;
	value_Hash[11].hash[MTK_AWB[group].AWB[3] % 1009]++;

	/////////////////////////////////// Golden
	item = color + "_Golden_R";
	GetPrivateProfileString(TEXT("MTK"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", MTK_AWB_Type);
	map_Push(H + 1, item + " L", "", MTK_AWB_Type);
	MTK_AWB[group].Golden[0] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << MTK_AWB[group].Golden[0] << endl;

	item = color + "_Golden_Gr";
	GetPrivateProfileString(TEXT("MTK"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", MTK_AWB_Type);
	map_Push(H + 1, item + " L", "", MTK_AWB_Type);
	MTK_AWB[group].Golden[1] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << MTK_AWB[group].Golden[1] << endl;

	item = color + "_Golden_Gb";
	GetPrivateProfileString(TEXT("MTK"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", MTK_AWB_Type);
	map_Push(H + 1, item + " L", "", MTK_AWB_Type);
	MTK_AWB[group].Golden[2] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << MTK_AWB[group].Golden[2] << endl;

	item = color + "_Golden_B";
	GetPrivateProfileString(TEXT("MTK"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", MTK_AWB_Type);
	map_Push(H + 1, item + " L", "", MTK_AWB_Type);
	MTK_AWB[group].Golden[3] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << MTK_AWB[group].Golden[3] << endl;

	fout << endl;

}


void EEPROM_Data_Verifier::LSI_AWB_Parse(int group) {

	TCHAR lpTexts[10]; int temp = 0, L, H;
	string s, color = "LSI_AWB_D50", item;

	if (group == 0)
		color = "LSI_AWB_D50";
	else if (group == 1)
		color = "LSI_AWB_A";
	else if (group == 2)
		color = "LSI_AWB_TL84";

	fout << color << " Data------" << endl;

	/////////////////////////////////// AWB 
	item = color + "_R";
	GetPrivateProfileString(TEXT("LSI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item , "", LSI_AWB_Type);
	LSI_AWB[group].AWB[0] = DecData[H] + DecData[H + 1]*256;
	fout << item << " :	" << LSI_AWB[group].AWB[0] << endl;


	item = color + "_Gr";
	GetPrivateProfileString(TEXT("LSI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", LSI_AWB_Type);
	LSI_AWB[group].AWB[1] = DecData[H] + DecData[H + 1] * 256;
	fout << item << " :	" << LSI_AWB[group].AWB[1] << endl;

	item = color + "_Gb";
	GetPrivateProfileString(TEXT("LSI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", LSI_AWB_Type);
	LSI_AWB[group].AWB[2] = DecData[H] + DecData[H + 1] * 256;
	fout << item << " :	" << LSI_AWB[group].AWB[2] << endl;

	item = color + "_B";
	GetPrivateProfileString(TEXT("LSI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", LSI_AWB_Type);
	LSI_AWB[group].AWB[3] = DecData[H] + DecData[H + 1] * 256;
	fout << item << " :	" << LSI_AWB[group].AWB[3] << endl;

	/////////////////////////////////// Golden
	item = color + "_Golden_R";
	GetPrivateProfileString(TEXT("LSI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", LSI_AWB_Type);
	LSI_AWB[group].Golden[0] = DecData[H] + DecData[H + 1] * 256;
	fout << item << " :	" << LSI_AWB[group].Golden[0] << endl;

	item = color + "_Golden_Gr";
	GetPrivateProfileString(TEXT("LSI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", LSI_AWB_Type);
	LSI_AWB[group].Golden[1] = DecData[H] + DecData[H + 1] * 256;
	fout << item << " :	" << LSI_AWB[group].Golden[1] << endl;

	item = color + "_Golden_Gb";
	GetPrivateProfileString(TEXT("LSI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", LSI_AWB_Type);
	LSI_AWB[group].Golden[2] = DecData[H] + DecData[H + 1] * 256;
	fout << item << " :	" << LSI_AWB[group].Golden[2] << endl;

	item = color + "_Golden_B";
	GetPrivateProfileString(TEXT("LSI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item, "", LSI_AWB_Type);
	LSI_AWB[group].Golden[3] = DecData[H] + DecData[H + 1] * 256;
	fout << item << " :	" << LSI_AWB[group].Golden[3] << endl;

	fout << endl;

}


void EEPROM_Data_Verifier::XiaoMi_AWB_Parse(int group) {

	TCHAR lpTexts[10]; int temp = 0, L, H;
	string s, color = "AWB_5100K ", item;
	fout << "-------" << color << " AWB Data------" << endl;
	if (group == 0)
		color = "AWB_5100K ";

	/////////////////////////////////// AWB 
	item = color + "R/Gr Ratio";
	GetPrivateProfileString(TEXT("XIAOMI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	XIAOMI_AWB_Data.AWB[0] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << XIAOMI_AWB_Data.AWB[0] << endl;

	value_Hash[6].item_name = item;
	value_Hash[6].hash[XIAOMI_AWB_Data.AWB[0]%1009]++;

	item = color + "B/Gr Ratio";
	GetPrivateProfileString(TEXT("XIAOMI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	XIAOMI_AWB_Data.AWB[1] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << XIAOMI_AWB_Data.AWB[1] << endl;

	value_Hash[7].item_name = item;
	value_Hash[7].hash[XIAOMI_AWB_Data.AWB[1] % 1009]++;

	item = color + "Gb/Gr Ratio";
	GetPrivateProfileString(TEXT("XIAOMI"), CA2CT(item.c_str()), TEXT(""), lpTexts, 9, CA2CT(EEPROM_Map.c_str()));
	s = CT2A(lpTexts);
	H = marking_Hex2int(s, item + " H", "", VIVO_AWB);
	map_Push(H + 1, item + " L", "", VIVO_AWB);
	XIAOMI_AWB_Data.AWB[2] = DecData[H] * 256 + DecData[H + 1];
	fout << item << " :	" << XIAOMI_AWB_Data.AWB[2] << endl;

	fout << endl;

}


void dual_Cal_Parse(int S, int E) {

	unsigned int Dou[2] = { 0,0 };
	double* dp = (double*)Dou;
	int e = S;

	if (modelSelect == 8)
		e++;

	if (modelSelect < 3 || modelSelect == 8) {
		S++;
		for (int p = 0; p < 2; p++) {
			for (int i = 0; i < 4; i++) {
				Dou[p] *= 256;
				Dou[p] += DecData[e + i];
			}
			e += 4;
		}
		fout << "Pan:	" << *dp << endl;

		for (int p = 0; p < 2; p++) {
			Dou[p] = 0;
			for (int i = 0; i < 4; i++) {
				Dou[p] *= 256;
				Dou[p] += DecData[e + i];
			}
			e += 4;
		}
		fout << "Roll:	" << *dp << endl;

		for (int p = 0; p < 2; p++) {
			Dou[p] = 0;
			for (int i = 0; i < 4; i++) {
				Dou[p] *= 256;
				Dou[p] += DecData[e + i];
			}
			e += 4;
		}
		fout << "Tilt:	" << *dp << endl;

		fout << "AF_calibration_code:	" << 256 * DecData[e] + DecData[e + 1] << endl;
	}

	if (modelSelect == 3 || modelSelect == 4) {
		e = e + 7;
		for (int p = 1; p >= 0; p--) {
			for (int i = 0; i < 4; i++) {
				Dou[p] *= 256;
				Dou[p] += DecData[e--];
			}
		}
		fout << "Pan:	" << *dp << endl;
		e = S + 15;
		for (int p = 1; p >= 0; p--) {
			Dou[p] = 0;
			for (int i = 0; i < 4; i++) {
				Dou[p] *= 256;
				Dou[p] += DecData[e--];
			}
		}
		fout << "Roll:	" << *dp << endl;
		e = S + 23;
		for (int p = 1; p >= 0; p--) {
			Dou[p] = 0;
			for (int i = 0; i < 4; i++) {
				Dou[p] *= 256;
				Dou[p] += DecData[e--];
			}
		}
		fout << "Tilt:	" << *dp << endl;

	}
}


int EEPROM_Data_Verifier::OIS_Parse() {

	int ret = 0;
	bool OIS_HL = HL & 2;

	for (int i = 0; i < 26; i++) {
		if (OIS_info_Item[i][1].length()>1) {
			if (i == 0){
				fout << "~~~~~~~~~~OIS Cal~~~~~~~~~~" << endl;
			}
			int e = unstringHex2int(OIS_info_Item[i][1]);
			if (e > 0){
				if (ret == -1)ret = 0;

				int d = unstringHex2int(OIS_info_Item[i][2]),c=0;
				if (d == 1) {
					c = DecData[e];
					marking_Hex2int(OIS_info_Item[i][1], OIS_info_Item[i][0], "", OIS_Hall);
					if (c == 255) {
						ret |= 1;
						string s = OIS_info_Item[i][0] + " Data in 0x" + OIS_info_Item[i][1] + " value NG!" + '\n';
						ui.log->insertPlainText(s.c_str());			
					}

				}
				if (d == 2) {
					c = short_Out(e, OIS_HL);
					if (OIS_HL == 0) {
						map_Push(e, OIS_info_Item[i][0] + "_L", "",  OIS_Hall);
						map_Push(e + 1, OIS_info_Item[i][0] + "_H","", OIS_Hall);
					}
					else {
						map_Push(e, OIS_info_Item[i][0] + "_H", "", OIS_Hall);
						map_Push(e + 1, OIS_info_Item[i][0] + "_L", "", OIS_Hall);
					}
					if (c == 0xFFFF) {
						ret |= 1;
						string s = OIS_info_Item[i][0] + " Data in 0x" + OIS_info_Item[i][1] + " value NG!" + '\n';
						ui.log->insertPlainText(s.c_str());
					}
				}
				else if (d == 4) {
					c = int_Out(e, OIS_HL);
					if (OIS_HL == 0) {
						map_Push(e, OIS_info_Item[i][0] + "[0]_L", "", OIS_Hall);
						map_Push(e + 1, OIS_info_Item[i][0] + "[1]_L", "", OIS_Hall);
						map_Push(e + 2, OIS_info_Item[i][0] + "[2]_H", "", OIS_Hall);
						map_Push(e + 3, OIS_info_Item[i][0] + "[3]_H", "", OIS_Hall);
					}
					else {
						map_Push(e, OIS_info_Item[i][0] + "[3]_H", "", OIS_Hall);
						map_Push(e + 1, OIS_info_Item[i][0] + "[2]_H", "", OIS_Hall);
						map_Push(e + 2, OIS_info_Item[i][0] + "[1]_L", "", OIS_Hall);
						map_Push(e + 3, OIS_info_Item[i][0] + "[0]_L", "", OIS_Hall);
					}
					if (c== 0xFFFFFFFFFF) {
						ret |= 1;
						string s = OIS_info_Item[i][0] + " Data in 0x" + OIS_info_Item[i][1] + " value NG!" + '\n';
						ui.log->insertPlainText(s.c_str());
					}
				}

				fout << OIS_info_Item[i][0] << ":	" << c << endl;
				//////////////////
				if (OIS_info_Item[i][3].length() > 1) {
					unsigned int start = unstringHex2int(OIS_info_Item[i][1]);
					unsigned int size = atoi(OIS_info_Item[i][2].c_str());
					string value;
					for (int a = start; a < (start + size);a++) {
						value += D[a][0];
						value += D[a][1];
					}
					if (strcmp(value.c_str(), OIS_info_Item[i][3].c_str()) != 0) {
						string log_out = OIS_info_Item[i][0];
						log_out += " Value NG!\n";
						ui.log->insertPlainText(log_out.c_str());
						ret |= 16;
					}
				}
				else {			
					value_Hash[i + 40].item_name = OIS_info_Item[i][0];
					value_Hash[i + 40].hash[abs(c) % 1009]++;
				}
			}
		}
	}


	float Gyro_Gain[2], SR[4];
	short offset[2] = { 0 };

	for (int i = 0; i < 8; i++) {
		int e = unstringHex2int(OIS_data_Item[i][1]);
		if (e > 0) {
			if (i < 2) {
				int d = unstringHex2int(OIS_data_Item[i][2]), c = 0;
				if (d == 2) {
					offset[i]=c = short_Out(e, OIS_HL);
					if (OIS_HL == 0) {
						map_Push(e, OIS_data_Item[i][0] + "_L", "", OIS_Gyro);
						map_Push(e + 1, OIS_data_Item[i][0] + "_H", "", OIS_Gyro);
					}
					else {
						map_Push(e, OIS_data_Item[i][0] + "_H", "", OIS_Gyro);
						map_Push(e + 1, OIS_data_Item[i][0] + "_L", "", OIS_Gyro);
					}
				}
				else if (d == 4) {
					offset[i] = c = int_Out(e, OIS_HL);
					if (OIS_HL == 0) {
						map_Push(e, OIS_data_Item[i][0] + "[0]_LL", "", OIS_Gyro);
						map_Push(e + 1, OIS_data_Item[i][0] + "[1]_LU", "", OIS_Gyro);
						map_Push(e + 2, OIS_data_Item[i][0] + "[2]_HL", "", OIS_Gyro);
						map_Push(e + 3, OIS_data_Item[i][0] + "[3]_HU", "", OIS_Gyro);
					}
					else {
						map_Push(e, OIS_data_Item[i][0] + "[3]_HU", "", OIS_Gyro);
						map_Push(e + 1, OIS_data_Item[i][0] + "[2]_HL", "", OIS_Gyro);
						map_Push(e + 2, OIS_data_Item[i][0] + "[1]_LU", "", OIS_Gyro);
						map_Push(e + 3, OIS_data_Item[i][0] + "[0]_LL", "", OIS_Gyro);
					}
				}
				fout << OIS_data_Item[i][0] << ":	" << c << endl;

				value_Hash[i + 66].item_name = OIS_info_Item[i][0];
				value_Hash[i + 66].hash[(0x7FFFFFFF+c) % 1009]++;

				if (c < -1*Gyro_offset_spec[i] || c>Gyro_offset_spec[i]) {
					string s = OIS_data_Item[i][0] + " Data in 0x" + OIS_data_Item[i][1] + " value NG!" + '\n';
					ui.log->insertPlainText(s.c_str());
					ret |= 1;
				}
			}
			else if (i > 1 && i < 4) {
				int d = unstringHex2int(OIS_data_Item[i][2]);
				if ((selection & 256)>0){
					Gyro_Gain[i - 2] = short_Out(e, OIS_HL);
					if (short_Out(e, OIS_HL) == -1 || short_Out(e, OIS_HL) == 0) {
						ret |= 2;
						string s = OIS_data_Item[i][0] + " Data in 0x" + OIS_data_Item[i][1] + " value NG!" + '\n';
						ui.log->insertPlainText(s.c_str());
					}
				}
				else if ((selection & 8) > 0) {
					if (d == 4) {
						Gyro_Gain[i - 2] = gyro_Out(e, OIS_HL);
					}
					else if (d == 2) {
						Gyro_Gain[i - 2] = Dcm_out2(e, OIS_HL);
					}
				}
				else {
					Gyro_Gain[i - 2] = flt_Out(e, OIS_HL);
					if (int_Out(e, OIS_HL) == -1 || int_Out(e, OIS_HL) == 0) {
						ret |= 2;
						string s = OIS_data_Item[i][0] + " Data in 0x" + OIS_data_Item[i][1] + " value NG!" + '\n';
						ui.log->insertPlainText(s.c_str());
					}
				}				
				

				if (OIS_HL == 0) {
					map_Push(e, OIS_data_Item[i][0] + "[0]_LL", "", OIS_Gyro);
					map_Push(e + 1, OIS_data_Item[i][0] + "[1]_LU", "", OIS_Gyro);
					map_Push(e + 2, OIS_data_Item[i][0] + "[2]_HL", "", OIS_Gyro);
					map_Push(e + 3, OIS_data_Item[i][0] + "[3]_HU", "", OIS_Gyro);
				}
				else {
					map_Push(e, OIS_data_Item[i][0] + "[3]_HU", "", OIS_Gyro);
					map_Push(e + 1, OIS_data_Item[i][0] + "[2]_HL", "", OIS_Gyro);
					map_Push(e + 2, OIS_data_Item[i][0] + "[1]_LU", "", OIS_Gyro);
					map_Push(e + 3, OIS_data_Item[i][0] + "[0]_LL", "", OIS_Gyro);
				}
				fout << OIS_data_Item[i][0] << ":	" << Gyro_Gain[i - 2] << endl;

				if (d == 4) {
					value_Hash[i + 66].item_name = OIS_info_Item[i][0];
					value_Hash[i + 66].hash[uint_Out(e, 1) % 1009]++;
				}
				else if (d == 2) {
					value_Hash[i + 66].item_name = OIS_info_Item[i][0];
					value_Hash[i + 66].hash[ushort_Out(e, 1) % 1009]++;
				}

			}
			else {
				bool SR_HL = HL & 4;
				if (ui.sr_hl->isChecked())
					SR[i - 4] = SR_Out_HL(e, !SR_HL);
				else
					SR[i - 4] = SR_Out_Hex(e, !SR_HL);

				if (ui.sr100->isChecked())
					SR[i - 4] = SR_Out100(e, !SR_HL);

				if (short_Out(e, OIS_HL) == -1)
					ret |= 4;

				if (OIS_HL == 0) {
					map_Push(e, OIS_data_Item[i][0] + "_L", "", OIS_Gyro);
					map_Push(e + 1, OIS_data_Item[i][0] + "_H", "", OIS_Gyro);
				}
				else {
					map_Push(e, OIS_data_Item[i][0] + "_H", "", OIS_Gyro);
					map_Push(e + 1, OIS_data_Item[i][0] + "_L", "", OIS_Gyro);
				}
				fout << OIS_data_Item[i][0] << ":	" << SR[i - 4] << endl;
				
				if (SR[i - 4] < SR_Spec[(i - 4) / 2][(i - 4)%2] || SR[i - 4] > 99) {
					string s = OIS_data_Item[i][0] + " Data in 0x" + OIS_data_Item[i][1] + " SR value NG!" + '\n';
					ui.log->insertPlainText(s.c_str());
					ret |= 4;
				}
				value_Hash[i + 66].item_name = OIS_info_Item[i][0];
				value_Hash[i + 66].hash[ushort_Out(e, 1) % 1009]++;
			}	
		}
	}

	fout << endl;
	return ret;
}


int EEPROM_Data_Verifier::info_Data_Parse() {

	int ret = 0;
	if (ui.info_date->isChecked()) {
		string s = "";
		int y1 = unstringHex2int(product_Date.item[0]);
		if (y1 > 0) {
			map_Push(y1, "Year_H", "", info1);
			s += to_string(DecData[y1]);
		}
		int y2 = unstringHex2int(product_Date.item[1]);
		if (y2 > 0) {
			if (y1 > 0)
				map_Push(y2, "Year_L", "", info1);
			else { map_Push(y2, "Year", "", info1); }

			s += to_string(DecData[y2]);

			if (DecData[y2] > 30)
				ret |= 1;
		}
		s += "-";
		int m = unstringHex2int(product_Date.item[2]);
		if (m > 0) {
			map_Push(m, "Month", "", info1);
			s += to_string(DecData[m]);
			s += "-";

			if (DecData[m] > 12)
				ret |= 1;
		}
		int d = unstringHex2int(product_Date.item[3]);
		if (d > 0) {
			map_Push(d, "Day", "", info1);
			s += to_string(DecData[d]);
			s += " ";
			if (DecData[d] > 31)
				ret |= 1;
		}
		int h = unstringHex2int(product_Date.item[4]);
		if (h > 0) {
			map_Push(h, "Hour", "", info1);
			s += to_string(DecData[h]);
			s += ":";
			if (DecData[h] > 24)
				ret |= 1;
		}
		m = unstringHex2int(product_Date.item[5]);
		if (m > 0) {
			map_Push(m, "Minute", "", info1);
			s += to_string(DecData[m]);
			s += ":";
			if (DecData[m] > 60)
				ret |= 1;
		}
		m = unstringHex2int(product_Date.item[6]);
		if (m > 0) {
			map_Push(m, "Second", "", info1);
			s += to_string(DecData[m]);
			if (DecData[m] > 60)
				ret |= 1;
		}
		m = unstringHex2int(product_Date.item[7]);
		if (m > 0) {
			map_Push(m, "Factory", "", info1);
			s += ", Fac:";
			s += to_string(DecData[m]);
		}
		m = unstringHex2int(product_Date.item[8]);
		if (m > 0) {
			map_Push(m, "Line", "", info1);
			s += ", Line:";
			s += to_string(DecData[m]);
		}

		fout << "Manufacture Time:	" << s << endl;
	}

	if ((ret & 1) > 0) {
		ui.log->insertPlainText("Date format is error!\n");
	}

	if (ui.info_QR->isChecked()) {
		fout << "QR:	";
		int start = marking_Hex2int(QR_Data.item[0],"QR Code"," ",QR);
		int end = unstringHex2int(QR_Data.item[1]);
		int k = 0;

		for (int i = start; i <= end; i++) {
			fout << (char)DecData[i];

			if (k < QR_Data.item[2].length()) {
				if (DecData[i] != QR_Data.item[2][k++]) {
					ui.log->insertPlainText("QR Data Fixed Value NG!\n");
					ret |= 1;
				}
			}
			useData[i] = 1;
		}
		if (DecData[start] == 255 && DecData[start + 1] == 255 && DecData[start + 2] == 255) {
			ui.log->insertPlainText("QR Data is ivalid!\n");
			ret |= 1;
		}
		fout << endl;
	}

	if (ui.info_fuse->isChecked()) {
		fout << "ID/Ver:	";
		int start = unstringHex2int(Fuse_Data.item[0]);
		int end = unstringHex2int(Fuse_Data.item[1]);
		int k = 0;
		for (int i = start; i <= end; i++) {
			map_Push(i, "ID/Ver." + to_string(i - start + 1), "", info1);
			fout << D[i][0]<< D[i][1];
			if (k < Fuse_Data.item[2].length()) {
				if (D[i][0] != Fuse_Data.item[2][k]|| D[i][1]!= Fuse_Data.item[2][k+1]) {
					ui.log->insertPlainText("ID/Ver Data Fixed Value NG!\n");
					ret |= 1;
				}
				k += 2;
			}
			useData[i] = 1;
		}
		if (DecData[start] == 255 && DecData[start + 1] == 255 && DecData[start + 2] == 255) {
			ui.log->insertPlainText("ID Data1 is ivalid!\n");
			ret |= 1;
		}

		fout << endl;
	}

	if (ui.info_fuse_2->isChecked()) {
		fout << "ID/Ver2:	";
		int start = unstringHex2int(Fuse_Data2.item[0]);
		int end = unstringHex2int(Fuse_Data2.item[1]);
		int k = 0;
		for (int i = start; i <= end; i++) {
	//		map_Push(i, "ID/Ver2." + to_string(i - start + 1), "", info1);
			fout << D[i][0] << D[i][1];
			if (k < Fuse_Data2.item[2].length()) {
				if (D[i][0] != Fuse_Data2.item[2][k] || D[i][1] != Fuse_Data2.item[2][k + 1]) {
					ui.log->insertPlainText("ID/Ver2 Data Fixed Value NG!\n");
					ret |= 1;
				}
				k += 2;
			}
			useData[i] = 1;
		}
		if (DecData[start] == 255 && DecData[start + 1] == 255 && DecData[start + 2] == 255) {
			ui.log->insertPlainText("ID Data2 is ivalid!\n");
			ret |= 1;
		}

		fout << endl;
	}

	fout  << endl;

	////////////////////////////////////////////////////////////
	fout << "-------Info Data Compare------" << endl;
	fout << "(Item)	(Data)	(Spec)" << endl;

	unsigned int spec;
	for (int i = 0; i < 32; i++) 
		if (infoData[i].item[0].length()>1) {
			if (i <22) {
				infoData[i].addr = marking_Hex2int(infoData[i].item[1], infoData[i].item[0], infoData[i].item[2], info1);
				fout << infoData[i].item[0] << ":	" << D[infoData[i].addr][0] << D[infoData[i].addr][1] << "	" << infoData[i].item[2] << endl;
				if(infoData[i].item[2].length()>1){
					infoData[i].spec = unstringHex2int(infoData[i].item[2]);
					if (DecData[infoData[i].addr] != infoData[i].spec) {
						string s = infoData[i].item[0] + " Data in 0x" + infoData[i].item[1] + " != " + infoData[i].item[2] + '\n';
						ui.log->insertPlainText(s.c_str());
						ret |= 2;
					}
				}
				else {
					if (DecData[infoData[i].addr] == 0xFF) {
						string s = infoData[i].item[0] + " Data in 0x" + infoData[i].item[1] + " = 0xFF"+ '\n';
						ui.log->insertPlainText(s.c_str());
						ret |= 4;
					}
				}
			}
			else {
				fout << infoData[i].item[0] << ":	";
				unsigned int addr = unstringHex2int(infoData[i].item[1]);
				unsigned int d = 0;
				if (HL&1 == 1) {
					string str = " ";
					if (infoData[i].item[2].length() >1)
						str = infoData[i].item[2].substr(0, 2);
					map_Push(addr, infoData[i].item[0] + "_H", str, info1);
					str = " ";
					if (infoData[i].item[2].length() == 4)
						str = infoData[i].item[2].substr(2, 2);
					map_Push(addr +1, infoData[i].item[0] + "_L",str, info1);
					fout << D[addr][0] << D[addr][1] << D[addr +1][0] << D[addr +1][1];
					d = DecData[addr+1] + DecData[addr] * 256;
				}
				else {
					string str = " ";
					if (infoData[i].item[2].length() == 4)
						str = infoData[i].item[2].substr(2, 2);
					map_Push(addr, infoData[i].item[0] + "_L",str, info1);
					str = " ";
					if (infoData[i].item[2].length() >1)
						str = infoData[i].item[2].substr(0, 2);
					map_Push(addr + 1, infoData[i].item[0] + "_H",str, info1);
					fout << D[addr + 1][0] << D[addr + 1][1] << D[addr][0] << D[addr][1] ;
					d = DecData[addr] + DecData[addr+1] * 256;
				}
				fout << "	" << infoData[i].item[2] << endl;

				if (infoData[i].item[2].length() > 1) {
					spec = unstringHex2int(infoData[i].item[2]);
					if (d != spec) {
						string s = infoData[i].item[0] + " Data in 0x" + infoData[i].item[1] + " != " + infoData[i].item[2] + '\n';
						ui.log->insertPlainText(s.c_str());
						ret |= 2;
					}
				}
				else {
					if (d == 0xFFFF) {
						string s = infoData[i].item[0] + " Data in 0x" + infoData[i].item[1] + " = 0xFFFF" + '\n';
						ui.log->insertPlainText(s.c_str());
						ret |= 4;
					}
				}
			}
		}
	fout << endl;

	///////////////////// Long INFO check
	for (int i = 0; i < 10; i++) {
		if (AF_info_Item[i][1].length()>1) { //各种字节长度一并检查
			string txtout = AF_info_Item[i][0]+": ";
			fout << txtout.c_str();
			unsigned int start = unstringHex2int(AF_info_Item[i][1]);
			unsigned int size = atoi(AF_info_Item[i][2].c_str());
			string value;
			for (int k = start; k < (start + size); k++) {
				value += D[k][0];
				value += D[k][1];
			}
			fout << value.c_str() << endl;

			string tmp = value.substr(0, AF_info_Item[i][3].length());

			if (AF_info_Item[i][3].length()>1)
				if (strcmp(tmp.c_str(), AF_info_Item[i][3].c_str()) != 0) {
					string log_out = AF_info_Item[i][0];
					log_out += " Value NG!\n";
					ui.log->insertPlainText(log_out.c_str());
					ret |= 16;
				}
				else {
					bool value_empty = true;
					for (int k = 0; k < value.length(); k++) {
						if (value[k] != 'F') {
							value_empty = false;
							break;
						}
					} 
					if (value_empty) {
						ret |= 32;
						string log_out = AF_info_Item[i][0];
						log_out += " Value NG!\n";
						ui.log->insertPlainText(log_out.c_str());
					}
				}
		}
	}

	return ret;
}


int EEPROM_Data_Verifier::value_Data_Parse() {

	int tret = 0,mHL=0;
	fout << "-------Value Data Spec Check------" << endl;
	/////////////////////////Value Data check
	for (int i = 0; i < 18; i++)
		if (sData_Item[i][1].length()>1&& sData_Item[i][2].length()>1) {
			unsigned int addr = unstringHex2int(sData_Item[i][1]);
			mHL = unstringHex2int(sData_Item[i][5]);
			value_Hash[i + 20].item_name = sData_Item[i][0];

			int d_type = get_Data_Type(i),s=0;
			long long d = 0;
			double dd = 0;

			if (d_type == 0 || addr==0)
				continue;

			switch (d_type){
			case 1:
				d = DecData[addr];
				if (d > 0x7F)
					d = d - 0x100;
				break;
			case 2:
				d = DecData[addr];
				break;
			case 3:
				d = short_Out(addr, mHL&1);
				break;
			case 4:
				d = short_Out(addr,mHL&1);
				if (d < 0)
					d += 65536;
				break;
			case 5:
				d = int_Out(addr,mHL&1);
				break;
			case 6:
				d = int_Out(addr, mHL&1);		
				if (d < 0)
					d += 0x100000000;
				break;
			case 7:
				dd = short_Out(addr, mHL&1);
				if (dd < 0)
					dd += 0x10000;
				if(mHL<9)
					value_Hash[i + 20].hash[((int)dd)% 1009] ++;
				dd /= 0x8000;
				break;
			case 8:
				dd = int_Out(addr, mHL&1);
				if (dd < 0)
					dd += 0x100000000;
				dd /= 0x80000000;
				break;
			case 9:
				dd = flt_Out(addr, mHL&1);
				break;
			case 10:
				dd = dbl_Out(addr, mHL&1);
				break;
			case 11:
				dd = dbl_Out(addr, mHL&1);
				break;
			default:
				break;
			}
			int data_len = 1;

			if (d_type > 100) {
				int Q_length = d_type - 100;
				if (Q_length > 15) {
					dd = uint_Out(addr, mHL&1)*1.0 / (1<< Q_length);
					data_len = 4;
				}
				else if(Q_length < 16) {
					dd = ushort_Out(addr, mHL & 1)*1.0 / (1 << Q_length);
 					data_len = 2;
				}
			}

		
			if (d_type == 3 || d_type == 4|| d_type == 7) {
				data_len = 2;
			}
			else if (d_type == 10) {
				data_len = 8;
			}
			else if (d_type >2) {
				data_len = 4;
			}

			s = 0;
			for (int a = addr; a < addr + data_len; a++) {
				s = (s * 256 + DecData[a]) % 1009;
			}
			if (mHL<9)
				value_Hash[i + 20].hash[s] ++;

			int ret = 0;
			//////////////////////value check		
			if (d_type < 7) {
				if (sData_Item[i][3].length() > 0) {
					int spec1 = atoi(sData_Item[i][3].c_str());
					if (d < spec1)
						ret |= 1;
				}
				if (sData_Item[i][4].length() > 0) {
					int spec2 = atoi(sData_Item[i][4].c_str());
					if (d > spec2)
						ret |= 1;
				}
				fout << sData_Item[i][0] << ":	" << d << endl;

			}
			else {
				if (sData_Item[i][3].length() > 0) {
					float spec1 = atof(sData_Item[i][3].c_str());
					if (dd < spec1)
						ret |= 2;
				}
				if (sData_Item[i][4].length() > 0) {
					float spec2 = atof(sData_Item[i][4].c_str());
					if (dd > spec2)
						ret |= 2;
				}
				fout << sData_Item[i][0] << ":	" << dd << endl;
			}

			if (ret>0) {
				string s = sData_Item[i][0] + " Data in 0x" + sData_Item[i][1] + " NG!\n";
				ui.log->insertPlainText(s.c_str());
				tret = tret * 10 + ret;
			}
	
		}
	fout << endl;
	fout << "-------Loop Data Spec Check------" << endl;
	TCHAR lpTexts[256];
	for (int i = 0; i < 20; i++) {
		string item;
		item = "Loop_Size_" + to_string(i + 1);
		GetPrivateProfileString(TEXT("LOOP_DATA"), CA2CT(item.c_str()), TEXT("0"), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		int loop_Size = _wtoi(lpTexts);
		item = "Loop_Count_" + to_string(i + 1);
		GetPrivateProfileString(TEXT("LOOP_DATA"), CA2CT(item.c_str()), TEXT("0"), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
		int Loop_Count = _wtoi(lpTexts);
		if (loop_Size >0&& Loop_Count>0) {
			for (int j = 0; j < 10; j++) {
				string Start1 = "Data_Start_" + to_string(i + 1) + to_string(j + 1);
				int addr = GetPrivateProfileInt(_T("LOOP_DATA"), CA2CT(Start1.c_str()), -1, CA2CT(EEPROM_Map.c_str()));
				string Type1 = "Data_Type_" + to_string(i + 1) + to_string(j + 1);
				GetPrivateProfileString(TEXT("LOOP_DATA"), CA2CT(Type1.c_str()), TEXT("0"), lpTexts, 64, CA2CT(EEPROM_Map.c_str()));
				int d_type = get_Data_TypeTchar(lpTexts);

				if (addr > 0) {
					if (d_type == 0) {
						tret |= 0x10000;
						continue;
					}
					for (int k = 0; k < Loop_Count; k++, addr+= loop_Size) {

						long long d = 0;
						double dd = 0;

						switch (d_type) {
						case 1:
							d = DecData[addr];
							if (d > 0x7F)
								d = d - 0x100;
							break;
						case 2:
							d = DecData[addr];
							break;
						case 3:
							d = short_Out(addr, HL & 1);
							break;
						case 4:
							d = short_Out(addr, HL & 1);
							if (d < 0)
								d += 65536;
							break;
						case 5:
							d = int_Out(addr, HL & 1);
							break;
						case 6:
							d = int_Out(addr, HL & 1);
							if (d < 0)
								d += 0x100000000;
							break;
						case 7:
							dd = short_Out(addr, HL & 1);
							if (dd < 0)
								dd += 0x10000;
							if (HL<9)
								value_Hash[i + 20].hash[((int)dd) % 1009] ++;
							dd /= 0x8000;
							break;
						case 8:
							dd = int_Out(addr, HL & 1);
							if (dd < 0)
								dd += 0x100000000;
							dd /= 0x80000000;
							break;
						case 9:
							dd = flt_Out(addr, HL & 1);
							break;
						case 10:
							dd = dbl_Out(addr, HL & 1);
							break;
						case 11:
							dd = uByte_3(addr, HL & 1);
							break;
						default:
							break;
						}

						int ret = 0;
						//////////////////////value check		
						string Spec_Min = "Spec_Min_" + to_string(i + 1) + to_string(j + 1) + to_string(k + 1);
						int spec1 = GetPrivateProfileInt(_T("LOOP_DATA"), CA2CT(Spec_Min.c_str()), -999999991, CA2CT(EEPROM_Map.c_str()));

						string Spec_Max = "Spec_Max_" + to_string(i + 1) + to_string(j + 1) + to_string(k + 1);
						int spec2 = GetPrivateProfileInt(_T("LOOP_DATA"), CA2CT(Spec_Max.c_str()), 999999991, CA2CT(EEPROM_Map.c_str()));

						if (d_type < 7) {
							if (d < spec1&&spec1!= -999999991)	
								ret |= 2;
							if (d > spec2&&spec2!= 999999991)	
								ret |= 2;
							fout << Start1 + to_string(k + 1) << ":	" << d << endl;
						}
						else {
							if (dd < spec1&&spec1 != -999999991)	ret |= 2;
							if (dd > spec2&&spec2 != 999999991)	ret |= 2;
							fout << Start1 + to_string(k + 1) << ":	" << dd << endl;
						}

						if (ret>0) {
							string s = Start1 +to_string(k + 1)+ " in addr" + to_string(addr) + " NG!\n";
							ui.log->insertPlainText(s.c_str());
							tret = tret * 1000 + ret;
						}

					}
				}
			}
		}
	}


	return tret;

}


int EEPROM_Data_Verifier::get_Data_TypeTchar(TCHAR Str[256]) {

	if (Str[0] == 'c'&&Str[1] == 'h'&&Str[2] == 'a') {
		return 1;
	}
	else if (Str[0] == 'u'&&Str[1] == 'c'&&Str[2] == 'h') {
		return 2;
	}
	else if (Str[0] == 's'&&Str[1] == 'h'&&Str[2] == 'o') {
		return 3;
	}
	else if (Str[0] == 'u'&&Str[1] == 's'&&Str[2] == 'h') {
		return 4;
	}
	else if (Str[0] == 'i'&&Str[1] == 'n'&&Str[2] == 't') {
		return 5;
	}
	else if (Str[0] == 'u'&&Str[1] == 'i'&&Str[2] == 'n') {
		return 6;
	}
	else if (Str[0] == '2'&&Str[1] == 'd'&&Str[2] == 'c') {
		return 7;
	}
	else if (Str[0] == '4'&&Str[1] == 'd'&&Str[2] == 'c') {
		return 8;
	}
	else if (Str[0] == 'f'&&Str[1] == 'l') {
		return 9;
	}
	else if (Str[0] == 'd'&&Str[1] == 'o'&&Str[2] == 'u') {
		return 10;
	}
	else if (Str[0] == 'u'&&Str[1] == '3'&&Str[2] == 'b') {
		return 11;
	}
	else if (Str[0] == 'Q') {
		int k = 1,sum= Str[0] -'0';
		if(Str[1]>='0')
			sum = sum * 10 + Str[1]-'0';	
		return sum+100;
	}

	return 0;
}

int EEPROM_Data_Verifier::get_Data_Type(int x) {

	int ret = 0;

	if (sData_Item[x][2][0] == 'c'&&sData_Item[x][2][1] == 'h'&&sData_Item[x][2][2] == 'a') {
		return 1;
	}
	else if (sData_Item[x][2][0] == 'u'&&sData_Item[x][2][1] == 'c'&&sData_Item[x][2][2] == 'h') {
		return 2;
	}
	else if (sData_Item[x][2][0] == 's'&&sData_Item[x][2][1] == 'h'&&sData_Item[x][2][2] == 'o') {
		return 3;
	}
	else if (sData_Item[x][2][0] == 'u'&&sData_Item[x][2][1] == 's'&&sData_Item[x][2][2] == 'h') {
		return 4;
	}
	else if (sData_Item[x][2][0] == 'i'&&sData_Item[x][2][1] == 'n'&&sData_Item[x][2][2] == 't') {
		return 5;
	}
	else if (sData_Item[x][2][0] == 'u'&&sData_Item[x][2][1] == 'i'&&sData_Item[x][2][2] == 'n') {
		return 6;
	}
	else if (sData_Item[x][2][0] == '2'&&sData_Item[x][2][1] == 'd'&&sData_Item[x][2][2] == 'c') {
		return 7;
	}
	else if (sData_Item[x][2][0] == '4'&&sData_Item[x][2][1] == 'd'&&sData_Item[x][2][2] == 'c') {
		return 8;
	}
	else if (sData_Item[x][2][0] == 'f'&&sData_Item[x][2][1] == 'l') {
		return 9;
	}
	else if (sData_Item[x][2][0] == 'd'&&sData_Item[x][2][1] == 'o'&&sData_Item[x][2][2] == 'u') {
		return 10;
	}
	else if (sData_Item[x][2][0] == 'Q') {
		int k = 1, sum = 0;
		while (k < sData_Item[x][2].length()) {
			sum = sum * 10 + sData_Item[x][2][k++] - '0';
		}
		return sum + 100;
	}

	return ret;
}


int EEPROM_Data_Verifier::AEC_Parse() {

	int addr, ret = 0;;
	for (int i = 0; i < 6; i++)
		if (i%2==0) {
			if (AEC_Data[i].item[0].length()>1) {
				if(i==0)
					fout << "-------AEC Cal Data------" << endl;
				if (HL&1 == 1) {
					addr= marking_Hex2int(AEC_Data[i].item[1], AEC_Data[i].item[0] + "_H", "", info1);
					map_Push(addr + 1, AEC_Data[i].item[0] + "_L", "", info1);
					fout << AEC_Data[i].item[0] << ":	" << D[addr][0] << D[addr][1] << D[addr + 1][0] << D[addr + 1][1] << endl;
				}
				else {
					addr = marking_Hex2int(AEC_Data[i].item[1], AEC_Data[i].item[0] + "_L", "", info1);
					map_Push(addr + 1, AEC_Data[i].item[0] + "_H", "", info1);

					if (ui.sony->isChecked() && i > 1 && i < 4) {
						fout << AEC_Data[i].item[0] << ":	" << flt_Out(addr,HL) << endl;
					}
					else {
						fout << AEC_Data[i].item[0] << ":	" << D[addr + 1][0] << D[addr + 1][1] << D[addr][0] << D[addr][1] << endl;
					}
				} 
				int x = DecData[addr] * 256 + DecData[addr + 1];
				if (x == 65535 || x == 0) {
					ret |= 1;
				}
				if (i == 2) {
					if (x < 5000 || x>40000) {
						ret |= 2;
					}
				}
			}
		}

	for (int i = 0; i < 6; i++)
		if (i % 2 == 1) {
			if (AEC_Data[i].item[0].length()>1) {
				if (HL&1 == 1) {
					addr = marking_Hex2int(AEC_Data[i].item[1], AEC_Data[i].item[0] + "_H", "", info1);
					map_Push(addr + 1, AEC_Data[i].item[0] + "_L", "", info1);
					fout << AEC_Data[i].item[0] << ":	" << D[addr][0] << D[addr][1] << D[addr + 1][0] << D[addr + 1][1] << endl;
				}
				else {
					addr = marking_Hex2int(AEC_Data[i].item[1], AEC_Data[i].item[0] + "_L", "", info1);
					map_Push(addr + 1, AEC_Data[i].item[0] + "_H", "", info1);
					fout << AEC_Data[i].item[0] << ":	" << D[addr + 1][0] << D[addr + 1][1] << D[addr][0] << D[addr][1] << endl;
				}
				int x = DecData[addr] * 256 + DecData[addr + 1];
				if (x == 65535 || x == 0) {
					ret |= 1;
				}
				if (i ==5) {
					if (x < 5000 || x>40000) {
						ret |= 2;
					}
				}
			}
		}

	fout << endl;

	for (int i = 6; i < 12; i++)
		if (i % 2 == 0) {

			if (AEC_Data[i].item[0].length()>1) {
				if (i == 0)
					fout << "-------AEC Cal Data------" << endl;
				if (HL&1 == 1) {
					addr = marking_Hex2int(AEC_Data[i].item[1], AEC_Data[i].item[0] + "_H", "", info1);
					map_Push(addr + 1, AEC_Data[i].item[0] + "_L", "", info1);
					fout << AEC_Data[i].item[0] << ":	" << D[addr][0] << D[addr][1] << D[addr + 1][0] << D[addr + 1][1] << endl;
				}
				else {
					addr = marking_Hex2int(AEC_Data[i].item[1], AEC_Data[i].item[0] + "_L", "", info1);
					map_Push(addr + 1, AEC_Data[i].item[0] + "_H", "", info1);
					fout << AEC_Data[i].item[0] << ":	" << D[addr + 1][0] << D[addr + 1][1] << D[addr][0] << D[addr][1] << endl;
				}
				int x = DecData[addr] * 256 + DecData[addr + 1];
				if (x == 65535 || x == 0) {
					ret |= 1;
				}
				if (i == 8) {
					if (x < 5000 || x>40000) {
						ret |= 2;
					}
				}
			}
		}

	for (int i = 6; i < 12; i++)
		if (i % 2 == 1) {
			if (AEC_Data[i].item[0].length()>1) {
				if (HL&1 == 1) {
					addr = marking_Hex2int(AEC_Data[i].item[1], AEC_Data[i].item[0] + "_H", "", info1);
					map_Push(addr + 1, AEC_Data[i].item[0] + "_L", "", info1);
					fout << AEC_Data[i].item[0] << ":	" << D[addr][0] << D[addr][1] << D[addr + 1][0] << D[addr + 1][1] << endl;
				}
				else {
					addr = marking_Hex2int(AEC_Data[i].item[1], AEC_Data[i].item[0] + "_L", "", info1);
					map_Push(addr + 1, AEC_Data[i].item[0] + "_H", "", info1);
					fout << AEC_Data[i].item[0] << ":	" << D[addr + 1][0] << D[addr + 1][1] << D[addr][0] << D[addr][1] << endl;
				}
				int x = DecData[addr] * 256 + DecData[addr + 1];
				if (x == 65535 || x == 0) {
					ret |= 1;
				}
				if (i == 11) {
					if (x < 5000 || x>40000) {
						ret |= 2;
					}
				}
			}
		}
	fout << endl;
	return ret;
}


int EEPROM_Data_Verifier::XiaoMi_Seg_Check() {

	int addr, ret = 0;;
	for (int i = 0; i < 20; i++) {
		if (Seg_Data[i].item[0].length()>1) {
			
			int index_addr = 0, seg_Num=0;
			if (Seg_Data[i].item[2].length() > 2) {

				string s = "Segment_" + Seg_Data[i].item[0] + "_H";
				index_addr = addr = marking_Hex2int(Seg_Data[i].item[2], s, "", info1);
				if (addr > 0xE000)
					index_addr = addr = addr - 0xE000;
				s = "Segment_" + Seg_Data[i].item[0] + "_L";
				map_Push(addr + 1, s, "", info1);
				seg_Num = DecData[addr] * 256 + DecData[addr + 1];
				int spec = i + 1;
				if (i == 19) {
					spec = 0x8000;
				}
				if (i ==17&& Seg_Data[19].item[0].length()<2) {
					spec = 0x8000;
				}
				if (seg_Num != spec) {
					s = "Segment_" + Seg_Data[i].item[0] + " value NG!\n";
					ui.log->insertPlainText(s.c_str());
					ret |= 1;
				}
			}

			if (Seg_Data[i].item[1].length() > 2) {		
				if (i < 19) {
					string s = "Segment_" + to_string(i+1)+ "_H";
					addr = marking_Hex2int(Seg_Data[i].item[1], s, "", info1);				
					s = "Segment_" + to_string(i + 1) + "_L";
					map_Push(addr + 1, s, "", info1);
				}
				else {
					string s = "Segment_checksum offset_H";
					addr = marking_Hex2int(Seg_Data[i].item[1], s, "", info1);
					s = "Segment_checksum offset_L";
					map_Push(addr + 1, s, "", info1);

				}
				int temp = DecData[addr] * 256 + DecData[addr + 1];
				if (temp > 0xE000)
					temp = temp - 0xE000;
				if(temp!=index_addr) {
					string s = "Segment_" + Seg_Data[i].item[0] + " value NG!\n";
					ui.log->insertPlainText(s.c_str());
					ret |= 1;
				}
			}
		}
	}
	return ret;
}


void EEPROM_Data_Verifier::History_Date_Parser() {

	int addr;
	for (int i = 0; i < 10; i++) {
		if (History_Data[i].item[1].length()>1) {
		
				if (i == 0)
					fout << "(Item)	(Flag)	(Line)	(Equip)	(Date)" << endl;

				addr = marking_Hex2int(History_Data[i].item[1], Seg_Data[i].item[0] + " Flag","", info1);

				map_Push(addr + 1, History_Data[i].item[0] + " Line number", "", info1);
				map_Push(addr + 2, History_Data[i].item[0] + " Equip number", "", info1);
				map_Push(addr + 3, History_Data[i].item[0] + "  Year", "", info1);
				map_Push(addr + 4, History_Data[i].item[0] + "  Month", "", info1);
				map_Push(addr + 5, History_Data[i].item[0] + "  Day", "", info1);
				map_Push(addr + 6, History_Data[i].item[0] + "  Hour", "", info1);

				fout << History_Data[i].item[0] << "	" << D[addr][0] << D[addr][1] <<"	" << D[addr+1][0] << D[addr+1][1] <<"	" << D[addr + 2][0] << D[addr + 2][1] <<"	" <<
					D[addr + 3][0] << D[addr + 3][1] <<"-" << D[addr + 4][0] << D[addr + 4][1] << "-" << D[addr + 5][0] << D[addr + 5][1] << " " << D[addr + 6][0] << D[addr + 6][1] << ":00\n";
		}
	}
}


int EEPROM_Data_Verifier::Reserved_Check() {
	int ret = 0;

	for (int i = 0; i <= customer_end; i++) 
		if (useData[i] == 0) {	
			if (DecData[i] != noData.dec) {
				QString strDisplay = "Data in ";
				strDisplay += address2hex(i);
				strDisplay += " is ";
				strDisplay.append(D[i][0]);
				strDisplay.append(D[i][1]);
				strDisplay += ", should be reserved value.\n";
				ui.log->insertPlainText(strDisplay);
				ret |= 1;
			}
		}
	return ret;
}


void EEPROM_Data_Verifier::display_EEP() {

	ui.input->clear();
	ui.input->setFontPointSize(9);
	for (int i = 0; i < EEP_Size; i++) {

		if (i % 16 == 0) {
			unsigned char a = i / 256;
			getHex(a);
			string s = chk;
			s[2] = '\0';
			ui.input->insertPlainText(s.c_str());
			unsigned char b = i % 256;
			getHex(b);
			char st[5] = { 0 };
			st[0] = chk[0]; st[1] = chk[1]; st[2] = ':'; st[3] = ' ';
			ui.input->insertPlainText(st);
		}

		getHex(DecData[i]);
		char st[4] = { 0 };
		st[0] = chk[0]; st[1] = chk[1]; st[2] = ' '; st[3] = '\0';
		ui.input->insertPlainText(st);

		if (i % 16 == 15)
			ui.input->insertPlainText("\n");
	}

	//	s = ui.input->document()->toPlainText().toLocal8Bit();
	//	fout << s << endl<<endl;

}


void EEPROM_Data_Verifier::on_pushButton_checkSum_clicked() {

	display_EEP();

}


void EEPROM_Data_Verifier::on_pushButton_openBIN_clicked() {

	ui.input->clear();

	QString filename = QFileDialog::getOpenFileName(this, tr("Open Bin"), "", tr("BIN File(*.bin *.rom)"));
	QTextCodec *code = QTextCodec::codecForName("gb18030");
	std::string name = code->fromUnicode(filename).data();

	ifstream fin(name, std::ios::binary);

	//获取文件大小方法一
	struct _stat info;
	_stat(name.c_str(), &info);
	EEP_Size = info.st_size;

	unsigned char szBuf[524288] = { 0 };
	fin.read((char*)&szBuf, sizeof(char) * EEP_Size);

	for (int i = 0; i < EEP_Size; i++) {

		if (i % 16 == 0) {
			unsigned char a = i / 256;
			getHex(a);
			string s = chk;
			s[2] = '\0';
			ui.input->insertPlainText(s.c_str());
			unsigned char b = i % 256;
			getHex(b);
			char st[5] = { 0 };
			st[0] = chk[0]; st[1] = chk[1]; st[2] = ':'; st[3] = ' ';
			ui.input->insertPlainText(st);
		}

		getHex(szBuf[i]);

		string s = "";
		s += chk[0];
		s += chk[1];
		s += ' ';

		ui.input->insertPlainText(s.c_str());

		if (i % 16 == 15)
			ui.input->insertPlainText("\n");
	}

	fin.close();
	on_pushButton_parser_clicked();
}


int EEPROM_Data_Verifier::read_Spec_Bin() {

	int ret = 1;
	name = EEPROM_Map.substr(0, EEPROM_Map.length() - 3)+"bin";
	ifstream fin(name, std::ios::binary);

	if (!fin)
		return 0;

	//获取文件大小方法一
	struct _stat info;
	_stat(name.c_str(), &info);
	EEP_Size = info.st_size;

	fin.read((char*)&spec_Data, sizeof(char) * EEP_Size);
	fin.close();

	return ret;
}


void EEPROM_Data_Verifier::on_pushButton_saveBIN_clicked() {

	if (!ui.pushButton_parser->isEnabled()) {

		src = ui.input->document()->toPlainText().toLocal8Bit();
		int now = 0, e = 0, len = src.length() - 1;

		while (now < len && e < EEP_Size) {
			if ((now == 0 || src[now - 1] == ' ' || src[now - 1] == '	' || src[now - 1] == '\n') &&
				((src[now + 2] == ' '&&src[now + 5] == ' '&&src[now + 8] == ' ')
					|| (src[now + 2] == '	'&&src[now + 5] == '	'&&src[now + 8] == '	'))) {

				for (int i = 0; i < 16; i++) {
					D[e][0] = src[now++];
					D[e][1] = src[now++];
					DecData[e] = hex2Dec(e);
					e++;
					now++;
				}
			}
			else now++;
		}
	}

	std::ofstream fout("Data.bin", std::ios::binary);

	for (int i = 0; i < EEP_Size; i++) {
		fout.write((char*)&DecData[i], sizeof(char));
	}

	fout.close();
	ui.log->insertPlainText("Data.bin saved. \n");
}


void EEPROM_Data_Verifier::on_pushButton_setsave_clicked() {

	save_EEPROM_Address();
	load_EEPROM_Address();

}


void EEPROM_Data_Verifier::on_pushButton_load_lsc_clicked() {

	selectModel();

	ifstream fin(".//QC_LSC.txt");

	for (int k = 0; k < 4; k++)
		for (int i = 0; i < 13; i++)
			for (int j = 0; j < 17; j++) {
				fin >> LSC[i][j][k];
			}

	if (modelSelect == 3 || modelSelect == 4 || modelSelect == 14) {
		int e = unstringHex2int(QC_LSC_Data[0].item[1]);;
		for (int i = 0; i < 13; i++)
			for (int j = 0; j < 17; j++) {
				DecData[e + 4] = 0;
				for (int k = 0; k < 4; k++) {
					DecData[e + k] = LSC[i][j][k] % 256;
					getHex(DecData[e + k]);
					D[e + k][0] = chk[0];
					D[e + k][1] = chk[1];
					DecData[e + 4] += (LSC[i][j][k] / 256) << (2 * (3 - k));
				}
				getHex(DecData[e + 4]);
				D[e + 4][0] = chk[0];
				D[e + 4][1] = chk[1];

				e += 5;
				if ((i * 17 + j + 1) % 51 == 0) {
					e++;
				}
			}
	}

	display_EEP();

}


int EEPROM_Data_Verifier::bin_Area_Check() {

	int start, end, ret=0;
	for (int i = 0; i < 6; i++) {
		if (Same_Item[i].item[0].length()>1) {
			start = unstringHex2int(Same_Item[i].item[1]);
			end = unstringHex2int(Same_Item[i].item[2]);

			unsigned int tsum = 0, FFcnt = 0, Zerocnt = 0;
			unsigned long long sumXsum = 0;

			for (int k = start; k <= end; k++) {
				sumXsum = ((sumXsum << 8) + DecData[k]) % 4000000007;

				if (DecData[k] == 255) {
					FFcnt++;
					if (FFcnt == 8) {
						ret |= (2 << i);
					}
				}
				else {
					FFcnt = 0;
				}
				if (DecData[k] == 0) {
					Zerocnt++;
					if (FFcnt == 16) {
						ret |= (2 << i);
					}
				}
				else {
					Zerocnt = 0;
				}
			}
			current_Hash.Same[i] = sumXsum;
			if ((ret&(2 << i)) > 0) {
				current_Hash.Same[i] = 0;
			}
		}

		if ((ret&(2 << i)) > 0) {
			string str = "bin area_" + to_string(i+1)+" NG!\n";
			ui.log->insertPlainText(str.c_str());
		}
	}

	return ret;
}


int EEPROM_Data_Verifier::duplicate_Check() {
	int ret = 0;
	size_t Size = dump_Hash.size() ;
	for (size_t i = 0; i < Size; i++) {
		if (!str_cmp(dump_Hash[i].fuse_ID, current_Hash.fuse_ID)){
			for (int k = 0; k < 6; k++)
				if (Same_Item[k].item[0].length()>1)
					if (current_Hash.Same[k] != 0) {
						if (dump_Hash[i].Same[k] == current_Hash.Same[k]) {
							ret |= (2 << k);
							fout << Same_Item[k].item[0] << " Duplicate ID:	" << dump_Hash[i].fuse_ID << endl;
						}
					}
			if (MTK_LSC_Data[0].item[0].length()>1)
				if (current_Hash.MTK_LSC != 0)
					if (dump_Hash[i].MTK_LSC == current_Hash.MTK_LSC) {
						ret |= 1;
						fout << MTK_LSC_Data[0].item[0] << " Duplicate ID:	" << dump_Hash[i].fuse_ID << endl;
					}
		}
	}
	return ret;
}

 
void EEPROM_Data_Verifier::on_pushButton_parser_clicked()
{
	mode = 0;
	memset(DecData, 0, sizeof(DecData));
	memset(useData, 0, sizeof(useData));
	memset(checkSum, 0, sizeof(checkSum));

	fout.open(".\\MemoryParseData.txt");

	selectModel();
	load_EEPROM_Address();
	load_Spec(mode);
	src = ui.input->document()->toPlainText().toLocal8Bit();

	if (src.size() > 192000)
		EEP_Size = 0x10000;
	else if (src.size() > 162000)
		EEP_Size = 0xD800;
	else if (src.size() > 96000)
		EEP_Size = 32768;
	else if (src.size() > 48000)
		EEP_Size = 16384;
	else if (src.size() > 36000)
		EEP_Size = 12288;
	else if (src.size() > 24000)
		EEP_Size = 8192;

	int now = 0, e = 0, len = src.length() - 1, TCSum = 0;
	unsigned int tmp = 0;
	float* fp = (float*)&tmp;
	ui.output->clear();

	while (now < len && e < EEP_Size) {
		if ((now == 0 || src[now - 1] == ' ' || src[now - 1] == '	' || src[now - 1] == '\n') &&
			((src[now + 2] == ' '&&src[now + 5] == ' '&&src[now + 8] == ' ')
				|| (src[now + 2] == '	'&&src[now + 5] == '	'&&src[now + 8] == '	')
				|| (src[now + 2] == '\n'&&src[now + 5] == '\n'&&src[now + 8] == '\n'))) {

			for (int i = 0; i < 16; i++) {
				D[e][0] = src[now++];
				D[e][1] = src[now++];
				DecData[e] = hex2Dec(e);
				e++;
				now++;
			}
		}
		else now++;
	}

	display_EEP();
	CheckSum_Check();
	info_Data_Parse();
	af_Parse();

	if (ui.oppo->isChecked()) {
		Oppo_AWB_Parse(0);
		Oppo_AWB_Parse(1);
		Oppo_AWB_Parse(2);
		if (OPPO_QC_AWB_FP_Check(OPPO_AWB) != 0) {
			ui.log->insertPlainText("QC AWB Data NG, please check FP_log. \n");
		}
	}

	if (ui.vivo->isChecked()) {
		VIVO_AWB_Parse(0);
		VIVO_AWB_Parse(1);
		if (VIVO_QC_AWB_FP_Check(VIVO_AWB_Data) != 0) {
			ui.log->insertPlainText("QC AWB Data NG, please check FP_log. \n");
		}

		if (ui.MTK->isChecked()) {
			vivo_MTK_AWB_Parse(0);
			vivo_MTK_AWB_Parse(1);
			if (VIVO_QC_AWB_FP_Check(VIVO_AWB_Data) != 0) {
				ui.log->insertPlainText("MTK AWB Data NG, please check FP_log. \n");
			}
		}
		if (ui.LSI->isChecked()) {
			LSI_AWB_Parse(0);
			LSI_AWB_Parse(1);
			if (LSI_AWB_FP_Check(LSI_AWB,1) != 0) {
				ui.log->insertPlainText("LSI AWB Data NG, please check FP_log. \n");
			}
		}
	}

	if (ui.xiaomi->isChecked()) {
		XiaoMi_AWB_Parse(0);
		if (XiaoMi_QC_AWB_FP_Check(XIAOMI_AWB_Data) != 0) {
			ui.log->insertPlainText("QC AWB Data NG, please check FP_log. \n");
		}
		XiaoMi_Seg_Check();
		if (ui.MTK->isChecked()) {
			MTK_AWB_Parse(0);
			if (MTK_AWB_FP_Check(MTK_AWB,1) != 0) {
				ui.log->insertPlainText("MTK AWB Data NG, please check FP_log. \n");
			}
		}
	}

	if (ui.sony->isChecked()) {
		SONY_AWB_Parse(0);
		SONY_AWB_Parse(1);
		if (SONY_AWB_FP_Check(VIVO_AWB_Data) != 0) {
			ui.log->insertPlainText("SONY AWB Data NG, please check FP_log. \n");
		}
	}

	if (ui.SAMSUNG->isChecked()) {	
		//MTK_AWB_Parse(0);
		//if (MTK_AWB_FP_Check(MTK_AWB, 1) != 0) {
		//	ui.log->insertPlainText("SAMSUNG AWB Data NG, please check FP_log. \n");
		//}	
	}
	OIS_Parse();
	AEC_Parse();

	for (int i = 0; i < 8; i++) {
		if (QC_LSC_Data[i].item[1].length() > 1) {
			e = marking_Hex2int(QC_LSC_Data[i].item[1], QC_LSC_Data[i].item[0], "", QC_LSC);
			fout << QC_LSC_Data[i].item[0] << endl;
			LSC_Parse(e,i);
		}
	}

	for (int i = 0; i < 2; i++) {
		e = unstringHex2int(MTK_LSC_Data[i].item[1]);
		if (e > 0) {
			marking_Hex2int(MTK_LSC_Data[i].item[1], MTK_LSC_Data[i].item[0], "", MTK_LSC_Type);
			MTK_LSC_Parse(e);
		}
	}

	drift_Parse();
	cross_Parse();
	PDAF_Parse();
	QSC_Parse();
	History_Date_Parser();
	bin_Area_Check();
	value_Data_Parse();
	fix_Data_Check();

	if(reserved_check==1)
		Reserved_Check();

	for (int i = 0; i < 20; i++)
		if (checkSum[i].end > 0 && checkSum[i].start > 0) {

			bool used_data = false;
			size_t Size = Map.size();
			for (size_t k = 0; k < Size; k++) {
				if (Map[k].addr == checkSum[i].start&&checkSum[i].start != checkSum[i].flag) {
					used_data = true;
					break;
				}
			}
			if (!used_data) {
				map_Push(checkSum[i].start, checkSum[i].item[0] + " data start", " ", info1);
				map_Push(checkSum[i].end, checkSum[i].item[0] + " data end", " ", package_Data);
			}
		}

	fout << endl;
	fout << "-------- Data End-------" << endl;

	DisplayOutput();
	fout.close();

	memset(checkSum_addr, 0, sizeof(checkSum_addr));
	for (size_t i = 0; i < 30; i++) {
		checkSum_addr[i][0] = checkSum[i].flag;
		checkSum_addr[i][1] = checkSum[i].start;
		checkSum_addr[i][2] = checkSum[i].end;
		checkSum_addr[i][3] = checkSum[i].checksum;
	}

	size_t Size = Map.size();
	for (size_t i = 0; i < Size; i++) {
		set_Map_inf(Map[i].info, Map[i].ref, Map[i].addr, Map[i].t);
	}
	EEPMap_Out();
	ui.log->insertPlainText("Data Parse finished. \n");
	fout.close();
}


void EEPROM_Data_Verifier::dump_Check() {

	int x = 0, ret = 0;

	if (CheckSum_Check() == 0) {
		dump_result << "Flag&Checksum PASS" << "	";
	}
	else {
		dump_result << "Flag&Checksum NG" << "	";
		ret |= 1;
	}

	if (info_Data_Parse() == 0) {
		dump_result << "info_Data PASS" << "	";
	}
	else {
		dump_result << "info_Data NG" << "	";
		ret |= 4;
	}

	if (ui.oppo->isChecked()) {
		Oppo_AWB_Parse(0);
		Oppo_AWB_Parse(1);
		Oppo_AWB_Parse(2);

		if (OPPO_QC_AWB_FP_Check(OPPO_AWB) == 0) {
			dump_result << "OPPO_AWB PASS" << "	";
		}
		else {
			dump_result << "OPPO_AWB NG" << "	";
			ret |= 8;
		}
	}

	if (ui.vivo->isChecked()) {
		VIVO_AWB_Parse(0);
		VIVO_AWB_Parse(1);

		if (VIVO_QC_AWB_FP_Check(VIVO_AWB_Data) == 0) {
			dump_result << "vivo_QC_AWB PASS" << "	";
		}
		else {
			dump_result << "vivo_QC_AWB NG" << "	";
			ret |= 8;
		}

		if (ui.MTK->isChecked()) {
			vivo_MTK_AWB_Parse(0);
			vivo_MTK_AWB_Parse(1);

			if (VIVO_QC_AWB_FP_Check(VIVO_AWB_Data) == 0) {
				dump_result << "vivo_MTK_AWB PASS" << "	";
			}
			else {
				dump_result << "vivo_MTK_AWB NG" << "	";
				ret |= 8;
			}
		}
		if (ui.LSI->isChecked()) {
			LSI_AWB_Parse(0);
			LSI_AWB_Parse(1);
			if (LSI_AWB_FP_Check(LSI_AWB, 1) != 0) {
				ui.log->insertPlainText("LSI AWB Data NG, please check FP_log. \n");
			}
		}

	}

	if (ui.xiaomi->isChecked()) {
		XiaoMi_AWB_Parse(0);

		if (XiaoMi_QC_AWB_FP_Check(XIAOMI_AWB_Data) == 0) {
			dump_result << "XiaoMi_QC_AWB PASS" << "	";
		}
		else {
			dump_result << "XiaoMi_QC_AWB NG" << "	";
			ret |= 16;
		}

		if (XiaoMi_Seg_Check() == 0) {
			dump_result << "XiaoMi_Seg PASS" << "	";
		}
		else {
			dump_result << "XiaoMi_Seg NG" << "	";
			ret |= 16;
		}

		if (ui.MTK->isChecked()) {
			MTK_AWB_Parse(0);
			if (MTK_AWB_FP_Check(MTK_AWB, 1) == 0) {
				dump_result << "MTK AWB PASS" << "	";
			}
			else {
				dump_result << "MTK AWB NG" << "	";
				ret |= 16;
			}
		}
	}

	for (int i = 0; i < 8; i++) {
		if (QC_LSC_Data[i].item[1].length() > 1) {
			int e = unstringHex2int(QC_LSC_Data[i].item[1]);
			if (LSC_Parse(e,i) == 0) {
				dump_result << "QC_LSC PASS" << "	";
			}
			else {
				dump_result << "QC_LSC NG" << "	";
				ret |= 32;
			}
		}
	}

	for (int i = 0; i < 2; i++) {
		int e = unstringHex2int(MTK_LSC_Data[i].item[1]);
		if (e > 0) {
			if (MTK_LSC_Parse(e) == 0) {
				dump_result << "MTK_LSC PASS" << "	";
			}
			else {
				dump_result << "MTK_LSC NG" << "	";
				ret |= 64;
			}
		}
	}

	if (unstringHex2int(shift_Item[0]) > 0) {
		x = drift_Parse();
		if (x == 0) {
			dump_result << "Drift_Data PASS" << "	";
		}
		else if (x > 0) {
			dump_result << "Drift_Data NG" << "	";
			ret |= 128;
		}
	}
	if (unstringHex2int(cross_Item[0]) > 0|| unstringHex2int(akm_cross_Item[0]) > 0) {
		x = cross_Parse();
		if (x == 0) {
			dump_result << "Cross_Talk PASS" << "	";
		}
		else if (x > 0) {
			dump_result << "Cross_Talk NG" << "	";
			ret |= 256;
		}
	}
	x = af_Parse();
	if (x == 0) {
		dump_result << "AF_Data PASS" << "	";
	}
	else if (x > 0) {
		dump_result << "AF_Data NG" << "	";
		ret |= 512;
	}

	if (OIS_data_Item[2][0].length() > 1) {
		x = OIS_Parse();
		if (x == 0) {
			dump_result << "OIS_Data PASS" << "	";
		}
		else if (x > 0) {
			dump_result << "OIS_Data NG" << "	";
			ret |= 1024;
		}
	}
	x = PDAF_Parse();
	if (x == 0) {
		dump_result << "PDAF_Data PASS" << "	";
	}
	else if (x > 0) {
		dump_result << "PDAF_Data NG" << "	";
		ret |= 2048;
	}

	if (reserved_check == 1) {
		x = Reserved_Check();
		if (x == 0) {
			dump_result << "Reserved_Data PASS" << "	";
		}
		else if (x > 0) {
			dump_result << "Reserved_Data NG" << "	";
			ret |= 4096;
		}
	}
	x = bin_Area_Check();
	if (x == 0) {
		dump_result << "Bin_Area_Data PASS" << "	";
	}
	else if (x > 0) {
		dump_result << "Bin_Area_Data NG" << "	";
		ret |= 8192;
	}

	x = value_Data_Parse();
	if (x == 0) {
		dump_result << "value_Data PASS" << "	";
	}
	else if (x > 0) {
		dump_result << "value_Data NG" << "	";
		ret |= 16384;
	}

	x = fix_Data_Check();
	if (x == 0) {
		dump_result << "Fix_Area PASS" << "	";
	}
	else if (x > 0) {
		dump_result << "Fix_Area NG" << "	";
		ret |= 32768;
	}

	x = duplicate_Check();
	if (x == 0) {
		dump_result << "duplicate_Check PASS" << "	";
	}
	else if (x > 0) {
		dump_result << "duplicate_Check NG_" << x << "	";
		ret |= 0x10000;
	}

	dump_Hash.push_back(current_Hash);

	////////////////// result check
	dump_result << endl;
	if (ret == 0) {
		OK++;
		ui.OK_cnt->setText(to_string(OK).c_str()); 
	}
	else {
		NG++;
		ui.NG_cnt->setText(to_string(NG).c_str());
	}
	ui.log->clear();

}


int EEPROM_Data_Verifier::value_duplicate_Check()
{
	int ret = 0;

	float duplicate_ratio = (OK + NG)*duplicate_value_NG_ratio / 100.0;

	for (int i = 0; i < 80; i++) {
		bool duplicate_result = true;
		if (value_Hash[i].item_name.length()>1) {
			for (int k = 0; k < 1009; k++) {
				if (value_Hash[i].hash[k] > duplicate_ratio) {
					duplicate_result = false;
					ret = 1;
				}
			}
		} 
		if (!duplicate_result) {
			dump_result << value_Hash[i].item_name << " value duplicate check NG!\n";
		}
	}
	dump_result << endl;

	if (ret > 0) {
		ui.NG_cnt->setText(to_string(NG+1).c_str());
		ui.log->insertPlainText("duplicate_value_check over_ratio NG!\n");
	}  

	return ret;
}


int EEPROM_Data_Verifier::fix_Data_Check()
{
	int ret = 0;
	for (int i = 0; i < 10; i++) {
		if (Fix_Data_Item[i][1].length()>1) { //各种字节长度一并检查
			unsigned int start = unstringHex2int(Fix_Data_Item[i][1]);
			unsigned int end = unstringHex2int(Fix_Data_Item[i][2]);
			for (int k = start; k <= end; k++) {
				if (DecData[k] != spec_Data[k]) {
					ret |= 1 << i;
					string s = Fix_Data_Item[i][0] + ", Fix Area in 0x" + Fix_Data_Item [i][1] + " NG!\n";
					ui.log->insertPlainText(s.c_str());
					break;
				}
			}
		}
	}

	return ret;
}


void EEPROM_Data_Verifier::on_pushButton_dump_clicked()
{
	mode = 1;
	selectModel();
	load_EEPROM_Address();

	QString filename = QFileDialog::getOpenFileName(this, tr("Open TXT"), "", tr("EEPROM File(*.txt)"));
	QTextCodec *code = QTextCodec::codecForName("gb18030");
	std::string name = code->fromUnicode(filename).data();

	ifstream in(name);

	if (!in.is_open())
	{
		QString strDisplay = "File open Failed";
		strDisplay += '\n';
		ui.log->insertPlainText(strDisplay);
		return;
	}

	dump_result.open(".\\dump_result.txt");
	fout.open(".\\MemoryParseData.txt");

	OK = 0; NG = 0;

	if (ui.full_log->isChecked())
		mode = 0;

	load_Spec(mode);

	while (getline(in, src))
	{
		int now = 0, e = 0;
		memset(DecData, 0, sizeof(DecData));
		memset(useData, 0, sizeof(useData));

		int len = src.length()+1;

		if (len > 96000)
			EEP_Size = 32768;
		else if (len > 48000)
			EEP_Size = 16384;
		else if (len > 36000)
			EEP_Size = 12288;
		else if (len > 24000)
			EEP_Size = 8192;

		if (len < 8192)
			continue;

    	while (now < len) {

			if (e < EEP_Size) {
				if ((now == 0 || src[now - 1] == ' ' || src[now - 1] == '	' || src[now - 1] == '\n') &&
					((src[now + 2] == ' '&&src[now + 5] == ' '&&src[now + 8] == ' ')
						|| (src[now + 2] == '	'&&src[now + 5] == '	'&&src[now + 8] == '	')
						|| (src[now + 2] == '\n'&&src[now + 5] == '\n'&&src[now + 8] == '\n'))) {

					for (int i = 0; i < 16; i++) {
						D[e][0] = src[now++];
						D[e][1] = src[now++];
						DecData[e] = hex2Dec(e);
						e++;
						now++;
					}
				}
				else now++;
			}
			else {
				string time_fuse = "";
				int x = 0, cnt = 0, ret = 0;
				while (cnt < 2 && x < len) {
					time_fuse += src[x];
					if (src[x] == '	') {
						cnt++;
					}
					x++;
				}
				dump_result << time_fuse << "	";
				fout << time_fuse << endl;

				x = 0;
				while (x < time_fuse.length() && time_fuse[x] != ' '&&time_fuse[x] != '	') { x++; }
				x++;
				if (x < time_fuse.length());
					current_Hash.fuse_ID = time_fuse.substr(x);

				fuse_ID_output(time_fuse);

				dump_Check();
				break;
			}
		}
		fuse_ID_output("\n");
	}
	value_duplicate_Check();
	dump_Hash.clear();
	FP_logFile_Close();
	fout.close();
	dump_result.close();

	ui.log->insertPlainText("Dump Data Read finished\n");

}


void EEPROM_Data_Verifier::on_pushButton_dump_value_clicked()
{
	selectModel();
	load_EEPROM_Address();

	QString filename = QFileDialog::getOpenFileName(this, tr("Open TXT"), "", tr("EEPROM File(*.txt)"));
	QTextCodec *code = QTextCodec::codecForName("gb18030");
	std::string name = code->fromUnicode(filename).data();

	ifstream in(name);

	if (!in.is_open())
	{
		QString strDisplay = "File open Failed";
		strDisplay += '\n';
		ui.log->insertPlainText(strDisplay);
		return;
	}

	fout.open(".\\MemoryParseData.txt");

	fout << "Time	Fuse	";

	for (int i = 0; i < 18; i++)
		if (sData_Item[i][1].length()>1 && sData_Item[i][2].length() > 2) {
			unsigned int addr = unstringHex2int(sData_Item[i][1]);
			fout << sData_Item[i][0]<<"	";
		}

	fout << endl;

	while (getline(in, src))
	{
		int now = 0, e = 0;
		memset(DecData, 0, sizeof(DecData));
		memset(useData, 0, sizeof(useData));

		int len = src.length() + 1;

		if (len > 96000)
			EEP_Size = 32768;
		else if (len > 48000)
			EEP_Size = 16384;
		else if (len > 36000)
			EEP_Size = 12288;
		else if (len > 24000)
			EEP_Size = 8192;

		if (len < 8192)
			continue;

		while (now < len) {

			if (e < EEP_Size) {
				if ((now == 0 || src[now - 1] == ' ' || src[now - 1] == '	' || src[now - 1] == '\n') &&
					((src[now + 2] == ' '&&src[now + 5] == ' '&&src[now + 8] == ' ')
						|| (src[now + 2] == '	'&&src[now + 5] == '	'&&src[now + 8] == '	')
						|| (src[now + 2] == '\n'&&src[now + 5] == '\n'&&src[now + 8] == '\n'))) {

					for (int i = 0; i < 16; i++) {
						D[e][0] = src[now++];
						D[e][1] = src[now++];
						DecData[e] = hex2Dec(e);
						e++;
						now++;
					}
				}
				else now++;
			}
			else {
				string time_fuse = "";
				int x = 0, cnt = 0, ret = 0;
				while (cnt < 2 && x < len) {
					time_fuse += src[x];
					if (src[x] == '	') {
						cnt++;
					}
					x++;
				}

				fout << time_fuse ;

				for (int i = 0; i < 18; i++)
					if (sData_Item[i][1].length()>1 && sData_Item[i][2].length()>2) {
						unsigned int addr = unstringHex2int(sData_Item[i][1]);

						int d_type = get_Data_Type(i);
						long long d = 0;
						double dd = 0;

						if (d_type == 0 || addr == 0)
							continue;

						switch (d_type) {
						case 1:
							d = DecData[addr];
							if (d > 0x7F)
								d = d - 0x100;
							break;
						case 2:
							d = DecData[addr];
							break;
						case 3:
							d = short_Out(addr, HL&1);
							break;
						case 4:
							d = short_Out(addr, HL&1);
							if (d < 0)
								d += 65536;
							break;
						case 5:
							d = int_Out(addr, HL&1);
							break;
						case 6:
							d = int_Out(addr, HL&1);
							if (d < 0)
								d += 0x100000000;
							break;
						case 7:
							dd = short_Out(addr, HL&1);
							if (dd < 0)
								dd += 0x10000;
							dd /= 0x8000;
							break;
						case 8:
							dd = int_Out(addr, HL&1);
							if (dd < 0)
								dd += 0x100000000;
							dd /= 0x80000000;
							break;
						case 9:
							dd = flt_Out(addr, HL&1);
							break;
						case 10:
							dd = dbl_Out(addr, HL&1);
							break;

						default:
							break;
						}

						if (d_type < 6) {
							fout << d << "	";
						}
						else {
							fout << dd << "	";
						}

					}
				fout << endl;
				break;
			}
		}
	}

	fout.close();
	ui.log->insertPlainText("Dump Data Read finished\n");

}


void EEPROM_Data_Verifier::on_pushButton_dump_SFR_clicked()
{
	selectModel();
	load_EEPROM_Address();

	QString filename = QFileDialog::getOpenFileName(this, tr("Open TXT"), "", tr("EEPROM File(*.txt)"));
	QTextCodec *code = QTextCodec::codecForName("gb18030");
	std::string name = code->fromUnicode(filename).data();
	ifstream in(name);

	if (!in.is_open())
	{
		QString strDisplay = "File open Failed";
		strDisplay += '\n';
		ui.log->insertPlainText(strDisplay);
		return;
	}

	int mac = unstringHex2int(AF_Item[0][1]);
	int inf = unstringHex2int(AF_Item[1][1]);

	fout.open(".\\MemoryParseData.txt");
	while (getline(in, src))
	{
		int now = 0, e = 0;
		memset(DecData, 0, sizeof(DecData));
		memset(useData, 0, sizeof(useData));

		int len = src.length() + 1;

		if (len > 96000)
			EEP_Size = 32768;
		else if (len > 48000)
			EEP_Size = 16384;
		else if (len > 36000)
			EEP_Size = 12288;
		else if (len > 24000)
			EEP_Size = 8192;

		if (len < 8192)
			continue;

		while (now < len) {

			if (e < EEP_Size) {
				if ((now == 0 || src[now - 1] == ' ' || src[now - 1] == '	' || src[now - 1] == '\n') &&
					((src[now + 2] == ' '&&src[now + 5] == ' '&&src[now + 8] == ' ')
						|| (src[now + 2] == '	'&&src[now + 5] == '	'&&src[now + 8] == '	')
						|| (src[now + 2] == '\n'&&src[now + 5] == '\n'&&src[now + 8] == '\n'))) {

					for (int i = 0; i < 16; i++) {
						D[e][0] = src[now++];
						D[e][1] = src[now++];
						DecData[e] = hex2Dec(e);
						e++;
						now++;
					}
				}
				else now++;
			}
			else {
				string time_fuse = "";
				int x = 0, cnt = 0, ret = 0;
				while (cnt < 2 && x < len) {
					time_fuse += src[x];
					if (src[x] == '	') {
						cnt++;
					}
					x++;
				}

				fout << time_fuse;
				for (int i = 0; i < 4; i++) {
					if (SFR_Item[i][2].length()>1) {
						int d = unstringHex2int(SFR_Item[i][2]);
						int cnt = atoi(SFR_Item[i][3].c_str());
						int f = unstringHex2int(SFR_Item[i][1]);

						for (int k = 0; k < cnt; k++) {
							if (SFR_Format == 0) {
								SFR_Data[k] = (int)(D[d + k][0] - '0') * 10 + (D[d + k][1] - '0');
								fout << "0." << D[d + k][0] << D[d + k][1] << "	";
							}
							else if (SFR_Format == 1) {
								SFR_Data[k] = DecData[d + k];
								fout << (float)SFR_Data[k] / 100.0 << "	";
							}
							else if (SFR_Format == 2) {
								SFR_Data[k] = DecData[d + 2 * k] * 256 + DecData[d + 2 * k + 1];
								fout << (float)SFR_Data[k] / 10000.0 << "	";
							}
							else if (SFR_Format == 3) {
								SFR_Data[k] = DecData[d + 2 * k] * 256 + DecData[d + 2 * k + 1];
								fout << SFR_Data[k] << "	";
							}
							else if (SFR_Format == 4) {
								if ((HL&1) == 0)
									SFR_Data[k] = DecData[d + 2 * k] + DecData[d + 2 * k + 1] * 256;
								else SFR_Data[k] = DecData[d + 2 * k] * 256 + DecData[d + 2 * k + 1] * 256;
								fout << SFR_Data[k] << "	";
							}
						}				
					}
				}
				fout << DecData[inf] * 256 + DecData[inf + 1]<< "	"<<DecData[mac] * 256 + DecData[mac + 1] << "	";
				fout << endl;
				break;
			}
		}
	}

	fout.close();
	ui.log->insertPlainText("Dump Data Read finished\n");

}


vector<string> getFiles(string cate_dir)
{
	vector<string> files;//存放文件名
	_finddata_t file;
	long lf = 0;
	//输入文件夹路径
	if ((lf = _findfirst(cate_dir.c_str(), &file)) == -1) {
		cout << cate_dir << " not found!!!" << endl;
	}
	else {
		while (_findnext(lf, &file) == 0) {
			//输出文件名
			//cout<<file.name<<endl;
			if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
				continue;
			files.push_back(file.name);
		}
	}
	_findclose(lf);

	//排序，按从小到大排序
//	sort(files.begin(), files.end());
	return files;
}


void EEPROM_Data_Verifier::on_pushButton_folder_clicked() {

	mode = 1;
	selectModel();
	load_EEPROM_Address();

	dump_result.open(".\\dump_result.txt");
	fout.open(".\\MemoryParseData.txt");

	OK = 0; NG = 0;

	if (ui.full_log->isChecked())
		mode = 0;

	load_Spec(mode);

	vector<string> files1 = getFiles(".\\bin_data\\*");
	vector<string> ::iterator iVector = files1.begin();
	bool ok = false;
	while (iVector != files1.end())
	{
		dump_result << (*iVector) << "	";
		fout << (*iVector) << endl;
		fuse_ID_output(*iVector);

		string fuse = "", time_fuse = (*iVector);
		int x = 0;

		if (time_fuse[0] == '2'&&time_fuse[1] == '0'&&time_fuse[2] == '2') {
			while (time_fuse[x] != ' '&&time_fuse[x] != '_'&&x < (*iVector).length()) {
				x++;
			}
			x++;
			while (time_fuse[x] != ' '&&time_fuse[x] != '_'&&x < (*iVector).length()) {
				fuse += time_fuse[x++];
			}
		}
		else {
			while (time_fuse[x] != ' '&&time_fuse[x] != '_'&&x < (*iVector).length()) {
				fuse += time_fuse[x++];
			}
		}
		current_Hash.fuse_ID = fuse;
		memset(DecData, 0, sizeof(DecData));
		memset(useData, 0, sizeof(useData));
		name = ".\\bin_data\\"+ *iVector;

		ifstream fin(name, std::ios::binary);

		//获取文件大小方法一
		struct _stat info;
		_stat(name.c_str(), &info);
		EEP_Size = info.st_size;

		unsigned char szBuf[262128] = { 0 };
		fin.read((char*)&szBuf, sizeof(char) * EEP_Size);

		for (int i = 0; i < EEP_Size; i++) {
			DecData[i] = (unsigned char)szBuf[i];
			getHex(DecData[i]);
			D[i][0] = chk[0];
			D[i][1] = chk[1];
		}

		dump_Check();
		fin.close();
		++iVector;
		fuse_ID_output("\n");
	}
	
	value_duplicate_Check();
	dump_Hash.clear();
	FP_logFile_Close();
	fout.close();
	dump_result.close();
	ui.log->insertPlainText("Dump Data Read finished\n");

}


void EEPROM_Data_Verifier::on_pushButton_folder_sorting_clicked() {

	mode = 1;
	selectModel();
	load_EEPROM_Address();

	dump_result.open(".\\dump_result.txt");
	fout.open(".\\MemoryParseData.txt");

	OK = 0; NG = 0;

	if (ui.full_log->isChecked())
		mode = 0;

	load_Spec(mode);

	vector<string> files1 = getFiles(".\\bin_data\\*");
	vector<string> ::iterator iVector = files1.begin();
	bool ok = false;
	while (iVector != files1.end())
	{
		dump_result << (*iVector) << "	";
		fout << (*iVector) << endl;
		fuse_ID_output(*iVector);

		string fuse = "", time_fuse = (*iVector);
		int x = 0;

		if (time_fuse[0] == '2'&&time_fuse[1] == '0'&&time_fuse[2] == '2') {
			while (time_fuse[x] != ' '&&time_fuse[x] != '_'&&x < (*iVector).length()) {
				x++;
			}
			x++;
			while (time_fuse[x] != ' '&&time_fuse[x] != '_'&&x < (*iVector).length()) {
				fuse += time_fuse[x++];
			}
		}
		else {
			while (time_fuse[x] != ' '&&time_fuse[x] != '_'&&x < (*iVector).length()) {
				fuse += time_fuse[x++];
			}
		}
		current_Hash.fuse_ID = fuse;
		memset(DecData, 0, sizeof(DecData));
		memset(useData, 0, sizeof(useData));
		name = ".\\bin_data\\" + *iVector;

		ifstream fin(name, std::ios::binary);

		//获取文件大小方法一
		struct _stat info;
		_stat(name.c_str(), &info);
		EEP_Size = info.st_size;

		unsigned char szBuf[262128] = { 0 };
		fin.read((char*)&szBuf, sizeof(char) * EEP_Size);

		for (int i = 0; i < EEP_Size; i++) {
			DecData[i] = (unsigned char)szBuf[i];
			getHex(DecData[i]);
		}

		dump_Check();

		fin.close();
		++iVector;
		fuse_ID_output("\n");
	}

	value_duplicate_Check();
	dump_Hash.clear();
	FP_logFile_Close();
	fout.close();
	dump_result.close();
	ui.log->insertPlainText("Dump Data Read finished\n");

}
