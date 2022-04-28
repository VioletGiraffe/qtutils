#pragma once

class QString;
class QByteArray;

QByteArray dataFromResource(const QString& resourcePath);
QString textFromResource(const QString& resourcePath);
