#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "map.h"
#include "entity.h"
#include "entityupdatetask.h"
#include "action.h"
#include <QThreadPool>
#include <QTimer>
#include "bytecodedialog.h"

const int entitiesPerTask = 2000;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow) {
	ui->setupUi(this);
	mUpdateTimer = new QTimer(this);
	connect(mUpdateTimer, &QTimer::timeout, this, &MainWindow::updateSimulation);
	mUpdateTimer->start(0);


	Map *map = new Map(500, 300);
	map->randomFillMapWithEntities(50);
	ui->mapViewWidget->setMap(map);
	mTicks = 0;
	mRunningAction = ui->mainToolBar->addAction(tr("Running"));
	mRunningAction->setCheckable(true);
	mRunningAction->setChecked(true);
	connect(mRunningAction, &QAction::toggled, [this](bool toggled) {
		if (toggled) {
			mUpdateTimer->start(0);
			mShowLongestByteCode->setEnabled(false);
			mShowOldestByteCode->setEnabled(false);
		}
		else {
			mUpdateTimer->stop();
			mShowLongestByteCode->setEnabled(true);
			mShowOldestByteCode->setEnabled(true);
		}
	});

	mShowLongestByteCode = ui->mainToolBar->addAction(tr("Show longest bytecode"));
	connect(mShowLongestByteCode, &QAction::triggered, this, &MainWindow::showLongestByteCode);
	mShowLongestByteCode->setEnabled(false);

	mShowOldestByteCode = ui->mainToolBar->addAction(tr("Show oldest bytecode"));
	connect(mShowOldestByteCode, &QAction::triggered, this, &MainWindow::showOldestByteCode);
	mShowOldestByteCode->setEnabled(false);


	mShowFoodLevels = ui->mainToolBar->addAction(tr("Show food levels"));
	mShowFoodLevels->setCheckable(true);
	mShowFoodLevels->setChecked(true);
	connect(mShowFoodLevels, &QAction::toggled, [this](bool toggled) {
		ui->mapViewWidget->setDrawFlags(ui->mapViewWidget->drawFlags() ^ (NoMFoodLevels | NoVFoodLevels));
	});

	mShowEntities = ui->mainToolBar->addAction(tr("Show entities"));
	mShowEntities->setCheckable(true);
	mShowEntities->setChecked(true);
	connect(mShowEntities, &QAction::toggled, [this](bool toggled) {
		ui->mapViewWidget->setDrawFlags(ui->mapViewWidget->drawFlags() ^ NoEntities);
	});

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

void MainWindow::updateSimulation() {
	Map *map = ui->mapViewWidget->map();
	if (!map || !mRunningAction->isChecked()) return;
	map->updateFoodLevels();
	QVector<Entity*> taskData;
	QVector<EntityUpdateTask*> tasks;
	for (Entity *e : map->entities()) {
		taskData.append(e);
		if (taskData.size() == entitiesPerTask) {
			EntityUpdateTask *task = new EntityUpdateTask(map, taskData);
			tasks.append(task);
			QThreadPool::globalInstance()->start(task);
			taskData.clear();
		}
	}
	if (!taskData.isEmpty()) {
		EntityUpdateTask *task = new EntityUpdateTask(map, taskData);
		tasks.append(task);
		QThreadPool::globalInstance()->start(task);
	}

	QThreadPool::globalInstance()->waitForDone();
	for (EntityUpdateTask *task : tasks) {
		for (Action *action : task->actions()) {
			EntityProperty result = action->exec(map);
			action->entity()->reportActionResult(result);
			delete action;
		}
		delete task;
	}
	map->deletePass();
	for (int i = 0; i < 50; i++) {
		map->createNewEntity();
	}
	mTicks++;
	ui->statusBar->showMessage(tr("%1  : Entities: %2   Tasks: %3").arg(mTicks).arg(map->entities().size()).arg(tasks.size()));
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
