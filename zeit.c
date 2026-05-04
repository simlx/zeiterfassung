#include <stdio.h>
#include "windows.h"
#include <stdarg.h>  
#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define DARK_SHADE 178
HANDLE console;	
DWORD bytesWritten;

char ARBEIT_START[16] = "00:00:00";
char PAUSE_START[16]  = "00:00:00";
char PAUSE_ENDE[16]   = "00:00:00";
char ARBEIT_ENDE[16]  = "00:00:00";

enum TimeSelect {
    S_NONE,
    S_ARBEIT_START,
    S_PAUSE_START,
    S_PAUSE_ENDE,
    S_ARBEIT_ENDE
};

void strip_newline(char *s) {
    s[strcspn(s, "\n")] = '\0';
}

enum TimeSelect select = S_NONE;
int str_index = 0;				// HH MM SS = 6
char *str_select = ARBEIT_START;

void DrawLine(COORD startpos, COORD endpos) {  
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
	putchar(DARK_SHADE);
    }
}

/**
 * PrintAt with Background color
 */
void PrintBgAt(int x, int y, const char *fmt, ...) {
    SetConsoleTextAttribute(console, BACKGROUND_RED);
    SetConsoleCursorPosition(console,(COORD){x+1,y}); //+1 offset fix
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    SetConsoleTextAttribute(console, 
	    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	    );
}

