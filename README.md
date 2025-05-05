# Sterownik Wibratora ATmega328P z Modbus RTU i Triakiem

Firmware dla ATmega328P do sterowania mocą wibratora poprzez Modbus RTU, z obsługą triaka (sterowanie fazowe) oraz funkcją zapamiętywania ustawień.

## Funkcje

* Sterowanie mocą (0-1000 promili) poprzez Modbus (rejestr 0x0001)
* Włączanie i wyłączanie triaka (Coil 0x0001)
* Odczyt wartości prądu (ADC) przez Modbus (rejestr 0x0002)
* Zmiana adresu Modbus oraz prędkości transmisji (rejestry 0x0004 i 0x0005)
* Przywracanie ustawień fabrycznych za pomocą zworki (PD6 do GND podczas resetu)
* Zabezpieczenie przed podwójnym załączeniem triaka (debounce + pulse scheduled)

---

## Funkcje Modbus

| Funkcja                | Kod  | Adres  | Opis                                              |
| ---------------------- | ---- | ------ | ------------------------------------------------- |
| Read Coils             | 0x01 | 0x0001 | Odczyt stanu triaka (0 = wyłączony, 1 = włączony) |
| Read Holding Registers | 0x03 | 0x0001 | Odczyt mocy (0–1000 promili)                      |
|                        |      | 0x0002 | Odczyt wartości prądu (ADC)                       |
|                        |      | 0x0004 | Odczyt aktualnego adresu Modbus                   |
|                        |      | 0x0005 | Odczyt aktualnej prędkości Modbus                 |
| Read Input Registers   | 0x04 | 0x0002 | Odczyt wartości prądu (ADC)                       |
| Write Single Coil      | 0x05 | 0x0001 | Włączenie/wyłączenie triaka (0xFF00 lub 0x0000)   |
| Write Single Register  | 0x06 | 0x0001 | Ustawienie mocy (0–1000 promili)                  |
|                        |      | 0x0004 | Ustawienie adresu Modbus                          |
|                        |      | 0x0005 | Ustawienie prędkości Modbus                       |

---

## Prędkości Modbus

| Kod | Prędkość  |
| --- | --------- |
| 1   | 4800 bps  |
| 2   | 9600 bps  |
| 3   | 19200 bps |

Zmiana prędkości wymaga restartu mikrokontrolera.

---

## Obsługa TRIAC (T835-600B-TR)

* Triak załączany tylko w dodatniej półfali
* Detekcja przejścia przez zero z zabezpieczeniem przed zakłóceniami (debounce)
* Każdy impuls triaka trwa 100us
* Używany tryb sterowania fazowego (opóźnienie po przejściu przez zero zależne od ustawionej mocy)

---

## Wejścia/Wyjścia

| Pin    | Funkcja                                               |
| ------ | ----------------------------------------------------- |
| PORTB0 | Sterowanie triakiem (impuls)                          |
| PORTB1 | Debug LED (sygnalizacja impulsu triaka)               |
| PORTD2 | RS485 DE (kierunek Modbus)                            |
| PORTC2 | Zero crossing (detekcja przejścia przez zero)         |
| PORTD6 | Przywracanie ustawień domyślnych (GND podczas resetu) |

---

## Przywracanie ustawień fabrycznych

Aby przywrócić ustawienia domyślne:

1. Zewrzyj PD6 do masy.
2. Włącz zasilanie.
3. Po około 10 ms ustawienia zostaną zresetowane do domyślnych (adres = 1, prędkość = 4800 bps).

---

## Domyślne ustawienia

| Parametr        | Wartość domyślna      |
| --------------- | --------------------- |
| Adres Modbus    | 1                     |
| Prędkość Modbus | 4800 bps              |
| Triac           | Wyłączony (coils = 0) |
| Moc             | 0 promili             |

---

## Uwagi

* Projekt zoptymalizowany dla sieci 50 Hz (dodatnia półfala co 20ms)
* Zabezpieczenie przed podwójnym załączaniem triaka (pulse scheduled)
* Debounce eliminujący zakłócenia przy detekcji przejścia przez zero
* Działa w trybie Modbus RTU (8N1)

## Licencja

MIT License
