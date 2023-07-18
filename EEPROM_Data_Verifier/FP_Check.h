#pragma once
#include <string>
#include <afx.h>

typedef struct {
	unsigned int AWB[4] = { 0 }, Golden[4] = { 0 }, Light[2] = { 0 };
	bool HL = true;
}oppo_AWB_Format;

typedef struct {
	unsigned int AWB[3] = { 0 }, Golden[3] = { 0 }, Light[4] = { 0 };
	bool HL = true;
}vivo_AWB_Format;

int QC_LSC_FP_Check(int LSC[25][33][4],int type);
int MTK_LSC_FP_Check(int LSC[15][15][4],int first_pixel);
void set_ini_Path(const std::string& str);
int OPPO_QC_AWB_FP_Check(oppo_AWB_Format OPPO_AWB[3]);
int MTK_AWB_FP_Check(oppo_AWB_Format OPPO_AWB[3],int mode);
int LSI_AWB_FP_Check(oppo_AWB_Format OPPO_AWB[3], int mode);

int VIVO_QC_AWB_FP_Check(vivo_AWB_Format VIVO_AWB[2]);
int MOTO_QC_AWB_FP_Check(vivo_AWB_Format VIVO_AWB[2]);

int SONY_AWB_FP_Check(vivo_AWB_Format VIVO_AWB[2]);
int XiaoMi_QC_AWB_FP_Check(vivo_AWB_Format AWB);

int drift_FP_Check(int shift_Data[2][21],int cnt, int step);
int SFR_FP_Check(int SFR_Data[50], int cnt ,int group,int SFR_Format);
int AF_FP_Check(int AF_Data[6][3]);
int GainMap_FP_Check(int PDgainLeft[30][30],int PDgainRight[30][30], int type);
int DCC_FP_Check(int DCC[15][17], int type);
int QSC_Check(float QSC_Data[4][4][12][16]);

void fuse_ID_output(const std::string& str);
void load_Spec(int x);
void FP_logFile_Open();
void FP_logFile_Close();



