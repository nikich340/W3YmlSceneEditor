#include <QDebug>
#include "StringStorage.h"

QPair<int, int> StringStorage::getRange(const QString& prefix)
{
    int prefLen = prefix.length();
    int maxLen;
    QStringRef ref;

    // [L..
    int L1 = 0;
    int R1 = m_units.count() - 1;
    while (R1 - L1 > 1) {
        int M1 = (L1 + R1) / 2;
        maxLen = std::min(m_units[M1].str.length(), prefLen);
        ref = m_units[M1].str.leftRef(maxLen);
        if (ref < prefix) {
            L1 = M1;
        } else {
            R1 = M1;
        }
    }
    while (L1 < m_units.count()) {
        maxLen = std::min(m_units[L1].str.length(), prefLen);
        ref = m_units[L1].str.leftRef(maxLen);
        if (ref < prefix) {
            L1 += 1; // -> m_units[L1].str >= prefix
        } else {
            break;
        }
    }

    // ..R]
    int L2 = 0;
    int R2 = m_units.count() - 1;
    while (R2 - L2 > 1) {
        int M2 = (L2 + R2) / 2;
        maxLen = std::min(m_units[M2].str.length(), prefLen);
        ref = m_units[M2].str.leftRef(maxLen);
        if (ref > prefix) {
            R2 = M2;
        } else {
            L2 = M2;
        }
    }
    while (R2 >= 0) {
        maxLen = std::min(m_units[R2].str.length(), prefLen);
        ref = m_units[R2].str.leftRef(maxLen);
        if (ref > prefix) {
            R2 -= 1; // -> m_units[L1].str <= prefix
        } else {
            break;
        }
    }
    // [L1..R2] is valid range now
    return {L1, R2};
}

StringStorage::StringStorage()
{

}

void StringStorage::addString(const QString& str, const QVariant& data)
{
    addStringUnsafe(str, data);
    sort();
}

void StringStorage::addStringUnsafe(const QString& str, const QVariant& data)
{
    StringStorageUnit unit;
    unit.str = str;
    unit.data = data;
    m_units.append(unit);
}

void StringStorage::sort()
{
    std::sort(m_units.begin(), m_units.end(), [](const StringStorageUnit& lhs, const StringStorageUnit& rhs)
    {
       return lhs.str < rhs.str;
    });
}

int StringStorage::count() const
{
    return m_units.count();
}

void StringStorage::clear()
{
    m_units.clear();
}

const QString& StringStorage::stringAt(int idx) const
{
    return m_units.at(idx).str;
}

const QVariant& StringStorage::dataAt(int idx) const
{
    return m_units.at(idx).data;
}

QStringList StringStorage::allStrings(int max)
{
    QStringList ret;
    int max_i = std::min(max, m_units.count());
    for (int i = 0; i < max_i; i += 1) {
        ret.append(m_units[i].str);
    }
    return ret;
}

void StringStorage::byPrefix(const QString& prefix, QStringList& outList, QList<QVariant>& outData, int max)
{
    QPair<int, int> range = getRange(prefix);
    if (range.first >= 0 && range.second < m_units.count() && range.second - range.first + 1 > 0) {
        range.second = range.first + std::min(range.second - range.first, max - 1);
        for (int i = range.first; i <= range.second; i += 1) {
            outList.append(m_units[i].str);
            outData.append(m_units[i].data);
        }
    }
}

QStringList StringStorage::stringsByPrefix(const QString& prefix, int max)
{
    QPair<int, int> range = getRange(prefix);
    QStringList ret;
    if (range.first >= 0 && range.second < m_units.count() && range.second - range.first + 1 > 0) {
        range.second = range.first + std::min(range.second - range.first, max - 1);
        for (int i = range.first; i <= range.second; i += 1) {
            ret.append(m_units[i].str);
        }
    }
    return ret;
}

QList<QVariant> StringStorage::dataByPrefix(const QString& prefix, int max)
{
    QPair<int, int> range = getRange(prefix);
    QList<QVariant> ret;
    if (range.first >= 0 && range.second < m_units.count() && range.second - range.first + 1 > 0) {
        range.second = range.first + std::min(range.second - range.first, max - 1);
        for (int i = range.first; i <= range.second; i += 1) {
            ret.append(m_units[i].data);
        }
    }
    return ret;
}
