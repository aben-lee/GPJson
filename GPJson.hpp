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

#ifndef GPJSON_H
#define GPJSON_H

#include <QVector>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QVariant>
#include <QHash>
#include <QJsonParseError>
#include <QtGlobal>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
//#include "FeaturesReader.hpp"
//#include "FeaturesWriter.hpp"

/**
 * ！说明
 * 解析数据可的基本 Node 类
 * 1.按对话框时间间隔存储数据文件（连续采集时间）
 * 2.发送信号存储文件（触发采集，或变换采集参数存储已有的数据新创建文件）
 * 3.创建文件名称，前缀+时间+.gpjson (.gpbin 采用QJsondocument的 binaryData()文件格式
 * 4.存储为GPJson::FeaturesCollect.
 * 5.为了节约内存，大Json数据但GPJson::Feature文件最多5000条数据，然后GPJson::Feature追加至Collect
 *  function as following
 *  (a) formatChect();  检查json是否GPJson::Feature 标准
 *  (b) Feature 对象，管理Featrue数据（包括 Aggregator），详见GPJSON specfication。
 *         inline void fromGPjsonDocument(const QJsonDocument &jdoc); // 解析gpjson 文档只改对象
 *         inline QJsonDocument toGPJsonDocument() const;  // 该对象输出gpjson文档
 *  (c) FeatureCollect ();  解析GPJSon文件 (由于每次仅读入1个Feature，支持大文件读取)，支持写入新的Feature
 *
 *  to do... 2020.7.29 参照QxmlStreamReader & QxmlStreamWriter增加对大GPJSON 文件的支持
 *  原则，单个Feature 读入内存，FeatureCollect建立索引表，根据需要再从文件读入内存。
 */

namespace GPJson {

struct Aggregator;
struct Feature;

using rowData = QJsonArray;
typedef QList<GPJson::Feature> featureCollect; //! refer to featureCollect


inline bool formatCheck(const QByteArray &json);   // Stringify any GPJSON type.
inline featureCollect parse(const QJsonDocument &);    // Parse any GPJSON type.
inline QJsonDocument serialize(const featureCollect&); // Stringify any GPJSON type.


/**
 * FeatureCollect 对文件的读写
 * Readonly,检索文件的匹配的Feature,然后记录器起始位置和长度，存入list，下次直接从文件读出。
 * WriteOnly，若文件不存在，创建文件，写入标识符，然后逐渐添加Feature,析构函数添加补充json格式
 * to do..WriteReady,若文件不存在，见WriteOnly；若存在，读取，创建临时文件写入，点击存储，临时文件替换旧文件。
 * 采用独立线程读取
 * to do.. 为了便于管理大文件（gpjson）,选用个专用json 数据库管理（Future），解析数据存之数据库，
 *          ，然后从数据库转存至本文文件
*/

struct FeatureCollect
{
    FeatureCollect() : id(""){ common = QJsonObject(); }

    QByteArray id;
    QList<GPJson::Feature> list;
    QJsonObject common; //! general refer to properties object in feture object of gpjson

    inline void clear();
    inline void fromGPjsonDocument(const QJsonDocument &jdoc);
    inline QJsonDocument toGPJsonDocument() const;
};

struct Aggregator
{
    Aggregator() : type(""),page(0),pageSize(0) {/*fields.clear(); data.clear(); */}
    Aggregator(const QString &t) : type(t),page(0),pageSize(0) {/*fields.clear(); data.clear(); */}
    Aggregator(const Aggregator &other) : type(other.type),page(other.page),pageSize(other.pageSize)
            , fields(other.fields), data(other.data) {}
    Aggregator &operator = (const Aggregator &other) {
        type = other.type; page = other.page; pageSize = other.pageSize;
        fields = other.fields; data = other.data;
        return *this;
    }
        ~Aggregator() {fields = QJsonArray();data.clear();}

    bool operator == (const Aggregator &other) const {
        return type== other.type && page == other.page && pageSize == other.pageSize
                && fields == other.fields && data == other.data;
    }

    QJsonValue at ( int row, int column) const {
        if(row < 0 || row  >= data.count() ||  column < 0 || column >= data.at(0).count() )
            return QJsonValue();
        else
            return data.at(row).at(column);
    }

    void replace ( int row, int column, const QJsonValue & value ){
        if(row < 0 || row  >= data.count() ||  column < 0 || column >= data.at(0).count() )
            return;
        else {
            QJsonArray array = data.at(row);
            array.replace(column, value);
            data.replace(row,array);
        }
    }

