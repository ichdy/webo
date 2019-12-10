#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <QString>
#include <QMap>
#include <QStringList>
#include <QTextStream>
#include <QStack>

class HtmlParser
{
public:
    HtmlParser() :
        mHtml(0),
        mCheckAttribute(false),
        mIndex(-1),
        mEndIndex(-1) {}

    HtmlParser(QString *html) :
        mCheckAttribute(false),
        mIndex(-1),
        mEndIndex(-1)
    {
        load(html);
    }

    void load(QString *html)
    {
        if (!html)
            return;

        mHtml = html;
        mCheckAttribute = false;
        mIndex = mHtml->indexOf("<");
        mEndIndex = mHtml->indexOf(">");
    }

    HtmlParser nextTagByTagName(const QString &name)
    {
        if (!isValid())
            return HtmlParser();

        QString tag = QString("<%1").arg(name);
        int index = mHtml->indexOf(tag, mEndIndex +1);
        int endIndex = mHtml->indexOf(">", index +1);

        HtmlParser parser;
        parser.mHtml = mHtml;
        parser.mIndex = index;
        parser.mEndIndex = endIndex;

        if (parser.tagName() != name)
            return parser.nextTagByTagName(name);

        return parser;
    }

    HtmlParser prevTagByTagName(const QString &name)
    {
        if (!isValid())
            return HtmlParser();

        QString tag = QString("<%1").arg(name);
        int index = mHtml->lastIndexOf(tag, mIndex -1);
        int endIndex = mHtml->indexOf(">", index +1);

        HtmlParser parser;
        parser.mHtml = mHtml;
        parser.mIndex = index;
        parser.mEndIndex = endIndex;

        if (parser.tagName() != name)
            return parser.nextTagByTagName(name);

        return parser;
    }

    HtmlParser nextTagById(const QString &id)
    {
        if (!isValid())
            return HtmlParser();

        int currentIndex(-1), currentEndIndex(-1);
        do {
            QString tag = QString("<");
            currentIndex = mHtml->indexOf(tag, currentEndIndex +1);
            currentEndIndex = mHtml->indexOf(">", currentIndex +1);

            HtmlParser parser;
            parser.mHtml = mHtml;
            parser.mIndex = currentIndex;
            parser.mEndIndex = currentEndIndex;

            if (parser.attribute("id") == id)
                return parser;
        } while (currentIndex != -1);

        return HtmlParser();
    }

    HtmlParser prevTagById(const QString &id)
    {
        if (!isValid())
            return HtmlParser();

        int currentIndex(-1), currentEndIndex(-1);
        do {
            QString tag = QString("<");
            currentIndex = mHtml->lastIndexOf(tag, currentIndex -1);
            currentEndIndex = mHtml->indexOf(">", currentIndex +1);

            HtmlParser parser;
            parser.mHtml = mHtml;
            parser.mIndex = currentIndex;
            parser.mEndIndex = currentEndIndex;

            if (parser.attribute("id") == id)
                return parser;
        } while (currentIndex != -1);

        return HtmlParser();
    }

    HtmlParser nextTagByClass(const QString &cls)
    {
        if (!isValid())
            return HtmlParser();

        int currentIndex(-1), currentEndIndex(-1);
        do {
            QString tag = QString("<");
            currentIndex = mHtml->indexOf(tag, currentEndIndex +1);
            currentEndIndex = mHtml->indexOf(">", currentIndex +1);

            HtmlParser parser;
            parser.mHtml = mHtml;
            parser.mIndex = currentIndex;
            parser.mEndIndex = currentEndIndex;

            if (parser.attribute("class") == cls)
                return parser;
        } while (currentIndex != -1);

        return HtmlParser();
    }

    HtmlParser prevTagByClass(const QString &cls)
    {
        if (!isValid())
            return HtmlParser();

        int currentIndex(-1), currentEndIndex(-1);
        do {
            QString tag = QString("<");
            currentIndex = mHtml->lastIndexOf(tag, currentIndex -1);
            currentEndIndex = mHtml->indexOf(">", currentIndex +1);

            HtmlParser parser;
            parser.mHtml = mHtml;
            parser.mIndex = currentIndex;
            parser.mEndIndex = currentEndIndex;

            if (parser.attribute("class") == cls)
                return parser;
        } while (currentIndex != -1);

        return HtmlParser();
    }

    QString tagName()
    {
        if (!isValid())
            return QString();

        QString result = mHtml->mid(mIndex +1, mEndIndex - mIndex -1);
        if (result.right(1) == "/")
            result.chop(1);

        return result.section(" ", 0, 0);
    }

