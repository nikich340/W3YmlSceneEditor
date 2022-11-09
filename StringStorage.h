#ifndef STRINGSTORAGE_H
#define STRINGSTORAGE_H
#include <algorithm>
#include <QVector>
#include <QString>
#include <QPair>
#include <QVariant>

struct StringStorageUnit {
    QString str;
    QVariant data;
};

class StringStorage
{
private:
    QVector<StringStorageUnit> m_units;

    QPair<int, int> getRange(const QString& prefix);

public:
    StringStorage();

    void addString(const QString& str, const QVariant& data = QVariant());
    void addStringUnsafe(const QString& str, const QVariant& data = QVariant());
    void sort();
    int count() const;
    void clear();
    const QString& stringAt(int idx) const;
    const QVariant& dataAt(int idx) const;
    QStringList allStrings(int max = 100);
    void byPrefix(const QString& prefix, QStringList& outList, QList<QVariant>& outData, int max = 100);
    QStringList stringsByPrefix(const QString& prefix, int max = 100);
    QList<QVariant> dataByPrefix(const QString& prefix, int max = 100);
};

#endif // STRINGSTORAGE_H
