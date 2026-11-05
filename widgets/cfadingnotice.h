#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QWidget>
RESTORE_COMPILER_WARNINGS

class QVariantAnimation;

// A one-line message over part of its parent, enlarged, that holds for a moment and then fades out.
// Painted in the content's inverse colours, Base lettering on a Text fill, so it stands out over a text view.
// Transparent to the mouse.
class CFadingNotice final : public QWidget
{
public:
	explicit CFadingNotice(QWidget* parent);

	// Centred in 'area', given in the parent's coordinates; restarts the hold and fade if a message is showing
	void showMessage(const QString& text, const QRect& area);

protected:
	void paintEvent(QPaintEvent*) override;

private:
	QVariantAnimation* _opacityAnimation = nullptr;
	QString _text;
};
