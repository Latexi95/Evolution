#include "bytecodedialog.h"
#include "ui_bytecodedialog.h"
#include "entity.h"

ByteCodeDialog::ByteCodeDialog(Entity *entity, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ByteCodeDialog),
	mEntity(entity)
{
	ui->setupUi(this);
	ui->textBrowser->setText(mEntity->byteCodeAsString());
}

ByteCodeDialog::~ByteCodeDialog() {
	delete ui;
}

void ByteCodeDialog::changeEvent(QEvent *e) {
	QDialog::changeEvent(e);
	switch (e->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}
