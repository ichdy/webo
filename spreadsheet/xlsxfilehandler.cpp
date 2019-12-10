#include "xlsxfilehandler.h"

#include <qmath.h>
#include <QImage>
#include <quazip.h>
#include <quazipfile.h>
#include <QTextDocument>
#include "cell.h"
#include "htmlparser.h"

#include "workbook.h"
#include "worksheet.h"

SpreadsheetXlsxFileHandler::SpreadsheetXlsxFileHandler(SpreadsheetWorkbook *workbook, QObject *parent) :
    SpreadsheetAbstractFileHandler(workbook, parent)
{
}

QStringList SpreadsheetXlsxFileHandler::extension()
{
    return QStringList() << "xlsx";
}

Spreadsheet::BorderType SpreadsheetXlsxFileHandler::toBorderType(const QString &style)
{
    if (style == "thin")
        return Spreadsheet::Thin;
    else if (style == "medium")
        return Spreadsheet::Medium;
    else if (style == "thick")
        return Spreadsheet::Thick;
    else if (style == "hair")
        return Spreadsheet::Dot;
    else if (style == "dashed")
        return Spreadsheet::Dash;
    else if (style == "dashDot")
        return Spreadsheet::DashDot;
    else if (style == "dashDotDot")
        return Spreadsheet::DashDotDot;

    return Spreadsheet::None;
}

