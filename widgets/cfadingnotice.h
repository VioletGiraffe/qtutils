#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QWidget>
RESTORE_COMPILER_WARNINGS

class QVariantAnimation;

// A one-line message over part of its parent, enlarged, that holds for a moment and then fades out, or stays until hidden.
// Painted in the content's inverse colours, Base lettering on a Text fill, so it stands out over a text view.
// Transparent to the mouse.
class CFadingNotice final : public QWidget
{
public:
	enum class Duration {
		Brief,      // Holds for a moment, then fades out
		UntilHidden // Stays at full opacity until hide()
	};

	explicit CFadingNotice(QWidget* parent);

	// Centred in 'area', given in the parent's coordinates. Replaces a message that is showing, and restarts a Brief one's hold and fade.
	void showMessage(const QString& text, const QRect& area, Duration duration = Duration::Brief);

protected:
	void paintEvent(QPaintEvent*) override;

private:
	QVariantAnimation* _opacityAnimation = nullptr;
	QString _text;
};
