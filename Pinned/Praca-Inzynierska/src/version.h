#pragma once
#include <string>

extern const std::string infoSystemVersion = "0.8.3";

/*
	Inforamcja o wersjach:
	0.6.0 - przeprojektowanie układu i wyglądu strony WWW
	0.6.1 - poprawione wyświetlanie w konsoli oraz na stronie WWW informacji o wersji, bądź jak zakończyła się dana operacja
	0.7.0 - dodane sortowanie wszystkich tabel po wartośći 'count' (liczebność) w celu ułatwienia czytania tabel
	0.7.1 - zmiana precyzji formatowania liczb miejsc po przecinku z 2 na 6 w wartości: 'expected' (wartość oczekiwana)
	0.8.0 - dodanie analizy testu chi-kwadrat i Kołmogorowa-Smirnowa
	0.8.1 - poprawki kodu: błąd krytyczny programu przy pobieraniu wartości z tabeli przy teście K-S
	0.8.2 - poprawki wizualne raportu i interfejsu konsolowego przy operacji pobierania danych
	0.8.3 - dodanie sprawdzenia czy klucz api występuje, jeśli nie, jest wysyłana informacja aby wpisać,
	w przypadku braku klucza, jak uzyskac klucz

*/