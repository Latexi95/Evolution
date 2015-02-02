#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
		Q_OBJECT
	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	protected:
		void changeEvent(QEvent *e);

	private slots:
		void updateSimulation();
		void showLongestByteCode();
		void showOldestByteCode();
		void save();
		void load();
	private:
		Ui::MainWindow *ui;
		QTimer *mUpdateTimer;
		QTimer *mAutoSave;
		QAction *mRunningAction;
		QAction *mShowLongestByteCode;
		QAction *mShowOldestByteCode;

		QAction *mShowFoodLevels;
		QAction *mShowEntities;
		QAction *mSave;
		QAction *mLoad;

		int mSaveNum;
};

#endif // MAINWINDOW_H
