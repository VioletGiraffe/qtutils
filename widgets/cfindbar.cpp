#include "cfindbar.h"
#include "chistorycombobox.h"
#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QSettings>
#include <QToolButton>
RESTORE_COMPILER_WARNINGS

#include <chrono>
#include <utility>

CFindBar::CFindBar(HostFunctions hostFunctions, const Keys& keys, QString settingsRootKey, QWidget* parent) :
	QFrame(parent),
	_hostFunctions{ std::move(hostFunctions) },
	_settingsRootKey{ std::move(settingsRootKey) }
{
	assert_r(_hostFunctions.findText && _hostFunctions.findRegex);

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
	layout->setSpacing(2);

	_patternBox = new CHistoryComboBox{ this };
	_patternBox->setCompleter(nullptr);
	_patternBox->lineEdit()->setPlaceholderText(tr("Find"));
	_patternBox->lineEdit()->setClearButtonEnabled(true);
	layout->addWidget(_patternBox, 1);

	const auto addToolButton = [&](const QString& glyph, const QString& toolTip) {
		auto* button = new QToolButton;
		button->setText(glyph);
		button->setToolTip(toolTip);
		button->setAutoRaise(true);
		button->setFocusPolicy(Qt::NoFocus); // the pattern field keeps the keyboard
		layout->addWidget(button);
		return button;
	};

	const auto addFindButton = [&](const QString& glyph, QAction* action) {
		const QString key = action->shortcut().toString(QKeySequence::NativeText);
		auto* button = addToolButton(glyph, key.isEmpty() ? action->text() : tr("%1 (%2)").arg(action->text(), key));
		connect(button, &QToolButton::clicked, action, &QAction::trigger);
	};
	addFindButton(QStringLiteral("˄"), _findPreviousAction);
	addFindButton(QStringLiteral("˅"), _findNextAction);

	_countLabel = new QLabel;
	layout->addWidget(_countLabel);

	_statusLabel = new QLabel;

	const auto addOptionBox = [&](const QString& text, const QString& settingName, bool checkedByDefault = false) {
		auto* box = new QCheckBox{ text };
		box->setFocusPolicy(Qt::NoFocus); // the pattern field keeps the keyboard; the mnemonics toggle the boxes
		box->setChecked(checkedByDefault);
		if (!_settingsRootKey.isEmpty())
		{
			const QString settingKey = _settingsRootKey + '/' + settingName;
			box->setChecked(QSettings{}.value(settingKey, checkedByDefault).toBool());
			connect(box, &QCheckBox::toggled, this, [settingKey](bool checked) { QSettings{}.setValue(settingKey, checked); });
		}
		layout->addWidget(box);
		return box;
	};
	_caseSensitiveBox = addOptionBox(tr("Match &case"), QStringLiteral("CaseSensitive"));
	_wholeWordsBox = addOptionBox(tr("&Whole words"), QStringLiteral("WholeWords"));
	_regexBox = addOptionBox(tr("Re&gex"), QStringLiteral("Regex"));
	_highlightAllBox = addOptionBox(tr("&Highlight all"), QStringLiteral("HighlightAll"), /*checkedByDefault=*/true);

	for (QCheckBox* const box : { _caseSensitiveBox, _wholeWordsBox, _regexBox })
		connect(box, &QCheckBox::toggled, this, &CFindBar::clearStatus);

	connect(_highlightAllBox, &QCheckBox::toggled, this, [this] {
		if (!_countLabel->text().isEmpty()) // A shown count belongs to the pattern and options in the bar
			updateMatchCount();
	});

	layout->addWidget(_statusLabel);

	auto* const closeButton = addToolButton(QStringLiteral("✕"), tr("Close (Esc)"));
	connect(closeButton, &QToolButton::clicked, this, &CFindBar::deactivate);

	if (!_settingsRootKey.isEmpty())
		_patternBox->enableAutoSave(_settingsRootKey + QStringLiteral("/Expressions"));

	connect(_patternBox, &CHistoryComboBox::itemActivated, this, [this](const QString&, Qt::KeyboardModifiers modifiers) {
		findMatch(modifiers.testFlag(Qt::ShiftModifier));
	});
	connect(_patternBox, &QComboBox::editTextChanged, this, &CFindBar::clearStatus);

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

void CFindBar::clearStatus()
{
	_countLabel->clear();
	_statusLabel->clear();
	if (_hostFunctions.clearHighlights)
		_hostFunctions.clearHighlights();
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
	clearStatus();
	hide();
}

static constexpr std::chrono::milliseconds matchCountingBudget{ 1000 };

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

	_countLabel->clear();

	const bool isRegex = _regexBox->isChecked();
	const QRegularExpression regex{ isRegex ? pattern : QString{} };
	if (isRegex && !regex.isValid())
	{
		_statusLabel->setText(tr("Invalid pattern: %1").arg(regex.errorString()));
		return;
	}

	QTextDocument::FindFlags flags = optionFlags();
	flags.setFlag(QTextDocument::FindBackward, backward);
	const FindResult result = isRegex ? _hostFunctions.findRegex(regex, flags) : _hostFunctions.findText(pattern, flags);

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

	if (result != FindResult::NotFound)
		updateMatchCount();
}

void CFindBar::updateMatchCount()
{
	const QString pattern = _patternBox->currentText();
	const QDeadlineTimer deadline{ matchCountingBudget };
	const bool highlight = _highlightAllBox->isChecked();

	MatchCount count;
	if (_regexBox->isChecked() && _hostFunctions.countRegex)
		count = _hostFunctions.countRegex(QRegularExpression{ pattern }, optionFlags(), deadline, highlight);
	else if (!_regexBox->isChecked() && _hostFunctions.countText)
		count = _hostFunctions.countText(pattern, optionFlags(), deadline, highlight);
	else
		return;

	const QString number = count.number > 0 ? QString::number(count.number) : QStringLiteral("?");
	_countLabel->setText(QStringLiteral("%1/%2%3").arg(number, QString::number(count.total), count.complete ? QString{} : QStringLiteral("+")));
}

QTextDocument::FindFlags CFindBar::optionFlags() const
{
	QTextDocument::FindFlags flags;
	flags.setFlag(QTextDocument::FindCaseSensitively, _caseSensitiveBox->isChecked());
	flags.setFlag(QTextDocument::FindWholeWords, _wholeWordsBox->isChecked());
	return flags;
}
