#include "stringutils.h"

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
#include <QStringConverter>
RESTORE_COMPILER_WARNINGS

#include <algorithm>

size_t lineNumberForPosition(const QString &text, int pos) noexcept
{
	return static_cast<size_t>(std::count(text.begin(), text.begin() + pos, '\n'));
}

std::optional<QString> decodedAsText(QByteArrayView bytes, qsizetype sniffBytes)
{
	// The mark is read before the NUL sniff below: UTF-16 text holds a NUL in most of its characters
	if (const std::optional<QStringConverter::Encoding> encoding = QStringConverter::encodingForData(bytes))
	{
		QStringDecoder decoder{ *encoding };
		return QString{ decoder.decode(bytes) };
	}

	if (bytes.first(std::min(sniffBytes, bytes.size())).contains('\0'))
		return {};

	return QString::fromUtf8(bytes);
}
