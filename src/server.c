#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <gtk/gtk.h>

#define PORT 3000

extern int errno;

// ------------------------------------ JSON ------------------------------------ //

struct json_object* read_json_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("[server] Eroare la deschiderea fisierului JSON");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(length + 1);
    if (!content) {
        perror("[server] Eroare la alocarea memoriei");
        fclose(file);
        return NULL;
    }

    fread(content, 1, length, file);
    content[length] = '\0';
    fclose(file);

    struct json_object *parsed_json = json_tokener_parse(content);
    free(content);

    if (!parsed_json) {
        fprintf(stderr, "[server] Eroare la parsarea JSON din %s\n", filename);
    }
    return parsed_json;
}
int write_json_file(const char *filename, struct json_object *json_obj) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("[server] Eroare la deschiderea fisierului JSON pentru scriere");
        return -1;
    }

    fprintf(file, "%s", json_object_to_json_string_ext(json_obj, JSON_C_TO_STRING_PRETTY));
    fclose(file);
    return 0;
}

void handle_server_info(const char *informatie, char *response, size_t response_size) {
    struct json_object *json = read_json_file("informatii.json");
    if (!json) {
        snprintf(response, response_size, "Eroare: Nu s-a putut deschide fisierul informatii.json\n");
        return;
    }

    struct json_object *requested_info;
    if (json_object_object_get_ex(json, informatie, &requested_info)) {
        struct json_object *description, *advantages, *examples, *help;

        if (json_object_object_get_ex(requested_info, "description", &description)) {
            snprintf(response + strlen(response), response_size - strlen(response), "Descriere:\n%s\n\n", json_object_get_string(description));
        }

        if (json_object_object_get_ex(requested_info, "advantages", &advantages)) {
            snprintf(response + strlen(response), response_size - strlen(response), "Avantaje:\n");
            for (size_t i = 0; i < json_object_array_length(advantages); i++) {
                snprintf(response + strlen(response), response_size - strlen(response), "- %s\n", json_object_get_string(json_object_array_get_idx(advantages, i)));
            }
            snprintf(response + strlen(response), response_size - strlen(response), "\n");
        }

        if (json_object_object_get_ex(requested_info, "examples", &examples)) {
            snprintf(response + strlen(response), response_size - strlen(response), "Exemple:\n");
            for (size_t i = 0; i < json_object_array_length(examples); i++) {
                snprintf(response + strlen(response), response_size - strlen(response), "- %s\n", json_object_get_string(json_object_array_get_idx(examples, i)));
            }
            snprintf(response + strlen(response), response_size - strlen(response), "\n");
        }

        if (json_object_object_get_ex(requested_info, "help", &help)) {
            snprintf(response + strlen(response), response_size - strlen(response), "Help: ");
            for (size_t i = 0; i < json_object_array_length(help); i++) {
                snprintf(response + strlen(response), response_size - strlen(response), "%s\n", json_object_get_string(json_object_array_get_idx(help, i)));
            }
            snprintf(response + strlen(response), response_size - strlen(response), "\n");
        }
    } else {
        snprintf(response, response_size, "Nu exista informatii despre: %s\n", informatie);
    }

    json_object_put(json);
}
void handle_server_friends(const char *username, char *response, size_t response_size) {
    struct json_object *json = read_json_file("users.json");
    if (!json) {
        snprintf(response, response_size, "Eroare: Nu s-a putut deschide fisierul users.json\n");
        return;
    }

    struct json_object *friends_section;
    if (json_object_object_get_ex(json, "friends", &friends_section)) {
        struct json_object *friends_list;
        if (json_object_object_get_ex(friends_section, username, &friends_list)) {
            for (size_t i = 0; i < json_object_array_length(friends_list); i++) {
                snprintf(response + strlen(response), response_size - strlen(response), "%s\n",
                         json_object_get_string(json_object_array_get_idx(friends_list, i)));
            }
        } else {
            snprintf(response, response_size, "Nu exista prieteni pentru utilizatorul: %s\n", username);
        }
    } else {
        snprintf(response, response_size, "Eroare: Structura JSON nu contine sectiunea 'friends'\n");
    }

    json_object_put(json);
}
void handle_get_bookmarks(const char *username, char *response, size_t response_size) {
    struct json_object *json = read_json_file("users.json");
    if (!json) {
        snprintf(response, response_size, "Eroare: Nu s-a putut deschide fisierul users.json\n");
        return;
    }

    struct json_object *bookmarks_section, *user_bookmarks;
    if (json_object_object_get_ex(json, "bookmarks", &bookmarks_section) &&
        json_object_object_get_ex(bookmarks_section, username, &user_bookmarks)) {
        for (size_t i = 0; i < json_object_array_length(user_bookmarks); i++) {
            snprintf(response + strlen(response), response_size - strlen(response), "%s\n",
                     json_object_get_string(json_object_array_get_idx(user_bookmarks, i)));
        }
    } else {
        snprintf(response, response_size, "Nu există bookmark-uri pentru utilizatorul %s.\n", username);
    }

    json_object_put(json);
}

