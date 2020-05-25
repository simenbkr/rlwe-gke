#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include "params.h"
#include "poly.h"
#include "keccak.h"
#include "randombytes.h"
#include "common.h"
#include "kem.h"

#include "monocypher.h"

pthread_mutex_t socket_lock;

typedef struct {
    uint8_t *secret_key;
    int *sockfd;
    player *player;
} thread_info_t;


int setup_connection(const char *hostname, size_t hostname_size, int port) {
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Could not create a socket.\n");
        exit(69);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(hostname);
    servaddr.sin_port = htons(port);

    int yes = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        printf("Could not connect to the remote server.\nExiting..\n");
        exit(69);
    }

    return sockfd;
}

void get_id(int sockfd, player *current) {

    char recv_buf[8];
    char *message = "myidn";
    if (write(sockfd, message, strlen(message)) < 0) {
        printf("Could not send request for ID.\n");
    }

    recv(sockfd, recv_buf, 6, 0);

    char *ptr;
    int id = strtol(recv_buf, &ptr, 10);
    current->id = id;
}

void send_poly_message(const int sockfd, const poly_message_t *msg, char *server_cmd) {

    size_t byte_count = MESSAGE_LENGTH + strlen(server_cmd);
    char *data = malloc(byte_count);
    bzero(data, byte_count);

    uint8_t raw_poly_msg[MESSAGE_LENGTH];
    poly_message_to_uint8(msg, raw_poly_msg);
    //printf("Sending data with %lu bytes.\n", byte_count);

    strcpy(data, server_cmd);
    memcpy(data + strlen(server_cmd), raw_poly_msg, MESSAGE_LENGTH);

    if(write(sockfd, data, byte_count) < 0) {
        printf("Could not send the polynomial to the remote server. Command that failed: %s.\nExiting..\n\n", server_cmd);
        exit(69);
    }

    //send(sockfd, data, byte_count, 0);

    free(data);
}

player **construct_player_list(int sockfd, player *current, int *count) {

    char *recv_buf;
    recv_buf = malloc(100 * sizeof(char));
    char *message = "allid";

    if (write(sockfd, message, strlen(message)) < 0) {
        printf("Could not construct a player list.\n");
    }

    recv(sockfd, recv_buf, 1, 0);
    recv(sockfd, recv_buf, 100, 0);

    int tmp;
    char *recv_ptr = recv_buf;
    for (tmp = 0; recv_ptr[tmp]; recv_ptr[tmp] == ',' ? tmp++ : *recv_ptr++);
    tmp++;

    char *end, *r, *tok;
    char *gp;
    gp = malloc(100);
    int id;
    r = end = strdup(recv_buf);

    player **playerlist;
    playerlist = malloc(sizeof(player) * tmp);

    int i = 0;

    while ((tok = strsep(&end, ",")) != NULL) {
        id = (int) strtol(tok, &gp, 10);

        if (id == current->id) {
            playerlist[i] = current;
        } else {
            player *tmp_player;
            tmp_player = malloc(sizeof(player));
            create_empty_player(tmp_player, id);
            playerlist[i] = tmp_player;
        }
        i++;
    }

    *count = tmp;

    free(recv_buf);

    return playerlist;
}

void wait_until_ready(int sockfd, char *message) {
    char recv_buf[10];

    for (int j = 0; j < 9; ++j) {
        recv_buf[j] = '\x00';
    }

    while (1) {
        for (int i = 0; i < 9; ++i) {
            recv_buf[i] = '\x00';
        }

        send(sockfd, message, strlen(message), 0);
        read(sockfd, recv_buf, 10);

        if (strcmp(recv_buf, "yes") == 0) {
            break;
        }

        //printf("Waiting for %s to complete...\n", message);
        sleep(1);
    }
}