    QJsonArray column(int index) const {
         if( (index < 0 ) || ( index >= data.at(0).count() ) ) return QJsonArray();
         QJsonArray result;
         for( int i = 0; i < data.count(); ++i )
             result << data.at(i).at(index);
         return result;
    }
    void insertColumn(int column, QJsonArray array){
        if(column <= 0) column = 0;
        if(column >= data.at(0).count() ) column = data.at(0).count();

        QVector<rowData>::iterator i;
        for(i = data.begin(); i != data.end(); ++i)
            (*i).insert(column,QJsonValue());

        int rows = qMin( data.count(), array.count() );
        for( int r = 0; r < rows; ++r)
            replace(r,column,array.at(r));
    }

    QJsonArray row(int index) const {
         if( ( index < 0 ) || ( index >= data.count() ) ) return QJsonArray();
         return  data.at(index);
    }
    void insertRow(int row, QJsonArray array){
        if( row <=0 )    row = 0;
        if(row>=data.count())  row = data.count();

        data.insert(row,array);
    }

    QString type;           //! refer to gpjson specification
    int page;
    int pageSize;
    QJsonArray fields;
    QVector<rowData> data;
};


struct Feature
{
    Feature() : id(""){ properties = QJsonObject();  aggregator = Aggregator(); }
    Feature(const QJsonDocument &jdoc) {fromGPjsonDocument(jdoc);}
    Feature(const QByteArray &index) : id(index){ properties = QJsonObject();  aggregator = Aggregator(); }
    Feature(const Feature &other) : id(other.id),aggregator(other.aggregator),properties(other.properties) {}
    Feature &operator = (const Feature &other) {
        id = other.id; aggregator = other.aggregator; properties = other.properties;
        return *this;
    }
        ~ Feature() {}

    bool operator == (const Feature &other) const {
        return id == other.id && aggregator == other.aggregator && properties == other.properties;
    }

    inline void fromGPjsonDocument(const QJsonDocument &jdoc);
    inline QJsonDocument toGPJsonDocument() const;