void create_discussion(const char *topic, char *response, size_t response_size) {
    struct json_object *json = read_json_file("discussions.json");
    if (!json) {
        snprintf(response, response_size, "Eroare: Nu s-a putut deschide fisierul discussions.json\n");
        return;
    }

    struct json_object *discussions;
    if (!json_object_object_get_ex(json, "discussions", &discussions)) {
        discussions = json_object_new_object();
        json_object_object_add(json, "discussions", discussions);
    }

    if (json_object_object_get_ex(discussions, topic, NULL)) {
        snprintf(response, response_size, "Discuția există deja.\n");
    } else {
        json_object_object_add(discussions, topic, json_object_new_array());
        if (write_json_file("discussions.json", json) == 0) {
            snprintf(response, response_size, "Discuția %s a fost creată.\n", topic);
        } else {
            snprintf(response, response_size, "Eroare: Nu s-a putut actualiza fisierul discussions.json\n");
        }
    }

    json_object_put(json);
}
void add_message_to_discussion(const char *topic, const char *message, const char *username, char *response, size_t response_size) {
    struct json_object *json = read_json_file("discussions.json");
    if (!json) {
        snprintf(response, response_size, "Eroare: Nu s-a putut deschide fisierul discussions.json\n");
        return;
    }

    struct json_object *discussions, *discussion;
    if (json_object_object_get_ex(json, "discussions", &discussions) &&
        json_object_object_get_ex(discussions, topic, &discussion)) {

        char prefixed_message[2048];
        snprintf(prefixed_message, sizeof(prefixed_message), "[%s]: %s", username, message);

        json_object_array_add(discussion, json_object_new_string(prefixed_message));

        if (write_json_file("discussions.json", json) == 0) {
            snprintf(response, response_size, "Mesaj adăugat la discuția %s.\n", topic);
        } else {
            snprintf(response, response_size, "Eroare: Nu s-a putut actualiza fisierul discussions.json\n");
        }
    } else {
        snprintf(response, response_size, "Discuția %s nu există.\n", topic);
    }

    json_object_put(json);
}
void list_discussions(char *response, size_t response_size) {
    struct json_object *json = read_json_file("discussions.json");
    if (!json) {
        snprintf(response, response_size, "Eroare: Nu s-a putut deschide fisierul discussions.json\n");
        return;
    }

    struct json_object *discussions;
    if (json_object_object_get_ex(json, "discussions", &discussions)) {
        json_object_object_foreach(discussions, key, val) {
            snprintf(response + strlen(response), response_size - strlen(response), "%s\n", key);
        }
    } else {
        snprintf(response, response_size, "Nu există discuții disponibile.\n");
    }

    json_object_put(json);
}
void add_bookmark(const char *username, const char *topic, char *response, size_t response_size) {
    struct json_object *json = read_json_file("users.json");
    if (!json) {
        snprintf(response, response_size, "Eroare: Nu s-a putut deschide fisierul users.json\n");
        return;
    }

    struct json_object *bookmarks_section, *user_bookmarks;
    if (!json_object_object_get_ex(json, "bookmarks", &bookmarks_section) ||
        !json_object_object_get_ex(bookmarks_section, username, &user_bookmarks)) {
        snprintf(response, response_size, "Eroare: Utilizatorul nu are secțiunea de bookmark-uri.\n");
        json_object_put(json);
        return;
    }

    for (size_t i = 0; i < json_object_array_length(user_bookmarks); i++) {
        if (strcmp(json_object_get_string(json_object_array_get_idx(user_bookmarks, i)), topic) == 0) {
            snprintf(response, response_size, "Topic-ul %s este deja în bookmark-uri.\n", topic);
            json_object_put(json);
            return;
        }
    }

    json_object_array_add(user_bookmarks, json_object_new_string(topic));

    if (write_json_file("users.json", json) == 0) {
        snprintf(response, response_size, "Topic-ul %s a fost adăugat în bookmark-uri.\n", topic);
    } else {
        snprintf(response, response_size, "Eroare: Nu s-a putut actualiza fisierul users.json\n");
    }

    json_object_put(json);
}