void get_val(int sockfd, player* player, char *message, int round) {

    poly_message_t msg;
    poly recv_poly;
    unsigned char recv_buf[MESSAGE_LENGTH + 16];

    // This is just to create command:id. Basically "{}:{}".format(command, id) in python :).
    char id_string[10];
    sprintf(id_string, "%d", player->id);
    char *cmd = malloc(strlen(message) + 10 * sizeof(char));
    strcpy(cmd, message);
    memcpy(cmd + strlen(message), ":", strlen(":"));
    memcpy(cmd + strlen(message) + strlen(":"), id_string, strlen(id_string));
    cmd[strlen(message) + strlen(":") + strlen(id_string)] = '\x00';

    //printf("Sending the command: %s\n", cmd);

    if(write(sockfd, cmd, strlen(cmd)) < 0) {
        printf("Something went wrong while attempting to send %s.\nExiting..", message);
        exit(69);
    }

    recv(sockfd, recv_buf, 1448, 0);
    recv(sockfd, recv_buf + 1448, 1448, 0);
    recv(sockfd, recv_buf + 2*1448, 1448, 0);
    recv(sockfd, recv_buf + 3*1448, 1448, 0);
    recv(sockfd, recv_buf + 4*1448, 24, 0);
    //recv(sockfd, recv_buf + 4*1448, 1448, 0);
    //recv(sockfd, recv_buf + 5*1448, 1448, 0);
    //recv(sockfd, recv_buf + 6*1448, 210, 0);
    uint8_to_poly_msg(recv_buf, &msg);

    //printf("Received message-id %u from player-id %u\n", msg.message_id, msg.player_id);

    if(verify_poly_message(player, &msg) != 1) {
        printf("WARNING: Could not verify the message received in round %d from player %d.\n", round, player->id);
    }

    uint8_to_poly(msg.message, &recv_poly);

    if(round == 1) {
        copy_poly(&recv_poly, &player->z);
    } else if (round == 2) {
        copy_poly(&recv_poly, &player->X);
    }

    free(cmd);
}

int highest_player(player **playerlist, int c, player *self) {
    int highest = -1;
    for (int i = 0; i < c; ++i) {
        if (playerlist[i]->id > highest) {
            highest = playerlist[i]->id;
        }
    }

    if (highest == self->id) {
        return 1;
    }

    return 0;
}

void get_rec(const int sockfd, poly *rec, player** playerlist, int player_count) {
    rec_message_t msg;
    uint8_t raw_buffer[REC_MSG_LENGTH];

    char *cmd = "rcval";

    if(write(sockfd, cmd, strlen(cmd)) < 0) {
        printf("Could not request the reconciliation vector.\nExiting..\n");
        exit(69);
    }

    recv(sockfd, raw_buffer, REC_MSG_LENGTH, 0);
    uint8_to_rec_message(raw_buffer, &msg);

    for (int i = 0; i < player_count; ++i) {
        if(playerlist[i]->id != msg.player_id) {
            continue;
        }
        if(verify_rec_message(playerlist[i], &msg)) {
            printf("WARNING: Could not verify the reconciliation vector message!\n");
        }
    }

    for(int i = 0; i < 128; i++) {
        rec->coefficients[8 * i + 0] = (msg.message[i] >> 7) & 1;
        rec->coefficients[8 * i + 1] = (msg.message[i] >> 6) & 1;
        rec->coefficients[8 * i + 2] = (msg.message[i] >> 5) & 1;
        rec->coefficients[8 * i + 3] = (msg.message[i] >> 4) & 1;
        rec->coefficients[8 * i + 4] = (msg.message[i] >> 3) & 1;
        rec->coefficients[8 * i + 5] = (msg.message[i] >> 2) & 1;
        rec->coefficients[8 * i + 6] = (msg.message[i] >> 1) & 1;
        rec->coefficients[8 * i + 7] = (msg.message[i] >> 0) & 1;
    }

}

void send_rec(const int sockfd, const poly *rec, const player *self) {
    size_t num_bytes = REC_MSG_LENGTH + strlen("srec:");
    rec_message_t msg;
    create_rec_message(self, rec, &msg);
    uint8_t buffer[REC_MSG_LENGTH];
    rec_message_to_uint8(&msg, buffer);

    char data[REC_MSG_LENGTH + strlen("srec:")];
    strcpy(data, "srec:");
    memcpy(data + strlen("srec:"), buffer, REC_MSG_LENGTH);

    //printf("Sending reconciliation vector. Length: %lu\n", num_bytes);
    send(sockfd, data, REC_MSG_LENGTH + strlen("srec:"), 0);
}

