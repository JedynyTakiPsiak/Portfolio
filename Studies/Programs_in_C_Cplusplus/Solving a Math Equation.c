#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

double zamiana_przecinka_na_kropke(char *test){
	int wielkosc = strlen(test);
	int index = -1;

	for (int i = 0; i != wielkosc; i++) {
		if (test[i] == ',') {
			index = i;
			break;
		}
	}
	if (index != -1) {
		test[index] = '.';
	}
	return atof(test);
}
int wyrazenie (int n, char *tab) {
	char oper[] = {'+', '-', '*', '/', ')'};
	char znak[] = {'x', 'y', 'z', '('};
	char random;
	int i, wyrazenie_check = 0, nawiasy_check = 0, czas;
	if (n < 60){
		czas = n * 1000;
	} 
	else {
		czas = (n * 1000) / 1.2;
	}
	for (i = 0; i != n; i++){
		usleep(czas);
		if (nawiasy_check >= n-i){
			for (i = nawiasy_check; i != 0; i--){
				tab[n - i] = ')';
				nawiasy_check--;
			}
			break;
		}
		if (wyrazenie_check == 1 && (tab[i-1] != '+' || tab[i-1] != '-' || tab[i-1] != '*' || tab[i-1] != '/' || tab[i-1] != ')')){
			random = oper[rand() % sizeof(oper)];
			if (random == '+' || random == '-' || random == '*' || random == '/'){
				wyrazenie_check--;
			}
			if (random == ')'){
				if (nawiasy_check == 0){
					i--;
					continue;
				}
				nawiasy_check--;
			}
		}
		else{
			random = znak[rand() % sizeof(znak)];
			if (random == 'x' || random == 'y' || random == 'z'){
				wyrazenie_check++;
			}
			if (random == '('){
				if (nawiasy_check+1 >= n-i){
					i--;
					continue;
				}
				nawiasy_check++;
			}
		}
		
		tab[i] = random;
	}
	for (i = 0; i != n; i++){
		if (tab[i-1] == '(' && tab[i+1] == ')'){
			printf("# Błąd w składni! Ponowne generowanie wyrażenia!\n");
			wyrazenie(n, tab);
			break;
		}
	}
}
int priorytet_operator(char oper){
	if (oper == '*' || oper == '/')
		return 100;
	else if (oper == '+' || oper == '-')
		return 10;
	else
		return 0;
}
void formatowanie_onp (char* tab, char* onp_tab) {
	int i, j, top = -1;
	char stos[100];
	
	for(i = 0, j = 0; tab[i] != '\0'; i++){
		if (tab[i] == '+' || tab[i] == '-' || tab[i] == '*' || tab[i] == '/'){
			while ((top >= 0) && (priorytet_operator(stos[top]) >= priorytet_operator(tab[i]))) {
				onp_tab[j++] = stos[top--];
			}
			stos[++top] = tab[i];
		}
		else if (tab[i] == '(') {
			stos[++top] = tab[i];
		}
		else if (tab[i] == ')'){
			while ((top >= 0) && (stos[top] != '(')) { // sprawdzania dopuki nie napotka ( i wrzuca do onp_tab
				onp_tab[j++] = stos[top--];
			}
			if ((top >= 0) && (stos[top] == '(')) { // usuwa ( ze stosu
				top--;
			}
		}
		else {
			onp_tab[j++] = tab[i];
		}
	}
	while (top >= 0){
		onp_tab[j++] = stos[top--];
	}
	onp_tab[j] = '\0';
}
void operacja_na_pliku (int n, char *onp_tab, double x, double y, double z) {
	int i = 0;
	FILE *plik;
	plik = fopen("dane.txt", "w");
	while (onp_tab[i] != '\0') {
		if (onp_tab[i] == 'x') {
			fprintf(plik, "%.2f", x);
		} else if (onp_tab[i] == 'y') {
			fprintf(plik, "%.2f", y);
		} else if (onp_tab[i] == 'z') {
			fprintf(plik, "%.2f", z);
		} else {
			fprintf(plik, "%c", onp_tab[i]);
		}
		if (onp_tab[i+1] != '\0'){
			fprintf(plik, " ");
		}
		i++;
	}
	fclose(plik);
}
double obliczanie_onp() {
	FILE *plik;
	plik = fopen("dane.txt", "r");
	int i, top = -1;
	double stos[100];
	char liczba[7]; // #Przypominajka! 7 bo max wielkość liczby taka może być w pliku (mowa o ujemnej liczbie lub dodatniej + spacja za liczbą)

	while (fscanf(plik, "%s", liczba) != EOF) {
		if (isdigit(liczba[0]) || (liczba[0] == '-' && isdigit(liczba[1]))) {
			double poprawiona_liczba = atof(liczba);
			stos[++top] = poprawiona_liczba;
		}
		else if (liczba[0] == '+' || liczba[0] == '-' || liczba[0] == '*' || liczba[0] == '/') {
			double oper2 = stos[top--];
			double oper1 = stos[top--];
			double obliczenie;
			switch (liczba[0]) {
				case '+':
					obliczenie = oper1 + oper2;
					break;
				case '-':
					obliczenie = oper1 - oper2;
					break;
				case '*':
					obliczenie = oper1 * oper2;
					break;
				case '/':
					obliczenie = oper1 / oper2;
					break;
			}
			stos[++top] = obliczenie;
		}
	}
	fclose(plik);
	return stos[top];
}
int main() {
	remove("dane.txt");
	srand(time(NULL));
	int i, n = 0;
	double x, y, z, xyz[3];
	char test1, test2, test3[100], test4[] = {'x', 'y', 'z'};
	printf("\nStałe wartości wpisz => p | Własne wartości wpisz => w | Losowe wartości wpisz => l\nWybieram: ");
	scanf("%c", &test1);
	getchar();
	if (test1 == 'p'){
		n = 13;
		x = -21.37;
		y = 64;
		z = -13.37;
	}
	else if (test1 == 'l'){
		x = (rand() % 20001 - 10000) / 100.0;
		y = (rand() % 20001 - 10000) / 100.0;
		z = (rand() % 20001 - 10000) / 100.0;
		do{
			n = rand() % 100;
		} while (n < 3 || n > 99 || n % 2 == 0);
	}
	else if (test1 == 'w'){
		do{
			printf("Podaj liczbę nieparzystą (między 3 a 99): ");
			fgets(test3, 100, stdin);
			if (sscanf(test3, "%i", &n) != 1){
				n = 0;
			}
		} while (n < 3 || n > 99 || n % 2 == 0);
		for (i = 0; i != 3; i++){
			do{
				printf("Podaj liczbę dla %c (między -99.99 a 99.99): ", test4[i]);
				fgets(test3, 100, stdin);
				if (sscanf(test3, "%lf", &x) != 1) {
					xyz[i] = 101;
				}
				else {
					xyz[i] = zamiana_przecinka_na_kropke(test3);
				}
			} while (xyz[i] < -99.99 || xyz[i] > 99.99 || xyz[i] == 0);
		}
		x = xyz[0];
		y = xyz[1];
		z = xyz[2];
	}
	else {
		printf("Źle podany wybór opcji!\n");
		return 0;
	}

	char tab[n+1];
	tab[n] = '\0';
	printf("\nRozpoczęcie generowania wyrażenia matematycznego o długości: %i znaków\n", n);
	wyrazenie(n, tab);
	printf("\nWylosowane równianie matematyczne: %s\n", tab);
	printf("n=%i x=%.2lf y=%.2lf z=%.2lf\n", n, x, y, z);
	
	char onp_tab[100];

	formatowanie_onp(tab, onp_tab);
	printf("\nWyrażenie ONP: %s\n", onp_tab);
		
	operacja_na_pliku(n, onp_tab, x, y, z);
	
	double wynik = obliczanie_onp();
	printf("\nKońcowy wynik: %.2lf\n", wynik);
	
	printf("\nUsunąć plik dane.txt? Wpisz y/n: ");
	scanf("%c", &test2);
	if (test2 == 'n'){
		printf("Zachowano plik dane.txt\n");
	}
	else{
		printf("Pomyślnie usunięto plik dane.txt\n");
		remove("dane.txt");
	}
	printf("\n");
	return 0;
}