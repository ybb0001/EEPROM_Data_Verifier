
#include "FP_Check.h"
#include<fstream>   
#include <afx.h>
#include<math.h>
#include <vector>

#ifndef CZY_MATH_FIT
#define CZY_MATH_FIT

/*
多项式拟合
*/
namespace czy {
	///
	/// \brief 曲线拟合类
	///
	class Fit {
		std::vector<double> factor; ///<拟合后的方程系数
		double ssr;                 ///<回归平方和
		double sse;                 ///<(剩余平方和)
		double rmse;                ///<RMSE均方根误差
		std::vector<double> fitedYs;///<存放拟合后的y值，在拟合时可设置为不保存节省内存
	public:
		Fit() :ssr(0), sse(0), rmse(0) { factor.resize(2, 0); }
		~Fit() {}
		///
		/// \brief 直线拟合-一元回归,拟合的结果可以使用getFactor获取，或者使用getSlope获取斜率，getIntercept获取截距
		/// \param x 观察值的x
		/// \param y 观察值的y
		/// \param isSaveFitYs 拟合后的数据是否保存，默认否
		///
		template<typename T>
		bool linearFit(const std::vector<typename T>& x, const std::vector<typename T>& y, bool isSaveFitYs = false)
		{
			return linearFit(&x[0], &y[0], getSeriesLength(x, y), isSaveFitYs);
		}
		template<typename T>
		bool linearFit(const T* x, const T* y, int length, bool isSaveFitYs = false)
		{
			factor.resize(2, 0);
			typename T t1 = 0, t2 = 0, t3 = 0, t4 = 0;
			for (int i = 0; i < length; ++i)
			{
				t1 += x[i] * x[i];
				t2 += x[i];
				t3 += x[i] * y[i];
				t4 += y[i];
			}
			factor[1] = (t3*length - t2*t4) / (t1*length - t2*t2);
			factor[0] = (t1*t4 - t2*t3) / (t1*length - t2*t2);
			//
			//计算误差
			calcError(x, y, length, this->ssr, this->sse, this->rmse, isSaveFitYs);
			return true;
		}
		///
		/// \brief 多项式拟合，拟合y=a0+a1*x+a2*x^2+……+apoly_n*x^poly_n
		/// \param x 观察值的x
		/// \param y 观察值的y
		/// \param poly_n 期望拟合的阶数，若poly_n=2，则y=a0+a1*x+a2*x^2
		/// \param isSaveFitYs 拟合后的数据是否保存，默认是
		/// 
		template<typename T, typename TD>
		void polyfit(const std::vector<typename T>& x
			, const std::vector<typename TD>& y
			, int poly_n
			, bool isSaveFitYs = true)
		{
			polyfit(&x[0], &y[0], getSeriesLength(x, y), poly_n, isSaveFitYs);
		}
		template<typename T, typename TD>
		void polyfit(const T* x, const TD* y, int length, int poly_n, bool isSaveFitYs = true)
		{
			factor.resize(poly_n + 1, 0);
			int i, j;
			//double *tempx,*tempy,*sumxx,*sumxy,*ata;
			std::vector<double> tempx(length, 1.0);

			std::vector<double> tempy(y, y + length);

			std::vector<double> sumxx(poly_n * 2 + 1);
			std::vector<double> ata((poly_n + 1)*(poly_n + 1));
			std::vector<double> sumxy(poly_n + 1);
			for (i = 0; i < 2 * poly_n + 1; i++) {
				for (sumxx[i] = 0, j = 0; j < length; j++)
				{
					sumxx[i] += tempx[j];
					tempx[j] *= x[j];
				}
			}
			for (i = 0; i < poly_n + 1; i++) {
				for (sumxy[i] = 0, j = 0; j < length; j++)
				{
					sumxy[i] += tempy[j];
					tempy[j] *= x[j];
				}
			}
			for (i = 0; i < poly_n + 1; i++)
				for (j = 0; j < poly_n + 1; j++)
					ata[i*(poly_n + 1) + j] = sumxx[i + j];
			gauss_solve(poly_n + 1, ata, factor, sumxy);
			//计算拟合后的数据并计算误差
			fitedYs.reserve(length);
			calcError(&x[0], &y[0], length, this->ssr, this->sse, this->rmse, isSaveFitYs);

		}
		/// 
		/// \brief 获取系数
		/// \param 存放系数的数组
		///
		void getFactor(std::vector<double>& factor) { factor = this->factor; }
		/// 
		/// \brief 获取拟合方程对应的y值，前提是拟合时设置isSaveFitYs为true
		///
		void getFitedYs(std::vector<double>& fitedYs) { fitedYs = this->fitedYs; }

