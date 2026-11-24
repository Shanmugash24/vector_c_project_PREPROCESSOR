#include "preprocessor.h"
#include <stdio.h>
#include <string.h>

static int is_sp(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

static int is_dir(const char *s, const char *w) {
    while (is_sp(*s)) s++;
    if (*s++ != '#') return 0;
    while (is_sp(*s)) s++;
    int n = strlen(w);
    return strncmp(s, w, n) == 0 && (is_sp(s[n]) || s[n] == '"' || s[n] == '<' || s[n] == '\0');
}

static int file_ok(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static int find_file(Preprocessor *pp, const char *cur, const char *name, char *res) {
    const char *slash = strrchr(cur, '/');
    if (slash) {
        snprintf(res, 1024, "%.*s%s", (int)(slash - cur + 1), cur, name);
        if (file_ok(res)) return 1;
    }
    if (file_ok(name)) { strcpy(res, name); return 1; }

    for (int i = 0; i < pp->include_dir_count; i++) {
        snprintf(res, 1024, "%s/%s", pp->include_dirs[i], name);
        if (file_ok(res)) return 1;
    }
    return 0;
}

static int do_include(Preprocessor *pp, const char *line, const char *cur, FILE *out, int depth) {
    const char *s = strchr(line, '"');
    char end = '"';
    if (!s) { s = strchr(line, '<'); end = '>'; }
    if (!s) return 0;

    const char *e = strchr(s + 1, end);
    if (!e) return 0;

    char name[512], res[1024];
    snprintf(name, sizeof(name), "%.*s", (int)(e - s - 1), s + 1);

    if (find_file(pp, cur, name, res))
        process_file(pp, res, out, depth + 1);
    else if (end == '>')
        fprintf(out, "#include <%s>\n", name);
    else {
        fprintf(stderr, "Error: cannot find \"%s\"\n", name);
        return -1;
    }
    return 1;
}

int process_file(Preprocessor *pp, const char *path, FILE *out, int depth) {
    if (depth > MAX_INCLUDE_DEPTH) return 0;

    FILE *in = fopen(path, "r");
    if (!in) { perror(path); return 0; }

    char line[8192], clean[8192], exp[32768];
    int in_blk = 0;

    while (fgets(line, sizeof(line), in)) {
        remove_comments(line, clean, &in_blk);

        if (is_dir(clean, "include")) {
            if (do_include(pp, clean, path, out, depth) < 0) { fclose(in); return 0; }
        } else if (is_dir(clean, "define")) {
            handle_define(pp, clean);
        } else if (is_dir(clean, "undef")) {
            handle_undef(pp, clean);
        } else {
            expand_macros(pp, clean, exp);
            fputs(exp, out);
        }
    }
    fclose(in);
    return 1;
}