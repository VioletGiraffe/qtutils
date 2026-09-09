#pragma once

class CLoggerInterface;

class QUrl;
class QWidget;

namespace ReportBugDialog {

// Shows `logger`'s contents for the user to select and copy into a report, plus a button opening `issueTracker`.
// The log holds only what the application has fed the logger, typically through an installed Qt message handler.
void show(QWidget* parent, const CLoggerInterface& logger, const QUrl& issueTracker);

}