void publish_vk(int sockfd, player *p) {

    char data[VK_MESSAGE_LENGTH + 5];
    uint8_t raw_vk_msg[VK_MESSAGE_LENGTH];

    vk_message_t vk_msg;
    vk_msg.player_id = p->id;
    memcpy(vk_msg.verification_key, p->falcon_pub, VK_LENGTH);
    deserialize_vk_msg(&vk_msg, raw_vk_msg);

    strcpy(data, "pubvk");
    memcpy(data + 5, raw_vk_msg, VK_MESSAGE_LENGTH);

    send(sockfd, data, VK_MESSAGE_LENGTH + 5, 0);
}

void get_vk(int sockfd, player *p) {

    uint8_t buffer[VK_MESSAGE_LENGTH];
    vk_message_t vk_msg;
    char cmd[12];
    sprintf(cmd, "gtvk:%d", p->id);

    send(sockfd, cmd, strlen(cmd), 0);

    recv(sockfd, buffer, VK_MESSAGE_LENGTH, 0);
    serialize_vk_msg(buffer, &vk_msg);

    memcpy(p->falcon_pub, vk_msg.verification_key, VK_LENGTH);
}

void publish_nonce(int sockfd, player *p) {

    char data[5 + 4];
    uint8_t tmp[4];
    strcpy(data, "pubnc");
    tmp[0] = (uint8_t) ((p->nonce >> 24) & 0xff);
    tmp[1] = (uint8_t) ((p->nonce >> 16) & 0xff);
    tmp[2] = (uint8_t) ((p->nonce >>  8) & 0xff);
    tmp[3] = (uint8_t) ((p->nonce >>  0) & 0xff);
    memcpy(data + 5, tmp, 4);

    send(sockfd, data, 9, 0);
}

void get_nonce(int sockfd, player *p) {
    uint8_t buffer[4];
    bzero(buffer, 4);

    char cmd[12];
    sprintf(cmd, "gtnc:%d", p->id);
    send(sockfd, cmd, strlen(cmd), 0);

    recv(sockfd, buffer, 4, 0);

    p->nonce = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3]);
}


void encrypt_data(const uint8_t *key, uint8_t *pt, chat_message_t *chat_msg, size_t pt_len) {
    randombytes((unsigned char *) chat_msg->nonce, 24);
    crypto_lock(chat_msg->mac, chat_msg->data, key, chat_msg->nonce, pt, pt_len);
}

void decrypt_data(const uint8_t *key, const chat_message_t *chat_msg, pt_chat_message_t *pt_msg, size_t pt_len) {
    if(crypto_unlock(pt_msg->message, key, chat_msg->nonce, chat_msg->mac, chat_msg->data, pt_len)) {
        printf("Could not decrypt the message!\n");
    }
    pt_msg->player_id = chat_msg->player_id;
}

void msg_receiver(void *args) {
    thread_info_t *thread_info = args;
    uint8_t buffer[CHAT_MESSAGE_LENGTH];
    uint16_t bytes_read;
    chat_message_t enc_msg;
    pt_chat_message_t pt_msg;

    while(1) {
        pthread_mutex_lock(&socket_lock);
        bytes_read = recv(*thread_info->sockfd, buffer, CHAT_MESSAGE_LENGTH, 0);
        pthread_mutex_unlock(&socket_lock);

        serialize_chat_msg(buffer, &enc_msg);
        decrypt_data(thread_info->secret_key, &enc_msg, &pt_msg, enc_msg.msg_len);
        pt_msg.message[enc_msg.msg_len] = '\0';

        if(pt_msg.player_id != thread_info->player->id) {
            printf("\n# %s: %s", enc_msg.player_name, pt_msg.message);
            printf(">");
            fflush(stdout);
        }
        sleep(1);
        bzero(&pt_msg, sizeof(pt_msg));
        bzero(&buffer, CHAT_MESSAGE_LENGTH);
    }
}

void msg_sender(void *args) {

    thread_info_t *thread_info = args;
    char buffer[1024];
    chat_message_t msg;
    msg.player_id = thread_info->player->id;
    strncpy(msg.player_name, thread_info->player->name, 20);

    uint8_t raw_msg[CHAT_MESSAGE_LENGTH];
    char data[CHAT_MESSAGE_LENGTH + 5];

    printf("=======================================================\n");
    printf("Your username: %s\n", thread_info->player->name);
    printf("=======================================================\n");

    while(1) {
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);

        msg.player_id = thread_info->player->id;
        msg.msg_len = strlen(buffer);
        encrypt_data(thread_info->secret_key, (uint8_t *) buffer, &msg, strlen(buffer));
        deserialize_chat_msg(&msg, raw_msg);

        strcpy(data, "snmsg");
        memcpy(data + strlen("snmsg"), raw_msg, CHAT_MESSAGE_LENGTH);

        //pthread_mutex_lock(&socket_lock);
        send(*thread_info->sockfd, data, CHAT_MESSAGE_LENGTH + 5, 0);
        //pthread_mutex_unlock(&socket_lock);

        bzero(buffer, 1024);
        bzero(raw_msg, CHAT_MESSAGE_LENGTH);
    }
}

