#include "cfindbar.h"
#include "clightningfastviewer.h"
#include "clineedit.h"

DISABLE_COMPILER_WARNINGS
#include <QCheckBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QShortcut>
#include <QTextDocument>
RESTORE_COMPILER_WARNINGS

CFindBar::CFindBar(CLightningFastViewerWidget* viewer, QWidget* parent) :
	QFrame(parent),
	_viewer{ viewer }
{
	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(8, 4, 8, 4);

	_patternEdit = new CLineEdit;
	_patternEdit->setSelectAllOnFocus(false); // a click places the caret; activate() selects the pattern
	_patternEdit->setPlaceholderText(tr("Find"));
	_patternEdit->setClearButtonEnabled(true);
	layout->addWidget(_patternEdit, 1);

	const auto addFindButton = [&](const QString& text, QKeySequence::StandardKey key, bool backward) {
		auto* button = new QPushButton{ text };
		button->setToolTip(QKeySequence{ key }.toString(QKeySequence::NativeText));
		button->setFocusPolicy(Qt::NoFocus); // the pattern field keeps the keyboard
		connect(button, &QPushButton::clicked, this, [this, backward] { findMatch(backward); });
		layout->addWidget(button);
	};
	addFindButton(tr("Previous"), QKeySequence::FindPrevious, true);
	addFindButton(tr("Next"), QKeySequence::FindNext, false);

	_caseSensitiveBox = new QCheckBox{ tr("Match &case") };
	_wholeWordsBox = new QCheckBox{ tr("&Whole words") };
	_regexBox = new QCheckBox{ tr("Re&gex") };
	for (QCheckBox* box : { _caseSensitiveBox, _wholeWordsBox, _regexBox })
	{
		box->setFocusPolicy(Qt::NoFocus); // the pattern field keeps the keyboard; the mnemonics toggle the boxes
		connect(box, &QCheckBox::toggled, this, [this] { _statusLabel->clear(); });
		layout->addWidget(box);
	}

	_statusLabel = new QLabel;
	layout->addWidget(_statusLabel);

	connect(_patternEdit, &CLineEdit::returnPressedWithModifiers, this, [this](Qt::KeyboardModifiers modifiers) {
		findMatch(modifiers.testFlag(Qt::ShiftModifier));
	});
	connect(_patternEdit, &QLineEdit::textChanged, _statusLabel, &QLabel::clear);

	// Parented to the viewer: a shortcut on a hidden widget never fires, and the bar stays hidden until Find
	new QShortcut{ QKeySequence::Find, viewer, this, [this] { activate(); } };
	new QShortcut{ QKeySequence::FindNext, viewer, this, [this] { findMatch(false); } };
	new QShortcut{ QKeySequence::FindPrevious, viewer, this, [this] { findMatch(true); } };

	hide();
}

void CFindBar::activate()
{
	show();
	_patternEdit->setFocus(Qt::ShortcutFocusReason);
	_patternEdit->selectAll();
}

void CFindBar::keyPressEvent(QKeyEvent* event)
{
	if (event->key() != Qt::Key_Escape)
	{
		QFrame::keyPressEvent(event);
		return;
	}

	hide();
	_viewer->setFocus();
}

void CFindBar::findMatch(bool backward)
{
	const QString pattern = _patternEdit->text();
	if (pattern.isEmpty())
	{
		activate();
		return;
	}

	QTextDocument::FindFlags flags;
	flags.setFlag(QTextDocument::FindBackward, backward);
	flags.setFlag(QTextDocument::FindCaseSensitively, _caseSensitiveBox->isChecked());
	flags.setFlag(QTextDocument::FindWholeWords, _wholeWordsBox->isChecked());

	bool found = false;
	if (_regexBox->isChecked())
	{
		const QRegularExpression regex{ pattern };
		if (!regex.isValid())
		{
			_statusLabel->setText(tr("Invalid pattern: %1").arg(regex.errorString()));
			return;
		}
		found = _viewer->find(regex, flags, /*wrapAround=*/true);
	}
	else
		found = _viewer->find(pattern, flags, /*wrapAround=*/true);

	_statusLabel->setText(found ? QString{} : tr("Not found"));
}
