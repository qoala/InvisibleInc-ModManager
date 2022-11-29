#include "modversion.h"

#include <QRegularExpression>

namespace iimodmanager {

bool isVersionLessThan(const QString &left, const QString &right)
{
    static const QRegularExpression versionRe(QStringLiteral("^v?(\\d+(?:\\.\\d+)*)([^\\d.]\\S*)?"));
    QRegularExpressionMatch leftMatch = versionRe.match(left);
    QRegularExpressionMatch rightMatch = versionRe.match(right);
    // Compare non-version strings lexically.
    if (!leftMatch.hasMatch() && !rightMatch.hasMatch())
        return left < right;
    // non-version strings are less than proper version strings.
    else if (leftMatch.hasMatch() != rightMatch.hasMatch())
        return !leftMatch.hasMatch();

    // Parse out individual version number components.
    auto leftNumberParts = leftMatch.capturedView(1).split('.');
    QVector<int> leftNumbers;
    leftNumbers.reserve(leftNumberParts.size());
    for (const auto &p : leftNumberParts)
        leftNumbers << p.toInt();
    auto rightNumberParts = rightMatch.capturedView(1).split('.');
    QVector<int> rightNumbers;
    rightNumbers.reserve(rightNumberParts.size());
    for (const auto &p : rightNumberParts)
        rightNumbers << p.toInt();

    // Compare each component.
    for (int i = 0; i < leftNumbers.size() && i < rightNumbers.size(); i++)
        if (leftNumbers.at(i) != rightNumbers.at(i))
            return leftNumbers.at(i) < rightNumbers.at(i);
    // Shorter component lists are first.
    if (leftNumbers.size() != rightNumbers.size())
        return leftNumbers.size() < rightNumbers.size();

    return leftMatch.capturedView(2) < rightMatch.capturedView(3);
}

} // namespace iimodmanager
