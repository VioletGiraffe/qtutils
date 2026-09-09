#include "reportbugdialog.h"
#include "logger/cloggerinterface.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QLabel>
#include <QObject>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextCursor>
#include <QUrl>
#include <QVBoxLayout>
RESTORE_COMPILER_WARNINGS

void ReportBugDialog::show(QWidget* parent, const CLoggerInterface& logger, const QUrl& issueTracker)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(QObject::tr("Report a bug"));
	dialog.resize(750, 520); // The log view hints at nothing useful, so the window size is chosen rather than derived

	QVBoxLayout* layout = new QVBoxLayout(&dialog);

	QLabel* instructions = new QLabel(QObject::tr("Please describe the problem on the issue tracker. The application log below may help - "
		"select and copy any relevant lines into your report."), &dialog);
	instructions->setWordWrap(true);
	layout->addWidget(instructions);

	QPlainTextEdit* logView = new QPlainTextEdit(&dialog);
	logView->setReadOnly(true);
	logView->setLineWrapMode(QPlainTextEdit::NoWrap);
	logView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	logView->setPlainText(logger.contents().join('\n'));
	logView->moveCursor(QTextCursor::End); // Reveal the most recent entries
	layout->addWidget(logView, 1);

	QDialogButtonBox* buttons = new QDialogButtonBox(&dialog);
	QPushButton* openTracker = buttons->addButton(QObject::tr("Open issue tracker"), QDialogButtonBox::ActionRole);
	buttons->addButton(QDialogButtonBox::Close);
	layout->addWidget(buttons);

	QObject::connect(openTracker, &QPushButton::clicked, &dialog, [issueTracker] {
		QDesktopServices::openUrl(issueTracker);
	});
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	dialog.exec();
}
