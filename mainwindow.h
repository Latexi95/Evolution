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
	private:
		Ui::MainWindow *ui;
		quint64 mTicks;
		QTimer *mUpdateTimer;
		QAction *mRunningAction;
		QAction *mShowLongestByteCode;
		QAction *mShowOldestByteCode;

		QAction *mShowFoodLevels;
		QAction *mShowEntities;
};

#endif // MAINWINDOW_H
