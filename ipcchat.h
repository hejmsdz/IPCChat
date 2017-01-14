/*
    MR & JP
    ver. 1.3.0
    16:15 14.01.2017 (w razie jakby coś się zmieniło jeszcze)
*/

#ifndef IPCCHAT_IPCCHAT_H
#define IPCCHAT_IPCCHAT_H

// maksymalna liczba użytkowników
#define MAX_USERS 32

// maksymalna liczba grup (użytkownik może być na raz podłączony do wszystkich grup)
#define MAX_GROUPS 32

// Można (nawet będzie łatwiej) trzymać użytkowników i grupy na liście zamiast w tablicy.
// Przy wyborze tablicy trzeba stosować się do powyższych dwóch stałych.

// maksymalna długość nazwy użytkownika/kanału
#define MAX_NAME_LENGTH 32

// maksymalna długość wiadomości
#define MAX_MESSAGE_LENGTH 2048


/*

# Wymiana danych

Serwer tworzy kolejkę i wypisuje jej ID, żeby klient wiedział, gdzie się podłączyć.
Klient też tworzy swoją kolejkę i zaraz po otwarciu kolejki serwera wysyła tam komendę login ze swoim queue_id.
Po wylogowaniu klient nie wyłącza się sam z siebie, ale czeka na wiadomość pożegnalną (typu 3).

Komunikacja klient -> serwer przez kolejkę serwera, struktura command
Komunikacja serwer -> klient przez kolejkę klienta, struktura message


# Lista komend

KOMENDA                 MTYPE   OPIS                        UWAGI
login [id_kolejki]      2       zalogowanie                 to powinna być pierwsza komenda wysyłana po podłączeniu klienta
logout                  1       wylogowanie                 trzeba odpowiedzieć wiadomością o mtype = 3, żeby wyłączyć klienta

join [nazwa_pokoju]     1       dołącz do pokoju
leave [nazwa_pokoju]    1       wyjdź z pokoju
rooms                   1       wyświetl pokoje             serwer powinien odpowiedzieć wiadomością (server jako nadawca)
users                   1       wyświetl użytkowników       jw.
help                    1       wyświetl dostępne komendy   jw.

@[nick] [treść]         1       wiadomość prywatna          loginy nie zawierają białych znaków, po @ nie ma spacji!
#[nazwa_pokoju] [treść] 1       wiadomość do pokoju         jw.
* [treść]               1       wiadomość do wszystkich

*/


// struktura komend wysyłanych przez klienta na serwer
// (wysłanie wiadomości też rozumiemy jako komendę)
struct command {
    long mtype; // zależnie od typu komendy, opisane wyżej

    char data[2*MAX_MESSAGE_LENGTH]; // treść komendy
    char username[MAX_NAME_LENGTH]; // nazwa użytkownika wysyłającego komendę
};

// struktura wiadomości wysyłanych od serwera do klienta
struct message {
    long mtype;
    /* 1    dla wiadomości od użytkowników
       2    dla wiadomości od serwera
                (np. odpowiedzi na rooms, users, help; ewentualna wiadomość powitalna)
       3    wiadomość specjalna od serwera, po której otrzymaniu klient powinien się zakończyć
                (np. komunikat o odrzuceniu połączenia z powodu przekroczonej liczby użytkowników,
                zajętego nicku, wiadomość pożegnalna po wylogowaniu albo gdy serwer dostał sygnał zamykający)
    */

    char from[MAX_NAME_LENGTH]; // nazwa użytkownika, który wysłał wiadomość
    char to_symbol; // @ - wiadomość prywatna, # - wiadomość na kanał, * - wiadomość do wszystkich
    char to[MAX_NAME_LENGTH];  // nazwa użytkownika/kanału albo puste, jeśli to_symbol = *
    char message[MAX_MESSAGE_LENGTH]; // treść wiadomości
};

#endif //IPCCHAT_IPCCHAT_H