		/// 
		/// \brief 根据x获取拟合方程的y值
		/// \return 返回x对应的y值
		///
		template<typename T>
		double getY(const T x) const
		{
			double ans(0);
			for (int i = 0; i < (int)factor.size(); ++i)
			{
				ans += factor[i] * pow((double)x, (int)i);
			}
			return ans;
		}
		/// 
		/// \brief 获取斜率
		/// \return 斜率值
		///
		double getSlope() { return factor[1]; }
		/// 
		/// \brief 获取截距
		/// \return 截距值
		///
		double getIntercept() { return factor[0]; }
		/// 
		/// \brief 剩余平方和
		/// \return 剩余平方和
		///
		double getSSE() { return sse; }
		/// 
		/// \brief 回归平方和
		/// \return 回归平方和
		///
		double getSSR() { return ssr; }
		/// 
		/// \brief 均方根误差
		/// \return 均方根误差
		///
		double getRMSE() { return rmse; }
		/// 
		/// \brief 确定系数，系数是0~1之间的数，是数理上判定拟合优度的一个量
		/// \return 确定系数
		///
		double getR_square() { return 1 - (sse / (ssr + sse)); }
		/// 
		/// \brief 获取两个vector的安全size
		/// \return 最小的一个长度
		///
		template<typename T, typename TD>
		int getSeriesLength(const std::vector<typename T>& x
			, const std::vector<typename TD>& y)
		{
			return (x.size() > y.size() ? y.size() : x.size());
		}
		/// 
		/// \brief 计算均值
		/// \return 均值
		///
		template <typename T>
		static T Mean(const std::vector<T>& v)
		{
			return Mean(&v[0], v.size());
		}
		template <typename T>
		static T Mean(const T* v, int length)
		{
			T total(0);
			for (int i = 0; i < length; ++i)
			{
				total += v[i];
			}
			return (total / length);
		}
		/// 
		/// \brief 获取拟合方程系数的个数
		/// \return 拟合方程系数的个数
		///
		int getFactorSize() { return factor.size(); }
		/// 
		/// \brief 根据阶次获取拟合方程的系数，
		/// 如getFactor(2),就是获取y=a0+a1*x+a2*x^2+……+apoly_n*x^poly_n中a2的值
		/// \return 拟合方程的系数
		///
		double getFactor(int i) { return factor.at(i); }
	private:
		template<typename T, typename TD>
		void calcError(const T* x
			, const TD* y
			, int length
			, double& r_ssr
			, double& r_sse
			, double& r_rmse
			, bool isSaveFitYs = true
			)
		{
			TD mean_y = Mean<TD>(y, length);
			T yi(0);
			fitedYs.reserve(length);
			for (int i = 0; i < length; ++i)
			{
				yi = getY(x[i]);
				r_ssr += ((yi - mean_y)*(yi - mean_y));//计算回归平方和
				r_sse += ((yi - y[i])*(yi - y[i]));//残差平方和
				if (isSaveFitYs)
				{
					fitedYs.push_back(double(yi));
				}
			}
			r_rmse = sqrt(r_sse / (double(length)));
		}
		template<typename T>
		void gauss_solve(int n
			, std::vector<typename T>& A
			, std::vector<typename T>& x
			, std::vector<typename T>& b)
		{
			gauss_solve(n, &A[0], &x[0], &b[0]);
		}
		template<typename T>
		void gauss_solve(int n
			, T* A
			, T* x
			, T* b)
		{
			int i, j, k, r;
			double max;
			for (k = 0; k < n - 1; k++)
			{
				max = fabs(A[k*n + k]); /*find maxmum*/
				r = k;
				for (i = k + 1; i < n - 1; i++) {
					if (max < fabs(A[i*n + i]))
					{
						max = fabs(A[i*n + i]);
						r = i;
					}
				}
				if (r != k) {
					for (i = 0; i < n; i++)         /*change array:A[k]&A[r] */
					{
						max = A[k*n + i];
						A[k*n + i] = A[r*n + i];
						A[r*n + i] = max;
					}
				}
				max = b[k];                    /*change array:b[k]&b[r]     */
				b[k] = b[r];
				b[r] = max;
				for (i = k + 1; i < n; i++)
				{
					for (j = k + 1; j < n; j++)
						A[i*n + j] -= A[i*n + k] * A[k*n + j] / A[k*n + k];
					b[i] -= A[i*n + k] * b[k] / A[k*n + k];
				}
			}

			for (i = n - 1; i >= 0; x[i] /= A[i*n + i], i--)
				for (j = i + 1, x[i] = b[i]; j < n; j++)
					x[i] -= A[i*n + j] * x[j];
		}
	};
}