bool SpreadsheetXlsxFileHandler::open(const QString &filename)
{
    QuaZip zip(filename);
    if (!zip.open(QuaZip::mdUnzip))
        return false;

    QStringList stringList;
    QList<QVariantMap> fontList;
    QList<QVariantMap> fillList;
    QList<QVariantMap> borderList;
    QList<QVariantMap> styleList;
    QHash<int, QString> sheetHash;
    QMap<int, QString> sheetNameMap;
    QHash<QString, QString> sheetXmlHash;
    QHash<QString, QMap<int, QString> > sheetRelsHash;
    QHash<QString, QList<QVariantMap> > drawingHash;
    QHash<QString, QHash<int, QString> > drawingRelsHash;
    QHash<QString, QImage> imageHash;

    {
        QuaZipFile file(&zip);
        for(bool f=zip.goToFirstFile(); f; f=zip.goToNextFile()) {
            QuaZipFileInfo fileInfo;
            zip.getCurrentFileInfo(&fileInfo);

            if (fileInfo.name.contains("xl/_rels/workbook.xml.rels")) {
                file.open(QIODevice::ReadOnly);
                QString xml = file.readAll();
                file.close();

                HtmlParser parser(&xml);
                {
                    QString relations = parser.nextTagByTagName("Relationships").html();
                    HtmlParser relationParser = HtmlParser(&relations).nextTagByTagName("Relationship");
                    while (relationParser.isValid()) {
                        QString type = relationParser.attribute("Type");
                        if (type.contains("/worksheet"))
                            sheetHash[relationParser.attribute("Id").remove("rId").toInt()] = relationParser.attribute("Target");

                        relationParser = relationParser.nextTagByTagName("Relationship");
                    }
                }
            }

            if (fileInfo.name.contains("xl/workbook.xml")) {
                file.open(QIODevice::ReadOnly);
                QString xml = file.readAll();
                file.close();

                HtmlParser parser(&xml);
                {
                    QString sheets = parser.nextTagByTagName("sheets").html();
                    HtmlParser sheetParser = HtmlParser(&sheets).nextTagByTagName("sheet");
                    while (sheetParser.isValid()) {
                        int id = sheetParser.attribute("r:id").remove("rId").toInt();
                        QString name = sheetParser.attribute("name");
                        sheetNameMap[id] = name;

                        sheetParser = sheetParser.nextTagByTagName("sheet");
                    }
                }
            }

            if (fileInfo.name.contains("xl/styles.xml")) {
                file.open(QIODevice::ReadOnly);
                QString xml = file.readAll();
                file.close();

                HtmlParser parser(&xml);
                {
                    QString fonts = parser.nextTagByTagName("fonts").html();
                    HtmlParser fontsParser = HtmlParser(&fonts).nextTagByTagName("font");
                    while (fontsParser.isValid()) {
                        QString font = fontsParser.html();
                        HtmlParser fontParser = HtmlParser(&font);

                        QVariantMap fontMap;
                        fontMap["name"] = fontParser.nextTagByTagName("name").attribute("val");
                        fontMap["size"] = fontParser.nextTagByTagName("sz").attribute("val").toInt();
                        fontMap["bold"] = fontParser.nextTagByTagName("b").isValid();
                        fontMap["italic"] = fontParser.nextTagByTagName("i").isValid();
                        fontMap["underline"] = fontParser.nextTagByTagName("u").isValid();

                        QString color = fontParser.nextTagByTagName("color").attribute("rgb");
                        if (!color.isEmpty())
                            fontMap["color"] = color.prepend("#");

                        fontList << fontMap;
                        fontsParser = fontsParser.nextTagByTagName("font");
                    }
                }

                {
                    QString borders = parser.nextTagByTagName("borders").html();
                    HtmlParser bordersParser = HtmlParser(&borders).nextTagByTagName("border");
                    while (bordersParser.isValid()) {
                        QString border = bordersParser.html();
                        HtmlParser borderParser = HtmlParser(&border);

                        QVariantMap borderMap;
                        borderMap["left"] = borderParser.nextTagByTagName("left").attribute("style");
                        borderMap["right"] = borderParser.nextTagByTagName("right").attribute("style");
                        borderMap["top"] = borderParser.nextTagByTagName("top").attribute("style");
                        borderMap["bottom"] = borderParser.nextTagByTagName("bottom").attribute("style");

                        borderList << borderMap;
                        bordersParser = bordersParser.nextTagByTagName("border");
                    }
                }

                {
                    QString fills = parser.nextTagByTagName("fills").html();
                    HtmlParser fillsParser = HtmlParser(&fills).nextTagByTagName("fill");
                    while (fillsParser.isValid()) {
                        QString fill = fillsParser.html();
                        HtmlParser fillParser = HtmlParser(&fill);

                        QString type = fillParser.nextTagByTagName("patternFill").attribute("patternType");

                        QVariantMap fillMap;
                        if (type == "solid") {
                            QString fgColor = fillParser.nextTagByTagName("fgColor").attribute("rgb");
                            QString bgColor = fillParser.nextTagByTagName("bgColor").attribute("rgb");

                            if (!fgColor.isEmpty())
                                fillMap["fgColor"] = fgColor.prepend("#");

                            if (!bgColor.isEmpty())
                                fillMap["bgColor"] = bgColor.prepend("#");
                        }

                        fillList << fillMap;
                        fillsParser = fillsParser.nextTagByTagName("fill");
                    }
                }

                {
                    QString xfs = parser.nextTagByTagName("cellXfs").html();
                    HtmlParser xfsParser = HtmlParser(&xfs).nextTagByTagName("xf");
                    while (xfsParser.isValid()) {
                        QString alignment = xfsParser.html();
                        HtmlParser alignmentParser = HtmlParser(&alignment).nextTagByTagName("alignment");

                        QVariantMap styleMap;
                        styleMap["borderId"] = xfsParser.attribute("borderId").toInt();
                        styleMap["fontId"] = xfsParser.attribute("fontId").toInt();
                        styleMap["fillId"] = xfsParser.attribute("fillId").toInt();
                        if (alignmentParser.isValid()) {
                            QString hAlign = alignmentParser.attribute("horizontal");
                            QString vAlign = alignmentParser.attribute("vertical");
                            QString wrapText = alignmentParser.attribute("wrapText");
                            QString textRotation = alignmentParser.attribute("textRotation");

                            if (!hAlign.isEmpty())
                                styleMap["hAlign"] = hAlign;

                            if (!vAlign.isEmpty())
                                styleMap["vAlign"] = vAlign;

                            if (!wrapText.isEmpty())
                                styleMap["wrapText"] = true;

                            if (!textRotation.isEmpty())
                                styleMap["textRotation"] = textRotation.toFloat() == 180? -90 : textRotation.toFloat();
                        }

                        styleList << styleMap;
                        xfsParser = xfsParser.nextTagByTagName("xf");
                    }
                }
            }

            if (fileInfo.name.contains("xl/sharedStrings.xml")) {
                file.open(QIODevice::ReadOnly);
                QString xml = file.readAll();
                file.close();

                HtmlParser parser(&xml);
                parser = parser.nextTagByTagName("si");
                while (parser.isValid()) {
                    QString si = parser.html();
                    HtmlParser siParser = HtmlParser(&si).nextTagByTagName("t");

                    QStringList textList;
                    while (siParser.isValid()) {
                        textList << siParser.text("", false);
                        siParser = siParser.nextTagByTagName("t");
                    }

                    QTextDocument doc;
                    doc.setHtml(textList.join(""));
                    stringList << doc.toPlainText();

                    parser = parser.nextTagByTagName("si");
                }
            }

            if (fileInfo.name.contains("xl/worksheets/_rels") && fileInfo.name.right(5).toLower() == ".rels") {
                file.open(QIODevice::ReadOnly);
                QString xml = file.readAll();
                file.close();

                HtmlParser parser(&xml);
                parser = parser.nextTagByTagName("Relationship");
                while (parser.isValid()) {
                    int id = parser.attribute("Id").remove("rId").toInt();
                    QString target = parser.attribute("Target").replace("../", "xl/");
                    sheetRelsHash[fileInfo.name][id] = target;

                    parser = parser.nextTagByTagName("Relationship");
                }
            }

            if (fileInfo.name.contains("xl/drawings") && fileInfo.name.right(4).toLower() == ".xml") {
                file.open(QIODevice::ReadOnly);
                QString xml = file.readAll();
                file.close();

                QStringList anchorList = QStringList() << "xdr:oneCellAnchor" << "xdr:twoCellAnchor";
                foreach (const QString &anchor, anchorList) {
                    HtmlParser parser(&xml);
                    parser = parser.nextTagByTagName(anchor);
                    while (parser.isValid()) {
                        QString xdr = parser.html();
                        HtmlParser xdrParser = HtmlParser(&xdr);

                        QVariantMap drawingMap;
                        {
                            QString xdrFrom = xdrParser.nextTagByTagName("xdr:from").html();
                            HtmlParser xdrFromParser = HtmlParser(&xdrFrom);

                            drawingMap["fromRow"] = xdrFromParser.nextTagByTagName("xdr:row").text().toInt();
                            drawingMap["fromCol"] = xdrFromParser.nextTagByTagName("xdr:col").text().toInt();
                            drawingMap["fromRowOff"] = xdrFromParser.nextTagByTagName("xdr:rowOff").text().toInt();
                            drawingMap["fromColOff"] = xdrFromParser.nextTagByTagName("xdr:colOff").text().toInt();
                        }

                        {
                            QString pic = xdrParser.nextTagByTagName("xdr:blipFill").html();
                            HtmlParser picParser = HtmlParser(&pic);

                            drawingMap["rId"] = picParser.nextTagByTagName("a:blip").attribute("r:embed");
                        }

                        {
                            QString spPr = parser.nextTagByTagName("xdr:spPr").html();
                            HtmlParser spPrParser = HtmlParser(&spPr);

                            QString xfrm = spPrParser.nextTagByTagName("a:xfrm").html();
                            HtmlParser xfrmParser = HtmlParser(&xfrm);

                            drawingMap["width"] = xfrmParser.nextTagByTagName("a:ext").attribute("cx");
                            drawingMap["height"] = xfrmParser.nextTagByTagName("a:ext").attribute("cy");
                        }

                        drawingHash[fileInfo.name] << drawingMap;
                        parser = parser.nextTagByTagName(anchor);
                    }
                }
            }

            if (fileInfo.name.contains("xl/drawings/_rels/") && fileInfo.name.right(5).toLower() == ".rels") {
                file.open(QIODevice::ReadOnly);
                QString xml = file.readAll();
                file.close();

                HtmlParser parser(&xml);
                parser = parser.nextTagByTagName("Relationship");
                while (parser.isValid()) {
                    int id = parser.attribute("Id").remove("rId").toInt();
                    QString target = parser.attribute("Target").replace("../", "xl/");
                    drawingRelsHash[fileInfo.name][id] = target;

                    parser = parser.nextTagByTagName("Relationship");
                }
            }

            if (fileInfo.name.contains("xl/media")) {
                file.open(QIODevice::ReadOnly);
                QByteArray data = file.readAll();
                file.close();

                QString extension = fileInfo.name.section(".", -1).toUpper();
                imageHash[fileInfo.name] = QImage::fromData(data, extension.toStdString().c_str());
           }

            if (fileInfo.name.contains("xl/worksheets/sheet")) {
                file.open(QIODevice::ReadOnly);
                QString xml = file.readAll();
                file.close();

                sheetXmlHash[fileInfo.name.remove("xl/")] = xml;
            }
        }

        QMapIterator<int, QString> iterator(sheetNameMap);
        while (iterator.hasNext()) {
            iterator.next();

            QString sheetFilepath = sheetHash[iterator.key()];
            QString xml = sheetXmlHash[sheetFilepath];

            SpreadsheetWorksheet *worksheet = mWorkbook->createWorksheet(iterator.value());

            HtmlParser parser(&xml);
            parser = parser.nextTagByTagName("c");
            while (parser.isValid()) {
                SpreadsheetCellPosition position = SpreadsheetCellPosition::fromCellAddress(parser.attribute("r"));
                QString type = parser.attribute("t");
                int styleIndex = parser.attribute("s").toInt();

                SpreadsheetCell *cell = worksheet->cell(position);
                {
                    QString c = parser.html();
                    HtmlParser cParser = HtmlParser(&c).nextTagByTagName("v");
                    if (cParser.isValid()) {
                        if (type == "s") {
                            int stringIndex = cParser.text().toInt();
                            cell->setValue(stringList[stringIndex]);
                        }
                        else {
                            cell->setValue(cParser.text());
                        }
                    }
                }

                {
                    QVariantMap style = styleList[styleIndex];
                    int borderIndex = style["borderId"].toInt();
                    int fontIndex = style["fontId"].toInt();
                    int fillIndex = style["fillId"].toInt();

                    QString hAlign = style["hAlign"].toString();
                    QString vAlign = style["vAlign"].toString();
                    cell->setAlignment((vAlign == "top"? Qt::AlignTop : (vAlign == "center"? Qt::AlignVCenter : Qt::AlignBottom)) | (hAlign == "center"? Qt::AlignHCenter : (hAlign == "right"? Qt::AlignRight : (hAlign == "justify"? Qt::AlignJustify : Qt::AlignLeft))));

                    bool wordWarp = style["wrapText"].toBool();
                    cell->setWordWarp(wordWarp);

                    float textRotation = style["textRotation"].toFloat();
                    cell->setRotation(textRotation);

                    QVariantMap border = borderList[borderIndex];
                    QString leftBorder = border["left"].toString();
                    QString rightBorder = border["right"].toString();
                    QString topBorder = border["top"].toString();
                    QString bottomBorder = border["bottom"].toString();

                    if (!leftBorder.isEmpty())
                        cell->setBorder(Spreadsheet::Left, toBorderType(leftBorder));

                    if (!rightBorder.isEmpty())
                        cell->setBorder(Spreadsheet::Right, toBorderType(rightBorder));

                    if (!topBorder.isEmpty())
                        cell->setBorder(Spreadsheet::Top, toBorderType(topBorder));

                    if (!bottomBorder.isEmpty())
                        cell->setBorder(Spreadsheet::Bottom, toBorderType(bottomBorder));

                    QVariantMap font = fontList[fontIndex];
                    QFont f;
                    f.setFamily(font["name"].toString());
                    f.setPointSize(font["size"].toInt());
                    f.setBold(font["bold"].toBool());
                    f.setItalic(font["italic"].toBool());
                    f.setUnderline(font["underline"].toBool());
                    cell->setFont(f);

                    QString color = font["color"].toString();
                    if (!color.isEmpty())
                        cell->setForegroundColor(QColor(color));

                    QVariantMap fill = fillList[fillIndex];
                    QString fillFgColor = fill["fgColor"].toString();
                    if (!fillFgColor.isEmpty())
                        cell->setBackgroundColor(fillFgColor);
                }

                parser = parser.nextTagByTagName("c");
            }

            {
                HtmlParser parser(&xml);
                parser = parser.nextTagByTagName("mergeCell");
                while (parser.isValid()) {
                    QStringList refList = parser.attribute("ref").split(":");
                    SpreadsheetCellPosition pos1 = SpreadsheetCellPosition::fromCellAddress(refList[0]);
                    SpreadsheetCellPosition pos2 = SpreadsheetCellPosition::fromCellAddress(refList[1]);

                    worksheet->mergeCell(pos1, pos2);
                    parser = parser.nextTagByTagName("mergeCell");
                }
            }

            {
                HtmlParser parser(&xml);
                parser = parser.nextTagByTagName("col");
                while (parser.isValid()) {
                    quint32 min = parser.attribute("min").toInt() - 1;
                    quint32 max = parser.attribute("max").toInt() - 1;
                    double width = ceil(parser.attribute("width").toDouble() * 7.00027344818157);
                    bool hidden = parser.attribute("hidden").toInt();

                    for (quint32 col=min; col<=max; col++)
                        worksheet->setColumnWidth(col, hidden? -1 : width);

                    parser = parser.nextTagByTagName("col");
                }
            }

            {
                HtmlParser parser(&xml);
                parser = parser.nextTagByTagName("row");
                while (parser.isValid()) {
                    if (!parser.attribute("ht").isNull()) {
                        quint32 row = parser.attribute("r").toInt() - 1;
                        quint64 height = parser.attribute("ht").toDouble() * 4 / 3;
                        bool hidden = parser.attribute("hidden").toInt();

                        worksheet->setRowHeight(row, hidden? -1 : height);
                    }

                    parser = parser.nextTagByTagName("row");
                }
            }

            {
                int rId = HtmlParser(&xml).nextTagByTagName("drawing").attribute("r:id").remove("rId").toInt();
                if (rId) {
                    QString sheetFileName = sheetFilepath.section("/", -1);
                    QString drawingFilepath = sheetRelsHash[QString("xl/worksheets/_rels/%1.rels").arg(sheetFileName)][rId];
                    QString drawingFilename = drawingFilepath.section("/", -1);
                    QList<QVariantMap> drawingList = drawingHash[drawingFilepath];
                    QHash<int, QString> &relationHash = drawingRelsHash[QString("xl/drawings/_rels/%1.rels").arg(drawingFilename)];

                    foreach (const QVariantMap &map, drawingList) {
                        int imageId = map["rId"].toString().remove("rId").toInt();
                        quint32 fromRow = map["fromRow"].toInt();
                        quint32 fromCol = map["fromCol"].toInt();
                        quint64 fromColOff = floor(map["fromColOff"].toDouble() / 9525);
                        quint64 fromRowOff = floor(map["fromRowOff"].toDouble() / 9525);
                        quint32 width = floor(map["width"].toDouble() / 9525);
                        quint32 height = floor(map["height"].toDouble() / 9525);

                        SpreadsheetDrawing *drawing = new SpreadsheetDrawing;
                        drawing->setPosition(SpreadsheetCellPosition(fromRow, fromCol));
                        drawing->setOffset(QPoint(fromColOff, fromRowOff));
                        drawing->setImage(imageHash[relationHash[imageId]]);
                        drawing->setSize(QSize(width, height));

                        worksheet->addDrawing(drawing);
                    }
                }
            }
        }
    }

    return true;
}

bool SpreadsheetXlsxFileHandler::save(const QString &)
{
    return false;
}


