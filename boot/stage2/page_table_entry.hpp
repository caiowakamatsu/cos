#ifndef COS_PAGE_TABLE_ENTRY_HPP
#define COS_PAGE_TABLE_ENTRY_HPP

namespace cos {

using qword = unsigned long long;

struct physical_page {
public:
  physical_page(unsigned long long page_number) : page_number(page_number) {}

  operator unsigned long long() const noexcept { return page_number; }

private:
  unsigned long long page_number;
};

struct ptl4_entry {
  qword present : 1;
  qword read_write : 1;
  qword user_supervisor : 1;
  qword page_write_through : 1;
  qword page_cache_disable : 1;
  qword accessed : 1;
  qword ignored : 1;
  qword must_be_zero : 2;
  qword available_to_software0 : 3;
  qword ptl3 : 40;
  qword available_to_software1 : 11;
  qword no_execute : 1;
};

struct ptl3_entry {
  qword present : 1;
  qword read_write : 1;
  qword user_supervisor : 1;
  qword page_write_through : 1;
  qword page_cache_disable : 1;
  qword accessed : 1;
  qword dirty : 1;
  qword page_size : 1;
  qword global : 1;
  qword available_to_software0 : 3;
  qword ptl2 : 40;
  qword available_to_software1 : 11;
  qword no_execute : 1;
};

struct ptl2_entry {
  qword present : 1;
  qword read_write : 1;
  qword user_supervisor : 1;
  qword page_write_through : 1;
  qword page_cache_disable : 1;
  qword accessed : 1;
  qword dirty : 1;
  qword page_size : 1;
  qword global : 1;
  qword available_to_software0 : 3;
  qword ptl1 : 40;
  qword available_to_software1 : 11;
  qword no_execute : 1;
};

struct ptl1_entry {
  qword present : 1;
  qword read_write : 1;
  qword user_supervisor : 1;
  qword page_write_through : 1;
  qword page_cache_disable : 1;
  qword accessed : 1;
  qword dirty : 1;
  qword page_attribute_table : 1;
  qword global : 1;
  qword available_to_software0 : 3;
  qword page_number : 40;
  qword available_to_software1 : 7;
  qword protection_keys : 4;
  qword no_execute : 1;
};

} // namespace cos

#endif
