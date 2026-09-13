#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

class QWidget;

// Multi-line report of what determines a widget's colours, for tracking down an unexpected one:
//   - the style chain down to the base style, stylesheet sizes, the colour scheme, the Qt version, the widget's state and background attributes;
//   - every palette role in the Active, Inactive and Disabled groups, marking the explicitly set ones;
//   - the application palette for the widget's class, where it differs;
//   - each ancestor's Base, Window, explicitly set roles and stylesheet size;
//   - pixels sampled on three rows, from grab() (what the widget paints) and from the screen (what is shown).
// Paints the widget synchronously through grab(): never call it from a paint event.
[[nodiscard]] QString widgetColorReport(QWidget& widget);
