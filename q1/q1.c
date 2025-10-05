#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { ADULT, YOUNG, CHILD, INVALID_AGE } Age;
typedef enum { G, PG12, R18, INVALID_RATING } Rating;

typedef struct {
    Age age;
    Rating rating;
    int startHH;
    int startMM;
    int durH;
    int durM;
    char row;
    int col;
} Ticket;

const int PRICE_ADULT = 1800;
const int PRICE_YOUNG = 1200;
const int PRICE_CHILD = 800;

const char *MSG_NEED_ADULT = "対象の映画の入場には大人の同伴が必要です";
const char *MSG_AGE_LIMIT = "対象の映画は年齢制限により閲覧できません";
const char *MSG_SEAT_LIMIT = "対象のチケットではその座席をご利用いただけません";

int parseTime(const char *s, int *h, int *m) {
    return sscanf(s, "%d:%d", h, m) == 2;
}

Age parseAge(const char *s) {
    if (strcmp(s, "Adult") == 0) return ADULT;
    if (strcmp(s, "Young") == 0) return YOUNG;
    if (strcmp(s, "Child") == 0) return CHILD;
    return INVALID_AGE;
}

Rating parseRating(const char *s) {
    if (strcmp(s, "G") == 0) return G;
    if (strcmp(s, "PG-12") == 0) return PG12;
    if (strcmp(s, "R18+") == 0) return R18;
    return INVALID_RATING;
}

int parseSeat(const char *s, char *row, int *col) {
    char c;
    if (sscanf(s, "%c-%d", &c, col) != 2) return 0;
    c = toupper(c);
    if (c < 'A' || c > 'L' || *col < 1 || *col > 24) return 0;
    *row = c;
    return 1;
}

int calcEndMinutes(const Ticket *t) {
    int start = t->startHH * 60 + t->startMM;
    int end = start + t->durH * 60 + t->durM;
    return end;
}

int checkRating(Age age, Rating rating, int hasAdult) {
    if (rating == G) return 1;
    if (rating == PG12) {
        if (age == CHILD && !hasAdult) return 0;
        return 1;
    }
    if (rating == R18) {
        return (age == ADULT);
    }
    return 0;
}

int checkSeat(const Ticket *t) {
    if ((t->row == 'J' || t->row == 'K' || t->row == 'L') && t->age == CHILD)
        return 0;
    return 1;
}

int checkTimeRule(const Ticket *t, int endMinutes, int hasAdult, int hasChild) {
    if (hasAdult) return 1;

    int endH = endMinutes / 60;
    int endM = endMinutes % 60; // chỉ để so sánh chính xác 16:00, 18:00
    (void)endM;

    if (hasChild) {
        // Có trẻ em mà không có người lớn
        if (endH > 16) return 0;
        if (endH == 16 && endM > 0) return 0;
    }
    if (t->age == YOUNG) {
        if (endH > 18) return 0;
        if (endH == 18 && endM > 0) return 0;
    }
    return 1;
}

int main() {
    char line[256];
    Ticket tickets[100];
    int n = 0;

    while (fgets(line, sizeof(line), stdin)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;

        char *parts[5];
        char *p = strtok(line, ",");
        int count = 0;
        while (p && count < 5) {
            while (*p == ' ') p++;
            parts[count++] = p;
            p = strtok(NULL, ",");
        }
        if (count != 5) {
            printf("不正な入力です\n");
            return 0;
        }

        Ticket t;
        t.age = parseAge(parts[0]);
        t.rating = parseRating(parts[1]);
        if (!parseTime(parts[2], &t.startHH, &t.startMM) ||
            !parseTime(parts[3], &t.durH, &t.durM) ||
            !parseSeat(parts[4], &t.row, &t.col) ||
            t.age == INVALID_AGE || t.rating == INVALID_RATING) {
            printf("不正な入力です\n");
            return 0;
        }

        tickets[n++] = t;
    }

    if (n == 0) return 0;

    // Thuộc tính nhóm
    int hasAdult = 0, hasChild = 0;
    for (int i = 0; i < n; i++) {
        if (tickets[i].age == ADULT) hasAdult = 1;
        if (tickets[i].age == CHILD) hasChild = 1;
    }

    Rating rating = tickets[0].rating;
    int endMinutes = calcEndMinutes(&tickets[0]);

    int anyNg = 0;
    char reasons[3][128];
    int reasonCount = 0;

    for (int i = 0; i < n; i++) {
        int needAdult = !checkTimeRule(&tickets[i], endMinutes, hasAdult, hasChild);
        int ageLimit = !checkRating(tickets[i].age, rating, hasAdult);
        int seatLimit = !checkSeat(&tickets[i]);

        if (needAdult || ageLimit || seatLimit) {
            anyNg = 1;
            if (needAdult && reasonCount < 3) strcpy(reasons[reasonCount++], MSG_NEED_ADULT);
            if (ageLimit && reasonCount < 3) strcpy(reasons[reasonCount++], MSG_AGE_LIMIT);
            if (seatLimit && reasonCount < 3) strcpy(reasons[reasonCount++], MSG_SEAT_LIMIT);
        }
    }

    if (anyNg) {
        int printed[3] = {0, 0, 0};
        for (int i = 0; i < reasonCount; i++) {
            if (strcmp(reasons[i], MSG_NEED_ADULT) == 0 && !printed[0]) {
                printf("%s\n", MSG_NEED_ADULT);
                printed[0] = 1;
            }
        }
        for (int i = 0; i < reasonCount; i++) {
            if (strcmp(reasons[i], MSG_AGE_LIMIT) == 0 && !printed[1]) {
                printf("%s\n", MSG_AGE_LIMIT);
                printed[1] = 1;
            }
        }
        for (int i = 0; i < reasonCount; i++) {
            if (strcmp(reasons[i], MSG_SEAT_LIMIT) == 0 && !printed[2]) {
                printf("%s\n", MSG_SEAT_LIMIT);
                printed[2] = 1;
            }
        }
        return 0;
    }

    for (int i = 0; i < n; i++) {
        int price = 0;
        if (tickets[i].age == ADULT) price = PRICE_ADULT;
        else if (tickets[i].age == YOUNG) price = PRICE_YOUNG;
        else if (tickets[i].age == CHILD) price = PRICE_CHILD;
        printf("%d円\n", price);
    }

    return 0;
}