void PrintAt(int x, int y, const char *fmt, ...) {
    SetConsoleCursorPosition(console,(COORD){x+1,y}); //+1 offset fix
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void ClearScreen() {
    system("cls");
}

void MoveTo(int x, int y) {
    SetConsoleCursorPosition(console,(COORD){x+1,y}); //+1 offset fix
}

void PrintTimes() {
    PrintAt(9,13,ARBEIT_START);
    PrintAt(23,13,"->");
    PrintAt(32,13,ARBEIT_ENDE);

    PrintAt(9,15,PAUSE_START);
    PrintAt(23,15,"->");
    PrintAt(32,15,PAUSE_ENDE);
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
	    PrintBgAt(7,13,">");
	    PrintBgAt(1,4,">");
	    break;
	case S_PAUSE_START:
	    PrintBgAt(7,15,">");
	    PrintBgAt(1,5,">");
	    break;    
	case S_PAUSE_ENDE:
	    PrintBgAt(30,15,">");
	    PrintBgAt(1,6,">");
	    break;
	case S_ARBEIT_ENDE:
	    PrintBgAt(30,13,">");
	    PrintBgAt(1,7,">");
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

void DrawCurrentChar(char c) {
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
}

void PrintGUI() {

    ClearScreen();
    int x = 55;
    int y = 20;

    DrawLine((COORD){0,1},(COORD){x,1});
    DrawLine((COORD){1,1},(COORD){1,y});
    DrawLine((COORD){1,y},(COORD){x,y});
    DrawLine((COORD){x,1},(COORD){x,y});

    PrintAt(16,1," Arbeitszeit Erfassung ");
    PrintAt(3, 4,"F1 - Arbeitszeit Anfang");   // 59
    PrintAt(3, 5,"F2 - Pause Anfang");         // 60
    PrintAt(3, 6,"F3 - Pause Ende");           // 61
    PrintAt(3, 7,"F4 - Arbeitszeit Ende");     // 62
    PrintAt(30,4,"F5 - Speichern");	       // 63
    PrintAt(30,5,"F6 - Netzwerk Upload");      // 64
    PrintAt(30,9,"A  - Aktuelle Uhrzeit");
    PrintAt(3,9,"R  - Zeiten zuruecksetzen");
    PrintAt(30,7,"F8 - Ausgang");              // 65

    // Print Time Template 
    PrintAt(19,12,"Arbeitszeit");
    PrintAt(21,14,"Pause");

    PrintAt(8,13," 00:00:00 ");
    PrintAt(23,13,"->");
    PrintAt(31,13," 00:00:00 ");

    PrintAt(8,15," 00:00:00 ");
    PrintAt(23,15,"->");
    PrintAt(31,15," 00:00:00 ");

    // Print Time
    PrintTimes();
    SetTimeCursor();
}

/**
 * Exceldatei zum Netzwerk hochladen
 */
void NetworkUpload() {

    ClearScreen();

    char networkpath[512];
    FILE *file = fopen("networkpath.txt", "r");

    if (file == NULL) {
	PrintAt(2, 18, "DATEI KONNTE NICHT GEÖFFNET WERDEN.");
	return;
    }

    if (fgets(networkpath, sizeof(networkpath), file))
	strip_newline(networkpath);

    fclose(file);

    char cmdstring[1024];
    snprintf(cmdstring, sizeof(cmdstring),
	    "copy Arbeitszeiterfassung.xlsx %s &&pause", networkpath);

    system(cmdstring);

    PrintGUI();
}

/**
 * Arbeitszeiten zu Datei speichern und exportieren 
 */
void Save() {

    ClearScreen();

    FILE *file = fopen("arbeitszeit.txt","w");
    if (file == NULL) {
	PrintAt(2,18,"DATEI KONNTE NICHT GE FFNET WERDEN.");  
	return;
    }
    fprintf(file, "%s\n", ARBEIT_START);
    fprintf(file, "%s\n", PAUSE_START);
    fprintf(file, "%s\n", PAUSE_ENDE);
    fprintf(file, "%s\n", ARBEIT_ENDE);

    fclose(file);

    FILE *file2 = fopen("zeiten.csv","w");
    if (file2 == NULL) {
	PrintAt(2,18,"CSV KONNTE NICHT GE FFNET WERDEN.");  
	return;
    }
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char datum[20];
    strftime(datum, sizeof(datum), "%Y-%m-%d", t);

    fprintf(file2, "1,%s,%s,%s,%s,%s,-1\n",
	    datum, ARBEIT_START, PAUSE_START, PAUSE_ENDE, ARBEIT_ENDE);
    fclose(file2);

    ClearScreen();
    MoveTo(2,2);

    int result = system(
	    "timeservice.exe Arbeitszeiterfassung.xlsx zeiten.csv");

    PrintGUI();
}

/**
 * Arbeitszeiten aus der Datei laden
 */
void LoadFromFile() {
    FILE *file = fopen("arbeitszeit.txt","r");
    if (file == NULL) {
	PrintAt(2,18,"DATEI KONNTE NICHT GE FFNET WERDEN.");  
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

int main() {
    // Arbeitszeit laden
    LoadFromFile();

    console = GetStdHandle(STD_OUTPUT_HANDLE);

    PrintGUI();

    int c;
    while (1) {
	c = _getch();
	//	 printf("%i",c);
	switch (c) {
	    case 114: // R
		strcpy(ARBEIT_START,"00:00:00");
		strcpy(ARBEIT_ENDE, "00:00:00");
		strcpy(PAUSE_START, "00:00:00");
		strcpy(PAUSE_ENDE, "00:00:00");
		PrintTimes();
		PrintAt(2,18,"Zeit auf 00:00:00 gesetzt.");
		break;
	    case 97: { // A
			 time_t t = time(NULL);
			 struct tm *tm_info = localtime(&t);
			 char buffer[9];
			 strftime(buffer, sizeof(buffer),
				 "%H:%M:%S",tm_info);
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
	    case 80: // DOWN ARROW
		     {
			 c = str_select[str_index];
			 int x = (c - '0')-1;
			 if (x < 0) x = 9;
			 str_select[str_index] = x + '0';
			 c = x + '0';
			 DrawCurrentChar(c);
		     }
		     break;
	    case 72: // UP ARROW
		     {
			 c = str_select[str_index];
			 int x = (c - '0')+1;
			 if (x > 9) x = 0;
			 str_select[str_index] = x + '0';
			 c = x + '0';
			 DrawCurrentChar(c);
		     }
		     break;
	    case 13:// Enter

		     break;
	    case 59:// F1
		     select = S_ARBEIT_START;
		     str_select = ARBEIT_START;
		     str_index = 0;
		     SetTimeCursor();
		     break;
	    case 60:// F2
		     select = S_PAUSE_START;
		     str_select = PAUSE_START;
		     str_index = 0;
		     SetTimeCursor();
		     break;
	    case 61:// F3    
		     select = S_PAUSE_ENDE;
		     str_select = PAUSE_ENDE;
		     str_index = 0;
		     SetTimeCursor();
		     break;
	    case 62:// F4    
		     select = S_ARBEIT_ENDE;
		     str_select = ARBEIT_ENDE;
		     str_index = 0;
		     SetTimeCursor();
		     break;
	    case 63:// F5 
		    // 1. 
		    // 2. Run Time Registration Service   
		     Save();
		     break;
	    case 64:// F6    
		     PrintAt(2,22,"Kopiere Datei zum Netzwerk...");
		     NetworkUpload();
		     break;
	    case 65:// F7    
		     PrintAt(2,22,"Nicht implementiert        ");
		     break;
	    case 66:// F8    
		     PrintAt(2,22,"Ausgang            ");
		     ExitProcess(0);
		     break;
	    case 48: // 0
	    case 49: // 1
	    case 50: // 2
	    case 51: // 3
	    case 52: // 4
	    case 53: // 5
	    case 54: // 6
	    case 55: // 7
	    case 56: // 8
	    case 57: // 9
		     str_select[str_index] = c;
		     DrawCurrentChar(c);
		     break;
	}
    }
}

