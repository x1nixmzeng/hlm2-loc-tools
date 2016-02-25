#include "xml.h"
#include "memory.h"

// Heavily inspired by yxml library by Yoran Heling

void xml_create(Xml** xml)
{
  *xml = (Xml*)mem_alloc(sizeof(Xml));
  
  (*xml)->range = 0;
  (*xml)->cursor = 0;
  (*xml)->context = kXmlHintUnknown;
  
  XmlRangeCreate(&(*xml)->range);
}

void xml_destroy(Xml** xml)
{
  XmlRangeDestroy(&(*xml)->range);

  mem_free(*xml);
}

void xml_skip_whitespace(Xml* xml)
{
  while (xml->cursor < xml->range->end) {
    switch (*xml->cursor) {
      case ' ':
      case '\r':
      case '\t':
      case '\n':
        ++xml->cursor;
        break;
      case 0:
      default:
        return;
    }
  }
}

int xml_char(const XmlStride c)
{
  if ((c >= 'a' && c <= 'z')
    || (c >= 'A' && c <= 'Z')
    || (c >= '0' && c <= '9')
    || c == '_' || c == '.' ) {
    return 0;
  } else {
    return 1;
  }
}

XmlStride xml_entity(Xml *xml)
{
  const XmlStride *ref;

  ref = xml->cursor +1;

  if (range_within(xml->range, ref + 5) == 0 && ref[4] == ';') {
    // quot
    if (ref[0] == 'q' && ref[1] == 'u' && ref[2] == 'o' && ref[3] == 't') {
      xml->cursor += 4;
      return (XmlStride)'"';
    }

    // apos
    if (ref[0] == 'a' && ref[1] == 'p' && ref[2] == 'o' && ref[3] == 's') {
      xml->cursor += 4;
      return (XmlStride)'\'';
    }

    // nbsp (faked)
    if (ref[0] == 'n' && ref[1] == 'b' && ref[2] == 's' && ref[3] == 'p') {
      xml->cursor += 4;
      return (XmlStride)' ';
    }
  }

  if (range_within(xml->range, ref + 4) == 0 && ref[3] == ';') {
    // amp
    if (ref[0] == 'a' && ref[1] == 'm' && ref[2] == 'p') {
      xml->cursor += 5;
      return (XmlStride)'&';
    }
  }

  if (range_within(xml->range, ref + 3) == 0 && ref[2] == ';') {
    // lt
    if (ref[0] == 'l' && ref[1] == 't') {
      xml->cursor += 3;
      return (XmlStride)'<';
    }

    // gt
    if (ref[0] == 'g' && ref[1] == 't') {
      xml->cursor += 3;
      return (XmlStride)'>';
    }
  }

  return *xml->cursor;
}

