#include "EEPROM_Data_Verifier.h"

//extern void EEPMap_Out();
using namespace std;
ofstream Map_out(".\\EEPROM_Map.txt");

char Hex_data[2];

typedef struct {
	string info = "", ref = " ";
	int addr = 0;
	data_Type t;
}EEPMap_Data;

vector<EEPMap_Data> EEPMap;


void Dec2Hex(unsigned int tmp) {
	int a = tmp / 16;
	int b = tmp % 16;
	if (a < 10)
		Hex_data[0] = '0' + a;
	else
		Hex_data[0] = 'A' + a - 10;

	if (b < 10)
		Hex_data[1] = '0' + b;
	else
		Hex_data[1] = 'A' + b - 10;
}


void set_Map_inf(string info, string ref, int addr, data_Type t) {

	EEPMap_Data temp;
	temp.info = info;
	temp.ref = ref;
	temp.addr = addr;
	temp.t = t;
	EEPMap.push_back(temp);

}




bool LessSort(EEPMap_Data a, EEPMap_Data b)
{
	return (a.addr < b.addr);
}


void addr_Out(int addr) {
	unsigned char a = addr / 256;
	Dec2Hex(a);
	string s = Hex_data;
	s[2] = '\0';
	Map_out << s;
	unsigned char b = addr % 256;
	Dec2Hex(b);
	s = Hex_data;
	s[2] = '\0';
	Map_out << s ;
}

