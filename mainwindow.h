#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "worker.h"

namespace Ui {
class MainWindow;
}
class QComboBox;
class MainWindow : public QMainWindow {
		Q_OBJECT
	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	protected:
		void changeEvent(QEvent *e);

	private slots:
		void showLongestByteCode();
		void showOldestByteCode();
		void save();
		void load();

		void updateStopped();
		void showResults(const WorkResults &results);

		void mapClicked(QPoint mapPoint);
	private:
		void addDrawModeItems(QComboBox *comboBox);
		void closeEvent(QCloseEvent *e);

		void drawIfNotRunning();

		Ui::MainWindow *ui;
		QTimer *mAutoSave;
		QAction *mRunningAction;
		QAction *mShowLongestByteCode;
		QAction *mShowOldestByteCode;

		QComboBox *mDrawModeR;
		QComboBox *mDrawModeG;
		QComboBox *mDrawModeB;
		QAction *mSave;
		QAction *mLoad;
		QAction *mDrawing;
		Worker *mWorker;
};

#endif // MAINWINDOW_H
