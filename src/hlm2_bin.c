#include "hlm2_bin.h"

#include "memory.h"

void table_create(Table** t)
{
  (*t) = (Table*)mem_alloc(sizeof(Table));

  (*t)->index = 0;
  (*t)->name = 0;
  (*t)->language_id = -1;
  (*t)->item_count = 0;
  (*t)->items = 0;  
}

void table_destroy(Table** t)
{
  if ((*t)->name) {
    string_destroy(&((*t)->name));
  }

  if ((*t)->item_count > 0) {
    int i;

    for (i = 0; i < (*t)->item_count; ++i) {
      string_destroy(&((*t)->items[i]));
    }

    mem_free((*t)->items);
    (*t)->items = 0;
  }

  mem_free((*t));
  (*t) = 0;
}

String* table_make_string(int idx, int* o_data, char* s_data)
{
  String* str;
  int offset;
  
  offset = o_data[idx];

  str = string_from_cstring(&s_data[offset]);

  return str;
}

Table* table_read(FStream* fs)
{
  Table* t;
  int count;

  table_create(&t);

  t->index = stream_read_int(fs);
  t->name = stream_read_cstring(fs);
  t->language_id = stream_read_int(fs);

  count = stream_read_int(fs);

  if (count > 0) {
    int i, size, string_data_size;
    void* offset_data;
    void* string_data;

    size = sizeof(int) * count;
    offset_data = mem_alloc(size);

    stream_read(fs, offset_data, size);
    string_data_size = stream_read_int(fs);

    string_data = mem_alloc(string_data_size);
    stream_read(fs, string_data, string_data_size);

    t->items = (String**)mem_alloc(sizeof(String*) * count);

    for (i = 0; i < count; ++i) {
      t->items[i] = table_make_string(i, (int *)offset_data, (char *)string_data);
    }

    mem_free(offset_data);
    mem_free(string_data);
  }

  t->item_count = count;

  return t;
}

void hlm2_bin_create(hlm2_bin** bin)
{
  (*bin) = (hlm2_bin*)mem_alloc(sizeof(hlm2_bin));

  (*bin)->tables = 0;
  (*bin)->table_count = 0;
}

void hlm2_bin_destroy(hlm2_bin** bin)
{
  if ((*bin)->table_count > 0) {
    int i;
    
    for (i = 0; i < (*bin)->table_count; ++i) {
      table_destroy(&((*bin)->tables[i]));
    }

    mem_free((*bin)->tables);
    (*bin)->tables = 0;
  }

  mem_free((*bin));
  (*bin) = 0;
}

int hlm2_find_index(hlm2_bin* bin)
{
  int i;

  for (i = 0; i < bin->table_count; ++i) {
    if (bin->tables[i]->language_id == 0) {
      return i;
    }
  }

  return -1;
}

int hlm2_bin_read(hlm2_bin* inst, FStream* fs)
{
  int num;

  num = stream_read_int(fs);

  if (num > 0) {
    int i;

    inst->table_count = num;
    inst->tables = (Table**)mem_alloc(sizeof(Table*) * num);

    for (i = 0; i < num; ++i) {
      inst->tables[i] = table_read(fs);
    }
  }

  return 0;
}

// 

const unsigned char xml_hdr[] = "<?xml version=\"1.0\"?>\r\n";

const unsigned char node_open[] = "<";
const unsigned char node_closing[] = "</";
const unsigned char node_close[] = ">\r\n";

// 

#define WRITE(stream, xml) stream_write(stream, xml, sizeof(xml) -1);

int hlm2_bin_write_xml(hlm2_bin* inst, FStream* fs)
{
  int key_index;

  key_index = hlm2_find_index(inst);

  WRITE(fs, xml_hdr);
  WRITE(fs, "<hlm2>\r\n");

  if (key_index != -1) {
    int num, i;
    Table *keys;

    keys = inst->tables[key_index];
    num = keys->item_count;

    for (i = 0; i < inst->table_count; ++i) {
      if (i != key_index){
        int j;

        Table *loc = inst->tables[i];

        WRITE(fs, "<lang name=\"");
        stream_write_string(fs, loc->name);
        WRITE(fs, "\">\r\n");

        for (j = 0; j < num; ++j) {
          
          WRITE(fs, "<trans name=\"");
          stream_write_string(fs, keys->items[j]);
          WRITE(fs, "\">");

          stream_write_xml_string(fs, loc->items[j]);

          WRITE(fs, "</trans>\r\n");
        }

        WRITE(fs, "</lang>\r\n");
      }
    }
  }

  WRITE(fs, "</hlm2>\r\n");

  return 0;
}
