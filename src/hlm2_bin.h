#ifndef HLM2_BIN_HEADER_H
#define HLM2_BIN_HEADER_H

#include "string.h"
#include "stream.h"
#define XML_WANT_UTF
#include "xml.h"

typedef struct LocTable
{
  int index;
  String* name;
  int language_id;

  int item_count;
  String** items;
} Table;

typedef struct hlm2_bin_type
{
  int table_count;
  Table** tables;
} hlm2_bin;

void table_create(Table** t);
void table_destroy(Table** t);

Table* table_read(FStream* fs);
void table_print_lang(Table* t);

void hlm2_bin_create(hlm2_bin** bin);
void hlm2_bin_destroy(hlm2_bin** bin);

//int hlm2_find_index(hlm2_bin* bin);
int hlm2_bin_read(hlm2_bin* inst, FStream* fs);

int hlm2_bin_write_xml(hlm2_bin* inst, FStream* fs);

void test();

#endif