void EEPMap_Out() {

	sort(EEPMap.begin(), EEPMap.end(), LessSort);
	string default_data = "	Default value: 0xFF";
	if(dflt_Data==0)
		default_data = "	Default value: 0x00";

	Map_out << "(Addr)	(Contents)	(Spec.)	(RealData)	(Description)" << endl;

	bool Data_Section[50] = { 0 };
	int e = 0;

	size_t Size = EEPMap.size()-1;
	for (size_t i = 0; i < Size; i++) {

		if (i > 0) {

			if (EEPMap[i].t== package_Data){
				Map_out << "..." << "	" << "..." << "	" << endl;
			}
			else {
				if (EEPMap[i].addr - e > 1) {
					addr_Out(e + 1);
					Map_out << "	Reserved	" << "	";				Map_out << D[e + 1][0] << D[e + 1][1] << default_data << endl;
				}
				if (EEPMap[i].addr - e > 2) {
					addr_Out(EEPMap[i].addr - 1);
					Map_out << "	Reserved	" << "	";
					Map_out << D[EEPMap[i].addr - 1][0] << D[EEPMap[i].addr - 1][1] << default_data << endl;
				}
			}
		}
		e = EEPMap[i].addr;

		if (EEPMap[i].t != QC_LSC&&EEPMap[i].t != QC_GainMap&&EEPMap[i].t != QC_GainMap&&EEPMap[i].t != QC_DCC
			&&EEPMap[i].t != QR&&EEPMap[i].t != MTK_LSC_Type) {

			addr_Out(EEPMap[i].addr);
			Map_out << "	";
			Map_out << EEPMap[i].info << "	";
			if (EEPMap[i].ref.length() > 0)
				Map_out << EEPMap[i].ref << "	";
			else
				Map_out << " " << "	";

			Map_out << D[EEPMap[i].addr][0] << D[EEPMap[i].addr][1] << "	";
		}

		//////////////////Flag1
		if (EEPMap[i].t == Flag1) {
			Map_out << "Flag FF:Empty; 01:Valid; 10:Invalid";
		}

		//////////////////CheckSum1
		if (EEPMap[i].t == CheckSum1) {
			Map_out << "SUM(";
			for (int k = 0; k< 30; k++){
				if (checkSum_addr[k][3] != 0&&EEPMap[i].addr == checkSum_addr[k][3] ) {
					Map_out << "0x";
					addr_Out(checkSum_addr[k][1]);
					Map_out << "~0x";
					addr_Out(checkSum_addr[k][2]);
					break;
				}
			}
			if ((selection & 32) > 0)
				Map_out << ")%255";
			if ((selection & 64) > 0)
				Map_out << ")%0xFF+1";
			if ((selection & 128) > 0)
				Map_out << ")%0xFF+1";
			Map_out << endl;
		}


		//////////////////QR
		if (EEPMap[i].t == QR) {

			if (DataFormat % 10 == 1) {

				addr_Out(e);
				Map_out << "	";
				Map_out << EEPMap[i].info;

				Map_out << "供应商代码		";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	物料代码_1st	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	物料代码_2nd	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	物料代码_3rd	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	物料代码_4th	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	物料代码_5th	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	物料代码_6th	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	物料代码_7th	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	日期1st	" << " " << "	";
				Map_out << D[e][0] << D[e][1] <<"	2019年 - 0x39, Mean 9 (ASCII)"<< endl;

				e++;
				addr_Out(e);
				Map_out << "	日期2nd	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	ASCII 1月：0x31, 10月（A）：0x41" << endl;

				e++;
				addr_Out(e);
				Map_out << "	日期3rd	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	ASCII 1日：0x31, 20日（K）：0x4B" << endl;

				e++;
				addr_Out(e);
				Map_out << "	流水码_1st	" << " " << "	";
				Map_out << D[e][0] << D[e][1] <<  endl;

				e++;
				addr_Out(e);
				Map_out << "	流水码_2nd	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	流水码_3rd	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	流水码_4th	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	流水码_5th	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	流水码_6th	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

			}

			if (DataFormat % 10 == 2) {

				addr_Out(e);
				Map_out << "	";
				Map_out << EEPMap[i].info;

				Map_out << " Camera type		";
				Map_out << D[e][0] << D[e][1] <<"	Front Cam: 05(0x35)  Rear Cam: 06(0x36)"<< endl;

				e++;
				addr_Out(e);
				Map_out << "	Vendor Code	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	Vendor: A(0x41)" << endl;

				e++;
				addr_Out(e);
				Map_out << "	Project No	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	Project No	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				e++;
				addr_Out(e);
				Map_out << "	Production year	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	2020=A(ASCII 0x41)" << endl;

				e++;
				addr_Out(e);
				Map_out << "	Production month	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	9月=0x39 10月(A)=0x41" << endl;

				e++;
				addr_Out(e);
				Map_out << "	Production day	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	10日(A)=0x41 31日(V)=0x56" << endl;

				e++;
				addr_Out(e);
				Map_out << "	Revison	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	0,1,2...(DEV state) A,B,C...(MP state)" << endl;

				e++;
				addr_Out(e);
				Map_out << "	Serial No 1st digit	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	ASCII code 0:0x30, A:0x41... Z=0x5A (not use I,O)" << endl;

				e++;
				addr_Out(e);
				Map_out << "	Serial No 2nd digit	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	ASCII code 0:0x30, A:0x41... Z=0x5A (not use I,O)" << endl;

				e++;
				addr_Out(e);
				Map_out << "	Serial No 3rd digit	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	ASCII code 0:0x30, A:0x41... Z=0x5A (not use I,O)" << endl;

				e++;
				addr_Out(e);
				Map_out << "	Serial No 4th digit	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	ASCII code 0:0x30, A:0x41... Z=0x5A (not use I,O)" << endl;

			}

		}

		//////////////////QC_LSC

		if (EEPMap[i].t == QC_LSC) {

			if (DataFormat % 10 == 2) {

				addr_Out(e);
				Map_out << "	";
				Map_out << EEPMap[i].info;

				Map_out << " R bit_H[15:8](Block 1)		";
				Map_out << D[e][0] << D[e][1] << "	QC platform 17*13*4*2=1768 byte" << endl;
				e++;
				addr_Out(e);
				Map_out << "	R bit_L[7:0](Block 1)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	Gr bit_H[15:8](Block 1)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	Gr bit_L[7:0](Block 1)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	Gb bit_H[15:8](Block 1)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	Gb bit_L[7:0](Block 1)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	B bit_H[15:8](Block 1)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	B bit_L[7:0](Block 1)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

				Map_out << "..." << "	" << "..." << "	" << endl;

				e= EEPMap[i].addr+1760;
				addr_Out(e);
				Map_out << "	R bit_H[15:8](Blcok 221)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	R bit_L[7:0](Blcok 221)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	Gr bit_H[15:8](Blcok 221)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	Gr bit_L[7:0](Blcok 221)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	Gb bit_H[15:8](Blcok 221)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	Gb bit_L[7:0](Blcok 221)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	B bit_H[15:8](Blcok 221)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				e++;
				addr_Out(e);
				Map_out << "	B bit_L[7:0](Blcok 221)	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;


		
			}

			if (DataFormat % 10 == 3) {

				addr_Out(e);
				Map_out << "	";
				Map_out << " LSC Data(5100K) start	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << "	QC platform 17*13*4*2=1768 byte" << endl;

				Map_out << "..." << "	" << "..." << "	" << endl;

				e = EEPMap[i].addr + 1767;
				addr_Out(e);
				Map_out << "	LSC Data(5100K) end	" << " " << "	";
				Map_out << D[e][0] << D[e][1];

			}


		}

		//////////////////QC_GainMap
		if (EEPMap[i].t == QC_GainMap) {

			addr_Out(e);
			Map_out << "	Left GainMap[0] H	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	Left GainMap[0] L	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			Map_out << "..." << "	" << "..." << "	" << endl;

			e+=439;
			addr_Out(e);
			Map_out << "	Left GainMap[220] H	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	Left GainMap[220] L	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;
	/////////////////////////		//////////////////////////
			e++;
			addr_Out(e);
			Map_out << "	Right GainMap[0] H	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	Right GainMap[0] L	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			Map_out << "..." << "	" << "..." << "	" << endl;

			e += 439;
			addr_Out(e);
			Map_out << "	Right GainMap[220] H	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	Right GainMap[220] L	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;
		}

		//////////////////QC_DCC
		if (EEPMap[i].t == QC_DCC) {

			string A = "H", B = "L";

			string str = EEPMap[i].info;
			addr_Out(e);
			Map_out << "	" << str + " (0,0) H	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	" << str + " (0,0) L	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	" << str + " (1,0) H	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	" << str + " (1,0) L	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			Map_out << "..." << "	" << "..." << "	" << endl;

			e += 11;
			addr_Out(e);
			Map_out << "	" << str + " (7,0) H	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	" << str + " (7,0) L	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	" << str + " (1,0) H	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	" << str + " (1,0) L	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			Map_out << "..." << "	" << "..." << "	" << endl;

			e+=77;
			addr_Out(e);
			Map_out << "	" << str + " (7,5) H	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;

			e++;
			addr_Out(e);
			Map_out << "	" << str + " (7,5) L	" << " " << "	";
			Map_out << D[e][0] << D[e][1] << endl;
		}

		//////////////////MTK_LSC
		if (EEPMap[i].t == MTK_LSC_Type) {

			if (1) {
				addr_Out(e);
				Map_out << "	" << EEPMap[i].info + " start	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;
				Map_out << "..." << "	" << "..." << "	" << endl;
				e += 1867;
				addr_Out(e);
				Map_out << "	" << EEPMap[i].info + " end	" << " " << "	";
				Map_out << D[e][0] << D[e][1] << endl;

			}

		}












		Map_out << endl;
	}

}