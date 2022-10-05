#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_EEPROM_Data_Verifier.h"
#include<iostream>
#include<fstream>   
#include <string>
#include<sstream>
#include <vector> 
#include <algorithm> 
#include "FP_Check.h"

extern char D[524288][2];
extern unsigned char DecData[524288], dflt_Data;
extern int EEP_Size, checkSum_addr[30][4],selection, DataFormat, mode;

enum data_Type {
	info1 = 1,
	info2 = 2,
	Flag1 = 3,
	CheckSum1 = 4,
	QC_AWB = 5,
	QC_LSC = 6,
	AF_Code = 7,
	QC_GainMap = 8,
	QC_DCC = 9,
	LSI_LSC = 10,
	Shift_Cal = 11,
	OIS_Hall = 12,
	AF_Hall = 13,
	OIS_Gyro = 14,
	QR = 15,
	FuseID = 16,
	VIVO_AWB = 17,
	VIVO_LSC = 18,
	MTK_LSC_Type = 19,
	Fuse_ID_Type = 20,
	package_Data = 21,
	Formula_Code = 22,
	QSC_Type = 23,
	MTK_AWB_Type=24,
	LSI_AWB_Type = 25,
};


class EEPROM_Data_Verifier : public QMainWindow
{
	Q_OBJECT

public:
	EEPROM_Data_Verifier(QWidget *parent = Q_NULLPTR);
	QImage img;
	QImage imgScaled;
	std::string name;
	

private:
	Ui::EEPROM_Data_VerifierClass ui;

	private slots:
	void on_pushButton_parser_clicked();
	void on_pushButton_dump_clicked();
	void on_pushButton_folder_clicked();
	void on_pushButton_clear_clicked();
	void on_pushButton_dump_value_clicked();
	void on_pushButton_dump_SFR_clicked();
	void selectModel();

	void DisplayOutput();
	void parameterDisplay();
	void load_EEPROM_Address();
	void save_EEPROM_Address();

	void load_Panel();
	void dump_Check();
	int get_Data_Type(int x);

	int CheckSum_Check();
	int info_Data_Parse();
	void Oppo_AWB_Parse(int group);
	void VIVO_AWB_Parse(int group);
	void SONY_AWB_Parse(int group);
	void XiaoMi_AWB_Parse(int group);
	void History_Date_Parser();

	int LSC_Parse(int start,int group);
	int MTK_LSC_Parse(int start);
	void vivo_MTK_AWB_Parse(int group);
	void MTK_AWB_Parse(int group);
	void LSI_AWB_Parse(int group);
	int drift_Parse();
	int cross_Parse();
	int af_Parse();
	int PDAF_Parse();
	int QSC_Parse();
	int OIS_Parse();
	int AEC_Parse();
	int XiaoMi_Seg_Check();
	int bin_Area_Check();
	int duplicate_Check();
	int value_duplicate_Check();
	int value_Data_Parse();
	int fix_Data_Check();
	int read_Spec_Bin();

	int Reserved_Check();

	void on_pushButton_openBIN_clicked();
	void on_pushButton_saveBIN_clicked();
	void on_pushButton_checkSum_clicked();

	void on_pushButton_setsave_clicked();
	void on_pushButton_load_lsc_clicked();

	void display_EEP();

};
