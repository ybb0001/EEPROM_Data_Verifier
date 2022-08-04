#include "EEPROM_Data_Verifier.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	EEPROM_Data_Verifier w;
	w.show();
	return a.exec();
}
