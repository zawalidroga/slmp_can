# Kontekst rozmowy z Gemini

Ten plik służy jako notatnik do śledzenia kontekstu i kluczowych punktów naszych rozmów.

## Zasady współpracy

Uczę się programować, więc nie zmieniaj kodu bez mojej wyraźnej prośby. Twoja rola polega na pomocy, podpowiadaniu i wsparciu przy architekturze.

## 2025-12-08

*   Zainicjowano pomysł prowadzenia tego pliku (`gemini.md`) w celu utrzymania ciągłości rozmów na temat projektu.

---

## Sesja z 2025-12-12

### Co zrobiliśmy:

1.  **Ustalenie zasad współpracy:** Potwierdziliśmy, że moja rola to bycie asystentem i mentorem, a nie bezpośrednie modyfikowanie kodu bez Twojej prośby. Zostało to zapisane w tym pliku.
2.  **Analiza struktury projektu:** Zapoznałem się ze strukturą plików projektu i dodałem ją do tego dokumentu w celu łatwiejszego odniesienia.
3.  **Implementacja flag statusu (`ServoStatusFlags`):**
    *   Wyjaśniliśmy, jak efektywnie zarządzać wieloma stanami (flagami) za pomocą jednego integera i operacji bitowych (`|`, `&`, `<<`, `~`).
    *   Zaprojektowaliśmy, jak zintegrować flagi statusu z klasą `ServoDevice`, włączając w to dodanie prywatnego pola `_status` oraz publicznych metod `setStatus`, `clearStatus` i `isStatusSet`.
4.  **Diagnoza błędu `signed`/`unsigned`:**
    *   Zidentyfikowaliśmy przyczynę błędu, przez który ujemne wartości prądu (`current`) z ramki CAN były błędnie interpretowane jako duże wartości dodatnie.
    *   Problem leży w funkcji `parseCanRxFrame` (`ServoControl.cpp`) oraz w sygnaturze funkcji `setServoMonitor` (`ServoDevice.h`), która wymuszała traktowanie wartości jako `unsigned`.

### Plan na następną sesję (jutro):

1.  **Naprawa błędu `signed`/`unsigned`:**
    *   **Krok 1:** Zmiana sygnatury metody `setServoMonitor` w `ServoDevice.h` z `void setServoMonitor(RealParameter parName, uint32_t value);` na `void setServoMonitor(RealParameter parName, int16_t value);`.
    *   **Krok 2:** Poprawa logiki w `parseCanRxFrame` w `ServoControl.cpp`, aby poprawnie składać bajty z ramki CAN i rzutować je na `int16_t` przed przekazaniem do `setServoMonitor`.
2.  **Dokończenie implementacji `ServoStatusFlags`:** Wprowadzenie zmian w kodzie, które omówiliśmy dzisiaj.
3.  **Kontynuacja głównych zadań (TODO):**
    *   Stworzyć i zintegrować klasę `SettingsManager` do zarządzania globalną konfiguracją.
    *   Rozwinąć protokół `PMP`.

---

## Podsumowanie sesji debugowania

W tej sesji zdiagnozowaliśmy i naprawiliśmy szereg błędów, które uniemożliwiały działanie aplikacji.

### Główne problemy i rozwiązania:

1.  **Problem z Watchdogiem (Ciągłe restarty):**
    *   **Rozwiązanie:** Dodano `vTaskDelay` do pętli w `_canSendTask`, aby zapobiec blokowaniu procesora.
... (reszta pliku bez zmian)
---
## Struktura plików projektu:

```
.
├── gemini.md
├── include
│   └── README
├── lib
│   └── README
├── platformio.ini
├── src
│   ├── app
│   │   ├── app.cpp
│   │   └── app.h
│   ├── can
│   │   ├── CanManager.cpp
│   │   ├── CanManager.h
│   │   ├── ServoControl.cpp
│   │   ├── ServoControl.h
│   │   ├── ServoDevice.cpp
│   │   └── ServoDevice.h
│   ├── gui
│   │   ├── TUIManager.cpp
│   │   └── TUIManager.h
│   ├── main.cpp
│   └── network
│       ├── AsyncTCPServer.cpp
│       ├── AsyncTCPServer.h
│       ├── AsyncUDPManager.cpp
│       ├── AsyncUDPManager.h
│       ├── CommandParser.cpp
│       ├── CommandParser.h
│       ├── NetworkManager.cpp
│       ├── NetworkManager.h
│       ├── PMPmanager.cpp
│       └── PMPmanager.h
└── test
    └── README
```