int main(int argc, char* argv[]) {

    /**
     * INITIALIZATION
     */


    if(argc != 4) {
        printf("Usage: %s <hostname> <port> <username>.\nExiting..\n", argv[0]);
        exit(42);
    }

    /*char *names[] = {"Alice", "Bob", "Charlie", "Dave", "Erik", "Fabian", "Garry", "Henry", "Indy", "Johnny", "Karen",
                     "Hirohito", "Larry", "Marry", "Nina", "Oscar", "Perry", "Queue", "Ronny", "Simen", "Terrance", "Kenobi", "Grevious"};
    char *username = names[random_u64() % 20];*/


    //char *hostname = "127.0.0.1";
    //int port = 6969;

    char *hostname = argv[1];
    int port = atoi(argv[2]);
    char *username = argv[3];

    printf("Connecting to %s on port %d.\n", hostname, port);
    printf("My username is: %s.\n", username);

    poly_message_t msg;
    poly public_element;
    poly rec;
    player *self = malloc(sizeof(player));

    // Not backdoored at all, I promise.
    uint64_t coeffs[] = {15541090734, 11007635855, 5861001444, 21859989641, 15033456806, 1104043339, 38423797459, 14496622580, 43371480870, 33874261893, 22461754935, 34895654990, 14856299486, 11546008385, 33634641034, 10626249990, 1455867280, 23172303194, 25510199398, 2463008499, 27501820434, 28884230565, 24313273866, 37318061516, 44954262395, 11917233095, 7922052427, 42650661888, 7194120498, 41574321874, 36544159375, 31017689491, 5846141968, 42366313397, 40819484917, 5090812299, 13626777972, 42233913311, 35379315778, 31934784461, 807940667, 26657221856, 2381114172, 13618080447, 35762288932, 27216934151, 37888730974, 45425638020, 247704940, 1451126118, 2769858055, 24799123138, 25051014958, 30365510500, 15259761267, 13526124296, 9894936773, 35687143073, 30853799156, 38234510485, 18875205873, 5458173448, 23325147797, 25296290342, 44761595257, 30236332513, 45091220391, 28248624495, 43496469700, 14530203439, 41597528148, 44739665004, 2215213386, 18817904617, 243245614, 30246436980, 30322750783, 990576303, 7899588743, 28643114553, 6361167525, 15388670559, 31077683507, 16419509633, 23147873192, 10795189808, 15072118604, 10125507298, 25369083141, 11015353258, 1071076600, 44341058478, 32268546113, 22996740405, 12965704825, 13309686666, 42601519284, 26261356197, 18641333800, 13760768157, 14552260628, 23976748538, 25413715410, 1069872108, 37005747003, 23570122442, 1013684400, 30408215621, 32341738183, 19768851907, 40273627879, 34905540930, 14035595554, 45075245488, 25178662748, 34807454541, 18496769302, 41679391384, 43058027521, 23369375453, 43562041427, 41420468540, 28600779959, 6483510583, 43267994712, 32765012912, 25175825372, 45009458736, 9385116995, 40140487801, 19160726754, 17191476328, 11094539995, 8949858066, 32923745723, 20095536084, 18159044297, 8597466570, 19106937485, 29750655538, 25221907125, 42216390276, 14833016396, 14628699679, 8430867085, 26890837767, 27500235013, 1698586283, 41407389200, 17249677918, 41963107776, 12228972006, 15651697308, 8815433123, 1918777969, 32932854250, 29356598212, 14449635062, 41881811322, 27197961912, 2706465916, 6215055818, 32410243168, 2167007705, 42971872348, 44813201435, 10897796287, 38700307753, 5978848395, 31958117177, 16187352741, 44291262252, 9582718919, 5731444377, 21103174084, 34282124094, 45057690065, 12665856701, 40056624792, 23328061999, 34078301976, 33653765755, 11253088111, 37995515597, 3291551222, 34926498155, 13037514625, 18257833219, 40444513984, 42863966468, 39514690630, 14978720273, 36490111510, 6019444460, 20593120995, 12734021420, 8416895148, 45258009842, 22400084153, 16551760093, 8310500275, 14246033246, 4791085354, 15872426731, 41206315576, 25047716013, 35002903393, 15218372956, 32264229046, 22539240017, 38598601629, 20652037315, 14608077429, 18887927277, 28754382605, 24163445918, 22224509285, 31678704045, 7483900888, 39526744776, 20245604824, 28380068686, 6975506078, 27113860499, 5119846538, 36053004610, 22627884856, 5841963903, 30411910592, 36896303095, 26577502648, 14293299487, 25738392699, 17200834818, 19478992813, 14825426979, 42479830206, 23566062416, 3189472225, 31798012466, 6702779381, 23232626068, 39249308526, 19453823600, 6187696855, 17905306004, 36773024587, 38088154471, 17029547093, 32976556345, 39804038488, 19019264514, 37396297665, 3331324532, 9623298607, 24121047944, 1581541770, 32973137463, 10866926773, 4998236875, 24464194315, 17096988090, 928027587, 14314378943, 36774122548, 1622554833, 21714879759, 25397188363, 15366379206, 29463534408, 35529228106, 38648706819, 19097631735, 14905491196, 35666891029, 15651057077, 32199871909, 31870417450, 4780165671, 5282104668, 12836803380, 12064987434, 9076948044, 7469628631, 34888467453, 25756379757, 30523586940, 44849114925, 25276080266, 38506445450, 32695237375, 37376529869, 11367834112, 3332580065, 26508280588, 30872404292, 41294029376, 2383322955, 41509653428, 19810448492, 37784568346, 19525096516, 14518127813, 42184701046, 38140521430, 45253947233, 16441578234, 15147103321, 34047290865, 22552931577, 37861986565, 41666097419, 34652698342, 21775639138, 2412391968, 36591764630, 2875985341, 19900312008, 7492089470, 18081865137, 4952437337, 23794904780, 24509255007, 8183419519, 41291200526, 35004187232, 2869467177, 8873068129, 37124658317, 6440366849, 41285470379, 21795924068, 6383368258, 45141653695, 22440083716, 5054199810, 27263509921, 19636062251, 17177757985, 28664935075, 6334237842, 7492046970, 19692546939, 18921901506, 35095427141, 31058212876, 8124001971, 40562374509, 3908001034, 4836678996, 43089236750, 8218002831, 31051323815, 36392145748, 12937768424, 25735153050, 39030662072, 15555316949, 13039740991, 17749068177, 31097798212, 7334465259, 12148185772, 8818249855, 15609267150, 43155395463, 39538706548, 13572698216, 13439360395, 28035168668, 11079881534, 29843246321, 23398932204, 2176498752, 7054471001, 4359095859, 41312195606, 16280085124, 11777986068, 17405672876, 39460429162, 19379080834, 43790086560, 21341607115, 38431760346, 32834303781, 27400424391, 2837588110, 26502330967, 16975719302, 29717288703, 10926364200, 44752723578, 43645793266, 5403312333, 20727602413, 37820238316, 9621160829, 35380197891, 23762599998, 32510102745, 42804356934, 6277364145, 6343065951, 19017116739, 35656604288, 29022188129, 35066898882, 19084465359, 31100421613, 30185653799, 23573002520, 27476339560, 32498035661, 39871323860, 32764962743, 6675310955, 5485296571, 35660840105, 41745552712, 2458345295, 2914815707, 35005037469, 18759727763, 3823925320, 26281349982, 24599743467, 1488500532, 36155364939, 19203983614, 11611990403, 27932226558, 23762510386, 44578019487, 10515237475, 19772159031, 27198022114, 35178691348, 43501950897, 10153100367, 24253131588, 6380829479, 21669853066, 34545602181, 9757945951, 30878531710, 32538389653, 1775061751, 3864762154, 44736736562, 31126398020, 39628783702, 7441364659, 3771743635, 11386824229, 15455399383, 8446752250, 22493040827, 40213092261, 10167833146, 4698358018, 11955772990, 32572057706, 22679198527, 44459579920, 44078093753, 21996716801, 5531668716, 34308919789, 30176755248, 12225955654, 24877085321, 44419785610, 39487760592, 19912931254, 36281823489, 6829260839, 6116075013, 1242882647, 19072948872, 16941528429, 1618595006, 653023960, 45227921240, 33443991555, 36882164724, 36999465589, 32543010845, 39947960893, 11351001561, 711756189, 30356438717, 9223024359, 35553384142, 37877455186, 29381936972, 39468851778, 9826686933, 29129605052, 7953067281, 7100819163, 22056912897, 39714428753, 40975495509, 27045423372, 31906176979, 4094802160, 3625528944, 43944045451, 44587193336, 6317411313, 13083909489, 35667182158, 34884601489, 30090259940, 42615838802, 41807347793, 16793313109, 39245523076, 31435739593, 29355566616, 44353705604, 18877259694, 29570259375, 17822929950, 39274511646, 10190958989, 9274218621, 37622734892, 3274811928, 40105553870, 30898049507, 21858992951, 2847819930, 25248006092, 14120910060, 17552278591, 39175091013, 6922682124, 24862349574, 25229704352, 879524708, 19743931171, 22126985654, 18034079169, 6275712463, 5880439359, 44386559941, 42883042445, 23785948871, 34104968074, 30196478717, 20817695053, 30193973049, 28645718188, 26549785948, 42525143606, 20499835171, 39231394644, 16593059670, 25015909612, 26391842060, 32372311693, 15514432134, 43756067370, 21887725625, 17752285791, 34442101594, 6452098110, 537834436, 36599986318, 33840902040, 7462474852, 32136185207, 7256767149, 14208024824, 28063016684, 2977811932, 25257324613, 14155318969, 19716954558, 6339468453, 24373851606, 21779458663, 14459687888, 37325256852, 32174886360, 8942691267, 25763995547, 23384253468, 1714000340, 1784407053, 32296351405, 3985516737, 15410656002, 33867411823, 31043048451, 1877390500, 35955957088, 12742214600, 44346921597, 1306739612, 32813877293, 7137293028, 19223645927, 10721308730, 1337399043, 20742272380, 14170028078, 5604690994, 5214246356, 44255974106, 40902586415, 26239742136, 32964850546, 14715954332, 41586600814, 3780745374, 11174258965, 15278285957, 13167636947, 15895650622, 16708448724, 28931047011, 23116082926, 32697059944, 40350045330, 7047940440, 4107929927, 25390908196, 43790419558, 42385059344, 711532189, 42239569816, 8460990505, 37499158706, 30396146012, 26730059156, 20982384110, 35589797865, 8409003892, 13318964715, 34266268507, 2918357091, 13969871794, 34582327927, 26639633298, 36147849353, 29696846592, 15143838637, 2311269987, 21397281989, 41493213775, 24391308601, 6629352637, 29668920074, 28071668831, 34563288850, 2714401722, 25402928875, 10698702250, 2509869898, 2641397041, 22485702337, 18727695891, 15724134118, 37670004728, 33274444927, 33931831596, 14013404692, 34509142614, 17153045140, 8438904765, 12628328983, 7608394577, 31146592775, 20590772761, 29932530276, 16480324818, 7827639927, 28718083172, 44618020829, 15300133093, 220608093, 43458124483, 4454876149, 32015606167, 24726275036, 2911784212, 20155130827, 10962805746, 39382227113, 2317434865, 15272713387, 37868418336, 10819705330, 16400258639, 13226913995, 28405136282, 25974648501, 27364046392, 15136233976, 8260538589, 45072290451, 4036543414, 35692517950, 41233602900, 905645419, 20417272974, 28537926053, 28522294063, 44158383132, 788642081, 29178085567, 579757665, 34715008489, 15956991786, 26856069579, 3538445088, 9327329013, 23523878179, 24846728741, 14536575074, 34320231336, 12854696676, 13399319920, 16130935382, 29223512457, 18103512450, 36035202142, 24954759797, 24828618642, 25672009117, 41148915300, 22148428548, 12039226656, 43074242831, 18164375509, 42238878015, 17134799996, 38505227538, 34479298925, 40994684855, 28497359291, 29408898014, 44929775438, 38504858792, 28287024052, 36820448100, 39498482157, 30519322951, 17775848396, 29234021939, 39248922041, 5111324723, 42883731177, 4431612820, 2424745546, 34542577411, 6381449829, 40093853606, 30780407856, 23169678338, 12974115343, 29388644656, 11653334678, 17282237404, 35653039492, 12579999774, 14733678232, 31883469455, 21312637865, 14597348785, 14936346777, 78772928, 25781221310, 23446747949, 42219444431, 28973503766, 37723087948, 41080488822, 11874609094, 36821049264, 14446684578, 435408398, 7890062960, 41682743440, 13325706892, 21051310722, 41557548877, 4130690277, 21105782235, 3585021969, 31862649666, 41933490868, 35818051993, 11220264103, 41459884657, 10782526189, 17083425393, 23411519976, 31499401441, 39755692849, 6482415918, 24792484712, 12078927018, 25943787179, 1712135513, 14098468623, 17577996812, 28482062792, 7741526426, 28857302368, 14514949236, 10769847907, 19405213389, 5461261167, 2914768888, 43891407673, 31418068423, 41259794166, 23589656381, 17613617795, 16033158759, 23913525653, 3692348131, 20708401445, 27413447844, 190881579, 43787181005, 34734946549, 45282265303, 36332309592, 26886318313, 19443720374, 11257887358, 29262493749, 19282063872, 27645289266, 9644514078, 23060128339, 8187290006, 9624032928, 15562393994, 4847869619, 38893237680, 32696459623, 37639714878, 32314391840, 23936810438, 4018527937, 8446595018, 11073103625, 37213996008, 35539223120, 41847243710, 28809218340, 40536467810, 25139308688, 24729728345, 2359706051, 13862483227, 8478212507, 39578597266, 29155275067, 33719103619, 40814113441, 5823775075, 38197133157, 29832965194, 23310300767, 3145374635, 7227414262, 44075597057, 24681454903, 37267060787, 7352030284, 16252362641, 30360684625, 18997902568, 18729949067, 7119504902, 32256615150, 16065946820, 1436261257, 29599063105, 44972486597, 32892286723, 24172113121, 10489417596, 42816258213, 21926337637, 19945846769, 10307890943, 9829600768, 35888527481, 37011650212, 22548931751, 30668745675, 36664429832, 42446070491, 10947547546, 2204094004, 35590028056, 43374979619, 38594262878, 8833662115, 4357392046, 6111853012, 12955206666, 35599518056, 27650340318, 37506755403, 40396356720, 30024379128, 11515691603, 716767948, 38598886696, 28744880501, 30627720628, 43536419528, 7867063975, 12526178483, 7697119978, 31440276475, 35396199455, 27791482187, 8345209157, 6492255959, 40599027946, 22299204211, 8328510406, 33205954805, 34163846570, 18169786179, 42750305314, 13149561524, 39254758341, 34846588721, 42184733749, 24026612835, 42799097674, 16265375356, 2015085765, 29415190939, 27245945646, 2025021110, 44681019675, 14716577295, 21774880326, 25799118733, 39974693513, 22775199288, 29641983983, 6129935424, 41432229747, 23658812373, 44226887245, 30229513016, 35164973799, 19209670936, 31139550979, 4659997411, 20174320643, 45368387355, 32733217112, 26342097039, 43270170968, 29276900796, 16543592399, 22229076413, 38555532193, 5789187356, 25599038601, 1071323827, 23081653418, 24331818760, 9696775994, 36139206301, 21796767824, 38968963522, 36446859577, 28969838950, 4101850230, 18964305260, 39310798589, 34885361450, 24970471972, 20454556021, 38842898573, 30191942033, 31880741378, 29482574205, 8177575092, 24207039520, 14528633028, 35492129413, 4433754198, 34194333858, 11244252264, 14032492792, 2548112354, 33414872763, 10869705575, 23735309193, 10224485317, 19844349375, 16430213820, 11234078910, 2941047356, 16998388811, 44420591935, 15877803158, 45327684196, 28515234163, 10314109708, 8476391624, 24148570280, 15511590387, 5598778313, 26817047548, 2428828726, 19936342136, 29541181262, };
    for (int i = 0; i < APON_N; i++) {
        public_element.coefficients[i] = coeffs[i];
    }

    int sockfd = setup_connection(hostname, port, port);

    create_player(-1, self, &public_element, 0);
    self->nonce = random_u64();
    strncpy(self->name, username, 20);
    //self->name = username;

    //printf("Sampled secret and error polynomials. My nonce is: %u. Waiting for the other participants..\n", self->nonce);
    wait_until_ready(sockfd, "plrdy");

    get_id(sockfd, self);
    printf("My ID number is %d\n", self->id);
    printf("My VK fingerprint is: ");
    for (int i = 0; i < 10; ++i) {
        printf("%x", self->falcon_pub[i]);
    }
    printf("\n");

    publish_nonce(sockfd, self);
    publish_vk(sockfd, self);

    int player_count = 0;
    player **playerlist = construct_player_list(sockfd, self, &player_count);
    printf("Number of participants: %d\n", player_count);


    /**
     * ROUND 1: BROADCAST PUBLIC KEYS
     */

    create_poly_message_from_round(self, 1, &msg);
    send_poly_message(sockfd, &msg, "setz:");
    wait_until_ready(sockfd, "zrddy");

    for (int i = 0; i < player_count; ++i) {
        if(playerlist[i]->id == self->id) {
            continue;
        }
        get_nonce(sockfd, playerlist[i]);

        //printf("Got nonce from player %d: %u\n", playerlist[i]->id, playerlist[i]->nonce);

        get_vk(sockfd, playerlist[i]);

        printf("Fingerprint of VK from Player %d: ", playerlist[i]->id);
        for (int j = 0; j < 10; ++j) {
            printf("%x", playerlist[i]->falcon_pub[j]);
        }
        printf("\n");
    }

    for (int i = 0; i < player_count; ++i) {
        if(playerlist[i]->id == self->id) {
            continue;
        }
        get_val(sockfd, playerlist[i], "gzvl", 1);
    }


    //printf("Retrieved all public keys.\n");

    /**
     * ROUND 2: BROADCAST ALL NEIGHBOURHOOD KEYS
     */
    calculate_x_from_playerlist(playerlist, player_count, self);
    create_poly_message_from_round(self, 2, &msg);
    send_poly_message(sockfd, &msg, "setX:");
    //printf("Sent X-value. Waiting for other X-values.\n");

    wait_until_ready(sockfd, "xrddy");

    for (int i = 0; i < player_count; ++i) {
        if(playerlist[i]->id == self->id) {
            continue;
        }

        get_val(sockfd, playerlist[i], "gxvl", 2);
    }

    //printf("Retrieved all X-values.\n");

    calculate_b_from_playerlist(playerlist, player_count, self);

    if(highest_player(playerlist, player_count, self)) {
        //printf("I have the highest value ID. Creating reconciliation vector.\n");

        poly_add(&self->b, &self->epp, &self->b);
        rec_msg_ct(&self->b, &rec);
        rec_key_ct(&rec, &self->b, &self->key);
        send_rec(sockfd, &rec, self);
    } else {
        //printf("I do not have the highest value ID. Waiting for reconciliation vector..\n");
        wait_until_ready(sockfd, "rcrdy");

        get_rec(sockfd, &rec, playerlist, player_count);
        rec_key_ct(&rec, &self->b, &self->key);
    }

    crypto_wipe(&self->b, sizeof(poly));
    crypto_wipe(&self->secret, sizeof(poly));


    uint8_t master_key[32];
    uint8_t tmp_key[128];
    key_poly_to_uint8(&self->key, tmp_key);
    crypto_wipe(&self->key, sizeof(poly));
    sha3(tmp_key, 128, master_key, 32);
    crypto_wipe(tmp_key, 128);

    printf("The shared secret:\n");
    for (int i = 0; i < 32; i += 2) {
        printf("%x", master_key[i] << 8 | master_key[i + 1]);
    }
    printf("\n");

    pthread_t threads[2];
    thread_info_t *thread_info = malloc(sizeof(thread_info_t));
    thread_info->secret_key = master_key;
    thread_info->sockfd = &sockfd;
    thread_info->player = self;

    pthread_create(&(threads[0]), NULL, &msg_sender, thread_info);
    pthread_create(&(threads[1]), NULL, &msg_receiver, thread_info);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    crypto_wipe(master_key, 32);



    pthread_mutex_destroy(&socket_lock);
    free(thread_info);
    close(sockfd);

    exit(1337);
}