#include <stdio.h>
#include "windows.h"
#include <stdarg.h>  
#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <time.h>

HANDLE console;	
DWORD bytesWritten;

char ARBEIT_START[16] = "00:00:00";
char PAUSE_START[16]  = "00:00:00";
char PAUSE_ENDE[16]   = "00:00:00";
char ARBEIT_ENDE[16]  = "00:00:00";

enum TimeSelect {
    S_ARBEIT_START,
    S_PAUSE_START,
    S_PAUSE_ENDE,
    S_ARBEIT_ENDE
};

void strip_newline(char *s) {
      s[strcspn(s, "\n")] = '\0';
}

enum TimeSelect select = S_ARBEIT_START;
int str_index = 0; // HH MM SS = 6
char *str_select = ARBEIT_START;

void DrawLine(COORD startpos, COORD endpos, char *c) {  
    COORD pos = startpos;
    COORD end = endpos;
    while (pos.X != end.X || pos.Y != end.Y) {

        if (end.X > pos.X) 
            pos.X++;
        else if (end.X < pos.X) 
            pos.X--;

        if (end.Y > pos.Y) 
            pos.Y++;
        else if (end.Y < pos.Y) 
            pos.Y--;	

        SetConsoleCursorPosition(console,pos);
        WriteFile(console,c,1,&bytesWritten,NULL);
    }
}