    QByteArray id;          //! option in feture object of gpjson
    Aggregator aggregator;  //! refer to aggregator object in feture object of gpjson
    QJsonObject properties; //! refer to properties object in feture object of gpjson
};

///////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline T parse(const QJsonDocument &);

template <typename T>
inline QJsonObject convert(const T &);

template <>
inline GPJson::Aggregator parse<GPJson::Aggregator>(const QJsonDocument &jdoc)
{
//    auto const& jdoc = QJsonDocument::fromJson(json);
    if (jdoc.isNull())
        qCritical("cannot load json");

    if (!jdoc.isObject())
        qCritical("Aggregator must be an object");

    if (!jdoc.object().contains("type"))
         qCritical("Aggregator must have a type property");
    if (!jdoc.object().contains("fields"))
         qCritical("Aggregator must have a fields property");
    if (!jdoc.object().contains("data"))
         qCritical("Aggregator must have a data property");

    GPJson::Aggregator agg;
    const auto &type = jdoc.object().value("type").toString().toLower();


    if (type == "sheet" || type == "binary" || type == "uri")
    {
        agg.type = type;

        if(jdoc.object().value("fields").isArray())
        {
            agg.fields = jdoc.object().value("fields").toArray();
        }
        if(jdoc.object().value("data").isArray())
        {
             for ( QJsonValue v : jdoc.object().value("data").toArray())
             {
                 agg.data.push_back(v.toArray());
             }
        }
        else {
           qCritical("the value of data must be json array");
        }

    }
    else {
       qCritical("Unknow the type value");
    }
    return agg;
}

template <>
inline GPJson::Feature parse<GPJson::Feature>( const QJsonDocument &jdoc)
{
//    auto const& jdoc = QJsonDocument::fromJson(json);
    if (jdoc.isNull())
        qCritical("cannot load json");

    if (!jdoc.isObject())
        qCritical("Feature must be an object");

    if (!jdoc.object().contains("type"))
         qCritical("Feature must have a type property");
    if (jdoc.object().value("type").toString() != "Feature")
         qCritical("Feature type must be Feature");

    if (!jdoc.object().contains("aggregator"))
         qCritical("Feature must have aggregator property");

    GPJson::Feature fea;
    fea.aggregator = parse<GPJson::Aggregator>(QJsonDocument(jdoc.object().value("aggregator").toObject()));

    if (jdoc.object().contains("id"))
        fea.id = jdoc.object().value("id").toVariant().toByteArray();

    if (!jdoc.object().contains("properties"))
        qCritical("Feature type must be properties");
    if (jdoc.object().value("properties").isObject() )
        fea.properties = jdoc.object().value("properties").toObject();
    else
        qCritical("properties must be an object");
    return fea;
}

template <>
inline featureCollect parse<featureCollect>(const QJsonDocument &jdoc)
{
//    auto const& jdoc = QJsonDocument::fromJson(json);

    if (jdoc.isNull())
        qCritical("cannot load json");
    if (!jdoc.isObject())
        qCritical("GPJson must be an object 2");

    if (!jdoc.object().contains("type"))
         qCritical("GPJson must have a type property");

    const auto &type = jdoc.object().value("type").toString();

    GPJson::Feature feature;
    QList<GPJson::Feature> featureCollect;
    if (type == "FeatureCollection") {
        if (!jdoc.object().contains("features"))
             qCritical("Feature must have a features property");

        const auto &json_features = jdoc.object().value("features");

        if (!json_features.isArray())
            qCritical("FeatureCollection features property must be an array");

        QJsonArray array = json_features.toArray();

        for (QJsonValue v : array) {
            feature = parse<GPJson::Feature>(QJsonDocument(v.toObject()));
            featureCollect.append(feature);
        }
    }
    if (type == "Feature")
    {
        featureCollect.reserve(1);
        feature = parse<GPJson::Feature>(jdoc);
        featureCollect.append(feature);
    }
    return featureCollect;
}

inline featureCollect parse(const QJsonDocument &jdoc) {

    return parse<featureCollect>(jdoc);
}

template <>
inline QJsonObject convert<GPJson::Aggregator>(const GPJson::Aggregator& element)
{
    QHash<QString, QVariant> result;
    result["type"]=element.type;
    result["page"]=element.page;
    result["pageSize"]=element.pageSize;

    QString str = element.type.toLower();
    if(str == "sheet" || str == "binary" || str == "uri" )
    {
        result["fields"] = element.fields;
        QJsonArray array;
        for (GPJson::rowData row : element.data  )
        {
           array << row;
        }
        result["data"] = array;
    }
    else
    {
        qCritical("The type in Aggregator is NOT list in GPJson specification");
    }

    return QJsonObject::fromVariantHash(result);
}

template <>
inline QJsonObject convert<GPJson::Feature>(const GPJson::Feature& element)
{
    QHash<QString, QVariant> result;
    result["type"] = "Feature";
    result["aggregator"] = convert<GPJson::Aggregator>(element.aggregator);
    result["properties"] = element.properties;
    if (!element.id.isNull()) {
        result["id"] = element.id;
    }
   return QJsonObject::fromVariantHash(result);
}

template <>
inline QJsonObject convert<featureCollect>(const featureCollect& element)
{
    QHash<QString, QVariant> result;
    result["type"] = "FeatureCollection";
    QJsonArray array;
    for(GPJson::Feature fea : element) {
        array << convert<GPJson::Feature>(fea);
    }
    result["features"] = array;
    return QJsonObject::fromVariantHash(result);
}

inline QJsonDocument serialize(const featureCollect &element)
{
   return QJsonDocument(convert<featureCollect>(element));
}

inline bool formatCheck(const QByteArray &json)
{
    QJsonParseError *error = new QJsonParseError;
    auto const& jdoc = QJsonDocument::fromJson(json,error);

    if(error->error != QJsonParseError::NoError)
        return  false; //ECF::Result(false, "It is not valid json data!", error->errorString());

    if (jdoc.isNull())
       return  false; //ECF::Result(false, "The json document is NULL!");

    if (!jdoc.isObject())
       return  false;  // ECF::Result(false, "GPJson must be an object!");

    if (!jdoc.object().contains("type"))
       return  false; //ECF::Result(false, "GPJson must have a {type: --} property!");

    const auto &type = jdoc.object().value("type").toString();

    if (type == "FeatureCollection") {
        if (!jdoc.object().contains("features"))
             return  false; //  ECF::Result(false, "GPJson must have a features:-- property for {type : FeatureCollection}!");

        const auto &json_features = jdoc.object().value("features");

        if (!json_features.isArray())
            return  false; //  ECF::Result(false, "features property must be an array for FeatureCollection!");

        QJsonArray array = json_features.toArray();
        for(QJsonArray::const_iterator it = array.constBegin(), end = array.constEnd(); it!= end; ++it)
        {
            formatCheck(QJsonDocument(it->toObject()).toJson());
        }
    }
    if (type == "Feature")
    {
        if (!jdoc.object().contains("aggregator"))
             return  false; //  ECF::Result(false, "GPJson must have a {aggregator: --} property!");

        if (!jdoc.object().contains("properties"))
             return  false; //  ECF::Result(false, "GPJson must have a {properties: --} property!");

        if(!jdoc.object().value("aggregator").isObject())
             return  false; //  ECF::Result(false, "Aggregator must be an object!");

        const auto &agg = jdoc.object().value("aggregator").toObject();
        if(!agg.contains("type"))
             return  false; //  ECF::Result(false, "The aggregator must have a type property!");

        if (!agg.contains("fields"))
            return  false; //  ECF::Result(false, "the aggregator must have a fields property!");
        if(!jdoc.object().value("aggregator").toObject().value("fields").isArray())
            return  false; //  ECF::Result(false, "the fields must be json array[]!");

        if (!agg.contains("data"))
            return  false; //  ECF::Result(false, "the aggregator must have a data property!");
        if(!jdoc.object().value("aggregator").toObject().value("data").isArray())
            return  false; //  ECF::Result(false, "the data must be json array[[]]!");
        if(!jdoc.object().value("aggregator").toObject().value("data").toArray().takeAt(0).isArray())
            return  false; //  ECF::Result(false, "the data must be json array[[]]!");

        const auto &type = jdoc.object().value("aggregator").toObject().value("type").toString();
        if (type != "Sheet" && type != "Binary" && type != "URI")
            return   false; // ECF::Result(false, "The type of aggregator is not recognized!");

        if(!jdoc.object().value("properties").isObject())
            return  false; //  ECF::Result(false, "The properties must be an object!");

    }
    return  true; //  ECF::Result(true, "The properties must be an object!");
}

inline void FeatureCollect::clear()
{
    id = QByteArray();
    common = QJsonObject();
    list.clear();
}

inline void FeatureCollect::fromGPjsonDocument(const QJsonDocument &jdoc)
{
//    if (jdoc.isNull())
//        qCritical("cannot load json");
//    if (!jdoc.isObject())
//        qCritical("GPJson must be an object ");

//    if (!jdoc.object().contains("type"))
//         qCritical("GPJson must have a type property");
    if(!formatCheck(jdoc.toJson()))
        return;

    const auto &type = jdoc.object().value("type").toString();

    GPJson::Feature fea;
    if (type == "FeatureCollection") {
//        if (!jdoc.object().contains("features"))
//             qCritical("Feature must have a features property");

        const auto &json_features = jdoc.object().value("features");

//        if (!json_features.isArray())
//            qCritical("FeatureCollection features property must be an array");

        QJsonArray array = json_features.toArray();
        const auto &size = array.size();
        list.reserve(size);

        for (QJsonValue v : array) {
            fea = parse<GPJson::Feature>(QJsonDocument(v.toObject()));
            list.append(fea);
        }
    }
    if (type == "Feature")
    {
        list.reserve(1);
        fea = parse<GPJson::Feature>(jdoc);
        list.append(fea);
    }

    if (!jdoc.object().contains("id"))
        id = jdoc.object().value("id").toVariant().toByteArray();

    if (!jdoc.object().contains("common"))
        common = jdoc.object().value("common").toObject();
}

inline QJsonDocument FeatureCollect::toGPJsonDocument() const
{
    QHash<QString, QVariant> result;
    result["type"] = "FeatureCollection";
    result["common"] = common;
    QJsonArray array;
    for(GPJson::Feature fe : list) {
        array << convert<GPJson::Feature>(fe);
    }
    result["features"] = array;
    return QJsonDocument(QJsonObject::fromVariantHash(result));
}

inline void Feature::fromGPjsonDocument(const QJsonDocument &jdoc)
{
//    if (jdoc.isNull())
//        qCritical("cannot load json");

//    if (!jdoc.isObject())
//        qCritical("Feature must be an object");

//    if (!jdoc.object().contains("type"))
//         qCritical("Feature must have a type property");
//    if (jdoc.object().value("type").toString() != "Feature")
//         qCritical("Feature type must be Feature");

//    if (!jdoc.object().contains("aggregator"))
//         qCritical("Feature must have aggregator property");

    if(!formatCheck(jdoc.toJson()))
        return;

    aggregator = parse<GPJson::Aggregator>(QJsonDocument(jdoc.object().value("aggregator").toObject()));

    if (jdoc.object().contains("id"))
        id = jdoc.object().value("id").toVariant().toByteArray();

//    if (!jdoc.object().contains("properties"))
//        qCritical("Feature type must be properties");
    if (jdoc.object().value("properties").isObject() )
        properties = jdoc.object().value("properties").toObject();
//    else
//        qCritical("properties must be an object");
}

QJsonDocument Feature::toGPJsonDocument() const
{
    QHash<QString, QVariant> result;
    result["type"] = "Feature";
    result["aggregator"] = convert<GPJson::Aggregator>(aggregator);
    result["properties"] = properties;
    if (!id.isNull()) {
        result["id"] = id;
    }
   return QJsonDocument(QJsonObject::fromVariantHash(result));
}

}

Q_DECLARE_TYPEINFO( GPJson::Feature, Q_MOVABLE_TYPE );

#endif // GPJSON_H
