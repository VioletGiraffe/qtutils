# qtutils

Reusable Qt Core/Gui/Widgets facilities for application plumbing, dialogs, diagnostics, and custom widgets. The library targets C++23 and Qt 6, and depends on `cpputils` plus the header-only `cpp-template-utils`.

## Facilities

### Application UI and dialogs

| Header | Facility |
|---|---|
| `appdialogs/caboutdialog.h` | About dialog populated from `QApplication` metadata, build date/time, Qt version, and optional application version/copyright values. `customContentLayout()` accepts application-specific widgets, placed between the version and copyright lines. |
| `dialogs/csimpleprogressdialog.h` | Cancellable progress dialog with configurable text/range and optional automatic show-on-progress and close-on-completion behavior. |
| `dialogs/messagedialog.h` | Arbitrary-label question boxes returning the selected option index, and OK-only notices; either can carry long selectable details that scroll instead of growing off-screen. |
| `appdialogs/csettingspage.h`, `appdialogs/csettingsdialog.h` | Abstract settings-page contract and a multi-page settings dialog that applies every page, announces the change through `CSettingsNotifier`, manages focus order, and offers an explicit wipe-and-exit shortcut. |
| `appdialogs/csettingsnotifier.h` | Application-wide `settingsChanged` signal for state derived from stored settings. |
| `appdialogs/reportbugdialog.h` | Bug-report dialog showing a supplied logger's contents for copying, with a button opening the application's issue tracker. |
| `ui/ui-inspector/cuiinspector.h` | Live widget/layout hierarchy inspector with property details, refresh/filter controls, selection highlighting, and focus-change reporting. |
| `ui/widget-gallery/cwidgetgallery.h` | Scrollable page of every commonly used stock control, in the states a style draws separately, for finding what a custom style or stylesheet does not cover; a colour scheme strip drives `CThemeController` so the same page can be read in both polarities. `CWidgetGalleryWindow` adds the menu bar, toolbar, dock widget and status bar, and `showNew()` opens one as a self-deleting window. |
| `ui/CProxyStyle.h` | Subclassing base that forwards a broad `QStyle` surface to the current application style, allowing a derived style to override only selected operations. |

### Settings, history, and logging

| Header | Facility |
|---|---|
| `settings/csettings.h` | Thin `QSettings` wrapper with process-wide application, organization, and format overrides plus value/key/clear operations. |
| `historylist/chistorylist.h` | Bounded, deduplicating generic history with cursor navigation, bulk restore, endpoint queries, and iteration. |
| `widgets/chistorycombobox.h` | Editable history combo box with move-to-front mode, activation signaling, previous-item selection, optional editor clearing, and automatic `QSettings` persistence. |
| `logger/cloggerinterface.h`, `logger/cloggerinmemory.h` | Logger interface, per-type singleton accessor, and thread-safe bounded in-memory logger that snapshots its retained lines. |

### Input and widgets

| Header | Facility |
|---|---|
| `mouseclickdetector/cmouseclickdetector.h` | Global or directly notified event filter that disambiguates single and double left-clicks per object and emits their positions. |
| `widgets/cclickablelabel.h` | `QLabel` emitting single- and double-click signals with global positions. |
| `widgets/clineedit.h` | `QLineEdit` with configurable queued select-all-on-focus behavior. |
| `widgets/clabelelided.h` | Label that paints elided text, in the middle unless told otherwise, and automatically shows the full text as a tooltip only when elided. |
| `widgets/cplaintexteditwithlinenumbers.h` | `QPlainTextEdit` with a synchronized line-number gutter. |
| `widgets/clightningfastviewer.h` | Read-only text and hex viewer that indexes visual lines instead of laying out a document, so arbitrarily large files open at once; stands in CP437 glyphs for non-printable characters, colours hex bytes by class, and offers word wrap, a wrap-aware line number column, selection, keyboard navigation, clipboard, and literal or regex search with optional wrap-around, reporting a match found after wrapping, plus deadline-bounded match counting that resumes across calls, and highlighting of the current and counted matches. |
| `widgets/cfindbar.h` | Find bar searching through host-supplied functions: pattern history, previous and next, case, whole-word, regex and highlight-all toggles, a match counter and match highlighting fed by optional host functions, a status for misses and wrap-arounds, optional `QSettings` persistence; its find actions carry host-chosen keys. |
| `widgets/findresult.h` | Search outcome (not found, found, or found after wrapping around) and match count, shared by `CLightningFastViewerWidget` and `CFindBar`'s host functions. |
| `widgets/cpersistenceenabler.h` | Event-filter helper that restores and saves widget geometry and `QMainWindow` state through `QSettings`, with delayed first-show restoration by default. |
| `widgets/layouts/cflowlayout.h` | Height-for-width layout that wraps items into rows using explicit, inherited, or style spacing. |
| `widgets/layouts/coverlaylayout.h` | Expanding layout that gives every child the same geometry, stacking items over one another. |

