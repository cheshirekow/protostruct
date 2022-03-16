// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <functional>

#include <libelf.h>
#include <loki/ScopeGuard.h>

#include "argue/argue.h"

struct ProgramOptions {
  std::string filepath;
};

enum ElfClass { ELF32 = 1, ELF64 = 2 };

struct Traits32 {
  typedef Elf32_Shdr Shdr;
  typedef Elf32_Sym Sym;

  Shdr* (*getshdr)(Elf_Scn*);
  Traits32() : getshdr{elf32_getshdr} {}
};

struct Traits64 {
  typedef Elf64_Shdr Shdr;
  typedef Elf64_Sym Sym;

  Shdr* (*getshdr)(Elf_Scn*);
  Traits64() : getshdr{elf64_getshdr} {}
};

template <class Traits>
bool symbol_table_has_sentinel(Elf* elf, Elf_Scn* symtab_scn) {
  typedef typename Traits::Shdr Shdr;
  typedef typename Traits::Sym Sym;
  Traits traits;

  Shdr* shdr = traits.getshdr(symtab_scn);
  Elf_Scn* string_scn = elf_getscn(elf, shdr->sh_link);
  Elf_Data* string_data = elf_getdata(string_scn, nullptr);
  const char* strings = static_cast<char*>(string_data->d_buf);

  for (Elf_Data* data = elf_getdata(symtab_scn, nullptr); data;
       data = elf_getdata(symtab_scn, data)) {
    assert(data->d_type == ELF_T_SYM);
    Sym* begin = static_cast<Sym*>(data->d_buf);
    Sym* end = begin + (data->d_size / sizeof(Sym));
    for (Sym* sym = begin; sym < end; sym++) {
      if (sym->st_name >= string_data->d_size) {
        continue;
      }
      const char* name = &strings[sym->st_name];
      if (std::strcmp(name, "ARGUE_AUTOCOMPLETE_ME") == 0) {
        return true;
      }
    }
  }
  return false;
}

template <class Traits>
int has_magic_symbol(Elf* elf) {
  typedef typename Traits::Shdr Shdr;
  Traits traits;

  for (Elf_Scn* scn = elf_getscn(elf, 0); scn; scn = elf_nextscn(elf, scn)) {
    Shdr* shdr = traits.getshdr(scn);
    switch (shdr->sh_type) {
      case SHT_SYMTAB:
      case SHT_DYNSYM:
        if (symbol_table_has_sentinel<Traits>(elf, scn)) {
          return 0;
        }
        break;
      default:
        continue;
    }
  }
  return 1;
}

int file_is_magic_elf(const std::string filepath) {
  int fd = open(filepath.c_str(), O_RDONLY);
  if (fd == -1) {
    return 1;
  }
  Loki::ScopeGuard _close = Loki::MakeGuard(close, fd);
  (void)_close;

  struct stat statbuf {};
  int err = fstat(fd, &statbuf);
  if (err) {
    // Can't stat the file
    return 1;
  }

  void* mem =
      mmap(nullptr, statbuf.st_size, PROT_READ, MAP_SHARED, fd, /*offset=*/0);
  if (!mem) {
    // Can't map the file
    return 1;
  }
  Loki::ScopeGuard _unmap = Loki::MakeGuard(munmap, mem, statbuf.st_size);
  (void)_unmap;

  char* image = static_cast<char*>(mem);
  const char magic[] = {0x7f, 'E', 'L', 'F'};
  if (std::memcmp(image, magic, 4) != 0) {
    // Header doesn't have the right magic
    return 1;
  }

  Elf* elf = elf_memory(image, statbuf.st_size);
  ElfClass elf_class = static_cast<ElfClass>(image[EI_CLASS]);

  switch (elf_class) {
    case ELF32:
      return has_magic_symbol<Traits32>(elf);
    case ELF64:
      return has_magic_symbol<Traits64>(elf);
    default:
      // unexpected
      return 1;
  }
}

int main(int argc, char** argv) {
  argue::Parser::Metadata meta{};
  meta.add_help = true;
  meta.name = "argue-can-complete";
  meta.prolog = R"prolog(
Any C++ program that uses the argue argument parsing library has built-in
shell-completion support. This support is provided automatically by the argue
library. In order for a program to indicate to external observers that it has
this support, it must include a particular symbol ARGUE_AUTOCOMPLETE_ME in
it's symbol table.

This program will exit with status code zero if the provided file is a valid
ELF program file which includes the sentinel symbol in it's symbol table.
)prolog";

  argue::Parser parser{meta};
  ProgramOptions progopts{};
  parser.add_argument("filepath", &progopts.filepath);

  int parse_result = parser.parse_args(argc, argv);
  switch (parse_result) {
    case argue::PARSE_ABORTED:
      return 0;
    case argue::PARSE_EXCEPTION:
      return 1;
    case argue::PARSE_FINISHED:
      break;
  }

  exit(file_is_magic_elf(progopts.filepath));
}
