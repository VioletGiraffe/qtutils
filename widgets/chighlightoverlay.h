#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QWidget>
RESTORE_COMPILER_WARNINGS

// A frame and tint over the whole of its parent, in the palette's accent colour: marks that widget as the
// target of something pending, a drag hovering it being the case this was written for.
// Transparent to the mouse, so childAt() skips it and the parent it marks stays the target.
class CHighlightOverlay final : public QWidget
{
public:
	explicit CHighlightOverlay(QWidget* parent, int cornerRadius = 4);

	// Sized to the parent and raised over the children it gained after this one
	void showOverParent();

protected:
	void paintEvent(QPaintEvent*) override;

private:
	const int _cornerRadius;
};