    QString text(const QString &joinSeparator = " ", bool _simplified = true)
    {
        if (!isValid())
            return QString();

        QStringList resultList;
        QString currentHtml = html();

        int pos(0);
        while (true) {
            int tagCloseIndex = currentHtml.indexOf(">", pos) + 1;
            int tagOpenIndex = currentHtml.indexOf("<", tagCloseIndex) + 1;

            if (tagOpenIndex == 0)
                break;

            QString temp = simplified(currentHtml.mid(tagCloseIndex, tagOpenIndex - tagCloseIndex - 1));
            if (!temp.isEmpty())
                resultList << temp;
            pos = tagOpenIndex;
        }

        QString result = resultList.join(joinSeparator);
        if (_simplified)
            return result.simplified();

        return result;
    }

    QString attribute(const QString &name)
    {
        if (!isValid())
            return QString();

        if (!mCheckAttribute) {
            QString attr = mHtml->mid(mIndex +1, mEndIndex - mIndex -1).section(" ", 1);
            mAttributeMap = readAttribute(attr);
            mCheckAttribute = true;
        }

        return mAttributeMap[name];
    }

    bool isValid() {
        return (mIndex > -1 && mEndIndex > -1);
    }

    QString html() {
        if (!isValid())
            return QString();

        QStack<bool> tagStack;
        tagStack.push(true);

        int pos(mIndex + 1);
        do {
            char c = mHtml->at(pos).toLatin1();
            pos++;

            switch (c) {
            case '/':
            case '<':
            {
                char ca = mHtml->at(pos).toLatin1();
                QString test;
                if (c == '/') {
                    if (ca == '>') {
                        tagStack.pop();
                        test = "pop";
                    }
                }
                else {
                    if (ca == '/') {
                        tagStack.pop();
                        test = "pop";
                    }
                    else {
                        tagStack.push(true);
                        test = "push";
                    }
                }
            }
                break;
            default:
                break;
            }

            if (tagStack.isEmpty())
                break;
        } while (pos < mHtml->size());

        int tagCloseIndex = mHtml->indexOf(">", pos - 1) + 1;
        return mHtml->mid(mIndex, tagCloseIndex - mIndex);
    }

private:
    QString simplified(const QString &string) {
        QString temp;

        QList<QChar> spaceList;
        spaceList << '\t' << '\n' << '\v' << '\f' << '\r';

        for (int i=0; i<string.size(); i++) {
            QChar c = string[i];
            if (spaceList.contains(c))
                continue;

            temp += c;
        }

        return temp;
    }

    QMap<QString, QString> readAttribute(const QString &data)
    {
        QMap<QString, QString> result;
        if (data.isEmpty())
            return result;

        QString name, val;
        bool getName(true), getVal(false), getText(false);
        QChar getTextChar('\0');
        for (int pos=0; pos<data.length(); pos++) {
            QChar current = data[pos];
            QChar peek = (pos+1 >= data.length()? 0 : data[pos+1]);

            if (getName) {
                if (name.size() == 0 && current == ' ')
                    continue;

                name.append(current);
                if (peek == '=' || peek == ' ') {
                    getName = false;
                    if (peek == '=')
                        getVal = true;

                    pos++;
                }

                if (name.size() > 0 && peek == QChar(0))
                    result[name] = QString();
            }
            else {
                if (getText || current != ' ') {
                    if (!getVal) {
                        if (current != '=') {
                            getName = true;
                            result[name] = QString();
                            name.clear();
                            name.append(current);
                        }
                        else getVal = true;
                    }
                    else {
                        if ((getTextChar != QChar(0) && current == getTextChar) ||
                                (getTextChar == QChar(0) && (current == '"' || current == '\''))) {
                            getText = !getText;
                            getTextChar = getText? current : QChar(0);

                            if (!getText) {
                                result[name] = val;
                                name.clear();
                                val.clear();
                                getName = true;
                                getVal = false;
                            }

                        }
                        else {
                            val.append(current);
                            if (!getText && peek == ' ') {
                                result[name] = val;
                                name.clear();
                                val.clear();
                                getName = true;
                                getVal = false;
                                pos++;
                            }
                        }
                    }
                }

                if (name.size() > 0 && peek == QChar(0))
                    result[name] = val;
            }
        }

        return result;
    }

private:
    QString *mHtml;
    QMap<QString, QString> mAttributeMap;
    bool mCheckAttribute;
    int mIndex;
    int mEndIndex;
};

#endif