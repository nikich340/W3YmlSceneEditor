#include "MainWindow.h"

#include <QApplication>
#include <fstream>
#include <QTextStream>

static std::ofstream logFile;
static bool logDebug = true; // TODO: set false in release build!
void logMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	switch (type) {
		case QtDebugMsg:
			if (logDebug)
				logFile  << "[Debug] " << msg.toStdString() << "\n";
			break;
		case QtInfoMsg:
			logFile  << "[Info] " << msg.toStdString() << "\n";
			break;
		case QtWarningMsg:
			logFile  << "[Warning] " << msg.toStdString() << "\n";
			break;
		case QtCriticalMsg:
			logFile  << "[Critical] " << msg.toStdString() << "\n";
			break;
		case QtFatalMsg:
			logFile  << "[Fatal] " << msg.toStdString() << "\n";
			abort();
	}
	if (logDebug)
		logFile.flush();
}

int main(int argc, char *argv[])
{
	logFile.open("log_ymlSceneEditor.txt", std::ofstream::out | std::ofstream::trunc);
	logFile.sync_with_stdio(0); logFile.tie(nullptr);
	if (argc > 1 && !strncmp(argv[1], "--debug", 7) ) {
		logDebug = true;
	}
	//qInstallMessageHandler(logMessageOutput);

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();

	int ret = a.exec();
	logFile.close();
	return ret;
}
