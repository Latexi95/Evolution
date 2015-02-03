#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "map.h"
#include "entity.h"
#include "entityupdatetask.h"
#include "action.h"
#include <QThreadPool>
#include <QTimer>
#include <map>
#include "bytecodedialog.h"

const int entitiesPerTask = 1500;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow) {
	ui->setupUi(this);
	mUpdateTimer = new QTimer(this);
	connect(mUpdateTimer, &QTimer::timeout, this, &MainWindow::updateSimulation);
	mUpdateTimer->start(0);

	QThreadPool::globalInstance()->setExpiryTimeout(-1);

	Map *map = new Map(QImage("map.png"));
	map->randomFillMapWithEntities(50);
	ui->mapViewWidget->setMap(map);
	mRunningAction = ui->mainToolBar->addAction(tr("Running"));
	mRunningAction->setCheckable(true);
	mRunningAction->setChecked(true);
	connect(mRunningAction, &QAction::toggled, [this](bool toggled) {
		if (toggled) {
			mUpdateTimer->start(0);
			mShowLongestByteCode->setEnabled(false);
			mShowOldestByteCode->setEnabled(false);
			mSave->setEnabled(false);
			mLoad->setEnabled(false);
		}
		else {
			mUpdateTimer->stop();
			mShowLongestByteCode->setEnabled(true);
			mShowOldestByteCode->setEnabled(true);
			mSave->setEnabled(true);
			mLoad->setEnabled(true);
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

	mSave = ui->mainToolBar->addAction(tr("Save"));
	mSave->setEnabled(false);
	connect(mSave, &QAction::triggered, this, &MainWindow::save);

	mLoad = ui->mainToolBar->addAction(tr("Load"));
	mLoad->setEnabled(false);
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

void MainWindow::updateSimulation() {
	Map *map = ui->mapViewWidget->map();
	if (!map || !mRunningAction->isChecked()) return;
	map->updateFoodLevels();
	QVector<Entity*> taskData;
	QVector<EntityUpdateTask*> tasks;
	quint64 generation = 0;
	for (Entity *e : map->entities()) {
		if (e->generation() > generation) generation = e->generation();
		taskData.append(e);
		if (taskData.size() == entitiesPerTask) {
			EntityUpdateTask *task = new EntityUpdateTask(map, taskData);
			tasks.append(task);
#ifndef DEBUG
			QThreadPool::globalInstance()->start(task);
#endif
			taskData.clear();
		}
	}


	if (!taskData.isEmpty()) {
		EntityUpdateTask *task = new EntityUpdateTask(map, taskData);
		tasks.append(task);
#ifndef DEBUG
		QThreadPool::globalInstance()->start(task);
#endif
	}

#ifdef DEBUG
	for (EntityUpdateTask *task : tasks) task->run();
#else
	QThreadPool::globalInstance()->waitForDone();
#endif



	std::multimap<EntityProperty::ValueType, Action*> speedSortedActions;
	for (EntityUpdateTask *task : tasks) {
		for (Action *action : task->actions()) {
			speedSortedActions.insert(std::pair<EntityProperty::ValueType, Action*>(action->speed().value(), action));
		}
		delete task;
	}

	for (auto it = speedSortedActions.begin(); it != speedSortedActions.end(); ++it) {
		EntityProperty result = it->second->exec(map);
		it->second->entity()->reportActionResult(result);
		delete it->second;
	}

	map->deletePass();
	if (map->entities().size() < 5000) {
		for (int i = 0; i < 30; i++) {
			map->createNewEntity();
		}
	}

	if (map->tick() % 5000 == 0) {
		map->save("autosave_" + QString::number(map->tick() / 5000));
	}

	ui->statusBar->showMessage(tr("%1  : Entities: %2   Tasks: %3   Generation %4").arg(map->tick()).arg(map->entities().size()).arg(tasks.size()).arg(generation));
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
}
