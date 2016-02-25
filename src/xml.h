#ifndef XML_HEADER_H
#define XML_HEADER_H

#include "xml_common.h"

//#ifdef XML_WANT_UTF
#include "textrange.h"

typedef char            XmlStride;
typedef Range           XmlRange;
#define XmlRangeCreate  range_create
#define XmlRangeDestroy range_destroy
//#else
////#include "wtextrange.h"
////
////typedef short           XmlStride;
////typedef WRange          XmlRange;
////#define XmlRangeCreate  wrange_create
////#define XmlRangeDestroy wrange_destroy
//#endif

typedef struct XmlRange
{
  XmlRange *range;
  const XmlStride* cursor;
  XmlHint context;
  //XmlStride last;
} Xml;

void xml_create(Xml** xml);
void xml_destroy(Xml** xml);

XmlHint xml_parse(Xml* xml);

XmlStride xml_extract(Xml* xml);

#endif