---

## Lista zadań (TODO) - Stara

*   ~~Zaimplementować sumę kontrolną (CRC) w protokole PMP dla większej niezawodności.~~ (Wykonane)
*   ~~Aktywne skanowanie serw na magistrali CAN.~~ (Pominięte na rzecz sprawdzania online)
*   ~~Stworzenie klasy `SettingsManager`~~ (Istnieje, do ewentualnej rozbudowy)
*   ~~Dalszy rozwój protokołu PMP i TUI~~ (Zastąpione nowym planem GUI)
*   ~~Sprawdzanie połączenia z serwami~~ (Wykonane)
*   ~~Analiza i integracja OTA~~ (Do zrobienia w przyszłości)

---

## Nowy plan i lista zadań (TODO) - Styczeń 2026

Naszym nowym celem jest stworzenie aplikacji desktopowej/terminalowej (`ServoSetter GUI`) do parametryzacji serw, która będzie komunikować się z ESP32 przez TCP z użyciem komunikatów JSON.

### Zadania po stronie ESP32

1.  **Integracja biblioteki `ArduinoJson`**: Dodać bibliotekę do `platformio.ini`.
2.  **Rozbudowa `CommandParser`**:
    *   Dodać logikę w `dataParser`, która rozpoznaje, czy przychodzący string to JSON (np. przez sprawdzenie, czy zaczyna się od `{`).
    *   Jeśli to JSON, przekazać go do nowej funkcji, np. `parseJsonCommand()`.
    *   Jeśli nie, przekazać go do `TUIManager` tak jak dotychczas.
3.  **Implementacja `parseJsonCommand`**: Stworzyć logikę, która parsuje JSON i na podstawie pola `command` wywołuje odpowiednie akcje w `ServoControl` (np. `setParameters`, `getServo`).
4.  **Implementacja odpowiedzi JSON**: Stworzyć funkcje, które budują odpowiedzi w formacie JSON (np. status `ok`/`error` lub pełne dane serwa) i odsyłają je do klienta TCP.
5.  **Implementacja mechanizmu monitoringu**: Dodać logikę do obsługi komend `start_monitoring` i `stop_monitoring`, która będzie cyklicznie wysyłać dane o stanie serw do klienta, który o to poprosił.

### Zadania po stronie PC (Aplikacja `ServoSetter GUI` w Pythonie)

1.  **Etap 1: Rdzeń komunikacyjny (Python)**
    *   Stworzyć klasę/moduł do obsługi połączenia TCP (nawiązywanie, zrywanie, wysyłanie, odbieranie danych).
    *   Stworzyć funkcje do budowania stringów JSON dla poszczególnych komend (`set_speed`, `go_to_pos` itd.).
    *   Implementacja wątku sieciowego do asynchronicznego odbierania danych.
2.  **Etap 2: Struktura interfejsu (Python TUI)**
    *   Zainicjować projekt z biblioteką `curses` (lub `textual`).
    *   Stworzyć główny layout aplikacji (np. okno menu, okno statusu, okno główne).
    *   Implementacja nawigacji po górnym menu.
3.  **Etap 3: Integracja i logika UI (Python)**
    *   Połączyć warstwę komunikacji z interfejsem: dane odebrane z ESP32 powinny być wyświetlane w odpowiednich oknach.
    *   Zaimplementować obsługę akcji użytkownika (np. edycja pola z wartością parametru, kliknięcie przycisku "Wyślij").
    *   Zbudować widoki dla parametryzacji i monitoringu.

---
## Sesja z 2025-12-15

### Co zrobiliśmy:

1.  **Implementacja sumy kontrolnej (CRC):**
    *   Zaprojektowaliśmy i wdrożyliśmy mechanizm sumy kontrolnej CRC16 dla protokołu PMP, aby zapewnić integralność przesyłanych danych.
    *   Poprawiliśmy kilka krytycznych błędów związanych z dostępem do pamięci (`StoreProhibited`), które pojawiały się podczas implementacji.

2.  **Ujednolicenie odpowiedzi PMP:**
    *   Zdecydowaliśmy, że odpowiedź na żądanie odczytu statusu (`PMP_SUBCMD_READ_STATUS`) będzie zawsze zawierać dane o wszystkich serwach, co upraszcza protokół.

3.  **Implementacja logiki statusów serw:**
    *   Zaprojektowaliśmy i wdrożyliśmy logikę dla flag statusu, w tym `IN_POSITION`.
    *   Rozróżniliśmy status "fizycznie zajęty" (`BUSY`, oparty na prędkości) od "logicznie zajęty" (`BUSY_POSITIONING`, oparty na stanie polecenia).

4.  **Stworzenie monitora diagnostycznego UDP:**
    *   Zaimplementowaliśmy mechanizm logowania pakietów UDP (przychodzących i wychodzących) i ich wyświetlania w TUI, co ułatwia diagnostykę.

### Plan na następne sesje (nasze TODO):

1.  **Aktywne skanowanie serw na magistrali CAN.**
2.  **Stworzenie klasy `SettingsManager`** do centralnego zarządzania konfiguracją.
3.  **Dalszy rozwój protokołu PMP i TUI** (dodanie brakujących komend i ekranów).
4.  **Sprawdzanie połączenia z serwami:** Implementacja mechanizmu, który weryfikuje, czy serwo odpowiada. Jeśli serwo jest offline, należy wstrzymać wysyłanie do niego ramek i oznaczyć jego status jako "offline".
5.  **Analiza wyników testu `HTTPUpdate` (OTA):** Sprawdzenie, czy aktualizacja przez HTTP powiodła się.
6.  **Integracja docelowego mechanizmu OTA:** Jeśli test `HTTPUpdate` się powiedzie, zintegrowanie go na stałe z aplikacją.



---

## Sesja z 2025-12-15 (Diagnostyka OTA)



### Co zrobiliśmy:



Dzisiejsza sesja była w całości poświęcona próbie implementacji i debugowaniu aktualizacji Over-The-Air (OTA). Proces był złożony i iteracyjny:



1.  **Pierwsza próba (`ArduinoOTA`):** Zaimplementowaliśmy standardową bibliotekę `ArduinoOTA`. Problem polegał na tym, że urządzenie nie było wykrywane w sieci.

2.  **Debugowanie połączenia sieciowego:** Zdiagnozowaliśmy, że usługa OTA startowała, zanim interfejs Ethernet był gotowy. Przeszliśmy przez kilka iteracji, próbując naprawić ten problem:

    *   Najpierw przez system zdarzeń (`ethEvent`), który okazał się nie działać w tej konfiguracji.

    *   Następnie przez aktywne sprawdzanie statusu linku (`ETH.linkUp()`), co **zakończyło się sukcesem** – urządzenie zaczęło poprawnie łączyć się z siecią.

3.  **Debugowanie `ArduinoOTA`:** Mimo działającej sieci, aktualizacja nadal się nie udawała z powodu błędu "No response from the ESP".

    *   Naprawiliśmy problem z brakiem hasła autoryzacyjnego w `platformio.ini`.

    *   Mimo to, komunikacja OTA oparta na UDP wciąż zawodziła, nawet po wykluczeniu problemów z firewallem (potwierdzone działającym połączeniem TCP przez `ncat`).

4.  **Zmiana strategii na `HTTPUpdate`:** Z powodu uporczywych problemów z `ArduinoOTA`, zdecydowaliśmy się na zmianę podejścia. Zaprojektowaliśmy test z użyciem aktualizacji przez HTTP, gdzie to ESP32 (klient) pobiera plik `firmware.bin` z serwera HTTP uruchomionego na komputerze PC.
