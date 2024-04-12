#include <elf.h>
#include <utils.h>

#define Elf_Sym MUXDEF(CONFIG_RV64, Elf64_Sym, Elf32_Sym)
#define Elf_Shdr MUXDEF(CONFIG_RV64, Elf64_Shdr, Elf32_Shdr)
#define Elf_Ehdr MUXDEF(CONFIG_RV64, Elf64_Ehdr, Elf32_Ehdr)
#define ELF_ST_TYPE MUXDEF(CONFIG_RV64, ELF64_ST_TYPE, ELF32_ST_TYPE)

struct AllFunc allFucn;

static int readStrTab(const Elf_Sym *Elf_Sym, size_t sym_num, Elf_Shdr *elf_sec, FILE *fp) {
  rewind(fp);
  fseek(fp, elf_sec->sh_offset, SEEK_SET);
  for (int i = 0; i < sym_num; i++) {
    if (ELF_ST_TYPE(Elf_Sym[i].st_info) != STT_FUNC) {
      continue;
    }
    allFucn.funcInfo[allFucn.num].addr = Elf_Sym[i].st_value;
    allFucn.funcInfo[allFucn.num].size = Elf_Sym[i].st_size;
    fseek(fp, elf_sec->sh_offset + Elf_Sym[i].st_name, SEEK_SET);
    if (fread(allFucn.funcInfo[allFucn.num].name, 1, CONFIG_FTRACE_NSIZE - 1, fp) != CONFIG_FTRACE_NSIZE - 1) {
      assert(0);
    }
    ++(allFucn.num);
  }
  rewind(fp);
  return 0;
}

static size_t readSymTab(Elf_Sym **Elf_Sym, Elf_Shdr *elf_sec, FILE *fp) {
  rewind(fp);
  fseek(fp, elf_sec->sh_offset, SEEK_SET);
  size_t entries = elf_sec->sh_size / elf_sec->sh_entsize;
  *Elf_Sym = malloc (elf_sec->sh_size);
  if (*Elf_Sym == NULL) {
    return 0;
  }
  size_t result = fread(*Elf_Sym, elf_sec->sh_entsize, entries, fp);
  rewind(fp);
  if (result != entries) {
    free(*Elf_Sym);
    return 0;
  }
  return entries;
}

static void readSection(Elf_Ehdr *elf_head, FILE *fp) {
  Elf_Shdr elf_strtab;
  fseek(fp, elf_head->e_shoff + elf_head->e_shentsize * elf_head->e_shstrndx, SEEK_SET);
  if (fread(&elf_strtab, sizeof(Elf_Shdr), 1, fp) != 1) {
    assert(0);
  }
  fseek(fp, elf_head->e_shoff, SEEK_SET);
  Elf_Shdr elf_sec;
  Elf_Sym* Elf_Sym = NULL;
  size_t sym_num = 0;
  for (int i = 0; i < elf_head->e_shnum; i++) {
    if (fread(&elf_sec, 1, sizeof(Elf_Shdr), fp) != sizeof(Elf_Shdr)) {
      assert(0);
    }
    if (elf_sec.sh_type == SHT_SYMTAB) {
      if(!(sym_num = readSymTab(&Elf_Sym, &elf_sec, fp))) {
        assert(0);
      }
      break;
    }
  }
  rewind(fp);
  fseek(fp, elf_head->e_shoff, SEEK_SET);
  for (int i = 0; i < elf_head->e_shnum; i++) {
    fseek(fp, elf_head->e_shoff + i * sizeof(Elf_Shdr), SEEK_SET);
    if (fread(&elf_sec, 1, sizeof(Elf_Shdr), fp) != sizeof(Elf_Shdr)) {
      assert(0);
    }
    if (elf_sec.sh_type == SHT_STRTAB) {
      rewind(fp);
      fseek(fp, elf_strtab.sh_offset + elf_sec.sh_name, SEEK_SET);
      uint8_t sec_name[7];
      if (fread(sec_name, 1, 7, fp) != 7) {
        assert(0);
      }
      if(memcmp(sec_name, ".strtab", 7)) {
        continue;
      }
      if (readStrTab(Elf_Sym, sym_num, &elf_sec, fp)) {
        assert(0);
      }
      break;
    }
  }
  rewind(fp);
  if (Elf_Sym != NULL) {
    free(Elf_Sym);
  }
}

static Elf_Ehdr read_elf_head(FILE *fp) {
  char headbuf[EI_NIDENT] = {0};
  rewind (fp);
  if (fread(headbuf, 1, EI_NIDENT, fp) != EI_NIDENT) {
    assert(0);
  }
  if(headbuf[0] != 0x7f || 
    headbuf[1] != 0x45 || 
    headbuf[2] != 0x4c || 
    headbuf[3] != 0x46
  ) {
    assert(0);
  }
  rewind(fp);
  Elf_Ehdr elf_head;
  if (fread(&elf_head, 1, sizeof(Elf_Ehdr), fp) != sizeof(Elf_Ehdr)) {
    assert(0);
  }
  rewind(fp);
  return elf_head;
}

static void parse_elf(const char* elf_file_name) {
  FILE* fp = fopen(elf_file_name, "rb");
  fseek(fp, 0, SEEK_END);
  word_t file_size = ftell(fp);
  if (file_size < sizeof(Elf_Ehdr)) {
    assert(0);
  }
  Elf_Ehdr elf_head = read_elf_head(fp);
  readSection(&elf_head, fp);
  fclose(fp);
}

void init_func_name(const char *elf_file) {
  if (elf_file == NULL || strlen(elf_file) == 0) return;
  Log("ELF file: %s", elf_file);
  parse_elf(elf_file);
  Log("|Num|      Value|    Size|                Name|");
  for (int i = 0; i < allFucn.num; i++) {
    Log("|%3d| %#010x|%8ld|%20s|", i, allFucn.funcInfo[i].addr,allFucn.funcInfo[i].size,allFucn.funcInfo[i].name);
  }
}