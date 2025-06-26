# Projekt – Symulacja Ruchów Ramienia Robota Posiadającego 2 Stopnie Swobody w Przestrzeni 2D

## 1. Zadanie projektowe
Celem naszego projektu było wykonanie modelu robota pracującego w dwóch trybach (manualny i automatyczny). Robotem w naszym projekcie nazywamy manipulator o 2 stopniach swobody.
Zaimplementowane zostały oba te tryby - przy czym tryb automatyczny został podzielony na odtwarzanie nagranego ruchu i automatyczynie wykonywane zadania.
Możliwe jest dostosowanie prędkości robota, a także dodawanie elementów (klocków). Robot pracuje w przestrzeni ograniczonej, jego zakres ruchu został ograniczony, aby nie wchodził w kolizję z samym sobą, albo podłożem.
Klocki nie podlegają żadnym prawom fizyki (nie zachodzi z nimi kolizja, nie działa na nie grawitacja). Za pomocą ramienia robota można przenosić, łapać i opuszczać klocki.
Robot wykonuje wszystkie automatyczne działania opisane w zadaniu.

## 2. Opis funkcjonalności

### 2.1 Tryby pracy robota

- **Tryb manualny**: Sterowanie ramieniem robota za pomocą klawiatury (strzałki).
- **Tryb odtwarzania**: Nagrywanie i odtwarzanie wcześniej nagranych ruchów robota.
- **Tryb automatyczny**: Automatyczne wykonywanie wybranego zadania.

### 2.2 Sterowanie
- Strzałki klawiatury: ruch ramienia; strzałki lewo/prawo poruszają pierwszym segmentem robota, a góra/dół - drugi segment.
- Przycisk „Record” – zapis ruchów do pamięci.
- Przycisk "Stop" - zatrzymaj nagrywanie.
- Przycisk „Play” – automatyczne wykonanie zapisanej sekwencji.
- Pole wyboru „Automation” – wybór zadania automatycznego.
- Przycisk "Start" - wykonaj zadanie automatyczne.
- Suwak "Speed" - ustaw prędkość robota (im większa, tym niższa precyzja!).
- Przycisk „Clear” – wyczyść wszystkie klocki na ekranie.
- Przycisk "Create" – dodawanie nowych klocków o wybranych parametrach (kształt, wysokość, waga, pozycja).
- Przycisk "Set" - ustawienie min i max wagi przenoszonych segmentów.

---

## 3. Model robota

- Robot porusza się w przestrzeni 2D ograniczonej długością jego segmentów.
- Ramię posiada **2 stopnie swobody** (x, y).
- Robot potrafi **chwytać i przenosić** obiekty.
- Zdefiniowano ograniczenia masy maksymalnej i minimalnej przenoszonego obiektu.
- Zaimplementowano płynność ruchów oraz regulację prędkości ramienia.
- Ograniczono ruch ramienia.

---

## 4. Zadania realizowane przez robota [nazwa zadania w polu automation]

### 1.1–1.4 Budowa wieży z 6 klocków [Tower *shape*]
- Kształty: kwadrat, koło, trójkąt, prostokąt.
- Wieże budowane przy użyciu trybu automatycznego.
- Wieże mogą być zbudowane z innej liczby bloków niż 6.
- Robot kończy pracę, jeśli klocek lub szczyt wieży znajduje siępoza jego zasięgiem.

### 1.5 Ograniczenie maksymalnej masy [Max Weight Move]
- Przenoszone są **3** klocki spełniające kryterium `masa ≤ udźwig`.
- Przy zbyt dużej wadze wyświetla się stosowny komunikat pod oknem Set.

### 1.6 Ograniczenie minimalnej i maksymalnej masy [Min&Max Weight]
- Przenoszone są **3** klocki mieszczące się w przedziale masowym `[min, max]`.
- Przy nieodpowiedniej wadze wyświetla się stosowny komunikat pod oknem Set.

### 1.7–1.8 Sortowanie wg wysokości [Height Min/Max to Max/Min], [Height Min/Max/Min]
- **1.7**: Od najwyższego do najniższego.
- **1.8**: Od najniższego do najwyższego.
- **1.8.2 W kolejności Min Max Min.
- Robot zakończy działanie jeśli skończą mu się klocki albo gdy znajdą się one poza jego zasięgiem.


### 1.9–1.10 Sortowanie wg masy [Weight Min/Max to Max/Min]
- **1.9**: Od najcięższego do najlżejszego.
- **1.10**: Od najlżejszego do najcięższego.
- Robot zakończy działanie jeśli skończą mu się klocki albo gdy znajdą się one poza jego zasięgiem.

---

## 5. Interfejs użytkownika

- **Przyciski**:
  - `Record`
  - `Play`
  - `Stop`
  - `Clear`
- **Pole wyboru**:
  - `Automation`: lista zadań (sort, build tower, mass sort itd.)
- **Pola tekstowe**:
  - Dodawanie klocków – wprowadzenie wysokości, pozycji, masy, kształtu.
  - Dodawanie limitów wagowych.
- **Wyświetlanie komunikatów**:
  - O przekroczeniu masy (tekstowy)
  - O nagrywaniu i odtwarzaniu ruchów (ikonki w lewym górnym rogu)

---

## 6. Przykład działania – Budowa wieży

### Dane wejściowe:
- 6 klocków o jednakowych wymiarach
- Pozycja startowa: Pierwszy znaleziony klocek
- Kształt: prostokąt

### Przebieg:
1. Użytkownik dodaje klocki.
2. Wybiera w menu Automation `Tower Rectangle`.
3. Zatwierdza przyciskiem Start
4. Robot podnosi każdy klocek w zasięgu i o kształcie protokąta, po czym ustawia jeden na drugim.

---

## 7. Przykład działania – Ograniczenie masy

### Dane wejściowe:
- 3 klocki o masach: `2kg`, `5kg`, `12kg`
- Maksymalny udźwig: `10kg`

### Oczekiwany wynik:
- Przeniesione zostaną tylko klocki o masie `2kg` i `5kg`.
- Użytkownik otrzyma komunikat o przekroczeniu dopuszczalnej masy przez klocek `12kg`.

---

## 8. Ustawienia prędkości ramienia

- Prędkość ustalana przez użytkownika z poziomu GUI.
- Wpływa na czas trwania ruchu pomiędzy punktami.
- Zmniejsza dokładność działania ramienia.

---

## 9. Podsumowanie

Symulacja spełnia wszystkie założenia projektowa, a także pozwala na wykonanie wszystkich zadań. Dokładność działania ramienia, a także kolizje z obiektami zostały uproszczone, ale spełniają wszystkie wymagania.
Projekt został wykonany jako klasyczna okienkowa aplikacja Windows (WinAPI) z wykorzystaniem biblioteki gdi+. Aplikacja została napisana w języku C++ 17.

---

## 10. Interfejs graficzny



## Wykonawcy Projektu
Szymon Lewicki 203253
Przemysław Olszewski 203217

Techniki Programowania 2025r. - 2 sem. ACIR PG
