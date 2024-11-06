
#include "FP_Check.h"
#include<fstream>   
#include <afx.h>
#include<math.h>

using namespace std;

int  LSC_MINMAX_CORNER_ONOFF = 1, LSC_MINMAX_ERRCOUNT = 1, LSC_MINMAX_ERRCOUNT_SPEC = 30;
int LSC_H_TABLE[8], LSC_V_TABLE[6], LSC_H_TABLE_RE[8], LSC_V_TABLE_RE[6];
float GainMap_Diff = 0.3, GainMap_LR_Diff = 0.3, GainMap_TB_Diff = 0.3, DCC_Diff=0.4;
float MTK_Proc1_Diff = 0.3, MTK_Proc1_LR_Diff = 0.3, MTK_Proc1_TB_Diff = 0.3, MTK_Proc2_Diff = 0.4;
float MTK_LSC_H_TABLE[7], MTK_LSC_V_TABLE[7], MTK_LSC_H_TABLE_RE[7], MTK_LSC_V_TABLE_RE[7];
float LSC_COR_USL = 0.9, LSC_COR_LSL = 0.1, LSC_MINMAX_SPEC[4], LSC_MINMAX_CORNER[3],MTK_LSC_MINMAX_SPEC[4], MTK_LSC_MINMAX_CORNER[3];
float golden_Spec[2][3] = {0};
unsigned int awb_distance[3], awb_tolerance[3][3];
int QSC_Min, QSC_Max, LSC_Sequence_check=1;
int log_type = 1, awb_golden_check=1;
string INI_Path ;
//ofstream FP_log(".\\FP_Check_Log.txt");
ofstream FP_log;


void set_ini_Path(const std::string& str) {
	INI_Path = str;
}


void fuse_ID_output(const std::string& str) {
	FP_log << str << "	";
}


void FP_logFile_Close() {
	FP_log.close();
}

typedef struct {
	int range_min = 0, range_max = 1020, POS1_diff=0, POS2_diff = 0,Mac_diff=0,Inf_diff=0;
	float mid1_coef = 0, mid2_coef = 0;
}AF_Code_Spec_Type;
AF_Code_Spec_Type af_Code_Spec;

