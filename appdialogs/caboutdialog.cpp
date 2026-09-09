#include "caboutdialog.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QDialogButtonBox>
#include <QFont>
#include <QLabel>
#include <QStringBuilder>
#include <QVBoxLayout>
#include <QWidget>
RESTORE_COMPILER_WARNINGS

namespace {

// Multipliers of the inherited UI font, so both lines follow the system font size.
constexpr qreal PROGRAM_NAME_FONT_SCALE = 1.75;
constexpr qreal COPYRIGHT_FONT_SCALE = 0.8;

void scaleFont(QWidget* widget, qreal factor)
{
	QFont font = widget->font();
	font.setPointSizeF(font.pointSizeF() * factor);
	widget->setFont(font);
}

}

CAboutDialog::CAboutDialog(QWidget* parent) : CAboutDialog(QString{}, parent)
{
}

CAboutDialog::CAboutDialog(const QString& versionString, QWidget *parent, const QString& inceptionYear, const QString& copyrightOwner) :
	QDialog(parent)
{
	const QString displayName = QApplication::applicationDisplayName();
	setWindowTitle("About " + displayName);

	QLabel* programName = new QLabel(displayName, this);
	programName->setAlignment(Qt::AlignCenter);
	scaleFont(programName, PROGRAM_NAME_FONT_SCALE);

	const QString versionText = versionString.isEmpty()
		? tr("Built on %1 at %2\nUsing Qt version %3").arg(__DATE__, __TIME__, QT_VERSION_STR)
		: tr("Version %1 (%2 %3)\nUsing Qt version %4").arg(versionString, __DATE__, __TIME__, QT_VERSION_STR);

	QLabel* version = new QLabel(versionText, this);
	version->setAlignment(Qt::AlignCenter);

	QLabel* copyright = new QLabel("©" % inceptionYear % ' ' % copyrightOwner, this);
	copyright->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	scaleFont(copyright, COPYRIGHT_FONT_SCALE);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	// A nested layout otherwise takes the style's margins, indenting custom content relative to the dialog's own.
	_customContentLayout = new QVBoxLayout;
	_customContentLayout->setContentsMargins(0, 0, 0, 0);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(programName);
	layout->addSpacing(20);
	layout->addWidget(version);
	layout->addLayout(_customContentLayout, 1); // takes the dialog's spare height: a gap when empty, room to grow when filled
	layout->addSpacing(33);
	layout->addWidget(copyright);
	layout->addWidget(buttons);

	// No resize(): an unresized widget takes its layout's size hint on first show, after the caller fills customContentLayout().
}

QVBoxLayout& CAboutDialog::customContentLayout()
{
	return *_customContentLayout;
}