### Theming

| Header | Facility |
|---|---|
| `theme/cbasepalette.h` | Colour set shared by themed apps: nine authored core colours, seven derived ones that `resolvedPalette()` fills in when left unset, and the mapping onto `QPalette` roles. |
| `theme/cthemecontroller.h` | Persisted colour scheme preference (system, light or dark) and one theme name per polarity, stored as opaque strings the app resolves against its own theme list; applies the polarity through `QStyleHints` and signals `themeChanged`. |
| `theme/colorutils.h` | Colour maths: sRGB mixing, WCAG luminance and contrast, readable text on a fill, a hue distinct from the palette's accents, a saturated colour reaching a target contrast against a background, and search-match fills built from these. |
| `theme/ctintedsvgiconengine.h` | Icons and pixmaps from monochrome SVGs, rendered per size and DPR in a colour read from a provider at each render, so icons follow a live theme change. |
| `theme/cthemeiconhandler.h` | `themeicon:` virtual file scheme serving tinted copies of SVG resources, so stylesheet `url()`s can use theme-coloured glyphs; `themeIconUrl()` builds the URLs. Built on private Qt API. |
| `theme/cstylefixups.h` | Startup-installed fixes for what QSS cannot do: rounded combo popups, hover on splitter handles, a wider push-button focus frame; each reads its parameters from a provider, so a theme change needs no reinstall. |
| `theme/widgetcolorreport.h` | Text report of what determines a widget's colours: style chain, stylesheets, colour scheme, Qt version, state and background attributes, the full palette with its explicitly set roles, the class palette, ancestors' palettes, and pixels sampled from the widget's rendering and from the screen. |

[`docs/qt-styling-quirks.md`](docs/qt-styling-quirks.md) lists known QSS and `QStyle` traps with their remedies.

### Icons, resources, sorting, and widget helpers

| Header | Facility |
|---|---|
| `utils/ciconengineqimage.h` | `QIconEngine` that renders each requested physical size directly from a source `QImage`, preserves aspect ratio with transparent padding, caches by size/mode, and accepts a custom scaler. |
| `utils/naturalsorting/cnaturalsorterqcollator.h` | Locale-aware numeric `QStringView` ordering with case-sensitive and case-insensitive comparators backed by thread-local `QCollator`s. |
| `utils/resources.h` | Reads a Qt resource into `QByteArray` or `QString`. |
| `widgets/widgetutils.h` | Recursive layout visibility, main-window/hierarchy lookup, non-invasive native owner handles, screen selection, centered geometry, and widget centering helpers. |
| `string/stringutils.h` | Computes the zero-based line number at a `QString` position. |

### Qt/std interoperation and small helpers

| Header | Facility |
|---|---|
| `qtcore_helpers/catch_qt.hpp` | Catch2 with printers for `QString`, `QStringView`, `QLatin1StringView`, `QByteArray` and `QChar`; test sources that use Qt include it instead of `catch.hpp`. Header-only: a test build adds qtutils to its include path without linking it. |
| `qtcore_helpers/qdatetime_helpers.hpp` | Converts between `QDateTime` and Unix `time_t` seconds. |
| `qtcore_helpers/qdebug_helpers.hpp` | `QDebug` output for `std::string` and hexadecimal `std::array` data. |
| `qtcore_helpers/qstring_helpers.hpp` | `QSL` and `QL1` aliases for `QStringLiteral` and `QLatin1String`. |
| `qtcore_helpers/qt_helpers.hpp` | `CR` assignment sink that asserts a Qt `connect()` result. |
| `std_helpers/qt_container_helpers.hpp` | Copying/moving conversion from Qt containers to `std::vector`, typed conversion, and conversion from simple standard containers to Qt containers. |
| `windows/windowsutils.h` | Windows-only conversion of a `QString` path to a normalized null-terminated wide-character array, adding the extended-length prefix to drive-absolute paths. |

### Taskbar progress

| Header | Facility |
|---|---|
| `taskbarprogress/taskbarprogress/ctaskbarprogress.h` | Native taskbar progress state/value abstraction. Windows implements normal, paused, error, indeterminate, and hidden states for one top-level window; macOS, Linux, and FreeBSD are currently no-ops. |
| `taskbarprogress/cprogressbartaskbar.h` | `QProgressBar` that mirrors its range, value, visibility, and state to a linked native taskbar button. |

## Building

Build `qtutils.pro` with qmake after making `cpputils` and `cpp-template-utils` available at the sibling paths expected by the project. The library links Qt Core, Gui, and Widgets; taskbar progress adds a native implementation only on Windows. The theming module, and any app including `theme/cthemeiconhandler.h`, needs Qt's private Core headers (`QT += core-private`); an app using `CTintedSvgIconEngine` must add `QT += svg` itself.
