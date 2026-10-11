#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAX_BUF 4096
#define MAX_LINE 512

char buf[MAX_BUF];
int buf_len = 0;
char filename[128];

void
display_editor(void)
{
  printf("\n==================================================\n");
  printf("  xv6 Text Editor (`edit`) - File: %s\n", filename);
  printf("==================================================\n");
  printf("Commands:\n");
  printf("  :s  -> Save file\n");
  printf("  :q  -> Exit editor\n");
  printf("  :p  -> Print buffer contents\n");
  printf("  :c  -> Clear buffer\n");
  printf("  Or type any line of text to append to file.\n");
  printf("==================================================\n");
  printf("Current Content:\n");
  printf("--------------------------------------------------\n");
  if (buf_len == 0) {
    printf("(empty file)\n");
  } else {
    printf("%s", buf);
    if (buf[buf_len - 1] != '\n') {
      printf("\n");
    }
  }
  printf("--------------------------------------------------\n\n");
}

int
save_file(void)
{
  int fd = open(filename, O_CREATE | O_WRONLY | O_TRUNC);
  if (fd < 0) {
    printf("[Error] Failed to open '%s' for writing.\n", filename);
    return -1;
  }

  int written = write(fd, buf, buf_len);
  close(fd);

  if (written < 0) {
    printf("[Error] Failed to write to file '%s'.\n", filename);
    return -1;
  }

  printf("[Status] File '%s' saved successfully (%d bytes).\n", filename, written);
  return 0;
}

int
load_file(void)
{
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    printf("[Status] File '%s' does not exist yet. Creating a new file on save.\n", filename);
    buf_len = 0;
    buf[0] = '\0';
    return 0;
  }

  int n = read(fd, buf, MAX_BUF - 1);
  close(fd);

  if (n < 0) {
    printf("[Error] Failed to read file '%s'.\n", filename);
    buf_len = 0;
    buf[0] = '\0';
    return -1;
  }

  buf_len = n;
  buf[buf_len] = '\0';
  printf("[Status] Loaded %d bytes from '%s'.\n", buf_len, filename);
  return 0;
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("Usage: edit <filename>\n");
    exit(1);
  }

  strcpy(filename, argv[1]);

  load_file();
  display_editor();

  char line[MAX_LINE];

  while (1) {
    printf("edit> ");
    memset(line, 0, sizeof(line));
    if (gets(line, sizeof(line)) == 0) {
      // EOF reached
      break;
    }

    // Strip trailing newline if any for command matching
    int line_len = strlen(line);
    if (line_len > 0 && line[line_len - 1] == '\n') {
      line[line_len - 1] = '\0';
      line_len--;
    }

    // Command checking
    if (strcmp(line, ":s") == 0 || line[0] == 19) { // 19 is Ctrl+S
      save_file();
    } else if (strcmp(line, ":q") == 0 || line[0] == 17) { // 17 is Ctrl+Q
      printf("Exiting editor.\n");
      break;
    } else if (strcmp(line, ":p") == 0) {
      display_editor();
    } else if (strcmp(line, ":c") == 0) {
      buf_len = 0;
      buf[0] = '\0';
      printf("[Status] Buffer cleared.\n");
    } else {
      // Append input line + newline to buffer
      if (buf_len + line_len + 1 >= MAX_BUF) {
        printf("[Error] Buffer full! Cannot add more text.\n");
      } else {
        strcpy(buf + buf_len, line);
        buf_len += line_len;
        buf[buf_len++] = '\n';
        buf[buf_len] = '\0';
        printf("[Status] Added line (%d total bytes in buffer).\n", buf_len);
      }
    }
  }

  exit(0);
}
