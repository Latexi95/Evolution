#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "map.h"
#include "entity.h"
#include "entityupdatetask.h"
#include "action.h"
#include <QThreadPool>
#include <QTimer>
#include <QComboBox>
#include <map>
#include "bytecodedialog.h"
#include "worker.h"



MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	mWorker(0) {
	ui->setupUi(this);

	QThreadPool::globalInstance()->setExpiryTimeout(-1);

	Map *map = new Map(QImage("map.png"));
	map->randomFillMapWithEntities(50);
	ui->mapViewWidget->setMap(map);
	mRunningAction = ui->mainToolBar->addAction(tr("Running"));
	mRunningAction->setCheckable(true);
	mRunningAction->setChecked(false);
	ui->mapViewWidget->setCurrentImage(map->draw());
	connect(mRunningAction, &QAction::toggled, [this](bool toggled) {
		if (toggled && !mWorker) {
			mShowLongestByteCode->setEnabled(false);
			mShowOldestByteCode->setEnabled(false);
			mSave->setEnabled(false);
			mLoad->setEnabled(false);
			mWorker = new Worker(ui->mapViewWidget->map());
			connect(mWorker, &Worker::finished, this, &MainWindow::updateStopped, Qt::QueuedConnection);
			connect(mWorker, &Worker::drawFinished, ui->mapViewWidget, &MapViewWidget::setCurrentImage, Qt::QueuedConnection);
			connect(mWorker, &Worker::workResults, this, &MainWindow::showResults, Qt::QueuedConnection);
			QThreadPool::globalInstance()->start(mWorker);
		}
		else if (!toggled && mWorker){
			mWorker->stop();
		}
	});

	mShowLongestByteCode = ui->mainToolBar->addAction(tr("Show longest bytecode"));
	connect(mShowLongestByteCode, &QAction::triggered, this, &MainWindow::showLongestByteCode);
	mShowLongestByteCode->setEnabled(true);

	mShowOldestByteCode = ui->mainToolBar->addAction(tr("Show oldest bytecode"));
	connect(mShowOldestByteCode, &QAction::triggered, this, &MainWindow::showOldestByteCode);
	mShowOldestByteCode->setEnabled(true);

	mDrawing = ui->mainToolBar->addAction(tr("Drawing"));
	mDrawing->setCheckable(true);
	mDrawing->setChecked(true);

	connect(mDrawing, &QAction::toggled, [this](bool drawing) {
		ui->mapViewWidget->setDrawing(drawing);
		drawIfNotRunning();
	});

	mDrawModeR = new QComboBox();
	mDrawModeG = new QComboBox();
	mDrawModeB = new QComboBox();
	addDrawModeItems(mDrawModeR);
	addDrawModeItems(mDrawModeG);
	addDrawModeItems(mDrawModeB);

	ui->mainToolBar->addWidget(mDrawModeR);
	ui->mainToolBar->addWidget(mDrawModeG);
	ui->mainToolBar->addWidget(mDrawModeB);

	mDrawModeR->setCurrentIndex(1);
	mDrawModeG->setCurrentIndex(2);
	mDrawModeB->setCurrentIndex(3);
	connect(mDrawModeR, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			[this](int index) {
		ui->mapViewWidget->map()->setDrawModeR(index);
		drawIfNotRunning();
	});
	connect(mDrawModeG, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			[this](int index) {
		ui->mapViewWidget->map()->setDrawModeG(index);
		drawIfNotRunning();
	});
	connect(mDrawModeB, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			[this](int index) {
		ui->mapViewWidget->map()->setDrawModeB(index);
		drawIfNotRunning();
	});

	mSave = ui->mainToolBar->addAction(tr("Save"));
	mSave->setEnabled(true);
	connect(mSave, &QAction::triggered, this, &MainWindow::save);

	mLoad = ui->mainToolBar->addAction(tr("Load"));
	mLoad->setEnabled(true);
	connect(mLoad, &QAction::triggered, this, &MainWindow::load);


}

MainWindow::~MainWindow() {
	delete ui;
}

void MainWindow::changeEvent(QEvent *e) {
	QMainWindow::changeEvent(e);
	switch (e->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}


void MainWindow::showLongestByteCode() {
	Entity *longestByteCodeEntity = 0;
	int longestByteCode = -1;
	for (Entity *e : ui->mapViewWidget->map()->entities()) {
		if (e->byteCode().size() > longestByteCode) {
			longestByteCode = e->byteCode().size();
			longestByteCodeEntity = e;
		}
	}
	if (longestByteCodeEntity) {
		ByteCodeDialog dialog(longestByteCodeEntity);
		dialog.setWindowTitle(QString::number(longestByteCode));
		dialog.exec();
	}
}

void MainWindow::showOldestByteCode() {
	Entity *entity = 0;
	quint64 oldest = 0;
	for (Entity *e : ui->mapViewWidget->map()->entities()) {
		if (e->lifeTime() > oldest) {
			oldest = e->byteCode().size();
			entity = e;
		}
	}
	if (entity) {
		ByteCodeDialog dialog(entity);
		dialog.setWindowTitle(QString::number(oldest));
		dialog.exec();
	}
}

void MainWindow::save() {
	Map *map = ui->mapViewWidget->map();
	map->save("save.sav");
}

void MainWindow::load() {
	Map *oldMap = ui->mapViewWidget->map();
	delete oldMap;
	Map *newMap = new Map();
	newMap->load("save.sav");
	ui->mapViewWidget->setMap(newMap);
	drawIfNotRunning();
}

void MainWindow::updateStopped() {
	delete mWorker;
	mWorker = 0;
	mShowLongestByteCode->setEnabled(true);
	mShowOldestByteCode->setEnabled(true);
	mSave->setEnabled(true);
	mLoad->setEnabled(true);
}

void MainWindow::showResults(const WorkResults &results) {
	ui->statusBar->showMessage(tr("%1  : Entities: %2   Tasks: %3   Generation %4    Timings: %5, %6  (%7%)")
							   .arg(results.mTicks)
							   .arg(results.mEntities)
							   .arg(results.mTaskSize)
							   .arg(results.mGeneration)
							   .arg(results.mExecutionTime)
							   .arg(results.mTotalTime)
							   .arg(results.mTotalTime ? (results.mExecutionTime * 100 / results.mTotalTime) : 0));
}

void MainWindow::addDrawModeItems(QComboBox *comboBox) {
	comboBox->addItem(tr("None"));
	comboBox->addItem(tr("Entity [Energy]"));
	comboBox->addItem(tr("Food [V]"));
	comboBox->addItem(tr("Food [M]"));
	comboBox->addItem(tr("Water level"));
	comboBox->addItem(tr("Food [V] generation"));
	comboBox->addItem(tr("Water generation"));
}

void MainWindow::closeEvent(QCloseEvent *e) {
	if (mWorker) {
		mWorker->stop();
	}
}

void MainWindow::drawIfNotRunning() {
	if (!mWorker) {
		ui->mapViewWidget->setCurrentImage(ui->mapViewWidget->map()->draw());
	}
}
