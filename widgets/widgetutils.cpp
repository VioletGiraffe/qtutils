#include "widgetutils.h"
#include "utils/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QLayout>
#include <QWidget>
RESTORE_COMPILER_WARNINGS

void WidgetUtils::setLayoutVisible(QLayout* layout, bool visible)
{
	if (!layout)
		return;

	for (int i = 0; i < layout->count(); ++i)
	{
		QWidget * widget = layout->itemAt(i)->widget();
		if (widget)
			widget->setVisible(visible);
		else
			setLayoutVisible(layout->itemAt(i)->layout(), visible);
	}
}
