#include "cthemeiconhandler.h"

#include "assert/advanced_assert.h"

DISABLE_COMPILER_WARNINGS
#include <QColor>
#include <QFile>
RESTORE_COMPILER_WARNINGS

#include <algorithm>
#include <cstring>
#include <optional>
#include <utility>

namespace {

const QString ThemeIconScheme = QStringLiteral("themeicon:/");

// Read-only file engine over bytes built in memory. Only what loading an image through QFile
// actually exercises is implemented; everything else keeps the base class's refusals.
class MemoryFileEngine final : public QAbstractFileEngine
{
public:
	MemoryFileEngine(QByteArray data, QString virtualName) :
		_data{ std::move(data) }, _name{ std::move(virtualName) } {}

	bool open(QIODevice::OpenMode mode, std::optional<QFile::Permissions>) override
	{
		return (mode & QIODevice::WriteOnly) == 0;
	}
	bool close() override { _pos = 0; return true; }
	qint64 size() const override { return _data.size(); }
	qint64 pos() const override { return _pos; }
	bool seek(qint64 pos) override
	{
		if (pos < 0 || pos > _data.size())
			return false;
		_pos = pos;
		return true;
	}
	qint64 read(char* out, qint64 maxlen) override
	{
		const qint64 n = std::min(maxlen, qint64(_data.size()) - _pos);
		if (n > 0)
		{
			std::memcpy(out, _data.constData() + _pos, size_t(n));
			_pos += n;
		}
		return std::max<qint64>(n, 0);
	}
	FileFlags fileFlags(FileFlags type) const override
	{
		return type & (ExistsFlag | FileType | ReadOwnerPerm | ReadUserPerm | ReadGroupPerm | ReadOtherPerm);
	}
	// The .svg suffix lets QImageReader shortlist the plugin by extension rather than probing every format.
	QString fileName(FileName) const override { return _name; }

private:
	QByteArray _data;
	QString _name;
	qint64 _pos = 0;
};

} // namespace

CThemeIconHandler::CThemeIconHandler(QString sourcePrefix) :
	_sourcePrefix{ std::move(sourcePrefix) }
{
}

std::unique_ptr<QAbstractFileEngine> CThemeIconHandler::create(const QString& fileName) const
{
	if (!fileName.startsWith(ThemeIconScheme))
		return nullptr; // not ours - let the next handler or the built-in engines have it

	// themeicon:/check-3b8fe0.svg -> source "check", tint "#3b8fe0". The last '-' separates them,
	// so source names themselves may contain dashes.
	const auto stem = QStringView{ fileName }.sliced(ThemeIconScheme.size());
	assert_and_return_r(stem.endsWith(QLatin1String(".svg")), nullptr);
	const auto nameAndTint = stem.chopped(4);
	const qsizetype dash = nameAndTint.lastIndexOf(QLatin1Char('-'));
	assert_and_return_r(dash > 0, nullptr);

	QFile source{ _sourcePrefix + QLatin1Char('/') + nameAndTint.first(dash).toString() + QLatin1String(".svg") };
	assert_and_return_r(source.open(QIODevice::ReadOnly), nullptr); // the stylesheet names an icon that does not exist

	QByteArray svg = source.readAll();
	svg.replace("currentColor", QStringLiteral("#%1").arg(nameAndTint.sliced(dash + 1)).toLatin1());
	return std::make_unique<MemoryFileEngine>(std::move(svg), fileName);
}

QString themeIconUrl(const QString& name, const QColor& color)
{
	return ThemeIconScheme + name + QLatin1Char('-') + color.name(QColor::HexRgb).sliced(1) + QLatin1String(".svg");
}