void add_friend_request(const char *sender, const char *receiver, char *response, size_t response_size) {
    struct json_object *json = read_json_file("users.json");
    if (!json) {
        snprintf(response, response_size, "Eroare: Nu s-a putut deschide fisierul users.json\n");
        return;
    }

    if (strlen(sender) == 0 || strlen(receiver) == 0) {
        snprintf(response, response_size, "Eroare: Expeditor sau destinatar invalid.\n");
        return;
    }

    struct json_object *pending_requests;
    if (!json_object_object_get_ex(json, "pending_requests", &pending_requests)) {
        pending_requests = json_object_new_object();
        json_object_object_add(json, "pending_requests", pending_requests);
    }

    struct json_object *receiver_requests;
    if (!json_object_object_get_ex(pending_requests, receiver, &receiver_requests)) {
        receiver_requests = json_object_new_array();
        json_object_object_add(pending_requests, receiver, receiver_requests);
    }

    json_object_array_add(receiver_requests, json_object_new_string(sender));

    if (write_json_file("users.json", json) == 0) {
        snprintf(response, response_size, "Cererea de prietenie a fost trimisa catre %s.\n", receiver);
    } else {
        snprintf(response, response_size, "Eroare: Nu s-a putut actualiza fisierul users.json\n");
    }

    json_object_put(json);
}
void accept_friend_request(const char *receiver, const char *sender, char *response, size_t response_size) {
    struct json_object *json = read_json_file("users.json");
    if (!json) {
        snprintf(response, response_size, "Eroare: Nu s-a putut deschide fisierul users.json\n");
        return;
    }

    struct json_object *pending_requests, *friends;
    if (!json_object_object_get_ex(json, "pending_requests", &pending_requests) ||
        !json_object_object_get_ex(json, "friends", &friends)) {
        snprintf(response, response_size, "Eroare: Structura JSON este invalida.\n");
        json_object_put(json);
        return;
    }

    if (strlen(receiver) == 0 || strlen(sender) == 0) {
        snprintf(response, response_size, "Eroare: Nume utilizator invalid.\n");
        return;
    }

    struct json_object *receiver_requests;
    if (json_object_object_get_ex(pending_requests, receiver, &receiver_requests)) {
        size_t index = SIZE_MAX;
        for (size_t i = 0; i < json_object_array_length(receiver_requests); i++) {
            if (strcmp(json_object_get_string(json_object_array_get_idx(receiver_requests, i)), sender) == 0) {
                index = i;
                break;
            }
        }

        if (index != SIZE_MAX) {
            json_object_array_del_idx(receiver_requests, index, 1);
        }
    }

    struct json_object *sender_friends, *receiver_friends;
    if (!json_object_object_get_ex(friends, sender, &sender_friends)) {
        sender_friends = json_object_new_array();
        json_object_object_add(friends, sender, sender_friends);
    }
    if (!json_object_object_get_ex(friends, receiver, &receiver_friends)) {
        receiver_friends = json_object_new_array();
        json_object_object_add(friends, receiver, receiver_friends);
    }

    json_object_array_add(sender_friends, json_object_new_string(receiver));
    json_object_array_add(receiver_friends, json_object_new_string(sender));

    if (write_json_file("users.json", json) == 0) {
        snprintf(response, response_size, "%s si %s sunt acum prieteni.\n", sender, receiver);
    } else {
        snprintf(response, response_size, "Eroare: Nu s-a putut actualiza fisierul users.json\n");
    }

    json_object_put(json);
}
void remove_friend(const char *username, const char *friend_name, char *response, size_t response_size) {
    struct json_object *json = read_json_file("users.json");
    if (!json) {
        snprintf(response, response_size, "Eroare: Nu s-a putut deschide fisierul users.json\n");
        return;
    }

    char clean_username[128], clean_friend_name[128];
    strncpy(clean_username, username, sizeof(clean_username) - 1);
    clean_username[sizeof(clean_username) - 1] = '\0';
    g_strstrip(clean_username);

    strncpy(clean_friend_name, friend_name, sizeof(clean_friend_name) - 1);
    clean_friend_name[sizeof(clean_friend_name) - 1] = '\0';
    g_strstrip(clean_friend_name);

    struct json_object *friends;
    if (json_object_object_get_ex(json, "friends", &friends)) {
        struct json_object *user_friends, *friend_friends;

        if (json_object_object_get_ex(friends, clean_username, &user_friends)) {
            for (size_t i = 0; i < json_object_array_length(user_friends); i++) {
                if (strcmp(json_object_get_string(json_object_array_get_idx(user_friends, i)), clean_friend_name) == 0) {
                    json_object_array_del_idx(user_friends, i, 1);
                    break;
                }
            }
        }

        if (json_object_object_get_ex(friends, clean_friend_name, &friend_friends)) {
            for (size_t i = 0; i < json_object_array_length(friend_friends); i++) {
                if (strcmp(json_object_get_string(json_object_array_get_idx(friend_friends, i)), clean_username) == 0) {
                    json_object_array_del_idx(friend_friends, i, 1);
                    break;
                }
            }
        }

        if (write_json_file("users.json", json) == 0) {
            snprintf(response, response_size, "%s a fost eliminat din lista de prieteni.\n", clean_friend_name);
        } else {
            snprintf(response, response_size, "Eroare: Nu s-a putut actualiza fisierul users.json\n");
        }
    } else {
        snprintf(response, response_size, "Eroare: Structura JSON este invalida.\n");
    }

    json_object_put(json);
}