XmlHint xml_parse(Xml* xml)
{
  switch (xml->context)
  {
    case kXmlHintUnknown:
    {
      // sanity check
      if (xml->cursor == 0) {
        xml->cursor = xml->range->begin;
      }
    }
    case kXmlHintEndDeclaration:
    case kXmlHintEndElementClose:
    case kXmlHintEndInnerText:
    {
      xml_skip_whitespace(xml);

      if (*xml->cursor == '<') {
        ++xml->cursor;

        switch (*xml->cursor) {
        case '/':
          ++xml->cursor;
          //xml->last = *xml->cursor;
          xml->context = kXmlHintStartElementClose;
          break;
        case '?':
          ++xml->cursor;
          //xml->last = *xml->cursor;
          xml->context = kXmlHintStartDeclaration;
          break;
        default:
          //xml->last = *xml->cursor;
          xml->context = kXmlHintStartElementOpen;
        }
      } else {
        xml->context = kXmlHintEnded;
      }
    }
    break;

    case kXmlHintEndElementOpen:
    {
      xml_skip_whitespace(xml);

      if(*xml->cursor == '<') {
        xml->cursor++;
        if (*xml->cursor == '/') {
          ++xml->cursor;
          xml->context = kXmlHintStartElementClose;
        } else {
          //xml->last = *xml->cursor;
          xml->context = kXmlHintStartElementOpen;
        }
      } else {
        xml->context = kXmlHintStartInnerText;

        if (*xml->cursor == '&') {
          //xml->last = xml_entity(xml);
          xml->context = kXmlHintInnerText;
        } else
        {
          //xml->last = *xml->cursor;
        }
      }
    }
    break;
    
    case kXmlHintStartInnerText:
    {
      ++xml->cursor;

      if (*xml->cursor == '<') {
        xml->context = kXmlHintEndInnerText;
      }
      else {
        xml->context = kXmlHintInnerText;
      }
    }
    break;

    case kXmlHintInnerText:
    {
      if (*xml->cursor == '<' ) {
        xml->context = kXmlHintEndInnerText;
      //} else if (*xml->cursor == '&') {
      //  //xml->last = xml_entity(xml);
      //  xml->context = kXmlHintInnerText;
      } else {
        ++xml->cursor;

        if (*xml->cursor == '<') {
          xml->context = kXmlHintEndInnerText;
        }
        //else if (*xml->cursor == '&') {
        //  //xml->last = xml_entity(xml);
        //  xml->context = kXmlHintInnerText;
        //}
        else {
          //xml->last = *xml->cursor;
          xml->context = kXmlHintInnerText;
        }
      }
    }
    break;

    case kXmlHintStartDeclaration:
    case kXmlHintStartElementOpen:
    case kXmlHintStartElementClose:
    {
      ++xml->cursor;
      XmlStride chr = *xml->cursor;
  
      if (xml_char(chr) == 0) {
        switch (xml->context) {
        case kXmlHintStartElementOpen:
          //xml->last = *xml->cursor;
          xml->context = kXmlHintElementOpen;
          break;
        case kXmlHintStartElementClose:
          //xml->last = *xml->cursor;
          xml->context = kXmlHintElementClose;
          break;
        case kXmlHintStartDeclaration:
          //xml->last = *xml->cursor;
          xml->context = kXmlHintDeclaration;
          break;
        default:
          xml->context = kXmlHintEnded;
        }
      } else {
        xml->context = kXmlHintEnded;
      }
    }
    break;

    case kXmlHintDeclaration:
    {
      ++xml->cursor;
      XmlStride chr = *xml->cursor;
  
      if (xml_char(chr) == 0) {
        //xml->last = *xml->cursor;
      }
      else if (chr == ' ') {
        ++xml->cursor;
        //xml->last = *xml->cursor;
        xml->context = kXmlHintStartAttributeName;
      } else if (chr == '?') {
        ++xml->cursor;
        if (*xml->cursor == '>') {
          ++xml->cursor;
          xml->context = kXmlHintEndDeclaration;
        } else {
          xml->context = kXmlHintEnded;
        }
      }
    }
    break;

    case kXmlHintElementOpen:
    {
      ++xml->cursor;
      XmlStride chr = *xml->cursor;
  
      if (xml_char(chr) == 0) {
        //xml->last = *xml->cursor;
      }
      else if (chr == ' ') {
        ++xml->cursor;
        //xml->last = *xml->cursor;
        xml->context = kXmlHintStartAttributeName;
      } else if (chr == '>') {
        ++xml->cursor;
        //xml->last = *xml->cursor;
        xml->context = kXmlHintEndElementOpen;
      }
    }
    break;

    case kXmlHintElementClose:
    {
      ++xml->cursor;
      XmlStride chr = *xml->cursor;
  
      if (xml_char(chr) == 0) {
        //xml->last = *xml->cursor;
      } else if (chr == '>') {
        ++xml->cursor;
        xml->context = kXmlHintEndElementClose;

        if (xml->cursor == xml->range->end) {
          xml->context = kXmlHintEnded;
        }
      }
    }
    break;

    case kXmlHintAttributeName:
    case kXmlHintStartAttributeName:
    {
      ++xml->cursor;
      XmlStride chr = *xml->cursor;

      if (xml_char(chr) == 0) {
        //xml->last = *xml->cursor;
        xml->context = kXmlHintAttributeName;
      } else if (chr == '=') {
        ++xml->cursor;
        xml->context = kXmlHintEndAttributeName;
      } else {
        xml->context = kXmlHintEnded;
      }
    }
    break;

    case kXmlHintEndAttributeName:
    {
      if (*xml->cursor == '"') {
        ++xml->cursor;
        //xml->last = *xml->cursor;
        xml->context = kXmlHintStartAttributeValue;
      } else if (*xml->cursor == '>') {
        ++xml->cursor;
        xml->context = kXmlHintEndElementOpen;
      }
    }
    break;

    case kXmlHintStartAttributeValue:
    {
      ++xml->cursor;
      XmlStride chr = *xml->cursor;

      if (xml_char(chr) == 0) {
        //xml->last = *xml->cursor;
        xml->context = kXmlHintAttributeValue;
      } else if (chr == '"') {
        xml->context = kXmlHintEndAttributeValue;
      } else if (chr == '>') {
        xml->context = kXmlHintEnded;
      }
    }
    break;

    case kXmlHintAttributeValue:
    {
      ++xml->cursor;
      XmlStride chr = *xml->cursor;

      if (xml_char(chr) == 0) {
        //xml->last = *xml->cursor;
      } else if (chr == '"') {
        xml->context = kXmlHintEndAttributeValue;
      } else if (chr == '>') {
        xml->context = kXmlHintEnded;
      }
    }
    break;

    case kXmlHintEndAttributeValue:
    {
      ++xml->cursor;

      switch (*xml->cursor) {
      case '>':
        ++xml->cursor;
        xml->context = kXmlHintEndElementOpen;
        break;
      case '?':
        ++xml->cursor;
        if (*xml->cursor == '>') {
          ++xml->cursor;
          xml->context = kXmlHintEndDeclaration;
        } else {
          xml->context = kXmlHintEnded;
        }
        break;
      case ' ':
        // todo: check this isn't just whitespace
        ++xml->cursor;
        //xml->last = *xml->cursor;
        xml->context = kXmlHintStartAttributeName;
        break;
      default:
        xml->context = kXmlHintEnded;
      }
    }
    break;

    default:
    {
      xml->context = kXmlHintEnded;
    }
    break;
  }

  return xml->context;
}

XmlStride xml_extract(Xml* xml)
{
  switch (xml->context)
  {
    case kXmlHintStartDeclaration:
    case kXmlHintStartElementOpen:
    case kXmlHintStartElementClose:
    case kXmlHintStartAttributeName:
    case kXmlHintDeclaration:
    case kXmlHintElementOpen:
    case kXmlHintElementClose:
    case kXmlHintAttributeName:
      return *xml->cursor;

    case kXmlHintStartAttributeValue:
    case kXmlHintStartInnerText:
    case kXmlHintAttributeValue:
    case kXmlHintInnerText:
      return xml_entity(xml);

    default:
      return (XmlStride)0;
  }
}