#endif



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
int MTK_LR_ratio_diff, MTK_UD_ratio_diff, MTK_LR_diff, MTK_LR_reverse, MTK_UD_diff, MTK_UD_reverse;
float Drift_Spec_X, Drift_Spec_Y;


string INI_Path ;
//ofstream FP_log(".\\FP_Check_Log.txt");
ofstream FP_log;
PDAF_d PDAF_Data;

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

	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("Drift_Spec_X"), TEXT("5.0"), lpTexts, 10, CA2CT(INI_Path.c_str()));
	Drift_Spec_X=atof(CT2A(lpTexts));
	GetPrivateProfileString(TEXT("Spec_Set"), TEXT("Drift_Spec_Y"), TEXT("5.0"), lpTexts, 10, CA2CT(INI_Path.c_str()));
	Drift_Spec_Y = atof(CT2A(lpTexts));

	QSC_Min= GetPrivateProfileInt(_T("Spec_Set"), TEXT("QSC_Min"), 920, CA2CT(INI_Path.c_str()));
	QSC_Max = GetPrivateProfileInt(_T("Spec_Set"), TEXT("QSC_Max"), 1080, CA2CT(INI_Path.c_str()));

	MTK_LR_ratio_diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("MTK_LR_RATIO_DIFF"), 100, CA2CT(INI_Path.c_str()));
	MTK_UD_ratio_diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("MTK_UD_RATIO_DIFF"), 100, CA2CT(INI_Path.c_str()));
	MTK_LR_diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("MTK_LR_DIFF"), 100, CA2CT(INI_Path.c_str()));
	MTK_LR_reverse = GetPrivateProfileInt(_T("Spec_Set"), TEXT("MTK_LR_REVERSE"), 20, CA2CT(INI_Path.c_str()));
	MTK_UD_diff = GetPrivateProfileInt(_T("Spec_Set"), TEXT("MTK_UD_DIFF"), 60, CA2CT(INI_Path.c_str()));
	MTK_UD_reverse = GetPrivateProfileInt(_T("Spec_Set"), TEXT("MTK_UD_REVERSE"), 20, CA2CT(INI_Path.c_str()));


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
		//int xx = 0;
		//if (LSC[0][0][0] * LSC[0][0][2] * 0.95 > LSC[0][0][3] * LSC[0][0][1])
		//	xx++;
		//if (LSC[0][16][0] * LSC[0][16][2] * 0.95 > LSC[0][16][3] * LSC[0][16][1])
		//	xx++;
		//if (LSC[12][0][0] * LSC[12][0][2] * 0.95 > LSC[12][0][3] * LSC[12][0][1])
		//	xx++;
		//if (LSC[12][16][0] * LSC[12][16][2] * 0.95 > LSC[12][16][3] * LSC[12][16][1])
		//	xx++;

		//if (xx > 2 && awb_golden_check == 1) {
		//	ret |= 4;
		//	FP_log << " QC LSC R Gr Gb B Sequence NG!" << endl;
		//}
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
	string color[3] = { "5100k","3100k","4000k" };

	for (int i = 0; i < mode; i++)
		for (int j = 0; j < 4; j++) {
			if (OPPO_AWB[i].AWB[j]>65530 || OPPO_AWB[i].AWB[j] < 10)
				return 4096;
			if (OPPO_AWB[i].Golden[j]>65530 || OPPO_AWB[i].Golden[j] < 10)
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
		golden_Spec[i][0] = GetPrivateProfileInt(_T("MOTO"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "golden_bg_" + color[i];
		golden_Spec[i][1] = GetPrivateProfileInt(_T("MOTO"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));
		item = "golden_grgb_" + color[i];
		golden_Spec[i][2] = GetPrivateProfileInt(_T("MOTO"), CA2CT(item.c_str()), 0, CA2CT(INI_Path.c_str()));

		for (int k = 0; k < 3; k++) {
			if (golden_Spec[i][k] != VIVO_AWB[i].Golden[k] && awb_golden_check == 1) {
				ret = ret | 4;
			}
		}
	}

	if ((ret & 4) > 0) {
		FP_log << "awb_golden spec check NG!" << endl;
	}

	//for (int i = 0; i < 1; i++)
	//	for (int j = 0; j < 4; j++) {
	//		if (VIVO_AWB[i].Light[j]>2000) {
	//			if (VIVO_AWB[i].Light[j] < 9700 || VIVO_AWB[i].Light[j]>10300) {
	//				ret = ret | 8;
	//			}
	//		}
	//		else {
	//			if (VIVO_AWB[i].Light[j] < 970 || VIVO_AWB[i].Light[j]>1030) {
	//				ret = ret | 8;
	//			}
	//		}
	//	}
	//if ((ret & 8) > 0) {
	//	FP_log << "Lightsource Coef Data NG!" << endl;
	//}

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


int drift_FP_Check(int shift_Data[2][21], int cnt, double fit_Data[2][21]) {

	int ret = 0, peak_cnt = 0;

	//for (int k = 0; k < 2; k++) {
	//	peak_cnt = 0;
	//	for (int i = 2; i < cnt - 2; i++) {
	//		if (shift_Data[k][i + 1] > shift_Data[k][i] && shift_Data[k][i - 1]>=shift_Data[k][i])
	//			peak_cnt++;
	//		if (shift_Data[k][i + 1] < shift_Data[k][i] && shift_Data[k][i - 1] <= shift_Data[k][i])
	//			peak_cnt++;
	//	}
	//	if (peak_cnt > 1)
	//		ret = ret | 1;
	//}
	
	czy::Fit fitX, fitY;
	vector<double> vecX1, vecY1, vecNX, vecNY;

	for (int i = 0; i < 21; i++) {
		bool useX = 0, useY = 0;
		if (i > 2 && i < 19) {
			useX = 1;
			useY = 1;
		}
		else {
#if 0
			if (shift_Data[0][0]>shift_Data[0][11]) {
				if (i<3 && shift_Data[0][i]>shift_Data[0][i + 1])
					useX = 1;
				if (i > 17 && shift_Data[0][i] > shift_Data[0][i - 1])
					useX = 1;
			}
			else {
				if (i < 3 && shift_Data[0][i] < shift_Data[0][i + 1])
					useX = 1;
				if (i > 17 && shift_Data[0][i] < shift_Data[0][i - 1])
					useX = 1;
			}
			if (shift_Data[1][0] > shift_Data[1][11]) {
				if (i<3 && shift_Data[1][i]>shift_Data[1][i + 1])
					useY = 1;
				if (i > 17 && shift_Data[1][i] > shift_Data[1][i - 1])
					useY = 1;
			}
			else {
				if (i < 3 && shift_Data[1][i] < shift_Data[1][i + 1])
					useY = 1;
				if (i > 17 && shift_Data[1][i] < shift_Data[1][i - 1])
					useY = 1;
			}
#endif
		}
		if (useX) {
			vecX1.push_back(shift_Data[0][i]);
			vecNX.push_back(i);
		}
		if (useY) {
			vecY1.push_back(shift_Data[1][i]);
			vecNY.push_back(i);
		}
	}

	fitX.polyfit(vecNX, vecX1, 2);
	fitY.polyfit(vecNY, vecY1, 2);
	
	for (int i = 0; i < 21; i++) {
		fit_Data[0][i] = fitX.getY(i);
		fit_Data[1][i] = fitY.getY(i);
		double diff_X = fit_Data[0][i] - shift_Data[0][i];
		double diff_Y = fit_Data[1][i] - shift_Data[1][i];

		if (i > 2&&i<19)
		if(diff_X>Drift_Spec_X || diff_Y>Drift_Spec_Y)
			ret = ret | 4;
	}

	if (ret) {
		FP_log << "----------- Shift Cal FP Check Result----------" << endl;
		FP_log << "shift Data has two Peak NG!" << endl;
	}

	return ret;
}


#define N 21
#define flatTH 1

static int upper_limit[N] = { 300, 300, 300,  60,  60,   50,   30,   20,   20,   10,   20,   20,   30,   30,   30,   40,   50,   60,   100,  100,  100 };
static int lower_limit[N] = { -60, -30, -30, -60, -100, -100, -100, -100, -130, -130, -130, -130, -130, -130, -130, -130, -130, -130, -130, -130, -130 };

int checkDiffNum(int data[21]) {
	int count = 0;
	int diff[N - 1] = { 0 };
	for (int j = 0; j < N - 2; j++) {
		diff[j] = data[j + 1] - data[j];
		if (abs(diff[j])<flatTH) {
			diff[j] = 0;
		}
		if (abs(diff[j]) > 40) {
			count = 10;
		}
	}

	for (int j = 0; j < N - 2; j++) {
		if (diff[j] * diff[j + 1] < 0)
			count++;
	}

	// cout << count << endl;
	return count;
}

bool check_clipping_limit(int data[21]) {
	int out_of_bounds_count = 0;

	for (int i = 0; i < N; i++) {
		// 判断当前数据是否超出上下限
		if (data[i] < lower_limit[i] || data[i] > upper_limit[i]) {
			out_of_bounds_count++;
			// 若已超过 1 个，提前返回 fail
			if (out_of_bounds_count >= 1) {
				return false;  // fail
			}
		}
	}

	return true;  // success：最多只有 1 个越界
}


int Xiaomi_Drift_Check(int X[21], int Y[21]) {

	int ret_X = 0, ret_Y = 0;

	if ((checkDiffNum(X) >= 7) || (!check_clipping_limit(X)))ret_X = 1;
	if ((checkDiffNum(Y) >= 7) || (!check_clipping_limit(Y)))ret_Y = 1;

	return ret_X * 10 + ret_Y;

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
	if (type == 2|| type == 4|| type>10) {
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

	if (type == 11) {
		
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 19; j++) {
				int d = PDgainLeft[i][j] - PDgainLeft[i][j + 1];
				if (abs(d)>MTK_LR_ratio_diff) {
					ret |= 4;
					FP_log << "MTK_LR_Ratio Diff :" << d << endl;
				}

				d = PDgainRight[i][j] - PDgainRight[i][j + 1];
				if (abs(d)>MTK_UD_ratio_diff) {
					ret |= 4;
					FP_log << "MTK_UD_Ratio Diff :" << d << endl;
				}
			}
	
	}
	if (type == 12) {

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 19; j++) {
				int d = PDgainLeft[i][j] - PDgainLeft[i][j + 1];
				if (abs(d)>MTK_LR_diff) {
					ret |= 4;

				}

				d = PDgainRight[i][j] - PDgainRight[i][j + 1];
				if (abs(d)>MTK_LR_diff) {
					ret |= 4;

				}
			}
			for (int j = 0; j < 9; j++) {
				int d = PDgainLeft[i][j] - PDgainLeft[i][j + 1];
				if (d>MTK_LR_reverse) {
					ret |= 4;

				}

				d = PDgainRight[i][j] - PDgainRight[i][j + 1];
				if (d>MTK_LR_reverse) {
					ret |= 4;

				}
			}
			for (int j = 19; j >10; j--) {
				int d = PDgainLeft[i][j] - PDgainLeft[i][j - 1];
				if (d>MTK_LR_reverse) {
					ret |= 4;

				}

				d = PDgainRight[i][j] - PDgainRight[i][j - 1];
				if (d>MTK_LR_reverse) {
					ret |= 4;

				}
			}
		}

	}

	if (type == 13) {

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 19; j++) {
				int d = PDgainLeft[i][j] - PDgainLeft[i][j + 1];
				if (abs(d)>MTK_UD_diff) {
					ret |= 4;

				}

				d = PDgainRight[i][j] - PDgainRight[i][j + 1];
				if (abs(d)>MTK_UD_diff) {
					ret |= 4;

				}
			}
			for (int j = 0; j < 9; j++) {
				int d = PDgainLeft[i][j] - PDgainLeft[i][j + 1];
				if (d>MTK_UD_diff) {
					ret |= 4;

				}

				d = PDgainRight[i][j] - PDgainRight[i][j + 1];
				if (d>MTK_UD_diff) {
					ret |= 4;

				}
			}
			for (int j = 19; j >10; j--) {
				int d = PDgainLeft[i][j] - PDgainLeft[i][j - 1];
				if (d>MTK_UD_diff) {
					ret |= 4;

				}

				d = PDgainRight[i][j] - PDgainRight[i][j - 1];
				if (d>MTK_UD_diff) {
					ret |= 4;

				}
			}
		}
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
				FP_log << "DCC value: " << y / x << " (" << diff_Spec << ")" << endl;
			}
			if (type == 0) {
				if (DCC[i][j]<PDAF_Data.DCC_min || DCC[i][j]>PDAF_Data.DCC_max) {
					FP_log << "DCC value: " << DCC[i][j]<<" (" << PDAF_Data.DCC_min << "~" << PDAF_Data.DCC_max<< ")"<<endl;
					ret |= 1;
				}
				if (j==W-2&&(DCC[i][j + 1]<PDAF_Data.DCC_min || DCC[i][j + 1]>PDAF_Data.DCC_max)) {
					FP_log << "DCC value: " << DCC[i][j+1] << " (" << PDAF_Data.DCC_min << "~" << PDAF_Data.DCC_max << ")" << endl;
					ret |= 1;
				}
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
