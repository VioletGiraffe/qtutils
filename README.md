# qtutils

Reusable Qt Core/Gui/Widgets facilities for application plumbing, dialogs, diagnostics, and custom widgets. The library targets C++23 and Qt 6, and depends on `cpputils` plus the header-only `cpp-template-utils`.

## Facilities

### Application UI and dialogs

| Header | Facility |
|---|---|
| `aboutdialog/caboutdialog.h` | About dialog populated from `QApplication` metadata, build date/time, Qt version, and optional application version/copyright values. |
| `dialogs/csimpleprogressdialog.h` | Cancellable progress dialog with configurable text/range and optional automatic show-on-progress and close-on-completion behavior. |
| `dialogs/messagebox.h` | Arbitrary-label question boxes returning the selected option index, plus bounded notices whose long selectable details scroll instead of growing off-screen. |
| `settingsui/csettingspage.h`, `settingsui/csettingsdialog.h` | Abstract settings-page contract and a multi-page settings dialog that applies every page, emits `settingsChanged`, manages focus order, and offers an explicit wipe-and-exit shortcut. |
| `ui/ui-inspector/cuiinspector.h` | Live widget/layout hierarchy inspector with property details, refresh/filter controls, selection highlighting, and focus-change reporting. |
| `ui/CProxyStyle.h` | Subclassing base that forwards a broad `QStyle` surface to the current application style, allowing a derived style to override only selected operations. |

### Settings, history, and logging

| Header | Facility |
|---|---|
| `settings/csettings.h` | Thin `QSettings` wrapper with process-wide application, organization, and format overrides plus value/key/clear operations. |
| `historylist/chistorylist.h` | Bounded, deduplicating generic history with cursor navigation, bulk restore, endpoint queries, and iteration. |
| `widgets/chistorycombobox.h` | Editable history combo box with move-to-front mode, activation signaling, previous-item shortcut, optional editor clearing, and automatic `CSettings` persistence. |
| `logger/cloggerinterface.h`, `logger/cloggerinmemory.h` | Logger interface, per-type singleton accessor, and thread-safe bounded in-memory logger that snapshots its retained lines. |

### Input and widgets

| Header | Facility |
|---|---|
| `mouseclickdetector/cmouseclickdetector.h` | Global or directly notified event filter that disambiguates single and double left-clicks per object and emits their positions. |
| `widgets/cclickablelabel.h` | `QLabel` emitting single- and double-click signals with global positions. |
| `widgets/clineedit.h` | `QLineEdit` with configurable queued select-all-on-focus behavior. |
| `widgets/clabelelided.h` | Label that paints elided text, in the middle unless told otherwise, and automatically shows the full text as a tooltip only when elided. |
| `widgets/ctexteditwithlinenumbers.h` | `QTextEdit` with a synchronized line-number gutter. |
| `widgets/circularprogressindicator/ccircularprogressindicator.h` | Configurable indeterminate spinner with color, animation interval, and stopped-state visibility properties. |
| `widgets/cpersistentwindow.h` | Event-filter helper that restores and saves widget geometry and `QMainWindow` state through `CSettings`, with delayed first-show restoration by default. |
| `widgets/layouts/cflowlayout.h` | Height-for-width layout that wraps items into rows using explicit, inherited, or style spacing. |
| `widgets/layouts/coverlaylayout.h` | Expanding layout that gives every child the same geometry, stacking items over one another. |

### Icons, resources, sorting, and widget helpers

| Header | Facility |
|---|---|
| `utils/ciconengineqimage.h` | `QIconEngine` that renders each requested physical size directly from a source `QImage`, preserves aspect ratio with transparent padding, caches by size/mode, and accepts a custom scaler. |
| `utils/naturalsorting/cnaturalsorterqcollator.h` | Locale-aware numeric `QString` ordering with case-sensitive and case-insensitive comparators backed by thread-local `QCollator`s. |
| `utils/resources.h` | Reads a Qt resource into `QByteArray` or `QString`. |
| `widgets/widgetutils.h` | Recursive layout visibility, main-window/hierarchy lookup, non-invasive native owner handles, screen selection, centered geometry, and widget centering helpers. |
| `string/stringutils.h` | Computes the zero-based line number at a `QString` position. |

### Qt/std interoperation and small helpers

| Header | Facility |
|---|---|
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

The Qxt-derived tooltip sources under `taskbarprogress/taskbarprogress/{linux,freebsd}` are vendored implementation material, not part of the supported `qtutils` API.

## Building

Build `qtutils.pro` with qmake after making `cpputils` and `cpp-template-utils` available at the sibling paths expected by the project. The library links Qt Core, Gui, and Widgets; taskbar progress adds a native implementation only on Windows.
