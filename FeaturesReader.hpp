/****************************************************************************
**
** Copyright (C) Eageo Information & Exploration Technology Co.,Ltd.,
**
** Use of this file is limited according to the terms specified by
** Eageo Exploration & Information Technology Co.,Ltd.  Details of
** those terms are listed in licence.txt included as part of the distribution
** package ** of this file. This file may not be distributed without including
** the licence.txt file.
**
** Contact aben.lee@foxmail.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FEATURESTREAMREADER_H
#define FEATURESTREAMREADER_H

#include <QThread>
#include <QTextStream>
#include <QFile>
#include <QLockFile>
#include <QRegularExpression>
#include <QDebug>
#include <QJsonObject>

/**
 *  FeaturesStreamReader, which is similar to QXmlStreamReader Class, is memory-conservative by design,
 *  since it doesn't store the entire JSON document memory unlike GPJson::FeatureCollect doing (in GPJson.hpp),
 *  but only the current GPJson::Feature at the time it is reported.
 *  读取大GPJson文件（节约内存方式），大致设计思想：
 *   1.多线程读入文件，全局信息ID 和 common 存入内存
 *   2.其他Feature在文件中的position 和 size 存入 List列表，等读取某个Feature式，从position 读入size字节作为当前Feature
 *   3.文件锁定，若文件发送变化，总发送信息重新扫描Feature信息列表。
 *   to do...
 *   写文件参见FeaturesWriter
 * */
namespace GPJson {

class  FeaturesReader  : public QThread
{
public:
   enum TokenType {

   };

    struct FeatureInfo {
         inline FeatureInfo(int p, int s ) : position(p), size(s)
                {}
        int position;    // the position of Feature in file.
        int size;   // the size of Feature in file.
    };

    explicit FeaturesReader(QObject *parent = nullptr);
        ~FeaturesReader();

    QByteArray id;
    QJsonObject common; //! general refer to properties object in feture object of gpjson
    QList<FeatureInfo> FeatureList;

    void  openFile(const QString &fileName);
    void    close();
    inline void  clear();

    bool atEnd() const;
    TokenType readNext();

protected:
    void run() override;  // 扫描文件产生不同Feature map表

private:
    Q_DISABLE_COPY(FeaturesReader)

    QByteArray _id;
    QList<FeatureInfo*> _links;

    int  _fileHandle;
    QTextStream _textStream;

    int _braceIDX;
    int _squareIDX;
    QString _fileName;
    QFile _file;
};

FeaturesReader::FeaturesReader(QObject *parent)
    : QThread(parent)
{
    id = QByteArray();
    common = QJsonObject();

    _braceIDX = 0;
    _squareIDX = 0;
    _textStream.setCodec("UTF-8");

    // connect(this,SIGNAL(findFeature()),this,SLOT());
}

FeaturesReader::~FeaturesReader()
{
    close();
}

void FeaturesReader::openFile(const QString &fileName)
{
        if(_fileHandle>0) close();

        _fileName = fileName;
        start();
}

void FeaturesReader::close()
{
    if(_fileHandle>0)
        FeatureList.clear();

    _fileHandle=-1;
}

void FeaturesReader::run()
{
    QFile file(_fileName);
    file.open( QIODevice::ReadOnly );
    QTextStream textStream( &file );
    textStream.setCodec("UTF-8");

    QString jsonKey;
    QString jsonString;
    QRegularExpression featuresRegex( "\"features\" \\s*:\\s*\\[\\s*\\s*\\{" );  //匹配 "features" : [ {
    QRegularExpression jsonRegex("[\\{\\}\\[\\] ]" );  //匹配 {}[] :
    FeatureInfo info = {-1,-1};

    int temPos = -1;
    while (!textStream.atEnd()) {
        temPos  += featuresRegex.match(   //查找  "features" : [ {
              textStream.read(100000)
               ).capturedEnd("{");   //查找矩阵标识[
        qDebug() << "The features : [ {  postion at :" << info.position;
        if(temPos >-1)
            break;  // 已找到首地址，跳出循环

        temPos += 100000 ; // add the amount of read data
        // to tell pos the file +10000?
    }

    if (temPos == -1) {  // 未找到features ：[{ 标识，返回
        qDebug() << "Not find Features!!!";
        return;
    }
    info.position += temPos;  // 第一个Feature 的{标识位置

    textStream.seek(info.position); // 定位

    int idPos =  info.position;
    while (!textStream.atEnd()) {   //迭代查找不同Feature 在文件中的信息 配对{}
        QRegularExpressionMatchIterator i = jsonRegex.globalMatch(textStream.read(100000));
        while (i.hasNext()) {
             QRegularExpressionMatch match = i.next();
             if (match.hasMatch()) {
                 QString matchString = match.captured(0); // matchString == "{" or" }" or "[" or "]";
                 if (matchString == "{")
                    _braceIDX += 1;
                 if (matchString == "}")
                    _braceIDX -= 1;
                 if (matchString == "[")
                    _squareIDX += 1;
                 if (matchString == "]")
                    _squareIDX -= 1;
             }
             if (_braceIDX == 0){  // feature 配对完成,此时matchString应为"}"
                 int matchPos = match.capturedStart(0);
                 info.size = matchPos + idPos - info.position;

                 _links.append( new FeatureInfo(info.position, info.size));  //  添加信息至_links

//                     if(!_links.isEmpty() && _links.count() == 1) //
                     /*emit findFeature()*/;   // 发现1个Feature，通知可以读取了
             }
             if (_squareIDX == -1){  // features 已全部配对完成，跳出循环
                 break;
             }
        }
        if (_squareIDX == -1){  // features 已全部配对完成，跳出循环
            break;
        }
        idPos += 100000;
    }
}

}

#endif // FEATURESTREAMREADER_H