// ---------------------------------- BACK-END ---------------------------------- //

char *conv_addr(struct sockaddr_in address) {
    static char str[25];
    char port[7];

    strcpy(str, inet_ntoa(address.sin_addr));

    bzero(port, 7);
    sprintf(port, ":%d", ntohs(address.sin_port));
    strcat(str, port);
    return (str);
}

int comunicare(int fd) {
    char msg[2048];
    char msgrasp[5000] = " ";
    int bytes;

    bzero(msg, 100);

    bytes = read(fd, msg, sizeof(msg) - 1);

    if (bytes <= 0) {
        if (bytes == 0) {
            printf("[server] Clientul cu descriptorul %d s-a deconectat.\n", fd);
        } else {
            perror("[server] Eroare la read() de la client.\n");
        }
        return 1;
    }

    msg[bytes] = '\0';
    printf("[server] Mesajul primit este: %s\n", msg);

    if (strcmp(msg, "exit") == 0) {
        printf("[server] Clientul cu descriptorul %d a solicitat deconectarea.\n", fd);
        return 1;
    }

    if (strncmp(msg, "serverINFO:", 11) == 0) {
        const char *informatie = msg + 11;
        handle_server_info(informatie, msgrasp, sizeof(msgrasp));
    } else if (strncmp(msg, "serverADD:", 10) == 0) {
        const char *data = msg + 10;
        char sender[128], receiver[128];
        if (sscanf(data, "%127[^:]:%127s", sender, receiver) == 2) {
            add_friend_request(sender, receiver, msgrasp, sizeof(msgrasp));
        } else {
            snprintf(msgrasp, sizeof(msgrasp), "Eroare: Mesaj invalid pentru serverADD.\n");
        }
    } else if (strncmp(msg, "serverACCEPT:", 13) == 0) {
        const char *data = msg + 13;
        char receiver[128], sender[128];
        if (sscanf(data, "%127[^:]:%127s", receiver, sender) == 2) {
            printf("[server] Receiver: '%s', Sender: '%s'\n", receiver, sender);
            accept_friend_request(receiver, sender, msgrasp, sizeof(msgrasp));
        } else {
            snprintf(msgrasp, sizeof(msgrasp), "Eroare: Mesaj invalid pentru serverACCEPT.\n");
        }
    } else if (strncmp(msg, "serverTOPIC:", 12) == 0) {
        const char *add = msg + 12;
        snprintf(msgrasp, sizeof(msgrasp), "Topicul %s a fost adaugat.\n", add);
    } else if (strncmp(msg, "serverFRIENDS:", 14) == 0) {
        const char *username = msg + 14;
        handle_server_friends(username, msgrasp, sizeof(msgrasp));
    } else if (strncmp(msg, "serverFRIEND_REQUESTS:", 22) == 0) {
        const char *username = msg + 22;
        struct json_object *json = read_json_file("users.json");
        if (!json) {
            snprintf(msgrasp, sizeof(msgrasp), "Eroare: Nu s-a putut deschide fisierul users.json\n");
            return 1;
        }
        struct json_object *pending_requests, *user_requests;
        if (json_object_object_get_ex(json, "pending_requests", &pending_requests) &&
            json_object_object_get_ex(pending_requests, username, &user_requests)) {
            snprintf(msgrasp, sizeof(msgrasp), "Cereri de prietenie pentru %s:\n", username);
            for (size_t i = 0; i < json_object_array_length(user_requests); i++) {
            snprintf(msgrasp + strlen(msgrasp), sizeof(msgrasp) - strlen(msgrasp), "%s", json_object_get_string(json_object_array_get_idx(user_requests, i)));}
        } else {
            snprintf(msgrasp, sizeof(msgrasp), "Nu exista cereri de prietenie pentru %s.\n", username);
        }
        json_object_put(json);
    } else if (strncmp(msg, "serverCREATE_DISCUSSION:", 24) == 0) {
        const char *topic = msg + 24;
        create_discussion(topic, msgrasp, sizeof(msgrasp));
    } else if (strncmp(msg, "serverADD_MESSAGE:", 18) == 0) {
        const char *data = msg + 18;
        char topic[128], message[256], username[128];
        if (sscanf(data, "%127[^:]:%127[^:]:%255[^\n]", username, topic, message) == 3) {
            add_message_to_discussion(topic, message, username, msgrasp, sizeof(msgrasp));
        } else {
            snprintf(msgrasp, sizeof(msgrasp), "Eroare: Mesaj invalid pentru serverADD_MESSAGE.\n");
        }
    } else if (strncmp(msg, "serverLIST_DISCUSSIONS:", 23) == 0) {
        list_discussions(msgrasp, sizeof(msgrasp));
    } else if (strncmp(msg, "serverVIEW_TOPIC:", 17) == 0) {
        const char *topic = msg + 17;
        struct json_object *json = read_json_file("discussions.json");
        if (!json) {
            snprintf(msgrasp, sizeof(msgrasp), "Eroare: Nu s-a putut deschide fisierul discussions.json\n");
            return 1;
        }

        struct json_object *discussions, *discussion;
        if (json_object_object_get_ex(json, "discussions", &discussions) &&
            json_object_object_get_ex(discussions, topic, &discussion)) {
            snprintf(msgrasp, sizeof(msgrasp), "Mesaje din %s:\n", topic);
            for (size_t i = 0; i < json_object_array_length(discussion); i++) {
                snprintf(msgrasp + strlen(msgrasp), sizeof(msgrasp) - strlen(msgrasp), "- %s\n",
                         json_object_get_string(json_object_array_get_idx(discussion, i)));
            }
        } else {
            snprintf(msgrasp, sizeof(msgrasp), "Discuția %s nu există.\n", topic);
        }

        json_object_put(json);

    } else if (strncmp(msg, "serverBOOKMARK:", 15) == 0) {
        const char *data = msg + 15;
        char username[128], topic[128];
        if (sscanf(data, "%127[^:]:%127[^\n]", username, topic) == 2) {
            add_bookmark(username, topic, msgrasp, sizeof(msgrasp));
        } else {
            snprintf(msgrasp, sizeof(msgrasp), "Eroare: Mesaj invalid pentru serverBOOKMARK.\n");
        }
    } else if (strncmp(msg, "serverGET_BOOKMARKS:", 20) == 0) {
        const char *username = msg + 20;
        handle_get_bookmarks(username, msgrasp, sizeof(msgrasp));
    } else if (strncmp(msg, "serverREMOVE_FRIEND:", 20) == 0) {
        const char *data = msg + 20;
        char username[128], friend_name[128];
        if (sscanf(data, "%127[^:]:%127[^\n]", username, friend_name) == 2) {
            g_strstrip(username);
            g_strstrip(friend_name);

            remove_friend(username, friend_name, msgrasp, sizeof(msgrasp));
        } else {
            snprintf(msgrasp, sizeof(msgrasp), "Eroare: Mesaj invalid pentru REMOVE_FRIEND.\n");
        }
    } else {
        snprintf(msgrasp, sizeof(msgrasp), "Comanda necunoscuta.\n");
    }

    if (write(fd, msgrasp, strlen(msgrasp)) < 0) {
        perror("[server] Eroare la write() catre client.\n");
        return 1;
    }

    printf("[server] Trimitem confirmarea: %s\n", msgrasp);
    return 0;
}

