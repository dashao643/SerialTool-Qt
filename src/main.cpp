#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// Use Fusion style for consistent QSS rendering across platforms
	// a.setStyle(QStyleFactory::create("Fusion"));

	// Load and apply stylesheet
	QFile styleFile(":/style.qss");
	if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
		a.setStyleSheet(styleFile.readAll());
		styleFile.close();
	}

	MainWindow w;
	w.show();
	return a.exec();
}
