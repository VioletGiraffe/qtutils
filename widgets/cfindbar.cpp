#include "cfindbar.h"
#include "chistorycombobox.h"

DISABLE_COMPILER_WARNINGS
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QToolButton>
RESTORE_COMPILER_WARNINGS

#include <utility>

CFindBar::CFindBar(FindText findText, FindRegex findRegex, const Keys& keys, QString settingsRootKey, QWidget* parent) :
	QFrame(parent),
	_findText{ std::move(findText) },
	_findRegex{ std::move(findRegex) },
	_settingsRootKey{ std::move(settingsRootKey) }
{
	const auto createAction = [this](const QString& text, const QKeySequence& key, auto onTriggered) {
		auto* action = new QAction{ text, this };
		action->setShortcut(key);
		connect(action, &QAction::triggered, this, onTriggered);
		return action;
	};
	_findAction = createAction(tr("Find"), keys.find, [this] { activate(); });
	_findNextAction = createAction(tr("Find next"), keys.findNext, [this] { findMatch(false); });
	_findPreviousAction = createAction(tr("Find previous"), keys.findPrevious, [this] { findMatch(true); });

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(8, 4, 8, 4);

	_patternBox = new CHistoryComboBox{ this };
	_patternBox->setCompleter(nullptr);
	_patternBox->lineEdit()->setPlaceholderText(tr("Find"));
	_patternBox->lineEdit()->setClearButtonEnabled(true);
	layout->addWidget(_patternBox, 1);

	const auto addFindButton = [&](const QString& text, QAction* action) {
		auto* button = new QPushButton{ text };
		button->setToolTip(action->shortcut().toString(QKeySequence::NativeText));
		button->setFocusPolicy(Qt::NoFocus); // the pattern field keeps the keyboard
		connect(button, &QPushButton::clicked, action, &QAction::trigger);
		layout->addWidget(button);
	};
	addFindButton(tr("Previous"), _findPreviousAction);
	addFindButton(tr("Next"), _findNextAction);

	_statusLabel = new QLabel;

	const auto addOptionBox = [&](const QString& text, const QString& settingName) {
		auto* box = new QCheckBox{ text };
		box->setFocusPolicy(Qt::NoFocus); // the pattern field keeps the keyboard; the mnemonics toggle the boxes
		connect(box, &QCheckBox::toggled, _statusLabel, &QLabel::clear);
		if (!_settingsRootKey.isEmpty())
		{
			const QString settingKey = _settingsRootKey + '/' + settingName;
			box->setChecked(QSettings{}.value(settingKey).toBool());
			connect(box, &QCheckBox::toggled, this, [settingKey](bool checked) { QSettings{}.setValue(settingKey, checked); });
		}
		layout->addWidget(box);
		return box;
	};
	_caseSensitiveBox = addOptionBox(tr("Match &case"), QStringLiteral("CaseSensitive"));
	_wholeWordsBox = addOptionBox(tr("&Whole words"), QStringLiteral("WholeWords"));
	_regexBox = addOptionBox(tr("Re&gex"), QStringLiteral("Regex"));

	layout->addWidget(_statusLabel);

	auto* closeButton = new QToolButton;
	closeButton->setText(QStringLiteral("✕"));
	closeButton->setToolTip(tr("Close (Esc)"));
	closeButton->setAutoRaise(true);
	closeButton->setFocusPolicy(Qt::NoFocus);
	connect(closeButton, &QToolButton::clicked, this, &CFindBar::deactivate);
	layout->addWidget(closeButton);

	if (!_settingsRootKey.isEmpty())
		_patternBox->enableAutoSave(_settingsRootKey + QStringLiteral("/Expressions"));

	connect(_patternBox, &CHistoryComboBox::itemActivated, this, [this](const QString&, Qt::KeyboardModifiers modifiers) {
		findMatch(modifiers.testFlag(Qt::ShiftModifier));
	});
	connect(_patternBox, &QComboBox::editTextChanged, _statusLabel, &QLabel::clear);

	hide();
}

QList<QAction*> CFindBar::findActions() const
{
	return { _findAction, _findNextAction, _findPreviousAction };
}

void CFindBar::activate()
{
	if (QWidget* const focused = QApplication::focusWidget(); focused && !isAncestorOf(focused))
		_focusBeforeActivation = focused;

	show();
	_patternBox->setFocus(Qt::ShortcutFocusReason);
	_patternBox->lineEdit()->selectAll();
}

bool CFindBar::eventFilter(QObject* watched, QEvent* event)
{
	const QEvent::Type type = event->type();
	if ((type != QEvent::ShortcutOverride && type != QEvent::KeyPress) || static_cast<QKeyEvent*>(event)->key() != Qt::Key_Escape || !isVisible())
		return QFrame::eventFilter(watched, event);

	// An accepted ShortcutOverride keeps the key from the window's shortcuts and delivers it as a KeyPress
	if (type == QEvent::KeyPress)
		deactivate();

	event->accept();
	return true;
}

void CFindBar::showEvent(QShowEvent* event)
{
	QFrame::showEvent(event);

	// Key events the focus widget and its parents ignore reach the window; installing again only moves the filter to the front
	window()->installEventFilter(this);
}

void CFindBar::deactivate()
{
	if (_focusBeforeActivation && isAncestorOf(QApplication::focusWidget()))
		_focusBeforeActivation->setFocus();
	hide();
}

void CFindBar::findMatch(bool backward)
{
	const QString pattern = _patternBox->currentText();
	if (pattern.isEmpty())
	{
		activate();
		return;
	}

	// Before the search: the status reports the result, and showing the bar shrinks the view the match gets scrolled into
	show();

	// Before the search: rebuilding the history rewrites the pattern text, which clears the status
	_patternBox->moveCurrentTextToTopOfHistory();

	QTextDocument::FindFlags flags;
	flags.setFlag(QTextDocument::FindBackward, backward);
	flags.setFlag(QTextDocument::FindCaseSensitively, _caseSensitiveBox->isChecked());
	flags.setFlag(QTextDocument::FindWholeWords, _wholeWordsBox->isChecked());

	FindResult result = FindResult::NotFound;
	if (_regexBox->isChecked())
	{
		const QRegularExpression regex{ pattern };
		if (!regex.isValid())
		{
			_statusLabel->setText(tr("Invalid pattern: %1").arg(regex.errorString()));
			return;
		}
		result = _findRegex(regex, flags);
	}
	else
		result = _findText(pattern, flags);

	switch (result)
	{
	case FindResult::NotFound:
		_statusLabel->setText(tr("Not found"));
		break;
	case FindResult::Found:
		_statusLabel->clear();
		break;
	case FindResult::FoundAfterWrapAround:
		_statusLabel->setText(backward ? tr("Continued from the bottom") : tr("Continued from the top"));
		break;
	}
}
