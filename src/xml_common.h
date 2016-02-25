#ifndef XML_COMMON_HEADER_H
#define XML_COMMON_HEADER_H

typedef enum
{
  kXmlHintUnknown,
  kXmlHintEnded,

  kXmlHintStartDeclaration,
  kXmlHintStartElementOpen,
  kXmlHintStartElementClose,
  kXmlHintStartAttributeName,
  kXmlHintStartAttributeValue,
  kXmlHintStartInnerText,

  kXmlHintDeclaration,
  kXmlHintElementOpen,
  kXmlHintElementClose,
  kXmlHintAttributeName,
  kXmlHintAttributeValue,
  kXmlHintInnerText,

  kXmlHintEndDeclaration,
  kXmlHintEndElementOpen,
  kXmlHintEndElementClose,
  kXmlHintEndAttributeName,
  kXmlHintEndAttributeValue,
  kXmlHintEndInnerText,

} XmlHint;

#endif
