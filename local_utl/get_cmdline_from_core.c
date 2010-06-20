#include <linux/elf.h>
#include <stdio.h>
//#include <unistd.h>
#define ELF_PRPSINFO_ARG_OFF 44
#define ELF_PRPSINFO_ARG_LEN 80

char *
get_cmdline_from_core(char *core)
{
	FILE *fp;
	static char buf[256];
	struct elfhdr elf;
	struct elf_note n;
	fp = fopen(core, "r");
	if (NULL == fp)
		return NULL;
	if (fread(&elf, 1, sizeof (elf), fp) != sizeof (elf))
		goto err;
	if (fseek
	    (fp,
	     sizeof (struct elfhdr) + elf.e_phnum * sizeof (struct elf_phdr),
	     SEEK_SET))
		goto err;
	if (fread(&n, 1, sizeof (n), fp) != sizeof (n))
		goto err;
	if (fseek(fp, (n.n_namesz + 3) >> 2 << 2, SEEK_CUR))
		goto err;
	if (fseek(fp, (n.n_descsz + 3) >> 2 << 2, SEEK_CUR))
		goto err;
	if (fread(&n, 1, sizeof (n), fp) != sizeof (n))
		goto err;
	if (fseek(fp, (n.n_namesz + 3) >> 2 << 2, SEEK_CUR))
		goto err;
	if (fseek(fp, ELF_PRPSINFO_ARG_OFF, SEEK_CUR))
		goto err;
	if (fread(buf, 1, ELF_PRPSINFO_ARG_LEN, fp) != ELF_PRPSINFO_ARG_LEN)
		goto err;
	fclose(fp);
	buf[80] = 0;
	return buf;
      err:
	fclose(fp);
	return NULL;
}

int
main(int argc, char *argv[])
{
	char *cmd;
	if (argc != 2) {
		printf("no core file specified!\n");
		return 0;
	}
	cmd = get_cmdline_from_core(argv[1]);
	if (cmd)
		printf("%s\n", cmd);
	else
		printf("error\n");
	return 0;
}
