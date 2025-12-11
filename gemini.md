# Kontekst rozmowy z Gemini

Ten plik służy jako notatnik do śledzenia kontekstu i kluczowych punktów naszych rozmów.

## 2025-12-08

*   Zainicjowano pomysł prowadzenia tego pliku (`gemini.md`) w celu utrzymania ciągłości rozmów na temat projektu.

---

## Podsumowanie sesji debugowania

W tej sesji zdiagnozowaliśmy i naprawiliśmy szereg błędów, które uniemożliwiały działanie aplikacji.

### Główne problemy i rozwiązania:

1.  **Problem z Watchdogiem (Ciągłe restarty):**
    *   **Rozwiązanie:** Dodano `vTaskDelay` do pętli w `_canSendTask`, aby zapobiec blokowaniu procesora.

2.  **Problem z siecią (Brak pingu):**
    *   **Rozwiązanie:** Poprawiono kolejność `ETH.begin()` i `ETH.config()` oraz przeniesiono start serwerów TCP/UDP z eventu do głównej funkcji `begin()`.

3.  **Problem z połączeniem TCP (`Connection refused`):**
    *   **Rozwiązanie:** Poprawiono tworzenie kopii managerów sieci w `main.cpp` na użycie referencji (`&`).

4.  **Problem z crashem po połączeniu TCP (`LoadProhibited`):**
    *   **Rozwiązanie:** Ustawiono wskaźnik `_activeClient` w `TUIManager` na początku funkcji `onNewClientConnect`.

5.  **Problem z echem zwrotnym komend:**
    *   **Rozwiązanie:** Zakomentowano testową linię `c->write(...)` w `AsyncTCPServer.cpp`.

6.  **Problem z uszkodzeniem pamięci (crashe, błędy wyświetlania):**
    *   **Przyczyna:** Użycie obiektów `String` i konkatenacji (`+`) w pętlach, co prowadziło do fragmentacji pamięci.
    *   **Rozwiązanie:** Zrefaktoryzowano kod, aby unikać tworzenia obiektów `String` w pętlach.

7.  **Problemy z zapisem/odczytem z pamięci `Preferences`:**
    *   **Błąd `NOT_INITIALIZED`:** Wywoływanie `Preferences` z konstruktora globalnego obiektu. Rozwiązane przez przeniesienie logiki do metody `begin()` wywoływanej z `setup()`.
    *   **Błąd `NOT_FOUND`:** Próba otwarcia nieistniejącej przestrzeni nazw/klucza w trybie "tylko do odczytu". Rozwiązane przez zmianę trybu otwarcia na "zapis/odczyt" (`false`) lub użycie `isKey()` do sprawdzenia istnienia klucza.

### Aktualny status:

Aplikacja jest w pełni stabilna. Wszystkie kluczowe moduły (sieć, TUI, CAN, zapis/odczyt konfiguracji serw) działają poprawnie.

---

### Ustalenia architektoniczne i wyjaśnienia:

*   **Uniwersalny mechanizm zapisu (Wzorzec Obserwator):**
    *   Ustaliliśmy, że najlepszym sposobem na "uniwersalny" zapis jest dodanie callbacka `onStateChanged` do `ServoDevice`. Dzięki temu obiekt serwa sam "informuje" o zmianie swojego stanu, co automatycznie uruchamia procedurę zapisu, niezależnie od tego, która część programu dokonała modyfikacji.

*   **Zarządzanie konfiguracją urządzenia:**
    *   Ustaliliśmy, że do zarządzania globalnymi ustawieniami (IP, porty) najlepiej będzie stworzyć dedykowaną klasę `SettingsManager` działającą jako Singleton.

*   **Pytania dot. C++:**
    *   **Wskaźniki:** Wyjaśniliśmy różnicę między deklaracją wskaźnika (`TYP *nazwa`) a zmiennej (`TYP nazwa`), a także działanie operatorów `&` (pobranie adresu) i `*` (dereferencja - pobranie wartości spod adresu). Przeanalizowaliśmy różnicę między arytmetyką wskaźników (`pointer + 1`) a arytmetyką wartości (`*pointer + 1`).
    *   **Endianness:** Wyjaśniliśmy, dlaczego istnieje Little-Endian (ułatwienia dla procesora w operacjach arytmetycznych) i Big-Endian (standard sieciowy, bardziej czytelny dla człowieka).
    *   **Błąd `jump to case label`:** Wyjaśniliśmy, że błąd ten wynika z deklaracji zmiennej wewnątrz `switch`, ale poza blokiem `{}` konkretnego `case`.
    *   **Iteracja po `enum`:** Wyjaśniliśmy, że nie można iterować bezpośrednio i pokazaliśmy wzorzec z dodaniem elementu `_COUNT` oraz konieczność użycia `static_cast` dla `enum class`.

### Zadania do wykonania (TODO):

*   Stworzyć i zintegrować klasę `SettingsManager` do zarządzania globalną konfiguracją urządzenia.
*   Rozwinąć protokół `PMP` w oparciu o zdobyte doświadczenia.
