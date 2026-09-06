#include "cwidgetgallery.h"

#include "theme/cthemecontroller.h"

DISABLE_COMPILER_WARNINGS
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QCommandLinkButton>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDial>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFontComboBox>
#include <QFontDialog>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QKeySequence>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QStringList>
#include <QStyle>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QToolBox>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>
RESTORE_COMPILER_WARNINGS

#include <initializer_list>
#include <utility>

namespace {

constexpr int VerticalSampleHeight = 90;
constexpr int ItemViewHeight = 130;
constexpr int TextEditHeight = 60;
constexpr int PageHeight = 110;
constexpr int SplitterHeight = 48;
constexpr int FrameSampleWidth = 90;
constexpr int StandardIconSize = 20;
constexpr int DialSize = 64;
constexpr int SliderWidth = 220;
constexpr int LinkedValueMaximum = 100;
constexpr int StatusProgressWidth = 120;
constexpr int WindowWidth = 1100;
constexpr int WindowHeight = 800;

// Left-aligned samples on one line, each at its natural size
[[nodiscard]] QWidget* row(std::initializer_list<QWidget*> widgets)
{
	auto* container = new QWidget;
	auto* layout = new QHBoxLayout{ container };
	layout->setContentsMargins(0, 0, 0, 0);
	for (QWidget* widget : widgets)
		layout->addWidget(widget);
	layout->addStretch();
	return container;
}

[[nodiscard]] QGroupBox* section(const QString& title, std::initializer_list<QWidget*> rows)
{
	auto* box = new QGroupBox{ title };
	auto* layout = new QVBoxLayout{ box };
	for (QWidget* r : rows)
		layout->addWidget(r);
	return box;
}

// Returning the argument keeps a one-off state change inline in the sample list.
// checked() requires an already checkable button: QAbstractButton::setChecked does nothing otherwise.
template <typename W>
[[nodiscard]] W* checkable(W* widget) { widget->setCheckable(true); return widget; }

template <typename W>
[[nodiscard]] W* checked(W* widget) { widget->setChecked(true); return widget; }

template <typename W>
[[nodiscard]] W* disabled(W* widget) { widget->setEnabled(false); return widget; }

[[nodiscard]] QIcon standardIcon(QStyle::StandardPixmap pixmap) { return QApplication::style()->standardIcon(pixmap); }

// Drives the application's own theming, not just Qt's palette: CThemeController::themeChanged is what an app
// rebuilds its stylesheet on, so a QSS-driven look follows the switch here as it does in the app's preferences.
[[nodiscard]] QWidget* schemeStrip()
{
	const struct { QString name; Qt::ColorScheme scheme; } choices[] = {
		{ QStringLiteral("System"), Qt::ColorScheme::Unknown },
		{ QStringLiteral("Light"), Qt::ColorScheme::Light },
		{ QStringLiteral("Dark"), Qt::ColorScheme::Dark }
	};

	auto* strip = new QWidget;
	auto* layout = new QHBoxLayout{ strip };
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(new QLabel{ QStringLiteral("Colour scheme:") });

	const Qt::ColorScheme preference = CThemeController::instance().schemePreference();
	for (const auto& choice : choices)
	{
		auto* button = new QRadioButton{ choice.name };
		button->setChecked(choice.scheme == preference); // before the connection, so the initial state is not re-applied
		QObject::connect(button, &QRadioButton::toggled, button, [scheme = choice.scheme](bool selected) {
			if (selected)
				CThemeController::instance().setSchemePreference(scheme);
		});
		layout->addWidget(button);
	}
	layout->addStretch();
	return strip;
}

// Every menu construct a style draws separately: icon, checkmark, separator, disabled entry, submenu arrow
[[nodiscard]] QMenu* sampleMenu(QWidget* menuParent)
{
	auto* menu = new QMenu{ menuParent };
	menu->addAction(standardIcon(QStyle::SP_DialogOpenButton), QStringLiteral("Item with icon"));
	QAction* checkableItem = menu->addAction(QStringLiteral("Checked item"));
	checkableItem->setCheckable(true);
	checkableItem->setChecked(true);
	menu->addSeparator();
	menu->addAction(QStringLiteral("Disabled item"))->setEnabled(false);
	menu->addMenu(QStringLiteral("Submenu"))->addAction(QStringLiteral("Nested item"));
	return menu;
}

// A tool button is icon-only by default, so its text never reaches the screen and the tooltip is what tells
// one sample from the next
[[nodiscard]] QToolButton* toolButton(const QString& text, const QString& tooltip,
	QStyle::StandardPixmap pixmap = QStyle::SP_DirIcon)
{
	auto* button = new QToolButton;
	button->setText(text);
	button->setToolTip(tooltip);
	button->setIcon(standardIcon(pixmap));
	return button;
}

[[nodiscard]] QToolButton* toolButtonWithMenu(const QString& text, const QString& tooltip,
	QToolButton::ToolButtonPopupMode popupMode)
{
	QToolButton* button = toolButton(text, tooltip, QStyle::SP_ArrowDown);
	button->setMenu(sampleMenu(button));
	button->setPopupMode(popupMode);
	return button;
}

[[nodiscard]] QGroupBox* buttonsSection()
{
	auto* defaultButton = new QPushButton{ QStringLiteral("Default") };
	defaultButton->setDefault(true);

	auto* flat = new QPushButton{ QStringLiteral("Flat") };
	flat->setFlat(true);

	auto* withMenu = new QPushButton{ QStringLiteral("With menu") };
	withMenu->setMenu(sampleMenu(withMenu));

	auto* autoRaise = toolButton(QStringLiteral("Auto-raise"),
		QStringLiteral("QToolButton with autoRaise: no frame until the mouse is over it"));
	autoRaise->setAutoRaise(true);

	auto* textBesideIcon = toolButton(QStringLiteral("Text beside icon"),
		QStringLiteral("QToolButton with ToolButtonTextBesideIcon: the only one here that shows its text"));
	textBesideIcon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	return section(QStringLiteral("Buttons"), {
		row({ new QPushButton{ QStringLiteral("Push") }, defaultButton, flat,
			checked(checkable(new QPushButton{ QStringLiteral("Checked") })),
			disabled(new QPushButton{ QStringLiteral("Disabled") }) }),
		row({ new QPushButton{ standardIcon(QStyle::SP_DialogSaveButton), QStringLiteral("With icon") }, withMenu }),
		row({ toolButton(QStringLiteral("Tool"), QStringLiteral("QToolButton in its default state")), autoRaise,
			checked(checkable(toolButton(QStringLiteral("Checked"), QStringLiteral("QToolButton, checkable and checked")))),
			disabled(toolButton(QStringLiteral("Disabled"), QStringLiteral("QToolButton, disabled"))) }),
		row({ textBesideIcon,
			toolButtonWithMenu(QStringLiteral("Instant"), QStringLiteral("InstantPopup: pressing anywhere opens the menu"),
				QToolButton::InstantPopup),
			toolButtonWithMenu(QStringLiteral("Split"), QStringLiteral("MenuButtonPopup: only the arrow opens the menu"),
				QToolButton::MenuButtonPopup) }),
		new QCommandLinkButton{ QStringLiteral("Command link"), QStringLiteral("With its descriptive second line") }
	});
}

[[nodiscard]] QGroupBox* choicesSection()
{
	auto* tristate = new QCheckBox{ QStringLiteral("Partially checked") };
	tristate->setTristate(true);
	tristate->setCheckState(Qt::PartiallyChecked);

	// One row is one parent widget, so these radio buttons are exclusive among themselves
	return section(QStringLiteral("Choices"), {
		row({ new QCheckBox{ QStringLiteral("Unchecked") }, checked(new QCheckBox{ QStringLiteral("Checked") }), tristate }),
		row({ disabled(new QCheckBox{ QStringLiteral("Disabled") }),
			disabled(checked(new QCheckBox{ QStringLiteral("Disabled, checked") })) }),
		row({ checked(new QRadioButton{ QStringLiteral("Selected") }), new QRadioButton{ QStringLiteral("Unselected") },
			disabled(new QRadioButton{ QStringLiteral("Disabled") }) })
	});
}

[[nodiscard]] QGroupBox* textEntrySection()
{
	auto* placeholder = new QLineEdit;
	placeholder->setPlaceholderText(QStringLiteral("Placeholder"));
	placeholder->setClearButtonEnabled(true);

	auto* readOnly = new QLineEdit{ QStringLiteral("Read-only") };
	readOnly->setReadOnly(true);

	auto* password = new QLineEdit{ QStringLiteral("secret") };
	password->setEchoMode(QLineEdit::Password);

	// An inline action is a button inside the field's frame, positioned by the style
	auto* withInlineAction = new QLineEdit{ QStringLiteral("Inline action") };
	withInlineAction->addAction(standardIcon(QStyle::SP_FileDialogContentsView), QLineEdit::TrailingPosition);

	auto* richText = new QTextEdit;
	richText->setHtml(QStringLiteral("<b>QTextEdit</b> with <i>rich</i> text and a <a href=\"#\">link</a>."));
	richText->setFixedHeight(TextEditHeight);

	auto* plainText = new QPlainTextEdit{ QStringLiteral("QPlainTextEdit\nSecond line") };
	plainText->setFixedHeight(TextEditHeight);

	return section(QStringLiteral("Text entry"), {
		row({ new QLineEdit{ QStringLiteral("Editable") }, placeholder, readOnly }),
		row({ password, withInlineAction, disabled(new QLineEdit{ QStringLiteral("Disabled") }) }),
		row({ richText, plainText }),
		row({ new QKeySequenceEdit{ QKeySequence{ QKeySequence::Copy } } })
	});
}

[[nodiscard]] QGroupBox* numbersAndPickersSection()
{
	auto* spin = new QSpinBox;
	spin->setRange(0, 100);
	spin->setValue(42);
	spin->setSuffix(QStringLiteral(" px"));

	auto* doubleSpin = new QDoubleSpinBox;
	doubleSpin->setValue(3.14);

	auto* combo = new QComboBox;
	combo->addItems({ QStringLiteral("First item"), QStringLiteral("Second item"), QStringLiteral("Third item") });

	auto* editableCombo = new QComboBox;
	editableCombo->setEditable(true);
	editableCombo->addItems({ QStringLiteral("Editable"), QStringLiteral("Another") });

	auto* date = new QDateEdit{ QDate::currentDate() };
	date->setCalendarPopup(true); // the popup is a whole calendar widget, styled apart from the field

	return section(QStringLiteral("Numbers and pickers"), {
		row({ spin, doubleSpin, disabled(new QSpinBox) }),
		row({ combo, editableCombo, disabled(new QComboBox) }),
		row({ new QFontComboBox }),
		row({ date, new QTimeEdit{ QTime::currentTime() }, new QDateTimeEdit{ QDateTime::currentDateTime() } })
	});
}

[[nodiscard]] QGroupBox* rangesSection()
{
	auto* slider = new QSlider{ Qt::Horizontal };
	slider->setRange(0, LinkedValueMaximum);
	slider->setMinimumWidth(SliderWidth); // its size hint is too narrow to drag through the whole range
	slider->setTickPosition(QSlider::TicksBelow);
	slider->setTickInterval(10);

	auto* dial = new QDial;
	dial->setRange(0, LinkedValueMaximum);
	dial->setNotchesVisible(true);
	dial->setFixedSize(DialSize, DialSize);

	auto* linkedSpin = new QSpinBox;
	linkedSpin->setRange(0, LinkedValueMaximum);
	linkedSpin->setSuffix(QStringLiteral(" %"));

	auto* progress = new QProgressBar;
	progress->setRange(0, LinkedValueMaximum);

	// One value behind four controls: what a style does to a handle, a spin box's text and a chunk while they
	// move is what a static page cannot show. setValue emits nothing when the value is unchanged, so no loop.
	QObject::connect(slider, &QSlider::valueChanged, dial, &QDial::setValue);
	QObject::connect(slider, &QSlider::valueChanged, linkedSpin, &QSpinBox::setValue);
	QObject::connect(slider, &QSlider::valueChanged, progress, &QProgressBar::setValue);
	QObject::connect(dial, &QDial::valueChanged, slider, &QSlider::setValue);
	QObject::connect(linkedSpin, &QSpinBox::valueChanged, slider, &QSlider::setValue);
	slider->setValue(40); // after the connections, so the other three start in step

	auto* verticalSlider = new QSlider{ Qt::Vertical };
	verticalSlider->setValue(60);
	verticalSlider->setFixedHeight(VerticalSampleHeight);

	auto* scrollBar = new QScrollBar{ Qt::Vertical };
	scrollBar->setFixedHeight(VerticalSampleHeight);

	auto* withoutText = new QProgressBar;
	withoutText->setRange(0, LinkedValueMaximum);
	withoutText->setValue(30);
	withoutText->setTextVisible(false);

	// On demand: a bar left permanently indeterminate animates for the whole session
	auto* indeterminate = new QCheckBox{ QStringLiteral("Indeterminate") };
	QObject::connect(indeterminate, &QCheckBox::toggled, withoutText, [withoutText](bool busy) {
		withoutText->setRange(0, busy ? 0 : LinkedValueMaximum);
		if (!busy)
			withoutText->setValue(30); // a zero range clamped the value away
	});

	auto* verticalProgress = new QProgressBar;
	verticalProgress->setOrientation(Qt::Vertical);
	verticalProgress->setValue(50);
	verticalProgress->setFixedHeight(VerticalSampleHeight);

	auto* disabledSlider = disabled(new QSlider{ Qt::Horizontal });
	disabledSlider->setMinimumWidth(SliderWidth);
	disabledSlider->setValue(40); // the same position as the live one, so the two differ only in state

	return section(QStringLiteral("Ranges and progress"), {
		row({ slider, linkedSpin }),
		progress, // straight into the section, so the bar spans its width
		row({ disabledSlider }),
		row({ verticalSlider, dial, scrollBar, verticalProgress }),
		row({ withoutText, indeterminate })
	});
}

[[nodiscard]] QGroupBox* itemViewsSection()
{
	auto* list = new QListWidget;
	list->addItems({ QStringLiteral("Plain item"), QStringLiteral("Selected item"), QStringLiteral("Checkable item"),
		QStringLiteral("Disabled item") });
	list->setAlternatingRowColors(true);
	list->setCurrentRow(1);
	list->item(2)->setCheckState(Qt::Checked);
	list->item(3)->setFlags(Qt::NoItemFlags);
	list->setFixedHeight(ItemViewHeight);

	auto* tree = new QTreeWidget;
	tree->setColumnCount(2);
	tree->setHeaderLabels({ QStringLiteral("Name"), QStringLiteral("Value") });
	tree->setAlternatingRowColors(true);
	for (int branchIndex = 1; branchIndex <= 2; ++branchIndex)
	{
		auto* branch = new QTreeWidgetItem(tree, QStringList{ QStringLiteral("Branch %1").arg(branchIndex), QStringLiteral("...") });
		for (int leafIndex = 1; leafIndex <= 2; ++leafIndex)
			new QTreeWidgetItem(branch, QStringList{ QStringLiteral("Leaf %1").arg(leafIndex), QStringLiteral("value") });
	}
	tree->expandAll();
	tree->setCurrentItem(tree->topLevelItem(0)->child(0));
	tree->setFixedHeight(ItemViewHeight);

	auto* table = new QTableWidget{ 3, 3 };
	table->setHorizontalHeaderLabels({ QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C") });
	table->horizontalHeader()->setStretchLastSection(true);
	table->setAlternatingRowColors(true);
	for (int rowIndex = 0; rowIndex < table->rowCount(); ++rowIndex)
	{
		for (int columnIndex = 0; columnIndex < table->columnCount(); ++columnIndex)
		{
			table->setItem(rowIndex, columnIndex,
				new QTableWidgetItem{ QStringLiteral("R%1C%2").arg(rowIndex + 1).arg(columnIndex + 1) });
		}
	}
	table->setCurrentCell(1, 1);
	table->setFixedHeight(ItemViewHeight);

	return section(QStringLiteral("Item views"), { list, tree, table });
}

[[nodiscard]] QLabel* framedLabel(const QString& text, QFrame::Shape shape = QFrame::StyledPanel,
	QFrame::Shadow shadow = QFrame::Plain)
{
	auto* label = new QLabel{ text };
	label->setFrameShape(shape);
	label->setFrameShadow(shadow);
	label->setAlignment(Qt::AlignCenter);
	label->setMinimumWidth(FrameSampleWidth);
	return label;
}

[[nodiscard]] QFrame* verticalLine()
{
	auto* line = new QFrame;
	line->setFrameShape(QFrame::VLine);
	line->setFrameShadow(QFrame::Sunken);
	return line;
}

[[nodiscard]] QGroupBox* containersSection()
{
	auto* tabs = new QTabWidget;
	tabs->addTab(new QLabel{ QStringLiteral("First page") }, standardIcon(QStyle::SP_ComputerIcon), QStringLiteral("With icon"));
	tabs->addTab(new QLabel{ QStringLiteral("Second page") }, QStringLiteral("Second"));
	tabs->addTab(new QLabel{ QStringLiteral("Third page") }, QStringLiteral("Disabled"));
	tabs->setTabEnabled(2, false);
	tabs->setTabsClosable(true); // the per-tab close button is a styled subcontrol of its own
	tabs->setMovable(true);
	tabs->setFixedHeight(PageHeight);
	// Deleting the page removes its tab; removeTab alone would leave the page widget behind
	QObject::connect(tabs, &QTabWidget::tabCloseRequested, tabs, [tabs](int index) { delete tabs->widget(index); });

	auto* toolBox = new QToolBox;
	toolBox->addItem(new QLabel{ QStringLiteral("First page") }, QStringLiteral("First page"));
	toolBox->addItem(new QLabel{ QStringLiteral("Second page") }, QStringLiteral("Second page"));
	toolBox->setFixedHeight(PageHeight);

	auto* checkableGroup = new QGroupBox{ QStringLiteral("Checkable group box") };
	checkableGroup->setCheckable(true);
	auto* checkableGroupLayout = new QVBoxLayout{ checkableGroup };
	checkableGroupLayout->addWidget(new QLabel{ QStringLiteral("Unchecking disables the contents") });

	// QSplitterHandle does not enable WA_Hover itself, so a QSS handle:hover rule is inert without a fixup
	auto* splitter = new QSplitter{ Qt::Horizontal };
	splitter->addWidget(framedLabel(QStringLiteral("Left")));
	splitter->addWidget(framedLabel(QStringLiteral("Right")));
	splitter->setFixedHeight(SplitterHeight);

	return section(QStringLiteral("Containers"), {
		tabs,
		toolBox,
		checkableGroup,
		row({ framedLabel(QStringLiteral("StyledPanel")), framedLabel(QStringLiteral("Box sunken"), QFrame::Box, QFrame::Sunken),
			framedLabel(QStringLiteral("Panel raised"), QFrame::Panel, QFrame::Raised), verticalLine() }),
		splitter
	});
}

[[nodiscard]] QGroupBox* labelsSection()
{
	// The tooltip names the enumerator: which standard pixmaps a style actually supplies is the question here
	const struct { QStyle::StandardPixmap pixmap; QString name; } standardPixmaps[] = {
		{ QStyle::SP_MessageBoxInformation, QStringLiteral("SP_MessageBoxInformation") },
		{ QStyle::SP_MessageBoxWarning, QStringLiteral("SP_MessageBoxWarning") },
		{ QStyle::SP_MessageBoxCritical, QStringLiteral("SP_MessageBoxCritical") },
		{ QStyle::SP_MessageBoxQuestion, QStringLiteral("SP_MessageBoxQuestion") },
		{ QStyle::SP_DirIcon, QStringLiteral("SP_DirIcon") },
		{ QStyle::SP_FileIcon, QStringLiteral("SP_FileIcon") },
		{ QStyle::SP_TrashIcon, QStringLiteral("SP_TrashIcon") },
		{ QStyle::SP_BrowserReload, QStringLiteral("SP_BrowserReload") },
		{ QStyle::SP_DialogApplyButton, QStringLiteral("SP_DialogApplyButton") }
	};

	auto* selectable = new QLabel{ QStringLiteral("A selectable label, long enough to show how the section wraps") };
	selectable->setWordWrap(true);
	selectable->setTextInteractionFlags(Qt::TextSelectableByMouse);

	auto* link = new QLabel{ QStringLiteral("A <a href=\"#\">link</a> inside rich text") };
	link->setTextInteractionFlags(Qt::TextBrowserInteraction); // openExternalLinks stays off: the anchor is inert

	auto* withTooltip = new QLabel{ QStringLiteral("Hover me for a tooltip") };
	withTooltip->setToolTip(QStringLiteral("A tooltip has palette roles of its own and is easy to leave unstyled"));

	auto* icons = new QWidget;
	auto* iconsLayout = new QHBoxLayout{ icons };
	iconsLayout->setContentsMargins(0, 0, 0, 0);
	for (const auto& standardPixmap : standardPixmaps)
	{
		auto* iconLabel = new QLabel;
		iconLabel->setPixmap(standardIcon(standardPixmap.pixmap).pixmap(StandardIconSize, StandardIconSize));
		iconLabel->setToolTip(standardPixmap.name);
		iconsLayout->addWidget(iconLabel);
	}
	iconsLayout->addStretch();

	return section(QStringLiteral("Labels and icons"), {
		row({ new QLabel{ QStringLiteral("Plain label") }, disabled(new QLabel{ QStringLiteral("Disabled label") }) }),
		selectable,
		row({ link, withTooltip }),
		icons
	});
}

template <typename Handler>
[[nodiscard]] QPushButton* dialogLauncher(QWidget* dialogParent, const QString& text, Handler&& handler)
{
	auto* button = new QPushButton{ text };
	QObject::connect(button, &QPushButton::clicked, dialogParent, std::forward<Handler>(handler));
	return button;
}

// A standard dialog is a styled surface that cannot be shown any other way, so these buttons open theirs
[[nodiscard]] QGroupBox* dialogsSection(QWidget* dialogParent)
{
	auto* information = dialogLauncher(dialogParent, QStringLiteral("Information"), [dialogParent] {
		QMessageBox::information(dialogParent, QStringLiteral("Information"), QStringLiteral("An informational message.")); });
	auto* warning = dialogLauncher(dialogParent, QStringLiteral("Warning"), [dialogParent] {
		QMessageBox::warning(dialogParent, QStringLiteral("Warning"), QStringLiteral("A warning message.")); });
	auto* critical = dialogLauncher(dialogParent, QStringLiteral("Critical"), [dialogParent] {
		QMessageBox::critical(dialogParent, QStringLiteral("Critical"), QStringLiteral("A critical message.")); });
	auto* question = dialogLauncher(dialogParent, QStringLiteral("Question"), [dialogParent] {
		QMessageBox::question(dialogParent, QStringLiteral("Question"), QStringLiteral("A question with Yes and No.")); });

	auto* file = dialogLauncher(dialogParent, QStringLiteral("File"),
		[dialogParent] { QFileDialog::getOpenFileName(dialogParent); });
	auto* colour = dialogLauncher(dialogParent, QStringLiteral("Colour"),
		[dialogParent] { QColorDialog::getColor(Qt::white, dialogParent); });
	auto* font = dialogLauncher(dialogParent, QStringLiteral("Font"),
		[dialogParent] { bool accepted = false; QFontDialog::getFont(&accepted, dialogParent); });
	auto* input = dialogLauncher(dialogParent, QStringLiteral("Input"), [dialogParent] {
		QInputDialog::getText(dialogParent, QStringLiteral("Input"), QStringLiteral("Enter something:")); });

	auto* buttonBox = new QDialogButtonBox{ QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply
		| QDialogButtonBox::Help };

	return section(QStringLiteral("Dialogs"), {
		row({ information, warning, critical, question }),
		row({ file, colour, font, input }),
		buttonBox
	});
}

void addMenus(QMainWindow& window)
{
	QMenuBar* menuBar = window.menuBar();

	QMenu* file = menuBar->addMenu(QStringLiteral("&File"));
	file->addAction(standardIcon(QStyle::SP_FileIcon), QStringLiteral("&New"))->setShortcut(QKeySequence::New);
	file->addAction(standardIcon(QStyle::SP_DialogOpenButton), QStringLiteral("&Open..."))->setShortcut(QKeySequence::Open);
	QMenu* recent = file->addMenu(QStringLiteral("Open &recent"));
	recent->addAction(QStringLiteral("One"));
	recent->addAction(QStringLiteral("Two"));
	file->addSeparator();
	file->addAction(QStringLiteral("E&xit"))->setShortcut(QKeySequence::Quit);

	QMenu* edit = menuBar->addMenu(QStringLiteral("&Edit"));
	edit->addAction(QStringLiteral("&Undo"))->setEnabled(false); // a greyed entry keeps its shortcut text, and is its own drawing case
	edit->addAction(QStringLiteral("&Redo"))->setEnabled(false);
	edit->addSeparator();
	QAction* wordWrap = edit->addAction(QStringLiteral("&Word wrap"));
	wordWrap->setCheckable(true);
	wordWrap->setChecked(true);

	// An exclusive group draws a radio mark where a lone checkable action draws a checkmark
	auto* sizes = new QActionGroup{ &window };
	QMenu* view = menuBar->addMenu(QStringLiteral("&View"));
	for (const QString& name : { QStringLiteral("Small"), QStringLiteral("Medium"), QStringLiteral("Large") })
	{
		QAction* action = view->addAction(name);
		action->setCheckable(true);
		sizes->addAction(action);
	}
	sizes->actions().constFirst()->setChecked(true); // the group holds only these three, whatever else the menu gains

	menuBar->addMenu(QStringLiteral("&Help"))->addAction(QStringLiteral("&About"));
}

void addMainToolBar(QMainWindow& window)
{
	QToolBar* toolBar = window.addToolBar(QStringLiteral("Main"));
	toolBar->addAction(standardIcon(QStyle::SP_FileIcon), QStringLiteral("New"));
	toolBar->addAction(standardIcon(QStyle::SP_DialogOpenButton), QStringLiteral("Open"));
	toolBar->addAction(standardIcon(QStyle::SP_DialogSaveButton), QStringLiteral("Save"));
	toolBar->addSeparator();

	QAction* autoRefresh = toolBar->addAction(standardIcon(QStyle::SP_BrowserReload), QStringLiteral("Auto-refresh"));
	autoRefresh->setCheckable(true);
	autoRefresh->setChecked(true);

	auto* zoom = new QComboBox;
	zoom->addItems({ QStringLiteral("50%"), QStringLiteral("100%"), QStringLiteral("200%") });
	zoom->setCurrentIndex(1);
	toolBar->addWidget(zoom); // an embedded widget inherits nothing from the toolbar's own styling

	toolBar->addAction(QStringLiteral("Disabled"))->setEnabled(false);
}

void addSampleDock(QMainWindow& window)
{
	auto* dock = new QDockWidget{ QStringLiteral("Dock widget"), &window };
	dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
	auto* contents = new QListWidget;
	contents->addItems({ QStringLiteral("Dock contents"), QStringLiteral("Float or close it"),
		QStringLiteral("to see the title bar buttons") });
	dock->setWidget(contents);
	window.addDockWidget(Qt::RightDockWidgetArea, dock);
}

void fillStatusBar(QMainWindow& window)
{
	QStatusBar* statusBar = window.statusBar();
	statusBar->showMessage(QStringLiteral("Status bar message"));

	auto* progress = new QProgressBar;
	progress->setValue(70);
	progress->setFixedWidth(StatusProgressWidth);
	statusBar->addPermanentWidget(progress); // a permanent widget sits past the message, on the size grip's side
	statusBar->addPermanentWidget(new QLabel{ QStringLiteral("Permanent widget") });
}

} // namespace

CWidgetGallery::CWidgetGallery(QWidget* parent) :
	QWidget{ parent }
{
	// Two independent columns rather than a grid: a tall section must not stretch the row beside it
	auto* leftColumn = new QVBoxLayout;
	for (QWidget* s : { buttonsSection(), choicesSection(), textEntrySection(), numbersAndPickersSection(), rangesSection() })
		leftColumn->addWidget(s);
	leftColumn->addStretch();

	auto* rightColumn = new QVBoxLayout;
	for (QWidget* s : { itemViewsSection(), containersSection(), labelsSection(), dialogsSection(this) })
		rightColumn->addWidget(s);
	rightColumn->addStretch();

	auto* columns = new QHBoxLayout;
	columns->addLayout(leftColumn);
	columns->addLayout(rightColumn);

	auto* page = new QWidget;
	auto* pageLayout = new QVBoxLayout{ page };
	pageLayout->addWidget(schemeStrip());
	pageLayout->addLayout(columns);

	auto* scrollArea = new QScrollArea;
	scrollArea->setWidgetResizable(true);
	scrollArea->setFrameShape(QFrame::NoFrame); // the gallery may be embedded in something that draws its own frame
	scrollArea->setWidget(page);

	auto* layout = new QVBoxLayout{ this };
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(scrollArea);
}

CWidgetGalleryWindow::CWidgetGalleryWindow(QWidget* parent) :
	QMainWindow{ parent }
{
	setWindowTitle(QStringLiteral("Widget gallery"));
	setCentralWidget(new CWidgetGallery);
	addMenus(*this);
	addMainToolBar(*this);
	addSampleDock(*this);
	fillStatusBar(*this);
	resize(WindowWidth, WindowHeight);
}

void CWidgetGalleryWindow::showNew()
{
	auto* window = new CWidgetGalleryWindow;
	window->setAttribute(Qt::WA_DeleteOnClose);
	window->show();
}
