#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QDialog>
#include <QString>
RESTORE_COMPILER_WARNINGS

class QVBoxLayout;

class CAboutDialog final : public QDialog
{
public:
	explicit CAboutDialog(QWidget *parent);
	explicit CAboutDialog(const QString& versionString, QWidget *parent, const QString& inceptionYear = QLatin1String(__DATE__).right(4), const QString& copyrightOwner = "Violet Giraffe");

	// Widgets added here appear between the version and the copyright lines.
	[[nodiscard]] QVBoxLayout& customContentLayout();

private:
	QVBoxLayout* _customContentLayout = nullptr;
};
