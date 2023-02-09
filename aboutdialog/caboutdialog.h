#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QDialog>
RESTORE_COMPILER_WARNINGS

namespace Ui {
class CAboutDialog;
}

class CAboutDialog final : public QDialog
{
public:
	explicit CAboutDialog(QWidget *parent);
	explicit CAboutDialog(const QString& versionString, QWidget *parent, const QString& inceptionYear = QLatin1String(__DATE__).right(4), const QString& copyrightOwner = "Violet Giraffe");
	~CAboutDialog() override;

private:
	Ui::CAboutDialog *ui;
};
