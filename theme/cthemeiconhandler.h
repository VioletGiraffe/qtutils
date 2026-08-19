#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <private/qabstractfileengine_p.h> // private API; see the class comment
#include <QString>
RESTORE_COMPILER_WARNINGS

#include <memory>

class QColor;

// Serves runtime-tinted copies of monochrome SVG resources under a virtual scheme, so that Qt
// stylesheets - whose url() accepts only a path, never bytes - can use theme-coloured glyphs:
//
//     image: url(themeicon:/check-3b8fe0.svg);   ->   ":/theme/check.svg" with currentColor -> #3b8fe0
//
// The colour is part of the path, so distinct tints are distinct URLs and no invalidation is ever
// needed; themeIconUrl() builds them. Sources mark their tintable parts with `currentColor`.
// Construct one instance (with the resource prefix the sources live under) before installing any
// stylesheet that references the scheme, and keep it alive for the application's lifetime.
//
// QAbstractFileEngineHandler is private API, accepted deliberately: everything is built from source
// against a pinned Qt, so the failure mode is a build break on a Qt upgrade, not a runtime one.
class CThemeIconHandler final : public QAbstractFileEngineHandler
{
public:
	// `sourcePrefix` example: ":/theme" - themeicon:/check-3b8fe0.svg reads ":/theme/check.svg".
	explicit CThemeIconHandler(QString sourcePrefix);

	std::unique_ptr<QAbstractFileEngine> create(const QString& fileName) const override;

private:
	const QString _sourcePrefix;
};

// The URL for `name` (a source file name without the .svg extension) tinted with `color`, for
// splicing into a stylesheet.
[[nodiscard]] QString themeIconUrl(const QString& name, const QColor& color);
