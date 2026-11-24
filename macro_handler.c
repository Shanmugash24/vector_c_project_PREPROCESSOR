#include "preprocessor.h"
#include <stdio.h>
#include <string.h>

static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
static int is_id(char c) {
    return is_alpha(c) || (c >= '0' && c <= '9');
}
static int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static int find_macro(Preprocessor *pp, const char *name) {
    for (int i = 0; i < pp->count; i++)
        if (strcmp(pp->items[i].name, name) == 0)
            return i;
    return -1;
}

int handle_define(Preprocessor *pp, const char *line) {
    char name[MAX_NAME];
    const char *p = line;

    while (is_space(*p)) p++;
    if (*p != '#') return 0;
    p++;
    while (is_space(*p)) p++;
    if (strncmp(p, "define", 6) != 0 || !is_space(p[6])) return 0;
    p += 6;

    while (is_space(*p)) p++;
    int n = 0;
    while (is_id(*p) && n < MAX_NAME - 1) name[n++] = *p++;
    name[n] = '\0';
    if (!n) return 1;

    int idx = find_macro(pp, name);
    if (idx == -1) {
        if (pp->count >= MAX_MACROS) return 1;
        idx = pp->count++;
    }

    Macro *m = &pp->items[idx];
    memset(m, 0, sizeof(*m));
    strcpy(m->name, name);

    if (*p == '(') {
        m->function_like = 1;
        p++;
        while (*p && *p != ')' && m->param_count < 32) {
            while (is_space(*p) || *p == ',') p++;
            if (*p == ')') break;
            int k = 0;
            while (is_id(*p) && k < MAX_NAME - 1)
                m->params[m->param_count][k++] = *p++;
            m->params[m->param_count++][k] = '\0';
        }
        if (*p == ')') p++;
    }

    while (is_space(*p)) p++;
    snprintf(m->value, MAX_VALUE, "%s", p);
    m->value[strcspn(m->value, "\r\n")] = '\0';
    return 1;
}

void handle_undef(Preprocessor *pp, const char *line) {
    char name[MAX_NAME];
    if (sscanf(line, " # undef %s", name) != 1) return;

    int idx = find_macro(pp, name);
    if (idx != -1) {
        for (int i = idx; i < pp->count - 1; i++)
            pp->items[i] = pp->items[i + 1];
        pp->count--;
    }
}

static void replace_params(Macro *m, char args[32][MAX_VALUE], int ac, char *out) {
    int i = 0, len = 0;
    while (m->value[i]) {
        if (is_alpha(m->value[i])) {
            char word[MAX_NAME];
            int w = 0, found = -1;
            while (is_id(m->value[i]) && w < MAX_NAME - 1) word[w++] = m->value[i++];
            word[w] = '\0';

            for (int j = 0; j < m->param_count; j++)
                if (strcmp(word, m->params[j]) == 0) found = j;

            const char *rep = (found >= 0 && found < ac) ? args[found] : word;
            len += snprintf(out + len, MAX_VALUE * 4 - len, "%s", rep);
        } else {
            out[len++] = m->value[i++];
        }
    }
    out[len] = '\0';
}

static void expand_level(Preprocessor *pp, const char *in, char *out, int depth) {
    if (depth > 10) { snprintf(out, MAX_VALUE * 4, "%s", in); return; }
    int i = 0, len = 0;

    while (in[i]) {
        if (in[i] == '"') {
            out[len++] = in[i++];
            while (in[i] && in[i] != '"') out[len++] = in[i++];
            if (in[i]) out[len++] = in[i++];
        } else if (is_alpha(in[i])) {
            char name[MAX_NAME];
            int w = 0;
            while (is_id(in[i]) && w < MAX_NAME - 1) name[w++] = in[i++];
            name[w] = '\0';

            int idx = find_macro(pp, name);
            if (idx == -1) {
                len += snprintf(out + len, MAX_VALUE * 4 - len, "%s", name);
                continue;
            }

            Macro *m = &pp->items[idx];
            char temp[MAX_VALUE * 4];

            if (!m->function_like) {
                expand_level(pp, m->value, temp, depth + 1);
                len += snprintf(out + len, MAX_VALUE * 4 - len, "%s", temp);
            } else {
                int k = i, ac = 0, a = 0, par = 0;
                char args[32][MAX_VALUE] = {{0}};
                while (is_space(in[k])) k++;
                if (in[k] != '(') {
                    len += snprintf(out + len, MAX_VALUE * 4 - len, "%s", name);
                    continue;
                }
                k++;

                while (in[k] && ac < 32) {
                    char c = in[k++];
                    if (c == '(') { par++; args[ac][a++] = c; }
                    else if (c == ')' && par > 0) { par--; args[ac][a++] = c; }
                    else if (c == ',' && par == 0) { args[ac++][a] = '\0'; a = 0; }
                    else if (c == ')' && par == 0) { args[ac++][a] = '\0'; break; }
                    else if (a < MAX_VALUE - 1) args[ac][a++] = c;
                }

                char replaced[MAX_VALUE * 4];
                replace_params(m, args, ac, replaced);
                expand_level(pp, replaced, temp, depth + 1);
                len += snprintf(out + len, MAX_VALUE * 4 - len, "%s", temp);
                i = k;
            }
        } else {
            out[len++] = in[i++];
        }
    }
    out[len] = '\0';
}

void expand_macros(Preprocessor *pp, const char *input, char *output) {
    expand_level(pp, input, output, 0);
}