int main() {
    struct sockaddr_in server;
    struct sockaddr_in from;
    fd_set readfds, actfds;
    struct timeval tv;
    int sd, client, optval = 1, fd, nfds, len;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[server] Eroare la socket().\n");
        return errno;
    }

    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        perror("[server] Eroare la bind().\n");
        return errno;
    }

    if (listen(sd, 5) == -1) {
        perror("[server] Eroare la listen().\n");
        return errno;
    }

    FD_ZERO(&actfds); //clean descriptori
    FD_SET(sd, &actfds); //adauga descriptorul sd in actfds

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    nfds = sd; //nr maxim descriptori = sd

    printf("[server] Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    while (1) {
        bcopy((char *)&actfds, (char *)&readfds, sizeof(readfds));

        if (select(nfds + 1, &readfds, NULL, NULL, &tv) < 0) {
            perror("[server] Eroare la select().\n");
            return errno;
        }

        if (FD_ISSET(sd, &readfds)) {
            len = sizeof(from);
            bzero(&from, sizeof(from));

            client = accept(sd, (struct sockaddr *)&from, &len);

            if (client < 0) {
                perror("[server] Eroare la accept().\n");
                continue;
            }

            if (nfds < client)
                nfds = client;

            FD_SET(client, &actfds);

            printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n", client, conv_addr(from));
            fflush(stdout);
        }

        for (fd = 0; fd <= nfds; fd++) {
            if (fd != sd && FD_ISSET(fd, &readfds)) {
                if (comunicare(fd)) {
                    printf("[server] S-a deconectat clientul cu descriptorul %d.\n", fd);
                    fflush(stdout);
                    close(fd);
                    FD_CLR(fd, &actfds);
                }
            }
        }
    }
}
