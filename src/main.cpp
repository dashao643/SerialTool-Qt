#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setApplicationName("dashao");
	a.setApplicationVersion(APP_VERSION);

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
