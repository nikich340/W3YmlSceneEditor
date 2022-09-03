#include "MainWindow.h"

#include <QApplication>
#include <fstream>
#include <QTextStream>
#include <QDateTime>

static std::ofstream logFile;
//static QFile logFile;
//static QTextStream logStream;
static bool debug_build = true; // TODO: set false in release build!

void logMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	switch (type) {
		case QtDebugMsg:
			if (debug_build)
				logFile  << "[Debug] " << msg.toStdString() << "\n";
			break;
		case QtInfoMsg:
			logFile  << "[Info] " << msg.toStdString() << "\n";
			break;
		case QtWarningMsg:
			logFile  << "[Warning] " << msg.toStdString() << "\n";
			break;
		case QtCriticalMsg:
			logFile << "[Critical] " << msg.toStdString() << "\n";
			break;
		case QtFatalMsg:
			logFile  << "[Fatal] " << msg.toStdString() << "\n";
			abort();
	}
	logFile.flush();
}

int main(int argc, char *argv[])
{
	if (argc > 1 && !strncmp(argv[1], "--debug", 7) ) {
		debug_build = true;
	}
	if ( !debug_build ) {
		logFile.open("log_ymlSceneEditor.txt", std::ofstream::out | std::ofstream::trunc);
		logFile << "--- RUN: " << QDateTime::currentDateTime().toString().toStdString() << " ---\n";
		qInstallMessageHandler(logMessageOutput);
	}

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    MainWindow window;
    window.showMaximized();

    int ret = app.exec();
	logFile.close();
	return ret;
}
