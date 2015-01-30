#ifndef BYTECODEDIALOG_H
#define BYTECODEDIALOG_H

#include <QDialog>

namespace Ui {
class ByteCodeDialog;
}

class Entity;
class ByteCodeDialog : public QDialog {
		Q_OBJECT

	public:
		explicit ByteCodeDialog(Entity *entity, QWidget *parent = 0);
		~ByteCodeDialog();

	protected:
		void changeEvent(QEvent *e);

	private:
		Ui::ByteCodeDialog *ui;
		Entity *mEntity;
};

#endif // BYTECODEDIALOG_H
