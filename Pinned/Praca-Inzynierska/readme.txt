0. Plik 'readme.txt' zawiera opis pomocy w pierwszych krokach uruchomienia systemu projektu inżynierskiego.

1. Krótki opis projektu: 
Program służy do pobierania danych z API, ich zapis, analizę oraz wygenerowanie raportu HTML.

2. Wymagania:
- system operacyjny Windows (najlepiej wersja 11)
- program IDE Visual Studio 2026 z obsługą projektów C++
- zainstalowane biblioteki zewnętrzne: cURL oraz nlohmann/json
- dostępne i stabilne połączenie z internetem
- poprawny klucz API od administratora, uzupełniony w pliku konfiguracyjnym 'config.ini'


3. Klucz API
Program wymaga wprowadzenia prawidłowego klucz API do pliku 'config.ini'.
W następujący format:
API_KEY=<KEY_VALUE>

W przypadku nieposiadania klucza API, należy napisać mail
do: openapi@totalizator.pl
zawierajacy: imię i nazwisko, nr telefonu i mail.
Przybliżony czas oczekiwania na klucz to 1 dzień roboczy.


4. Instalacja zewnętrznych bibliotek
Opracowane poniżej komendy były testowane dla czystego systemu operacyjnego Windows 11 na wierszu poleceń.
Opracowanie komend jest własne na zasadzie prób i błędów ze stron Microsoft Learn:
https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell

Tworzymy folder roboczy dla bibliotek w lokalizacji: 'C:\dev\vcpkg'
Aby wykonać instalacji trzeba uruchomić wiersz poleceń i wpisac poniższe komendy:

mkdir C:\dev
cd /d C:\dev

powershell -Command "Invoke-WebRequest -Uri 'https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip' -OutFile 'C:\dev\vcpkg.zip'"

powershell -Command "Expand-Archive -Path 'C:\dev\vcpkg.zip' -DestinationPath 'C:\dev' -Force"

if exist C:\dev\vcpkg-master ren C:\dev\vcpkg-master vcpkg
if exist C:\dev\vcpkg-main ren C:\dev\vcpkg-main vcpkg

cd /d C:\dev\vcpkg
bootstrap-vcpkg.bat

set "VCPKG_ROOT=C:\dev\vcpkg"
set "PATH=%VCPKG_ROOT%;%PATH%"

vcpkg install curl:x64-windows
vcpkg install nlohmann-json:x64-windows

vcpkg integrate install


5. Zalecany sposób uruchomienia, w celu prostego i bezproblemowego uruchomienia projektu
- otworzyć plik rozwiązania: 'inzynierka.slnx'
- uruchomić projekt w Visual Studio
- skompilowac projekt w wersji najlepiej 'Debug 64x'