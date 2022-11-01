#include "MainWindow.h"

#include <QApplication>
#include <fstream>
#include <QElapsedTimer>
#include <QTextStream>
#include <QDateTime>
#define VERSION "v0.1"

static std::ofstream logFile;
static bool debug_build = true; // TODO: set false in release build!

void logMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
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
void splashAppendMessage(const QApplication& app, QSplashScreen& splash, const QString& message) {
    splash.showMessage(splash.message() + "\n\t\t\t" + message, Qt::AlignLeft | Qt::AlignTop, QColorConstants::Svg::crimson);
    app.processEvents();
}

int main(int argc, char *argv[])
{
    QElapsedTimer timer;
    timer.start();
	if (argc > 1 && !strncmp(argv[1], "--debug", 7) ) {
		debug_build = true;
	}
	if ( !debug_build ) {
        logFile.open("log_YmlSceneEditor.txt", std::ofstream::out | std::ofstream::trunc);
		logFile << "--- RUN: " << QDateTime::currentDateTime().toString().toStdString() << " ---\n";
		qInstallMessageHandler(logMessageOutput);
	}

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    QPixmap splashPix = QPixmap(":/splash_screen.jpg").scaled(app.primaryScreen()->geometry().size() / 2.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QSplashScreen splash(splashPix, Qt::WindowStaysOnTopHint);
    splash.setFont(QFont("Comic Sans MS", 12));
    splash.show();
    splash.showMessage(QString("\tYML Scene Editor for Radish\n\t\t\t\t\t\tby @nikich340 [build %1 (%2)]").arg(VERSION).arg(__DATE__), Qt::AlignLeft | Qt::AlignTop, QColorConstants::Svg::crimson);
    splashAppendMessage(app, splash, "Loading UI...");

    MainWindow window;
    splashAppendMessage(app, splash, "Loading YML repositories...");
    window.loadYmlRepos();
    splashAppendMessage(app, splash, "Loading CSV dialog lines...");
    window.loadCsvLines();
    splashAppendMessage(app, splash, "READY!");
    window.showMaximized();
    splash.finish(&window);
    window.print_info(QString("*** <b>READY!</b> [Total load time: %1 ms] ***").arg(timer.elapsed()));

    int ret = app.exec();
	logFile.close();
	return ret;
}