void load_Spec(int x) {

	FP_log.open(".\\FP_Check_Log.txt");
	string color[3] = { "5100","4000","3100" };
	log_type = x;

	TCHAR lpTexts[30]; int temp = 0; string str;
	USES_CONVERSION;
	for (int i = 0; i < 8; i++) {
		string item = "LSC_H_TABLE_" + to_string(i);
		LSC_H_TABLE[i] = GetPrivateProfileInt(_T("Spec_Set"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "LSC_H_TABLE_" + to_string(i) + "_RE";
		LSC_H_TABLE_RE[i] = GetPrivateProfileInt(_T("Spec_Set"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
	}
	for (int i = 0; i < 6; i++) {
		string item = "LSC_V_TABLE_" + to_string(i);
		LSC_V_TABLE[i] = GetPrivateProfileInt(_T("Spec_Set"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "LSC_V_TABLE_" + to_string(i) + "_RE";
		LSC_V_TABLE_RE[i] = GetPrivateProfileInt(_T("Spec_Set"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
	}

	for (int i = 0; i < 7; i++) {
		string item = "MTK_LSC_H_TABLE_" + to_string(i);
		GetPrivateProfileString(TEXT("Spec_Set"), CA2CT(item.c_str()), TEXT("0"), lpTexts, 8, CA2CT(INI_Path.c_str()));
		MTK_LSC_H_TABLE[i] = atof(CT2A(lpTexts));

		item = "MTK_LSC_H_TABLE_" + to_string(i) + "_RE";
		GetPrivateProfileString(TEXT("Spec_Set"), CA2CT(item.c_str()), TEXT("0"), lpTexts, 8, CA2CT(INI_Path.c_str()));
		MTK_LSC_H_TABLE_RE[i] = atof(CT2A(lpTexts));

		item = "MTK_LSC_V_TABLE_" + to_string(i);
		GetPrivateProfileString(TEXT("Spec_Set"), CA2CT(item.c_str()), TEXT("0"), lpTexts, 8, CA2CT(INI_Path.c_str()));
		MTK_LSC_V_TABLE[i] = atof(CT2A(lpTexts));

		item = "MTK_LSC_V_TABLE_" + to_string(i) + "_RE";
		GetPrivateProfileString(TEXT("Spec_Set"), CA2CT(item.c_str()), TEXT("0"), lpTexts, 8, CA2CT(INI_Path.c_str()));
		MTK_LSC_V_TABLE_RE[i] = atof(CT2A(lpTexts));
	}


	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("LSC_COR_USL"), TEXT(""), lpTexts, 8, CA2CT(INI_Path.c_str()));
	LSC_COR_USL = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("LSC_COR_LSL"), TEXT(""), lpTexts, 8, CA2CT(INI_Path.c_str()));
	LSC_COR_LSL = atof(CT2A(lpTexts));

	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("LSC_MINMAX_SPEC_R"), TEXT(""), lpTexts, 8, CA2CT(INI_Path.c_str()));
	LSC_MINMAX_SPEC[0] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("LSC_MINMAX_SPEC_GR"), TEXT(""), lpTexts, 8, CA2CT(INI_Path.c_str()));
	LSC_MINMAX_SPEC[1] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("LSC_MINMAX_SPEC_GB"), TEXT(""), lpTexts, 8, CA2CT(INI_Path.c_str()));
	LSC_MINMAX_SPEC[2] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("LSC_MINMAX_SPEC_B"), TEXT(""), lpTexts, 8, CA2CT(INI_Path.c_str()));
	LSC_MINMAX_SPEC[3] = atof(CT2A(lpTexts));

	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("LSC_MINMAX_CORNER_ONOFF"), TEXT("0"), lpTexts, 8, CA2CT(INI_Path.c_str()));
	LSC_MINMAX_CORNER[0] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("LSC_MINMAX_ERRCOUNT"), TEXT("1"), lpTexts, 8, CA2CT(INI_Path.c_str()));
	LSC_MINMAX_CORNER[1] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("LSC_MINMAX_ERRCOUNT_SPEC"), TEXT("26"), lpTexts, 8, CA2CT(INI_Path.c_str()));
	LSC_MINMAX_CORNER[2] = atof(CT2A(lpTexts)); 

	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("MTK_LSC_MINMAX_SPEC_R"), TEXT(""), lpTexts, 8, CA2CT(INI_Path.c_str()));
	MTK_LSC_MINMAX_SPEC[0] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("MTK_LSC_MINMAX_SPEC_GR"), TEXT(""), lpTexts, 8, CA2CT(INI_Path.c_str()));
	MTK_LSC_MINMAX_SPEC[1] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("MTK_LSC_MINMAX_SPEC_GB"), TEXT(""), lpTexts, 8, CA2CT(INI_Path.c_str()));
	MTK_LSC_MINMAX_SPEC[2] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("MTK_LSC_MINMAX_SPEC_B"), TEXT(""), lpTexts, 8, CA2CT(INI_Path.c_str()));
	MTK_LSC_MINMAX_SPEC[3] = atof(CT2A(lpTexts));

	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("MTK_LSC_MINMAX_CORNER_ONOFF"), TEXT("0"), lpTexts, 8, CA2CT(INI_Path.c_str()));
	MTK_LSC_MINMAX_CORNER[0] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("MTK_LSC_MINMAX_ERRCOUNT"), TEXT("1"), lpTexts, 8, CA2CT(INI_Path.c_str()));
	MTK_LSC_MINMAX_CORNER[1] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("MTK_LSC_MINMAX_ERRCOUNT_SPEC"), TEXT("26"), lpTexts, 8, CA2CT(INI_Path.c_str()));
	MTK_LSC_MINMAX_CORNER[2] = atof(CT2A(lpTexts));

	GetPrivateProfileString(TEXT("SONY"), TEXT("golden_rg_High_Color"), TEXT("0"), lpTexts, 10, CA2CT(INI_Path.c_str()));
	
	golden_Spec[0][0] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("SONY"), TEXT("golden_bg_High_Color"), TEXT("0"), lpTexts, 10, CA2CT(INI_Path.c_str()));
	golden_Spec[0][1] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("SONY"), TEXT("golden_gbgr_High_Color"), TEXT("0"), lpTexts, 10, CA2CT(INI_Path.c_str()));
	golden_Spec[0][2] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("SONY"), TEXT("golden_rg_Low_Color"), TEXT("0"), lpTexts, 10, CA2CT(INI_Path.c_str()));
	golden_Spec[1][0] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("SONY"), TEXT("golden_bg_Low_Color"), TEXT("0"), lpTexts, 10, CA2CT(INI_Path.c_str()));
	golden_Spec[1][1] = atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("SONY"), TEXT("golden_gbgr_Low_Color"), TEXT("0"), lpTexts, 10, CA2CT(INI_Path.c_str()));
	golden_Spec[1][2] = atof(CT2A(lpTexts));

	QSC_Min= GetPrivateProfileInt(_T("Spec_Set"), TEXT("QSC_Min"), 920, CA2CT(INI_Path.c_str()));
	QSC_Max = GetPrivateProfileInt(_T("Spec_Set"), TEXT("QSC_Max"), 1080, CA2CT(INI_Path.c_str()));

	GainMap_Diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("GainMap_Diff"), 25, CA2CT(INI_Path.c_str()));
	GainMap_Diff = GainMap_Diff / 100;
	GainMap_LR_Diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("GainMap_LR_Diff"),20, CA2CT(INI_Path.c_str()));
	GainMap_LR_Diff = GainMap_LR_Diff / 100;
	GainMap_TB_Diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("GainMap_TB_Diff"), 30, CA2CT(INI_Path.c_str()));
	GainMap_TB_Diff = GainMap_TB_Diff / 100;
	DCC_Diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("DCC_Diff"), 40, CA2CT(INI_Path.c_str()));
	DCC_Diff = DCC_Diff / 100;

	MTK_Proc1_Diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("MTK_Proc1_Diff"), 50, CA2CT(INI_Path.c_str()));
	MTK_Proc1_Diff = MTK_Proc1_Diff / 100;
	MTK_Proc1_LR_Diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("MTK_Proc1_LR_Diff"), 30, CA2CT(INI_Path.c_str()));
	MTK_Proc1_LR_Diff = MTK_Proc1_LR_Diff / 100;
	MTK_Proc1_TB_Diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("MTK_Proc1_TB_Diff"), 30, CA2CT(INI_Path.c_str()));
	MTK_Proc1_TB_Diff = MTK_Proc1_TB_Diff / 100;
	MTK_Proc2_Diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("MTK_Proc2_Diff"), 50, CA2CT(INI_Path.c_str()));
	MTK_Proc2_Diff = MTK_Proc2_Diff / 100;

	awb_golden_check = GetPrivateProfileInt(_T("EEPROM_Set"), TEXT("awb_golden_check"), 1, CA2CT(INI_Path.c_str()));
	LSC_Sequence_check = GetPrivateProfileInt(_T("EEPROM_Set"), TEXT("LSC_Sequence_check"), 1, CA2CT(INI_Path.c_str()));

	for (int i = 0; i < 3; i++) {
		string item = "awb_distance_" + color[i];
		awb_distance[i] = GetPrivateProfileInt(_T("Spec_Set"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
	}

	for (int i = 0; i < 3; i++) {
		string item = "awb_tolerance_" + color[i] + "_rg";
		awb_tolerance[i][0] = GetPrivateProfileInt(_T("Spec_Set"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "awb_tolerance_" + color[i] + "_bg";
		awb_tolerance[i][1] = GetPrivateProfileInt(_T("Spec_Set"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "awb_tolerance_" + color[i] + "_grgb";
		awb_tolerance[i][2] = GetPrivateProfileInt(_T("Spec_Set"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
	}

	af_Code_Spec.range_min = GetPrivateProfileInt(_T("Spec_Set"), TEXT("VCA_GAP_INF2MAC2"), 100, CA2CT(INI_Path.c_str()));
	af_Code_Spec.range_max = GetPrivateProfileInt(_T("Spec_Set"), TEXT("VCA_GAP_INF2MAC2_H"), 1000, CA2CT(INI_Path.c_str()));
	af_Code_Spec.POS1_diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("POS1_Diff_Spec"), 0, CA2CT(INI_Path.c_str()));
	af_Code_Spec.POS2_diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("POS2_Diff_Spec"), 0, CA2CT(INI_Path.c_str()));
	af_Code_Spec.Mac_diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("Mac_twice_Diff"), 0, CA2CT(INI_Path.c_str()));
	af_Code_Spec.Inf_diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("Inf_twice_Diff"), 0, CA2CT(INI_Path.c_str()));

	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("POS1_LENS_SHIFT"), TEXT("0"), lpTexts, 64, CA2CT(INI_Path.c_str()));
	str = CT2A(lpTexts);
	af_Code_Spec.mid1_coef = atof(str.c_str());
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("POS2_LENS_SHIFT"), TEXT("0"), lpTexts, 64, CA2CT(INI_Path.c_str()));
	str = CT2A(lpTexts);
	af_Code_Spec.mid2_coef = atof(str.c_str());

}


int QC_LSC_FP_Check(int LSC[25][33][4],int type) {
	int ret = 0;
	string color[4] = { "LSC Table1 ","LSC Table2 ","LSC Table3 ","LSC Table4 " };

	if (LSC[6][8][0] == 0xFFFF || LSC[6][8][0] == 0) {
		ret |= 1;
	}
	int W = 17, H = 13, T = 4, C = 1024;

	if (type<2) {
		///////////////  H/V Diff Check
		for (int k = 0; k < 4; k++)
			for (int i = 0; i < 13; i++)
				for (int j = 0; j < 17; j++) {
					if (j < 8) {
						if (LSC[i][j + 1][k] - LSC[i][j][k]>LSC_H_TABLE[j] || LSC[i][j][k] - LSC[i][j + 1][k]>LSC_H_TABLE_RE[j]) {
							ret = ret | 2;
							FP_log << color[k] << "_H_[" << i << "," << j << "]" << "[" << i << "," << j + 1 << "]" << " Diff NG!" << endl;
						}
					}
					else if (j > 8) {
						if (LSC[i][j - 1][k] - LSC[i][j][k]>LSC_H_TABLE[16 - j] || LSC[i][j][k] - LSC[i][j - 1][k]>LSC_H_TABLE_RE[16 - j]) {
							ret = ret | 2;
							FP_log << color[k] << "_H_[" << i << "," << j + 1 << "]" << "[" << i << "," << j << "]" << " Diff NG!" << endl;
						}
					}
				}

		for (int k = 0; k < 4; k++)
			for (int j = 0; j < 17; j++)
				for (int i = 0; i < 13; i++) {
					if (i < 6) {
						if (LSC[i + 1][j][k] - LSC[i][j][k]>LSC_V_TABLE[i] || LSC[i][j][k] - LSC[i + 1][j][k]>LSC_V_TABLE_RE[i]) {
							ret = ret | 2;
							FP_log << color[k] << "_V_[" << i << "," << j << "]" << "[" << i + 1 << "," << j << "]" << " Diff NG!" << endl;
						}
					}
					else if (i > 6) {
						if (LSC[i - 1][j][k] - LSC[i][j][k]>LSC_V_TABLE[12 - i] || LSC[i][j][k] - LSC[i - 1][j][k] > LSC_V_TABLE_RE[12 - i]) {
							ret = ret | 2;
							FP_log << color[k] << "_V_[" << i + 1 << "," << j << "]" << "[" << i << "," << j << "]" << " Diff NG!" << endl;
						}
					}
				}

		if ((ret & 2) > 0 || log_type == 0) {

			for (int k = 0; k < 4; k++) {
				for (int i = 0; i < 13; i++) {
					for (int j = 0; j < 16; j++) {
						FP_log << LSC[i][j][k] - LSC[i][j + 1][k] << "	";
					}
					FP_log << endl;
				}
				FP_log << endl;
			}

			for (int k = 0; k < 4; k++) {
				for (int j = 0; j < 17; j++) {
					for (int i = 0; i < 12; i++) {
						FP_log << LSC[i][j][k] - LSC[i + 1][j][k] << "	";
					}
					FP_log << endl;
				}
				FP_log << endl;
			}
		}

		///////////////  R & B Swap check 
		int xx = 0;
		if (LSC[0][0][0] * LSC[0][0][2] * 0.95 > LSC[0][0][3] * LSC[0][0][1])
			xx++;
		if (LSC[0][16][0] * LSC[0][16][2] * 0.95 > LSC[0][16][3] * LSC[0][16][1])
			xx++;
		if (LSC[12][0][0] * LSC[12][0][2] * 0.95 > LSC[12][0][3] * LSC[12][0][1])
			xx++;
		if (LSC[12][16][0] * LSC[12][16][2] * 0.95 > LSC[12][16][3] * LSC[12][16][1])
			xx++;

		if (xx > 2 && awb_golden_check == 1) {
			ret |= 4;
			FP_log << " QC LSC R Gr Gb B Sequence NG!" << endl;
		}
	}
	else if (type == 2) {
		W = 13, H = 9, T = 3,C=128;
		for (int i = 1; i < H; i++)
			for (int j = 1; j < W; j++) {
				if (LSC[i][j][3] < 123 || LSC[i][j][3]>133) {
 					ret = ret | 16;
					break;
				}
			}

		if ((ret&16) > 0) {
			FP_log << "LSC Gb/Gr Diff check NG!" << endl;
		}
	}
	else if (type == 4) {
		W = 33, H = 25, C = 4095;

	}
	else if (type == 5) {
		C = 4095;

	}


	///////////////  LSC balance Check
	float LSC_AVG[25][33][4] = { 0 };
	for (int k = 0; k < T; k++)
		for (int i = 1; i < H-1; i++)
			for (int j = 1; j < W-1; j++) {

				float sum = 0;
				for (int a = -1; a < 2; a++)
					for (int b = -1; b < 2; b++) {
						if (a != 0 || b != 0)
							sum += LSC[i + a][j + b][k];
					}
				LSC_AVG[i][j][k] = sum / 8 - LSC[i][j][k];
			}

	float LSC_MAX_DIFF[6][8][4];
	int ERRCOUNT = 0, cnt = 0, except_cnt = 0;
	for (int k = 0; k < T; k++)
		for (int i = 1; i < H/2; i++)
			for (int j = 1; j < W / 2; j++) {
				float max = -70000, min = 70000;

				if (LSC_AVG[i][j][k] < min)
					min = LSC_AVG[i][j][k];
				if (LSC_AVG[i][j][k] > max)
					max = LSC_AVG[i][j][k];
				if (LSC_AVG[i][W - 1 - j][k] < min)
					min = LSC_AVG[i][W - 1 - j][k];
				if (LSC_AVG[i][W - 1 - j][k] > max)
					max = LSC_AVG[i][W - 1 - j][k];
				if (LSC_AVG[H - 1 - i][j][k] < min)
					min = LSC_AVG[H - 1 - i][j][k];
				if (LSC_AVG[H - 1 - i][j][k] > max)
					max = LSC_AVG[H - 1 - i][j][k];
				if (LSC_AVG[H - 1 - i][W - 1 - j][k] < min)
					min = LSC_AVG[H - 1 - i][W - 1 - j][k];
				if (LSC_AVG[H - 1 - i][W - 1 - j][k] > max)
					max = LSC_AVG[H - 1 - i][W - 1 - j][k];

				LSC_MAX_DIFF[i][j][k] = max - min;
				if (LSC_MINMAX_CORNER[0] > 0){
					if (LSC_MAX_DIFF[i][j][k] >LSC_MINMAX_CORNER[2] * C / 1024.0) {
						ret = ret | 8;
						FP_log << color[k] << " [" << i << "," << j << "]" << "LSC block NG!" << endl;
					}
					else {
						if (LSC_MAX_DIFF[i][j][k] > LSC_MINMAX_SPEC[k] * C / 1024.0) {
							except_cnt++;
						}
					}
					if (except_cnt > LSC_MINMAX_CORNER[1]) {
						ret = ret | 8;
						FP_log << color[k] << " [" << i << "," << j << "]" << "LSC block NG!" << endl;		
					}
				}
				else {
					if (LSC_MAX_DIFF[i][j][k] > LSC_MINMAX_SPEC[k] * C / 1024.0) {
						cnt++;
					}
					if(cnt>1)
						ret = ret | 8;
				}
			}

	if ((ret & 8) > 0 || log_type == 0) {
		FP_log << "LSC balance check log:	" << endl;
		for (int k = 0; k < T; k++) {
			for (int i = 1; i < H/2; i++) {
				for (int j = 1; j < W/2; j++) {
					FP_log << LSC_MAX_DIFF[i][j][k] << "	";
				}
				FP_log << endl;
			}
			FP_log << endl;
		}
	}

	return ret;

}


int MTK_LSC_FP_Check(int LSC[15][15][4], int first_pixel) {
	int ret = 0;
	string color[4] = { "CH1","CH2","CH3","CH4" };

	if (LSC[7][7][0] == 0xFFFF || LSC[7][7][0] == 0) {
		ret |= 1;
	}

	//	FP_log << "----------- MTK LSC FP Check Result----------" << endl;
	float LSC_AVG[15][15][4] = { 0 }, MTK_LSC[15][15][4] = { 0 };

	for (int k = 0; k < 4; k++)
		for (int i = 0; i < 15; i++)
			for (int j = 0; j < 15; j++) {
				MTK_LSC[i][j][k] = LSC[i][j][k] / 8192.0;
			}

	///////////////  R & B Swap check 
	int a = 0, b = 1, c = 2, d = 3;
	if (first_pixel == 1) {
		a = 1; b = 0; c = 3; d = 2;
	}
	else if (first_pixel == 2) {
		a = 2; b = 3; c = 0; d = 1;
	}
	else if (first_pixel == 3) {
		a = 3; b = 2; c = 1; d = 0;
	}
	int xx = 0;
	if (LSC[0][0][a] * LSC[0][0][c] < LSC[0][0][d] * LSC[0][0][b])
		xx++;
	if (LSC[0][14][a] * LSC[0][14][c] < LSC[0][14][d] * LSC[0][14][b])
		xx++;
	if (LSC[14][0][a] * LSC[12][0][c] < LSC[12][0][d] * LSC[12][0][b])
		xx++;
	if (LSC[14][14][a] * LSC[12][16][c] < LSC[12][16][d] * LSC[12][16][b])
		xx++;

	if (xx > 3 && awb_golden_check == 1) {
		ret |= 4;
		FP_log << " MTK LSC R Gr Gb B Sequence NG!" << endl;
	}

	///////////////  H/V Diff Check
	for (int k = 0; k < 4; k++) {
		for (int i = 1; i < 14; i++) {
			for (int j = 1; j < 14; j++) {
				if (j > 7) {
					if (MTK_LSC[i][j + 1][k] - MTK_LSC[i][j][k]>MTK_LSC_H_TABLE[13 - j] || MTK_LSC[i][j][k] - MTK_LSC[i][j + 1][k]>MTK_LSC_H_TABLE_RE[13 - j]) {
						ret = ret | 4;
						FP_log << color[k] << "_H_[" << i << "," << j << "]" << "[" << i << "," << j + 1 << "]" << " Diff NG!" << endl;
					}
				}
				else if (j < 7) {
					if (MTK_LSC[i][j - 1][k] - MTK_LSC[i][j][k]>MTK_LSC_H_TABLE[j-1] || MTK_LSC[i][j][k] - MTK_LSC[i][j - 1][k]>MTK_LSC_H_TABLE_RE[j-1]) {
						ret = ret | 4;
						FP_log << color[k] << "_H_[" << i << "," << j + 1 << "]" << "[" << i << "," << j << "]" << " Diff NG!" << endl;
					}
				}
			}
		}
	}
	for (int k = 0; k < 4; k++){
		for (int j = 1; j < 14; j++) {
			for (int i = 1; i < 14; i++) {
				if (i > 7) {
					if (MTK_LSC[i + 1][j][k] - MTK_LSC[i][j][k]>MTK_LSC_V_TABLE[13 - i] || MTK_LSC[i][j][k] - MTK_LSC[i + 1][j][k]>MTK_LSC_V_TABLE_RE[13 - i]) {
						ret = ret | 4;
						FP_log << color[k] << "_V_[" << i << "," << j << "]" << "[" << i + 1 << "," << j << "]" << " Diff NG!" << endl;
					}
				}
				else if (i < 7) {
					if (MTK_LSC[i - 1][j][k] - MTK_LSC[i][j][k]>MTK_LSC_V_TABLE[i-1] || MTK_LSC[i][j][k] - MTK_LSC[i - 1][j][k] > MTK_LSC_V_TABLE_RE[i-1]) {
						ret = ret | 4;
						FP_log << color[k] << "_V_[" << i + 1 << "," << j << "]" << "[" << i << "," << j << "]" << " Diff NG!" << endl;
					}
				}
			}
		}
	}

	if ((ret & 4) >0|| log_type == 0) {

		for (int k = 0; k < 4; k++) {
			for (int i = 0; i < 15; i++) {
				for (int j = 0; j < 14; j++) {
					FP_log << MTK_LSC[i][j][k] - MTK_LSC[i][j + 1][k] << "	";
				}
				FP_log << endl;
			}
			FP_log << endl;
		}

		for (int k = 0; k < 4; k++) {
			for (int j = 0; j < 15; j++) {
				for (int i = 0; i < 14; i++) {
					FP_log << MTK_LSC[i][j][k] - MTK_LSC[i + 1][j][k] << "	";
				}
				FP_log << endl;
			}
			FP_log << endl;
		}
	}

	///////////////  LSC balance Check
	for (int k = 0; k < 4; k++)
		for (int i = 1; i < 14; i++)
			for (int j = 1; j < 14; j++) {
				float sum = 0;
				for (int a = -1; a < 2; a++)
					for (int b = -1; b < 2; b++) {
						if (a != 0 || b != 0)
							sum += MTK_LSC[i + a][j + b][k];
					}
				LSC_AVG[i][j][k] = abs(sum / 8 - MTK_LSC[i][j][k]);
			}

	float LSC_MAX_DIFF[7][7][4];
	int cnt = 0, except_cnt=0;
	for (int k = 0; k < 4; k++)
		for (int i = 1; i < 7; i++)
			for (int j = 1; j < 7; j++) {
				float max = 0, min = 1024;

				if (LSC_AVG[i][j][k] < min)
					min = LSC_AVG[i][j][k];
				if (LSC_AVG[i][j][k] > max)
					max = LSC_AVG[i][j][k];
				if (LSC_AVG[i][13 - j][k] < min)
					min = LSC_AVG[i][13 - j][k];
				if (LSC_AVG[i][13 - j][k] > max)
					max = LSC_AVG[i][13 - j][k];
				if (LSC_AVG[13 - i][j][k] < min)
					min = LSC_AVG[13 - i][j][k];
				if (LSC_AVG[13 - i][j][k] > max)
					max = LSC_AVG[13 - i][j][k];
				if (LSC_AVG[13 - i][13 - j][k] < min)
					min = LSC_AVG[13 - i][13 - j][k];
				if (LSC_AVG[13 - i][13 - j][k] > max)
					max = LSC_AVG[13 - i][13 - j][k];

				LSC_MAX_DIFF[i][j][k] = max - min;

				//if (LSC_MAX_DIFF[i][j][k] > MTK_LSC_MINMAX_SPEC[k]) {
 			//		ret = ret | 8;
				//	FP_log << color[k] << " [" << i << "," << j << "]" << " MTK_LSC block NG!" << endl;
				//}

				if (MTK_LSC_MINMAX_CORNER[0] > 0) {
					if (LSC_MAX_DIFF[i][j][k] > MTK_LSC_MINMAX_CORNER[2]) {
						ret = ret | 8;
						FP_log << color[k] << " [" << i << "," << j << "]" << "MTK_LSC block NG!" << endl;
					}
					else {
						if (LSC_MAX_DIFF[i][j][k] >  MTK_LSC_MINMAX_SPEC[k] ) {
							except_cnt++;
						}
					}
					if (except_cnt >  MTK_LSC_MINMAX_CORNER[1]) {
						ret = ret | 8;
						FP_log << color[k] << " [" << i << "," << j << "]" << "MTK_LSC block NG!" << endl;
					}
				}
				else {
					if (LSC_MAX_DIFF[i][j][k] >  MTK_LSC_MINMAX_SPEC[k]) {
						cnt++;
					}
					if (cnt>1)
						ret = ret | 8;
				}
			}

	if ((ret & 8) > 0||log_type==0) {
	
		for (int k = 0; k < 4; k++) {
			for (int i = 1; i < 7; i++) {
				for (int j = 1; j < 7; j++) {
					FP_log << LSC_MAX_DIFF[i][j][k] << "	";
				}
				FP_log << endl;
			}
			FP_log << endl;
		}
	}
	return ret;
}


int OPPO_QC_AWB_FP_Check(oppo_AWB_Format OPPO_AWB[3]) {

	int ret = 0;
	string color[3] = { "5100k","4000k","3100k" };

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 4; j++)
			if(OPPO_AWB[i].AWB[j]!=0) {
			if (OPPO_AWB[i].AWB[j]>1100 || OPPO_AWB[i].AWB[j] < 100)
				return 4096;
			if (OPPO_AWB[i].Golden[j]>1100 || OPPO_AWB[i].Golden[j] < 100)
				return 4096;
		}

	/////////////awb_distance check
	for (int i = 0; i < 3; i++) {

		if (OPPO_AWB[i].AWB[1] == 0 || OPPO_AWB[i].AWB[2] == 0 || OPPO_AWB[i].Golden[1] == 0 || OPPO_AWB[i].Golden[2] == 0)
			continue;

		float R_Gr = (float)OPPO_AWB[i].AWB[0] / OPPO_AWB[i].AWB[1];
		float B_Gb = (float)OPPO_AWB[i].AWB[3] / OPPO_AWB[i].AWB[1];

		float Golden_R_Gr = (float)OPPO_AWB[i].Golden[0] / OPPO_AWB[i].Golden[1];
		float Golden_B_Gr = (float)OPPO_AWB[i].Golden[3] / OPPO_AWB[i].Golden[1];

		float d = sqrt(pow(R_Gr - Golden_R_Gr, 2) + pow(B_Gb - Golden_B_Gr, 2));
		if(log_type==0)FP_log << color[i] << "QC awb_distance=: " << d << endl;

		if (d > awb_distance[i] / 100.0) {
			ret = ret | 1;
		}
	}

	if ((ret&1) > 0) {
		FP_log << "QC awb_distance spec check NG!" << endl;
	}


	for (int i = 0; i < 3; i++) {

		if (OPPO_AWB[i].AWB[1] == 0 || OPPO_AWB[i].AWB[2] == 0 || OPPO_AWB[i].AWB[0] == 0 || OPPO_AWB[i].Golden[2] == 0)
			continue;

		float R_G = (float)OPPO_AWB[i].AWB[0] * 2 / (OPPO_AWB[i].AWB[1] + OPPO_AWB[i].AWB[2]);
		float B_G = (float)OPPO_AWB[i].AWB[3] * 2 / (OPPO_AWB[i].AWB[1] + OPPO_AWB[i].AWB[2]);
		float Gr_Gb = (float)OPPO_AWB[i].AWB[1] / OPPO_AWB[i].AWB[2];

		float Golden_R_G = (float)OPPO_AWB[i].Golden[0] * 2 / (OPPO_AWB[i].Golden[1] + OPPO_AWB[i].Golden[2]);
		float Golden_B_G = (float)OPPO_AWB[i].Golden[3] * 2 / (OPPO_AWB[i].Golden[1] + OPPO_AWB[i].Golden[2]);
		float Golden_Gr_Gb = (float)OPPO_AWB[i].Golden[1] / OPPO_AWB[i].Golden[2];

		float d = abs(R_G / Golden_R_G - 1);
		if (d < 0.0001)d = 0;
		if (log_type == 0)FP_log << "QC R_G awb_tolerance=: " << d << endl;
		if (d > awb_tolerance[i][0] / 100.0) {
 			ret = ret | 2;
		}
		d = abs(B_G / Golden_B_G - 1);
		if (d < 0.0001)d = 0;
		if (log_type == 0)FP_log << "QC B_G awb_tolerance=: " << d << endl;
		if (d > awb_tolerance[i][1] / 100.0) {
			ret = ret | 2;
		}
		d = abs(Gr_Gb / Golden_Gr_Gb - 1);
		if (d < 0.0001)d = 0;
		if (log_type == 0)FP_log << "QC Gr_Gb awb_tolerance=: " << d << endl;
		if (d > awb_tolerance[i][2] / 10000.0) {
			ret = ret | 2;
		}
	}

	if ((ret&2) > 0) {
		FP_log << "QC awb_tolerance spec check NG!" << endl;
	}

	int golden_Spec[3][4];
	for (int i = 0; i < 3; i++) {

		if (OPPO_AWB[i].AWB[1] == 0 || OPPO_AWB[i].AWB[2] == 0 || OPPO_AWB[i].AWB[0] == 0 || OPPO_AWB[i].Golden[2] == 0)
			continue;

		string item = "golden_r_" + color[i];
		golden_Spec[i][0] = GetPrivateProfileInt(_T("OPPO"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "golden_gr_" + color[i];
		golden_Spec[i][1] = GetPrivateProfileInt(_T("OPPO"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "golden_gb_" + color[i];
		golden_Spec[i][2] = GetPrivateProfileInt(_T("OPPO"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "golden_b_" + color[i];
		golden_Spec[i][3] = GetPrivateProfileInt(_T("OPPO"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));

		for (int k = 0; k < 4; k++) {
			if (OPPO_AWB[i].Golden[k] == 0)
				continue;
			if (golden_Spec[i][k] != OPPO_AWB[i].Golden[k] && awb_golden_check == 1) {
				ret = ret | 4;
			}
		}
	}

	if ((ret&4) > 0) {
		FP_log << "OPPO QC awb_golden spec check NG!" << endl;
	}

	return ret;
}


int MTK_AWB_FP_Check(oppo_AWB_Format OPPO_AWB[3], int mode) {

	int ret = 0;
	string color[3] = { "5100k","4000k","3100k" };

	for (int i = 0; i < mode; i++)
		for (int j = 0; j < 4; j++) {
			if (OPPO_AWB[i].AWB[j]>65530 || OPPO_AWB[i].AWB[j] < 100)
				return 4096;
			if (OPPO_AWB[i].Golden[j]>65530 || OPPO_AWB[i].Golden[j] < 100)
				return 4096;
		}

	/////////////awb_distance check
	for (int i = 0; i < mode; i++) {

		if (OPPO_AWB[i].AWB[1] == 0 || OPPO_AWB[i].AWB[2] == 0)
			return 1;
		if (OPPO_AWB[i].Golden[1] == 0 || OPPO_AWB[i].Golden[2] == 0)
			return 1;

		float R_Gr = (float)OPPO_AWB[i].AWB[0] / OPPO_AWB[i].AWB[1];
		float B_Gb = (float)OPPO_AWB[i].AWB[3] / OPPO_AWB[i].AWB[2];

		float Golden_R_Gr = (float)OPPO_AWB[i].Golden[0] / OPPO_AWB[i].Golden[1];
		float Golden_B_Gb = (float)OPPO_AWB[i].Golden[3] / OPPO_AWB[i].Golden[2];

		float d = sqrt(pow(R_Gr - Golden_R_Gr, 2) + pow(B_Gb - Golden_B_Gb, 2));
	

		if (d > awb_distance[i] / 100.0) {
			ret = ret | 1;
		}
		if (log_type == 0||(ret&1)>0)
			FP_log << color[i] << " MTK awb_distance=: " << d << endl;
	}

	for (int i = 0; i < mode; i++) {
		float R_G = (float)OPPO_AWB[i].AWB[0] * 2 / (OPPO_AWB[i].AWB[1] + OPPO_AWB[i].AWB[2]);
		float B_G = (float)OPPO_AWB[i].AWB[3] * 2 / (OPPO_AWB[i].AWB[1] + OPPO_AWB[i].AWB[2]);
		float Gr_Gb = (float)OPPO_AWB[i].AWB[1] / OPPO_AWB[i].AWB[2];

		float Golden_R_G = (float)OPPO_AWB[i].Golden[0] * 2 / (OPPO_AWB[i].Golden[1] + OPPO_AWB[i].Golden[2]);
		float Golden_B_G = (float)OPPO_AWB[i].Golden[3] * 2 / (OPPO_AWB[i].Golden[1] + OPPO_AWB[i].Golden[2]);
		float Golden_Gr_Gb = (float)OPPO_AWB[i].Golden[1] / OPPO_AWB[i].Golden[2];

		float d = abs(R_G / Golden_R_G - 1);
		if (d < 0.0001)d = 0;

		if (d > awb_tolerance[i][0] / 100.0) {
			ret = ret | 2;
		}
		d = abs(B_G / Golden_B_G - 1);
		if (d < 0.0001)d = 0;
		
		if (d > awb_tolerance[i][1] / 100.0) {
			ret = ret | 2;
		}
		d = abs(Gr_Gb / Golden_Gr_Gb - 1);
		if (d < 0.0001)d = 0;
		
		if (d > awb_tolerance[i][2] / 10000.0) {
			ret = ret | 2;
		}

		if (log_type == 0|| (ret&2)>0) {
			FP_log << "MTK R_G awb_tolerance=: " << d << endl;
			FP_log << "MTK B_G awb_tolerance=: " << d << endl;
			FP_log << "MTK Gr_Gb awb_tolerance=: " << d << endl;
		}

	}

	if ((ret & 2) > 0) {
		FP_log << "MTK awb_tolerance spec check NG!" << endl;
	}

	int golden_Spec[3][4];
	for (int i = 0; i < mode; i++) {
		string item = "MTK_golden_r_" + color[i];
		golden_Spec[i][0] = GetPrivateProfileInt(_T("MTK"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "MTK_golden_gr_" + color[i];
		golden_Spec[i][1] = GetPrivateProfileInt(_T("MTK"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "MTK_golden_gb_" + color[i];
		golden_Spec[i][2] = GetPrivateProfileInt(_T("MTK"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "MTK_golden_b_" + color[i];
		golden_Spec[i][3] = GetPrivateProfileInt(_T("MTK"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));

		for (int k = 0; k < 4; k++) {
			if (golden_Spec[i][k] != OPPO_AWB[i].Golden[k] && awb_golden_check == 1) {
				ret = ret | 4;
			}
		}
	}

	if ((ret & 4) > 0) {
		FP_log << "MTK_awb_golden spec check NG!" << endl;
	}

	return ret;
}


int LSI_AWB_FP_Check(oppo_AWB_Format OPPO_AWB[3], int mode) {

	int ret = 0;
	string color[3] = { "D50","A","LT84" };

	for (int i = 0; i < mode; i++)
		for (int j = 0; j < 4; j++) {
			if (OPPO_AWB[i].AWB[j]>1100 || OPPO_AWB[i].AWB[j] < 100)
				return 4096;
			if (OPPO_AWB[i].Golden[j]>1100 || OPPO_AWB[i].Golden[j] < 100)
				return 4096;
		}

	/////////////awb_distance check
	for (int i = 0; i < mode; i++) {

		if (OPPO_AWB[i].AWB[1] == 0 || OPPO_AWB[i].AWB[2] == 0)
			return 1;
		if (OPPO_AWB[i].Golden[1] == 0 || OPPO_AWB[i].Golden[2] == 0)
			return 1;

		float R_Gr = (float)OPPO_AWB[i].AWB[0] / OPPO_AWB[i].AWB[1];
		float B_Gb = (float)OPPO_AWB[i].AWB[3] / OPPO_AWB[i].AWB[2];

		float Golden_R_Gr = (float)OPPO_AWB[i].Golden[0] / OPPO_AWB[i].Golden[1];
		float Golden_B_Gb = (float)OPPO_AWB[i].Golden[3] / OPPO_AWB[i].Golden[2];

		float d = sqrt(pow(R_Gr - Golden_R_Gr, 2) + pow(B_Gb - Golden_B_Gb, 2));


		if (d > awb_distance[i] / 100.0) {
			ret = ret | 1;
		}
		if (log_type == 0 || (ret & 1)>0)
			FP_log << color[i] << " MTK awb_distance=: " << d << endl;
	}

	for (int i = 0; i < mode; i++) {
		float R_G = (float)OPPO_AWB[i].AWB[0] * 2 / (OPPO_AWB[i].AWB[1] + OPPO_AWB[i].AWB[2]);
		float B_G = (float)OPPO_AWB[i].AWB[3] * 2 / (OPPO_AWB[i].AWB[1] + OPPO_AWB[i].AWB[2]);
		float Gr_Gb = (float)OPPO_AWB[i].AWB[1] / OPPO_AWB[i].AWB[2];

		float Golden_R_G = (float)OPPO_AWB[i].Golden[0] * 2 / (OPPO_AWB[i].Golden[1] + OPPO_AWB[i].Golden[2]);
		float Golden_B_G = (float)OPPO_AWB[i].Golden[3] * 2 / (OPPO_AWB[i].Golden[1] + OPPO_AWB[i].Golden[2]);
		float Golden_Gr_Gb = (float)OPPO_AWB[i].Golden[1] / OPPO_AWB[i].Golden[2];

		float d = abs(R_G / Golden_R_G - 1);
		if (d < 0.0001)d = 0;

		if (d > awb_tolerance[i][0] / 100.0) {
			ret = ret | 2;
		}
		d = abs(B_G / Golden_B_G - 1);
		if (d < 0.0001)d = 0;

		if (d > awb_tolerance[i][1] / 100.0) {
			ret = ret | 2;
		}
		d = abs(Gr_Gb / Golden_Gr_Gb - 1);
		if (d < 0.0001)d = 0;

		if (d > awb_tolerance[i][2] / 10000.0) {
			ret = ret | 2;
		}

		if (log_type == 0 || (ret & 2)>0) {
			FP_log << "LSI R_G awb_tolerance=: " << d << endl;
			FP_log << "LSI B_G awb_tolerance=: " << d << endl;
			FP_log << "LSI Gr_Gb awb_tolerance=: " << d << endl;
		}

	}

	if ((ret & 2) > 0) {
		FP_log << "LSI awb_tolerance spec check NG!" << endl;
	}

	int golden_Spec[3][4];
	for (int i = 0; i < mode; i++) {
		string item = "LSI_golden_r_" + color[i];
		golden_Spec[i][0] = GetPrivateProfileInt(_T("LSI"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "LSI_golden_gr_" + color[i];
		golden_Spec[i][1] = GetPrivateProfileInt(_T("LSI"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "LSI_golden_gb_" + color[i];
		golden_Spec[i][2] = GetPrivateProfileInt(_T("LSI"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "LSI_golden_b_" + color[i];
		golden_Spec[i][3] = GetPrivateProfileInt(_T("LSI"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));

		for (int k = 0; k < 4; k++) {
			if (golden_Spec[i][k] != OPPO_AWB[i].Golden[k] && awb_golden_check == 1) {
				ret = ret | 4;
			}
		}
	}

	if ((ret & 4) > 0) {
		FP_log << "LSI_awb_golden spec check NG!" << endl;
	}

	return ret;
}


int VIVO_QC_AWB_FP_Check(vivo_AWB_Format VIVO_AWB[2]) {

	int ret = 0;
	string color[2] = { "5100k","3100k" };
	float AWB_diff[2][3];

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++) {
			if (VIVO_AWB[i].AWB[j]>1100 || VIVO_AWB[i].AWB[j] < 100)
				return 4096;
			if (VIVO_AWB[i].Golden[j]>1100 || VIVO_AWB[i].Golden[j] < 100)
				return 4096;
		}

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++) {
			AWB_diff[i][j] = abs(VIVO_AWB[i].AWB[j]*1.0 - VIVO_AWB[i].Golden[j]) / VIVO_AWB[i].Golden[j];
		}

	/////////////awb_distance check
	for (int i = 0; i < 2; i++) {
		//float d = sqrt(pow(AWB_diff[i][0], 2) + pow(AWB_diff[i][1], 2));
		float d = sqrt(pow(VIVO_AWB[i].AWB[0] / 1024.0 - golden_Spec[i][0] / 1000.0, 2) + pow(VIVO_AWB[i].AWB[1] / 1024.0 - golden_Spec[i][1] / 1000.0, 2));

		if (d > awb_distance[i] / 100.0) {
			FP_log << color[i] << " awb_distance=: " << d << endl;
			ret = ret | 1;
		}
	}

	if ((ret & 1) > 0) {
		FP_log << "awb_distance spec check NG!" << endl;
	}

	for (int i = 0; i < 2; i++) {

		if (abs(AWB_diff[i][0]) > awb_tolerance[i][0] / 100.0) {
			FP_log << "QC " << color[i] << "R_G awb_tolerance=: " << AWB_diff[i][0] << endl;
			ret = ret | 2;
		}

		if (abs(AWB_diff[i][1]) > awb_tolerance[i][1] / 100.0) {
			FP_log << "QC " << color[i] << "B_G awb_tolerance=: " << AWB_diff[i][1] << endl;
			ret = ret | 2;
		}

		if (abs(AWB_diff[i][2]) > awb_tolerance[i][2] / 10000.0) {
			FP_log << "QC " << color[i] << "Gr_Gb awb_tolerance=: " << AWB_diff[i][1] << endl;
			ret = ret | 2;
		}
	}

	if ((ret & 2) > 0) {
		FP_log << "awb_tolerance spec check NG!" << endl;
	}

	int golden_Spec[2][3];
	for (int i = 0; i < 2; i++) {
		string item = "golden_rg_" + color[i];
		golden_Spec[i][0] = GetPrivateProfileInt(_T("VIVO"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "golden_bg_" + color[i];
		golden_Spec[i][1] = GetPrivateProfileInt(_T("VIVO"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "golden_grgb_" + color[i];
		golden_Spec[i][2] = GetPrivateProfileInt(_T("VIVO"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));

		for (int k = 0; k < 3; k++) {
			if (golden_Spec[i][k] != VIVO_AWB[i].Golden[k]&& awb_golden_check==1) {
				ret = ret | 4;
			}
		}
	}

	if ((ret & 4) > 0) {
		FP_log << "awb_golden spec check NG!" << endl;
	}

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++) {
			if (VIVO_AWB[i].Light[j]>2000){
				if (VIVO_AWB[i].Light[j] < 8000 || VIVO_AWB[i].Light[j]>12000) {
					ret = ret | 8;
				}
			}
			else {
				if (VIVO_AWB[i].Light[j] < 800 || VIVO_AWB[i].Light[j]>1200) {
					ret = ret | 8;
				}
			}
		}
	if ((ret & 8) > 0) {
		FP_log << "Lightsource Coef Data NG!" << endl;
	}

	return ret;
}


int HONOR_QC_AWB_FP_Check(vivo_AWB_Format VIVO_AWB[3]) {

	int ret = 0;
	string color[3] = { "5100k","4000k","3100k" };
	float AWB_diff[3][3];

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++) {
			if (VIVO_AWB[i].AWB[j]>1100 || VIVO_AWB[i].AWB[j] < 100)
				return 4096;
			if (VIVO_AWB[i].Golden[j]>1100 || VIVO_AWB[i].Golden[j] < 100)
				return 4096;
		}

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++) {
			AWB_diff[i][j] = abs(VIVO_AWB[i].AWB[j] * 1.0 - VIVO_AWB[i].Golden[j]) / VIVO_AWB[i].Golden[j];
		}

	/////////////awb_distance check
	for (int i = 0; i < 3; i++) {
		//float d = sqrt(pow(AWB_diff[i][0], 2) + pow(AWB_diff[i][1], 2));
		float d = sqrt(pow(VIVO_AWB[i].AWB[0] / 1024.0 - golden_Spec[i][0] / 1000.0, 2) + pow(VIVO_AWB[i].AWB[1] / 1024.0 - golden_Spec[i][1] / 1000.0, 2));

		if (d > awb_distance[i] / 100.0) {
			FP_log << color[i] << " awb_distance=: " << d << endl;
			ret = ret | 1;
		}
	}

	if ((ret & 1) > 0) {
		FP_log << "awb_distance spec check NG!" << endl;
	}

	for (int i = 0; i < 3; i++) {

		if (abs(AWB_diff[i][0]) > awb_tolerance[i][0] / 100.0) {
			FP_log << "QC " << color[i] << "R_G awb_tolerance=: " << AWB_diff[i][0] << endl;
			ret = ret | 2;
		}

		if (abs(AWB_diff[i][1]) > awb_tolerance[i][1] / 100.0) {
			FP_log << "QC " << color[i] << "B_G awb_tolerance=: " << AWB_diff[i][1] << endl;
			ret = ret | 2;
		}

		if (abs(AWB_diff[i][2]) > awb_tolerance[i][2] / 10000.0) {
			FP_log << "QC " << color[i] << "Gr_Gb awb_tolerance=: " << AWB_diff[i][1] << endl;
			ret = ret | 2;
		}
	}

	if ((ret & 2) > 0) {
		FP_log << "awb_tolerance spec check NG!" << endl;
	}

	//for (int i = 0; i < 3; i++)
	//	for (int j = 0; j < 3; j++) {
	//		if (VIVO_AWB[i].Light[j]>2000) {
	//			if (VIVO_AWB[i].Light[j] < 8000 || VIVO_AWB[i].Light[j]>12000) {
	//				ret = ret | 8;
	//			}
	//		}
	//		else {
	//			if (VIVO_AWB[i].Light[j] < 800 || VIVO_AWB[i].Light[j]>1200) {
	//				ret = ret | 8;
	//			}
	//		}
	//	}
	//if ((ret & 8) > 0) {
	//	FP_log << "Lightsource Coef Data NG!" << endl;
	//}

	return ret;
}


int MOTO_QC_AWB_FP_Check(vivo_AWB_Format VIVO_AWB[2]) {

	int ret = 0;
	string color[2] = { "5100k","3100k" };
	float AWB_diff[2][3];

	for (int i = 0; i < 1; i++)
		for (int j = 0; j < 3; j++) {
			if (VIVO_AWB[i].AWB[j]>65530 || VIVO_AWB[i].AWB[j] < 100)
				return 4096;
			if (VIVO_AWB[i].Golden[j]>65530 || VIVO_AWB[i].Golden[j] < 100)
				return 4096;
		}

	for (int i = 0; i < 1; i++)
		for (int j = 0; j < 3; j++) {
			AWB_diff[i][j] = abs(VIVO_AWB[i].AWB[j] * 1.0 - VIVO_AWB[i].Golden[j]) / VIVO_AWB[i].Golden[j];
		}

	for (int i = 0; i < 1; i++) {

		if (abs(AWB_diff[i][0]) > awb_tolerance[i][0] / 100.0) {
			FP_log << "QC " << color[i] << "R_G awb_tolerance=: " << AWB_diff[i][0] << endl;
			ret = ret | 2;
		}

		if (abs(AWB_diff[i][1]) > awb_tolerance[i][1] / 100.0) {
			FP_log << "QC " << color[i] << "B_G awb_tolerance=: " << AWB_diff[i][1] << endl;
			ret = ret | 2;
		}

		if (abs(AWB_diff[i][2]) > awb_tolerance[i][2] / 10000.0) {
			FP_log << "QC " << color[i] << "Gr_Gb awb_tolerance=: " << AWB_diff[i][1] << endl;
			ret = ret | 2;
		}
	}

	if ((ret & 2) > 0) {
		FP_log << "awb_tolerance spec check NG!" << endl;
	}

	int golden_Spec[2][3];
	for (int i = 0; i < 1; i++) {
		string item = "golden_rg_" + color[i];
		golden_Spec[i][0] = GetPrivateProfileInt(_T("OTHER"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "golden_bg_" + color[i];
		golden_Spec[i][1] = GetPrivateProfileInt(_T("OTHER"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "golden_grgb_" + color[i];
		golden_Spec[i][2] = GetPrivateProfileInt(_T("OTHER"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));

		for (int k = 0; k < 3; k++) {
			if (golden_Spec[i][k] != VIVO_AWB[i].Golden[k] && awb_golden_check == 1) {
				ret = ret | 4;
			}
		}
	}

	if ((ret & 4) > 0) {
		FP_log << "awb_golden spec check NG!" << endl;
	}

	for (int i = 0; i < 1; i++)
		for (int j = 0; j < 4; j++) {
			if (VIVO_AWB[i].Light[j]>2000) {
				if (VIVO_AWB[i].Light[j] < 9700 || VIVO_AWB[i].Light[j]>10300) {
					ret = ret | 8;
				}
			}
			else {
				if (VIVO_AWB[i].Light[j] < 970 || VIVO_AWB[i].Light[j]>1030) {
					ret = ret | 8;
				}
			}
		}
	if ((ret & 8) > 0) {
		FP_log << "Lightsource Coef Data NG!" << endl;
	}

	return ret;
}


int SONY_AWB_FP_Check(vivo_AWB_Format VIVO_AWB[2]) {

	int ret = 0;
	string color[2] = { "High color","Low color" };

	float AWB_diff[2][3]; float max_value = 1 << 24;

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++) {
			if (VIVO_AWB[i].AWB[j]>max_value*2 || VIVO_AWB[i].AWB[j] < 100)
				return 4096;
		}

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++) {
			AWB_diff[i][j] = abs(VIVO_AWB[i].AWB[j]/max_value - golden_Spec[i][j]) / golden_Spec[i][j];
		}

	/////////////awb_distance check
	for (int i = 0; i < 2; i++) {
		float d = sqrt(pow(AWB_diff[i][0], 2) + pow(AWB_diff[i][1], 2));
		if (d > awb_distance[i] / 100.0) {
			FP_log << color[i] << " SONY awb_distance=: " << d << endl;
			ret = ret | 1;
		}
	}

	if ((ret & 1) > 0) {
		FP_log << "SONY awb_distance spec check NG!" << endl;
	}

	for (int i = 0; i < 2; i++) {

		if (abs(AWB_diff[i][0]) > awb_tolerance[i][0] / 100.0) {
			FP_log << "SONY " << color[i] << "R_G awb_tolerance=: " << AWB_diff[i][0] << endl;
			ret = ret | 2;
		}

		if (abs(AWB_diff[i][1]) > awb_tolerance[i][1] / 100.0) {
			FP_log << "SONY " << color[i] << "B_G awb_tolerance=: " << AWB_diff[i][1] << endl;
			ret = ret | 2;
		}

		if (abs(AWB_diff[i][2]) > awb_tolerance[i][2] / 10000.0) {
			FP_log << "SONY " << color[i] << "Gr_Gb awb_tolerance=: " << AWB_diff[i][1] << endl;
			ret = ret | 2;
		}
	}

	if ((ret & 2) > 0) {
		FP_log << "SONY awb_tolerance spec check NG!" << endl;
	}

	return ret;

}


int XiaoMi_QC_AWB_FP_Check(vivo_AWB_Format AWB) {

	int ret = 0;
	float AWB_diff[3], golden_Spec[3];

	string item = "master_rg_5100k";
	golden_Spec[0] = GetPrivateProfileInt(_T("XIAOMI"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
	item = "master_bg_5100k";
	golden_Spec[1] = GetPrivateProfileInt(_T("XIAOMI"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
	item = "master_gbgr_5100k";
	golden_Spec[2] = GetPrivateProfileInt(_T("XIAOMI"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));

	for (int j = 0; j < 3; j++) {
		AWB_diff[j] = (AWB.AWB[j] - golden_Spec[j]*1.024) / (golden_Spec[j]*1.024);
		if (AWB.AWB[j]>1100 || AWB.AWB[j] < 100)
			return 4096;
	}
	/////////////awb_distance check
	float d = sqrt(pow(AWB.AWB[0]/1024.0 - golden_Spec[0]/1000.0, 2) + pow(AWB.AWB[1] / 1024.0 - golden_Spec[1] / 1000.0, 2));
	if (d > awb_distance[0] / 100.0) {
		FP_log << "QC 5100K awb_distance=: " << d << endl;
		ret = ret | 1;
	}

	if (abs(AWB_diff[0]) > awb_tolerance[0][0] / 100.0) {
		FP_log << "QC R_G awb_tolerance=: " << AWB_diff[0] << endl;
		ret = ret | 2;
	}

	if (abs(AWB_diff[1]) > awb_tolerance[0][1] / 100.0) {
		FP_log << "QC B_G awb_tolerance=: " << AWB_diff[1] << endl;
		ret = ret | 2;
	}

	if (abs(AWB_diff[2]) > awb_tolerance[0][2] / 10000.0) {
		FP_log << "QC Gr_Gb awb_tolerance=: " << AWB_diff[2] << endl;
		ret = ret | 2;
	}
	
	if ((ret & 2) > 0) {
		FP_log << "awb_tolerance spec check NG!" << endl;
	}

	return ret;
}


int drift_FP_Check(int shift_Data[2][21], int cnt, int step) {

	int ret = 0, peak_cnt = 0;
	for (int k = 0; k < 2; k++) {
		peak_cnt = 0;
		for (int i = 2; i < cnt - 2; i++) {
			if (shift_Data[k][i + 1] > shift_Data[k][i] && shift_Data[k][i - 1]>=shift_Data[k][i])
				peak_cnt++;
			if (shift_Data[k][i + 1] < shift_Data[k][i] && shift_Data[k][i - 1] <= shift_Data[k][i])
				peak_cnt++;
		}
		if (peak_cnt > 1)
			ret = ret | 1;
	}

	if ((ret & 1) > 0) {
		FP_log << "----------- Shift Cal FP Check Result----------" << endl;
		FP_log << "shift Data has two Peak NG!" << endl;
	}

	return ret;
}


int SFR_FP_Check(int SFR_Data[50], int cnt, int group,int SFR_Format) {
	int ret = 0;
	for (int i = 0; i < cnt; i++) {
		string item = "SFR_Spec" + to_string(group) + "_" + to_string(i + 1);
		int SFR = GetPrivateProfileInt(_T("Spec_Set"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		if (SFR_Data[i] < SFR)
			ret = ret | 1;

		if(group<3&&SFR<100&&SFR_Data[i]>100)
			ret = ret | 1;

	}

	if ((ret & 1) > 0)
		FP_log << group << "# Set, SFR Spec Check NG!" << endl;

	return ret;
}


int AF_FP_Check(int AF_Data[6][3]) {
	int ret = 0;

	for (int i = 0; i < 6; i++) {

	//	if (AF_Data[i][0] != 0 && i < 2) {
			if (AF_Data[i][1] != 0)
				if (AF_Data[i][0] < AF_Data[i][1]) {
					ret |= (int)pow(2, i);
				}
			if (AF_Data[i][2] != 0)
				if (AF_Data[i][0] > AF_Data[i][2]) {
					ret |= (int)pow(2, i);
				}
		//}
		//if (AF_Data[i][0] != 0 && i >= 4) {
		//	if (AF_Data[i][1] != 0)
		//		if (AF_Data[i][0] - AF_Data[i - 4][0] < AF_Data[i][1]) {
		//			ret |= (int)pow(2, i);
		//		}
		//	if (AF_Data[i][2] != 0)
		//		if (AF_Data[i][0] - AF_Data[i - 4][0] > AF_Data[i][2]) {
		//			ret |= (int)pow(2, i);
		//		}
		//}
	}

	int code_range = AF_Data[0][0] - AF_Data[1][0];
	if(code_range > af_Code_Spec.range_max|| code_range<af_Code_Spec.range_min)
		ret |= (int)pow(2, 6);

	if (af_Code_Spec.mid1_coef != 0 && af_Code_Spec.POS1_diff != 0&& AF_Data[2][0]!=0) {
		if (abs(AF_Data[2][0] - AF_Data[1][0] - code_range*af_Code_Spec.mid1_coef)>af_Code_Spec.POS1_diff)
			ret |= (int)pow(2, 3);
	}

	if (af_Code_Spec.mid2_coef != 0 && af_Code_Spec.POS2_diff != 0 && AF_Data[3][0] != 0) {
		if (abs(AF_Data[3][0] - AF_Data[1][0] - code_range*af_Code_Spec.mid2_coef)>af_Code_Spec.POS2_diff)
			ret |= (int)pow(2, 4);
	}

	if (AF_Data[4][0] != 0 && af_Code_Spec.Mac_diff != 0) {
		if (abs(AF_Data[4][0] - AF_Data[0][0] )>af_Code_Spec.Mac_diff)
			ret |= (int)pow(2, 5);
	}
	if (AF_Data[5][0] != 0 && af_Code_Spec.Inf_diff != 0) {
		if (abs(AF_Data[5][0] - AF_Data[1][0])>af_Code_Spec.Inf_diff)
			ret |= (int)pow(2, 6);
	}

	if (ret > 0)
		FP_log << " AF Code Check NG!\n";

	return ret;
}


int GainMap_FP_Check(int PDgainLeft[30][30], int PDgainRight[30][30], int type) {

	int ret = 0, W=17, H=13;

	float diff_spec = GainMap_Diff;
	float LR_diff_spec = GainMap_LR_Diff;
	float TB_diff_spec = GainMap_TB_Diff;

	if (type == 1|| type == 3) {
		W = 16, H = 12;
	}
	if (type == 2|| type == 4) {
		W = 20, H = 4;
		diff_spec = MTK_Proc1_Diff;
		LR_diff_spec = MTK_Proc1_LR_Diff;
		TB_diff_spec = MTK_Proc1_TB_Diff;
	}
	if (PDgainLeft[H/2][W/2] == 0xFFFF || PDgainLeft[H / 2][W / 2] == 0) {
		ret |= 8;
	}
	if (PDgainRight[H/2][W/2] == 0xFFFF || PDgainRight[H / 2][W / 2] == 0) {
		ret |= 8;
	}

	if ( (ret&8) > 0){
		FP_log << "Gainmap invalid!" << endl;
		return ret;
	}

	float Hmax = 0, Vmax = 0;

	for (int i = 0; i < H; i++)
		for (int j = 0; j < W-1; j++) {
			int x = max(PDgainLeft[i][j], PDgainLeft[i][j + 1]);
			float y = abs(PDgainLeft[i][j] - PDgainLeft[i][j + 1]);
			if (y / x > diff_spec) {
				FP_log << "left_Gainmap [" << i << "," << j << "]= " << y / x << " H_Diff Check NG!" << endl;
				ret |= 1;
			}

			if (y / x > Hmax)
				Hmax = y / x;
			x = max(PDgainRight[i][j], PDgainRight[i][j + 1]);
			y = abs(PDgainRight[i][j] - PDgainRight[i][j + 1]);
			if (y/x > diff_spec) {
				ret |= 1;
				FP_log << "right_Gainmap [" << i << "," << j << "]= " << y / x << " H_Diff Check NG!" << endl;
			}
			if (y / x > Hmax)
				Hmax = y / x;
		}

	for (int j = 0; j < W; j++)
		for (int i = 0; i < H-1; i++) {
			int x = max(PDgainLeft[i + 1][j], PDgainLeft[i][j]);
			float y = abs(PDgainLeft[i + 1][j] - PDgainLeft[i][j]);
			if (y / x > diff_spec) {
				ret |= 1;
				FP_log << "Left_Gainmap [" << i << "," << j << "]= " << y / x << " V_Diff Check NG!" << endl;
			}
			if (y / x > Vmax)
				Vmax = y / x;
			x = max(PDgainRight[i + 1][j], PDgainRight[i][j]);
			y = abs(PDgainRight[i + 1][j] - PDgainRight[i][j]);
			if (y/x > diff_spec) {
				ret |= 1;
				FP_log << "right_Gainmap [" << i << "," << j << "]= " << y / x << " V_Diff Check NG!" << endl;
			}
			if (y / x > Vmax)
				Vmax = y / x;
		}

	FP_log << "GM Max_Hdiff_Vdiff=	" << Hmax <<"	" << Vmax << endl;

	/////////////// Left vs Right Mirror Diff Check
	int tsum = 0, diff_sum = 0;
	for (int i = 0; i < H; i++)
		for (int j = 0; j < W; j++) {
			diff_sum += abs(PDgainLeft[i][j] - PDgainRight[i][W-1 - j]);
			tsum += PDgainLeft[i][j] + PDgainRight[i][W-1 - j];
		}

	float max_diff = diff_sum / (float)tsum;
	if (max_diff> LR_diff_spec || log_type==0) {
		FP_log << "Gainmap Left vs Right Mirror Diff=	" << max_diff << endl;
		if (max_diff> LR_diff_spec)
			ret |= 2;
	}
	/////////////// Seft Top_Down Diff Check
	for (int i = 0; i < H/2; i++)
		for (int j = 0; j < W; j++) {
			int x = max(PDgainLeft[i][j], PDgainLeft[H-1-i][j]);
			float y = abs(PDgainLeft[i][j] - PDgainLeft[H-1-i][j]);

			if (y / x >TB_diff_spec) {
				ret |= 4;
			}

			x = min(PDgainRight[i][j], PDgainRight[H-1 - i][j]);
			y = abs(PDgainRight[i][j] - PDgainRight[H-1 - i][j]);

			if (y / x >TB_diff_spec) {
				FP_log << "Gainmap Top [" << i << "," << j << "]= " << y / x << " vs Self Bottom Diff Check NG!" << endl;
				ret |= 4;
			}
		}

	if ((ret & 4) > 0) {
		FP_log << "Gainmap Seft Top_Down Diff Check NG!" << endl;
	}

	return ret;

}


int DCC_FP_Check( int DCC[15][17], int type) {

	int ret = 0,W=8,H=6;
	float diff_Spec = DCC_Diff;
	if (type == 1) {
		W = 16, H = 12;
	}

	if (type == 2||type == 3 ) {
		W = 8, H = 8;
		diff_Spec = MTK_Proc2_Diff;
	}

	if (DCC[H/2][W/2] == 0xFFFF || DCC[H / 2][W / 2] == 0) {
		ret |= 8;
	}

	if ((ret & 8) > 0) {
		FP_log << "DCC Data invalid!" << endl;
		return ret;
	}

	for (int i = 0; i < H; i++)
		for (int j = 0; j < W-1; j++) {
			int x = max(DCC[i][j], DCC[i][j + 1]);
			float y = abs(DCC[i][j] - DCC[i][j + 1]);
			if (y / x > diff_Spec) {
				ret |= 1;
			}
		}
	for (int j = 0; j < W; j++)
		for (int i = 0; i < H-1; i++) {
			int x = max(DCC[i + 1][j], DCC[i][j]);
			float y = abs(DCC[i + 1][j] - DCC[i][j]);
			if (y / x > diff_Spec) {
				ret |= 2;
			}
		}
	return ret;
} 


int QSC_Check(float QSC_Data[4][4][12][16]) {

	int ret = 0;
	for (int a = 0; a < 4; a++) {
		for (int k = 0; k < 4; k++) {
			for (int i = 0; i < 12; i++) {
				for (int j = 0; j < 16; j++)
				{
					if (QSC_Data[a][k][i][j]> QSC_Max/1000.0 || QSC_Data[a][k][i][j] < QSC_Min / 1000.0){
						ret |= 1;
						FP_log << "QSC CH" << a << "_" << k << "Tablep [" << i << "," << j << "]=	" << QSC_Data[a][k][i][j] << "	NG!" << endl;
					}
				}
			}
		}
	}

	return ret;
}
