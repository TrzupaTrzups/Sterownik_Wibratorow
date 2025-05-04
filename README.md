# Sterownik Wibratora ATmega328P z Modbus RTU

Firmware dla ATmega328P do sterowania wibratorem (triakiem) za pomocą regulacji fazowej oraz komunikacji Modbus RTU.
Sterownik umożliwia ustawienie mocy (w promilach), odczyt prądu (z ADC) oraz włączanie/wyłączanie triaka poprzez Modbus.

## Funkcje

* **Modbus RTU (4800 baud, 8N1, bez parzystości)**

  * Obsługiwane funkcje:

    * `0x01` (Read Coils) — odczyt stanu włączenia triaka
    * `0x03` (Read Holding Registers) — odczyt mocy i prądu
    * `0x04` (Read Input Registers) — odczyt prądu
    * `0x05` (Write Single Coil) — włączanie/wyłączanie triaka
    * `0x06` (Write Single Register) — ustawianie mocy (0-1000 promili)

* **Sterowanie triakiem (regulacja fazowa)**

  * Ustawianie mocy w promilach (0-1000)
  * Detekcja przejścia przez zero dla dokładnego załączania
  * Timer1 do opóźniania impulsu

* **Pomiar prądu**

  * ADC0 (PC0) odczytuje analogową wartość z przekładnika prądowego

## Mapa rejestrów Modbus

| Adres  | Typ                   | Opis                                          | Dostęp |
| ------ | --------------------- | --------------------------------------------- | ------ |
| 0x0001 | Coil                  | Włączenie triaka (0/1)                        | R/W    |
| 0x0001 | Rejestr Holding       | Ustawienie mocy (0-1000 promili)              | R/W    |
| 0x0002 | Rejestr Holding/Input | Prąd (surowa wartość ADC 0-1023)              | R      |
| 0x0003 | Rejestr Holding       | Włączenie triaka (alternatywnie jako rejestr) | R/W    |

## Domyślna konfiguracja

* **F\_CPU**: 1 MHz (wewnętrzny RC lub zewnętrzny kwarc z odpowiednimi fuse bitami)
* **Modbus baudrate**: 4800 baud
* **Adres** :1

## Ustawienia fuse (zalecane dla zewnętrznego kwarca 16MHz jak Arduino UNO)

| Fuse | Wartość | Opis                                                               |
| ---- | ------- | ------------------------------------------------------------------ |
| LOW  | 0xFF    | Zewnętrzny kwarc (8+ MHz)                                          |
| HIGH | 0xDE    | SPI włączone, bootloader włączony (lub 0xD9 jeśli bez bootloadera) |
| EXT  | 0xFD    | BOD na 2.7V                                                        |

## Użycie

Podłącz przez dowolny program Modbus (QModbus pymodbus itp.)

* Aby włączyć/wyłączyć triak:

  * Funkcja `0x05` (Write Single Coil) adres `0x0001`: `0xFF00` (ON), `0x0000` (OFF)
* Aby ustawić moc:

  * Funkcja `0x06` (Write Single Register) adres `0x0001`: `0 - 1000`
* Aby odczytać prąd:

  * Funkcja `0x04` lub `0x03` adres `0x0002`

## Kompilacja

Kompilacja w Microchip Studio. Upewnij się, że `F_CPU` i `USART_BAUDRATE` są poprawnie zdefiniowane w plikach `main.c` i `modbus.c`.

