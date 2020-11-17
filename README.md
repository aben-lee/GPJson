The GPJSON Format Specification

——Json Format for Geophysics Specification


Abstract: GPJSON which derived from GeoJSON is only for encoding geophysical sensor data structures based on JavaScript Object Notation (JSON). It defines several types of JSON objects and the manner in which they are combined to represent a tree-table structure (the tree structure represent base information and table structure present sensor data in geophysical data acquisition and process). It also defines several types of JSON objects and the manner in which they are combined to represent could represent a concrete binary metadata (in string format in json) or link external file (local or URL). The goal of GPJSON is to integrate the geoscience data compatible with existing GeoJSON format and to parse and transmit data from client (remote acquisition) to server (processing and interpretation) easily. 

Note: Geoarch is a geoscience data processing platform focusing on Data-flow diagram schemes.

1. Introduction
Because GeoJSON [RFC7946] is only a geospatial data interchange format based on JavaScript Object Notation (JSON) [RFC7159], GPJSON define several types of JSON objects the manner in which they are combined to represent geophysical sensor data mainly described as a combination of Tree - Table structure. In addition, GPJSON also define a several types that make a relevant GPJSON object represent binary data or link external file. For example, the data GPJSON object (in fact, in [[…]] type which mean arrays’ array of json formation) is a netCDF (network common data format) metadata according concrete type which have been define in this document, or is a URL directory of external file (*.nc) path.
Like GeoJSON, GPJSON present a feature collection of individual features. Each Feature has, at least 3 “attributes”: a fixed value “type” (“type”:” Feature”), an “aggregator” (which is similar to “geometry” of GeoJSON, but not for geospatial data) and a “properties”. The aggregator object SHOULD have “type”: “…”refer to “data” significance( “data” : [[…]] for the concrete data or external file or tree - table structure.
The GPJSON is mainly concerned with geoscience sensor data in the broadest sense, which mainly goal is to make geophysical sensor data parse and transmit easily from client (remote acquisition) to server (processing and interpretation) consistently via internet of Things. GPJSON can be used independently, it also compatible with GeoJSON format as GPJSON is derived from GeoJSON. 
1.1	Requirements Language
The key words “MUST”, “MUST NOT”, “REQUIRED”, “SHALL”, “SHALL NOT”, “SHOULD”, “SHOULD NOT”, “RECOMMENDED”, “NOT RECOMMENDED”, “MAY”, and “OPTIONAL” in this document are to be interpreted as described in [RFC2119].
1.2	Conventions Used in This Document
JSON (JavaScript Object Notation) is a lightweight, text-based, language-independent data interchange format. It includes 4 basic data types and 2 composite data types, and a total of 6 data types. The ordering of the members of any JSON object defined in this document MUST be considered irrelevant, as specified by [RFC7159]. 
In the following sections, JSON data types are represented as JSON+ space + data types, such as JSON Array. 
Some examples use the combination of a JavaScript single-line comment (//) followed by an ellipsis (...) as placeholder notation for content deemed irrelevant by the authors. These placeholders must of course be deleted or otherwise replaced, before attempting to validate the corresponding JSON code example. 
Whitespace is used in the examples inside this document to help illustrate the data structures, but it is not required. Unquoted whitespace is not significant in JSON.
1.3	Definitions
JavaScript Object Notation (JSON), and the terms object, member, name, value, array, number, true, false, and null, are to be interpreted as defined in [RFC7159]. 
Inside this document, the terms follow the definitions of [RFC7946]. There are specify additional term as following: type, id(option), features, properties, aggregator, FeatureCollection, Sheet, Binary and URI, stamps(option) etc.. 
For Tree-Table structure, the aggregator should be parsed as sheet structure if type is “Sheet” and the properties should be parsed as tree structure. There are a specific type (header attributes) for column header. 
1.4	Example
A GPJSON FeatureCollection:
{
       “type”: “FeatureCollection”,
       “id” : 1234，  // nember or string(option)
       “features”: [{
           “type”: “Feature”,
           “ aggregator “: {
               “type”: “Sheet”,  // the following json array should be parsed row table data structure
               “ fields”: [“Time/second”, “X/meter”, “Y/meter” ],  // as header of table
               “data” : [[“10.0”, 12.4, 12.4], [“30.0”, 23.4,23.4],[“50.0,34.7,34.7 ]] // the data of table
           },
           “properties”: {   // should be parsed as tree structure 
               “prop0”: “value0”
           }
         },{
           “type”: “Feature”,
           “ aggregator “: {
              “type”: “Binary”,
“ fields”: [“….”],   // “…” may as filename, e.g. “abc.nc” 
              “data”:  [[….]]   // “…” is binary data. see Section 4.4 of [RFC7493]. 
           },
           “properties”: {
               “prop0”: “value0”
           }
         },{
           “type”: “Feature”,
           “ aggregator “: {
              “type”: “URI”,   // see Section 5 of [RFC4648]
“ fields”: [….],  // “…”may as filename, e.g. “abc.nc” 
              “data”: [[….]],   //”… “ is URL directory of file,
           },
           “properties”: {
               “prop0”: “value0” 
           }
        }]
}
Note：The GPJSON texts should follow the constraints of Internet JSON (I-JSON) [RFC7493] for maximum interoperability. 
2. GPJSON object and text
GPJSON is derived from GeoJSON specification, that mean GPJSON object represents a Geometry, Feature, or collection of Features, see the Section 3 of see Section 2 & 3 of [RFC7493]. However, there are a special Feature (aggregator object) represent of geophysical sensor data. 
2.1	The properties of Feature
The properties object of Feature in GPJSON SHOULD be parsed a tree structure that is essential information for understanding the aggregator. Including data description and creator and creating data etc.  
3. Aggregator
The aggregator object of Feature is for encoding a variety of geophysical sensor data which are different from Geospatial data (or are not convenient for GeoJSON representation). There are 3 aggregator types for representing data manner. Every aggregator object is a GeoJSON object no matter where it occurs in a GeoJSON text. 
The “type” value of an aggregator MUST be one of “Sheet”, “Binary” and “URI”, similar to type value of Geometry of GeoJSON. 
3.1	Sheet
Mostly sensor data can be represented by table (or column sheet) structure. For type “Sheet”, the other attributes (except for type attributes) of aggregator object SHOULD be json array. The header of table (“fields”: […]) MAY be options, which represents the column header information. Every column name MAY be split two string by using “/” separator. The first string is column name in accordance with data array’s name, and the second string refers to this column units, e.g. Temperature/Celsius be split Temperature and Celsius for the column data value and its unit.


Alternative 2D table For example:
{    
    “ type”: “Sheet”,
    “fields”: [“Time/second”,	“X/meter”,	“Y/meter”,	“Temperature/Celsius”],
    “data”: [
         [1.00 ,23.00 ,13.00,20.00],
         [2.00,23.00 ,14.00 ,19.90]
         ……
    ]
}

The above compact format of aggregator should be parsed as following:
Time/second	X/meter	Y/meter	Temperature/Celsius
1.00 	23.00 	13.00 	20.00 
2.00 	23.00 	14.00 	19.90 
3.00 	23.00 	15.00 	19.80 
4.00 	23.00 	16.00 	19.70 
5.00 	23.00 	17.00 	19.60 
6.00 	23.00 	18.00 	19.50 
7.00 	23.00 	19.00 	19.40 
8.00 	23.00 	20.00 	19.30 
9.00 	23.00 	21.00 	19.20 
10.00 	23.00 	22.00 	19.10 


The 2D table, type known as table, is the main data structure of the relational model. 2D table structures have alternative data formats. Standard 2D table data MUST be represented as a one-dimensional JSON Array, in which each item is a JSON Object representing a record. Each member of a JSON Object includes fields (keys) and recordings (values) which also be parsed as column structure. The primary key of each record MUST be named “id”.

The standard format of of aggregator should be parsed as following:
[
    {
        “id”: 250,
        “name”: “Tom A Dijkstra “,
        “sex”: 1,
        “age”: 18
    },
    {
        “id”: 251,
        “name”: “aben lee”,
        “sex”: 1,
        “age”: 28
    }
]

3.2	Data page info
A data page is a common data method for tabular data, which may be retrieved by querying or paging. A data page is a wrapper around two-dimensional table data, containing more information about the list data itself.

A data page can contain OPTIONAL properties that represent information about the current data page. The following table lists the optional properties of the data page. Data page optional information as following:
	{Number} page - current page Number, the count MUST be an integer not less than 0, starting at 0.
	{Number} pageSize - Number of bars per page, MUST be greater than 0.
	{Number} total - the total Number of records in a list. MUST be an integer not less than 0.Represents the number of records under the current condition, not the number of records on this page.
	{String} orderBy - list sort rule. Multiple collations are separated by commas (,);The ascending or descending order is represented by asc or desc, separated by a space from the field name. Example: “id desc, name asc”.
	{String} Keyword - the search keyword to which the list belongs.
	{Object} condition - the collection of search conditions to which the list belongs. A property may or may not contain a keyword field, and if it does not, it is recommended that a keyword condition be attached to the search term when parsing.

For example:
{
    “page”: 0,
    “pageSize”: 30,
    “ type”: “Sheet”,
    “fields”: [“Time/second”,	“X/meter”,	“Y/meter”,	“Temperature/Celsius”],
    “data”: [
         [1.00 ,23.00 ,13.00,20.00],
         [2.00,23.00 ,14.00 ,19.90]
         ……
]
}

3.3	Binary
For type “Binary”, the data object attributes (JSON pairs) SHOULD be a binary, the identify name (name of JSON pair) is similar to filename, and the string (value of JSON pair), which is a binary data encoding string format, is relevant to identify name. 
For example:
{
   “type”: “Feature”,
   “aggregator”: {
      “type”: “Binary”,
       “fields”: [“abc.nc”],    // fillname
       “data”: [[“…..”]]  // data of fillename, “…” is binary data. see Section 4.4 of [RFC7493]. 
   },
   “properties”: {
       “prop0”: “value0”
   }
 }

The [“abc.nc”] is the identify name, and [[“…”]] is a binary data which is encode string. Since the identify name is suffix with”.nc”, that mean the [[“…”]] string is NetCDF encoding formation.
However, it is RECOMMENDED that this data be encoded in a string value in base64url just as Section 4.3.
3.4	URI
For type “URI”, the data object attributes (JSON pairs) SHOULD be link to an external file following the constraints of Internet JSON (I-JSON) [RFC7493]. The identify name (name of JSON pair) is filename (Uniform Resource Name, URN), and the string (value of data) is location of identify name (also is Uniform Resource Locator, URL). 

For example:
{
   “type”: “Feature”,
   “aggregator”: {
      “type”: “URI”,
      “fields”: [“abc.nc”],   // filename is abc.nc
“data”: [[“c: /data/abc.nc” ]]  // the location of file is  c:, of course, it may be a web location. 
   },
   “properties”: {
       “prop0”: “value0”
   }
 }

The “abc.nc” is the filename, and “c://abc.nc” is the filename stored location (a data subdirectory of parent of working directory). So, the “../data/abc.nc” is identified the file path and name. 
4. Date & Time  
The Date & Time is not a JSON data type. For date types, we MUST use JSON strings. To make it easier for dates to be displayed and parsed, we SHOULD use a more internet-friendly format for dates in ISO 8601 format [rfc3339].
Example:
{
    “created”: “2018-07-12T16:20:57-05:00”,
    “modified”: “2018-07-12T16:20:57-05:00”,
}

5. Stamp  
The stamp of properties can contain OPTIONAL attributes that may trace back the lifecycle of data. They are separated by “;” for different node processing information, and the information of each node can by split by “@” for Data & Time and node name. 
For example
properties :{    
    “stamp”: “2018-07-12T16:20:57-05:00@filter_10; 2018-07-12T16:21:57-03:20@fit_13 ”,
}
The lifecycle can track after there are 2 nodes (ilter_10 and fit_13) to processing the data, and the created time is 2018-07-12T16:20:57-05:00 and 2018-07-12T16:21:57-05:00 respectively.


References:
http://GeoJson.org
NETCDF format specification
[RFC 2119] “Key words for use in RFCs to Indicate Requirement Levels”
[RFC 4627] “The application/json Media Type for JavaScript Object Notation (JSON)”
[RFC 2616] “Hypertext Transfer Protocol”
[RFC 3339] “Date and Time on the Internet: Timestamps”
Dixson, N., Milliken, G., Mukunda, K., Murray, R., & Starry, R. (2020). GeoJSON Data Curation Primer.
Joan Maso and Alaitz Zabala. (2017) Testbed-12 JSON and GeoJSON User Guide. https://docs.opengeospatial.org/guides/16-122r1.html.
[RFC 7493 ] The I-JSON Message Format.
[RFC 7946 ] The GeoJSON Format