void PrintAt(int x, int y, const char *fmt, ...) {
    SetConsoleCursorPosition(console,(COORD){x+1,y}); //+1 offset fix
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void ClearScreen() {
      for (int i = 0; i < 30; i++) {
            PrintAt(-1,i,"                                                                     ");
      }
}

void MoveTo(int x, int y) {
    SetConsoleCursorPosition(console,(COORD){x+1,y}); //+1 offset fix
}

/**
* Arbeitszeiten zu Datei speichern 
*/
void SaveToFile() {
    FILE *file = fopen("arbeitszeit.txt","w");
    if (file == NULL) {
        PrintAt(2,18,"DATEI KONNTE NICHT GEÖFFNET WERDEN.");  
        return;
    }
    fprintf(file, "%s\n", ARBEIT_START);
    fprintf(file, "%s\n", PAUSE_START);
    fprintf(file, "%s\n", PAUSE_ENDE);
    fprintf(file, "%s\n", ARBEIT_ENDE);

    fclose(file);
}

/**
* Arbeitszeiten aus der Datei laden
*/
void LoadFromFile() {
    FILE *file = fopen("arbeitszeit.txt","r");
    if (file == NULL) {
        PrintAt(2,18,"DATEI KONNTE NICHT GEÖFFNET WERDEN.");  
        return;
    }

    if(fgets(ARBEIT_START, sizeof(ARBEIT_START), file))
      strip_newline(ARBEIT_START);

    if(fgets(PAUSE_START, sizeof(PAUSE_START), file));
      strip_newline(PAUSE_START);

    if(fgets(PAUSE_ENDE, sizeof(PAUSE_ENDE), file));
      strip_newline(PAUSE_ENDE);

    if(fgets(ARBEIT_ENDE, sizeof(ARBEIT_ENDE), file));
      strip_newline(ARBEIT_ENDE);

    fclose(file);
}
void SetTimeCursor() {

// Reset Cursor Char Bottom
    PrintAt(2,18,"                                   ");
    PrintAt(7,13," ");
    PrintAt(30,13," ");
    PrintAt(7,15," ");
    PrintAt(30,15," ");

// Reset Cursor Char Top
    PrintAt(1,4," ");
    PrintAt(1,5," ");
    PrintAt(1,6," ");
    PrintAt(1,7," ");

    switch (select) {
        case S_ARBEIT_START:
            PrintAt(7,13,">");
            PrintAt(1,4,">");
        break;
        case S_PAUSE_START:
            PrintAt(7,15,">");
            PrintAt(1,5,">");
        break;    
        case S_PAUSE_ENDE:
            PrintAt(30,15,">");
            PrintAt(1,6,">");
        break;
        case S_ARBEIT_ENDE:
            PrintAt(30,13,">");
            PrintAt(1,7,">");
        break;
    }
    // Move Cursor to current pos
    switch (select) {
        case S_ARBEIT_START:
            MoveTo(9+str_index,13);
        break;
        case S_ARBEIT_ENDE:
            MoveTo(32+str_index,13);
        break;
        case S_PAUSE_START:
            MoveTo(9+str_index,15);
        break;       
        case S_PAUSE_ENDE:
            MoveTo(32+str_index,15);
        break;
    }
}

void PrintTimes() {
    PrintAt(9,13,ARBEIT_START);
    PrintAt(23,13,"->");
    PrintAt(32,13,ARBEIT_ENDE);
    
    PrintAt(9,15,PAUSE_START);
    PrintAt(23,15,"->");
    PrintAt(32,15,PAUSE_ENDE);
}

int main() {


    // Arbeitszeit laden
    LoadFromFile();

    console = GetStdHandle(STD_OUTPUT_HANDLE);
	
    ClearScreen();

    int x = 52;
    int y = 16;
    
    DrawLine((COORD){0,1},(COORD){x,1},"-");
    DrawLine((COORD){1,1},(COORD){1,y},"|");
    DrawLine((COORD){1,y},(COORD){x,y},"-");
    DrawLine((COORD){x,1},(COORD){x,y},"|");

    PrintAt(15,2,"Arbeitszeit Erfassung");
    PrintAt(3, 4,"F1 - Arbeitszeit Anfang");   // 59
    PrintAt(3, 5,"F2 - Pause Anfang");         // 60
    PrintAt(3, 6,"F3 - Pause Ende");           // 61
    PrintAt(3, 7,"F4 - Arbeitszeit Ende");     // 62
    PrintAt(30,5,"F5 - Synchronisieren");      // 63
    PrintAt(3,9," Z - Aktuelle Zeit Setzen");
    PrintAt(3,10,"Enter - Speichern");
    PrintAt(30,7,"F8 - Ausgang");              // 65

// Print Time Template 
    PrintAt(19,12,"Arbeitszeit");
    PrintAt(21,14,"Pause");

    PrintAt(8,13,"[00:00:00]");
    PrintAt(23,13,"->");
    PrintAt(31,13,"[00:00:00]");

    PrintAt(8,15,"[00:00:00]");
    PrintAt(23,15,"->");
    PrintAt(31,15,"[00:00:00]");

// Print Time
    PrintTimes();
    SetTimeCursor();

    int c;
    while (1) {
          c = _getch();
//          printf("%i",c);
        switch (c) {
            case 114: // R
                  strcpy(ARBEIT_START,"00:00:00");
                  strcpy(ARBEIT_ENDE, "00:00:00");
                  strcpy(PAUSE_START, "00:00:00");
                  strcpy(PAUSE_ENDE, "00:00:00");
                  PrintTimes();
                  PrintAt(2,18,"Zeit auf 00:00:00 gesetzt.");
            break;
            case 122: { // Z
                  time_t t = time(NULL);
                  struct tm *tm_info = localtime(&t);
                  char buffer[9];
                  strftime(buffer, sizeof(buffer), "%H:%M:%S",tm_info);
                  strcpy(str_select,buffer);

                  PrintTimes();
                  SetTimeCursor();
            } 
            break;
            case 75: // LEFT ARROW
                  str_index--;
                  if (str_index < 0) str_index = 0;
                  if (str_index == 2) str_index--;
                  if (str_index == 5) str_index--;
                  SetTimeCursor();
            break;
            case 77: // RIGHT ARROW
                  str_index++;
                  if (str_index > 7) str_index = 7;
                  if (str_index == 2) str_index++;
                  if (str_index == 5) str_index++;
                  SetTimeCursor();
            break;
            case 13://Enter
                SaveToFile();
                PrintAt(2,18,"Gespeichert.");
            break;
            case 59://F1
                select = S_ARBEIT_START;
                str_select = ARBEIT_START;
                str_index = 0;
                SetTimeCursor();
            break;
            case 60://F2
                select = S_PAUSE_START;
                str_select = PAUSE_START;
                str_index = 0;
                SetTimeCursor();
            break;
            case 61://F3    
                select = S_PAUSE_ENDE;
                str_select = PAUSE_ENDE;
                str_index = 0;
                SetTimeCursor();
            break;
            case 62://F4    
                select = S_ARBEIT_ENDE;
                str_select = ARBEIT_ENDE;
                str_index = 0;
                SetTimeCursor();
            break;
            case 63://F5 
            // 1. 
            // 2. Run Time Registration Service   

            break;
            case 64://F6    
                PrintAt(2,18,"Nicht implementiert        ");
            break;
            case 65://F7    
                PrintAt(2,18,"Nicht implementiert        ");
            break;
            case 66://F8    
                PrintAt(2,18,"Ausgang            ");
                ExitProcess(0);
            break;
            case 48:
            case 49:
            case 50:
            case 51:
            case 52:
            case 53: 
            case 54:
            case 55:
            case 56:
            case 57:
                  str_select[str_index] = c;
                switch (select) {
                    case S_ARBEIT_START:
                        PrintAt(9+str_index,13,"%c",c);
                    break;
                    case S_ARBEIT_ENDE:
                        PrintAt(32+str_index,13,"%c",c);
                    break;
                    case S_PAUSE_START:
                        PrintAt(9+str_index,15,"%c",c);
                    break;       
                    case S_PAUSE_ENDE:
                        PrintAt(32+str_index,15,"%c",c);
                    break;
                }
                  SetTimeCursor();
            break;
        }
    }
}

