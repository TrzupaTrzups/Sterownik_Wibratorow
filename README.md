# Sterownik Wibratora ATmega328P z Modbus RTU

Firmware dla ATmega328P do sterowania wibratorem (triakiem) za pomocą regulacji fazowej oraz komunikacji Modbus RTU.
Sterownik umożliwia ustawienie mocy (w promilach), odczyt prądu (z ADC) oraz włączanie/wyłączanie triaka poprzez Modbus.

## Funkcje

- **Modbus RTU (4800 baud, 8N1, bez parzystości)**
  - Obsługiwane funkcje:
    - `0x01` (Read Coils) — odczyt stanu włączenia triaka
    - `0x03` (Read Holding Registers) — odczyt mocy i prądu
    - `0x04` (Read Input Registers) — odczyt prądu
    - `0x05` (Write Single Coil) — włączanie/wyłączanie triaka
    - `0x06` (Write Single Register) — ustawianie mocy (0-1000 promili) oraz parametrów Modbus

- **Sterowanie triakiem (regulacja fazowa)**
  - Ustawianie mocy w promilach (0-1000)
  - Detekcja przejścia przez zero dla dokładnego załączania
  - Timer1 do opóźniania impulsu

- **Pomiar prądu**
  - ADC0 (PC0) odczytuje analogową wartość z przekładnika prądowego

- **Zdalna konfiguracja Modbus**
  - Możliwość zmiany adresu i prędkości (baudrate) Modbus przez rejestry
  - Przywracanie ustawień domyślnych poprzez zworkę na PD6 (zwarta do masy podczas resetu)

## Mapa rejestrów Modbus

| Adres | Typ | Opis | Dostęp |
|-------|-----|------|--------|
| 0x0001 | Coil | Włączenie triaka (0/1) | R/W |
| 0x0001 | Rejestr Holding | Ustawienie mocy (0-1000 promili) | R/W |
| 0x0002 | Rejestr Holding/Input | Prąd (surowa wartość ADC 0-1023) | R |
| 0x0003 | Rejestr Holding | Włączenie triaka (alternatywnie jako rejestr) | R/W |
| 0x0004 | Rejestr Holding | Adres Modbus (1-247) | R/W |
| 0x0005 | Rejestr Holding | Prędkość Modbus (1 = 4800, 2 = 9600, 3 = 19200) | R/W |

## Domyślna konfiguracja

- **F_CPU**: 16 MHz (zewnętrzny kwarc lub wewnętrzny zegar, jak w Arduino UNO)
- **Modbus baudrate (domyślny)**: 4800 baud (możliwa zmiana na 9600 lub 19200 poprzez rejestr 0x0005)

## Ustawienia fuse (zalecane dla zewnętrznego kwarca 16MHz jak Arduino UNO)

| Fuse | Wartość | Opis |
|------|--------|------|
| LOW  | 0xFF | Zewnętrzny kwarc (8+ MHz) |
| HIGH | 0xDE | SPI włączone, bootloader włączony (lub 0xD9 jeśli bez bootloadera) |
| EXT  | 0xFD | BOD na 2.7V |

## Użycie

Podłącz przez dowolny program Modbus (QModMaster, SCADA, Python pymodbus itp.)

- Aby włączyć/wyłączyć triak:
  - Funkcja `0x05` (Write Single Coil) adres `0x0001`: `0xFF00` (ON), `0x0000` (OFF)
- Aby ustawić moc:
  - Funkcja `0x06` (Write Single Register) adres `0x0001`: `0 - 1000`
- Aby odczytać prąd:
  - Funkcja `0x04` lub `0x03` adres `0x0002`
- Aby zmienić adres Modbus:
  - Funkcja `0x06` adres `0x0004`: wartość 1-247
- Aby zmienić prędkość Modbus:
  - Funkcja `0x06` adres `0x0005`: 1=4800, 2=9600, 3=19200 (wymaga restartu)

## Kompilacja

Kompilacja w Microchip Studio, avr-gcc lub PlatformIO. Upewnij się, że `F_CPU` i `USART_BAUDRATE` są poprawnie zdefiniowane w plikach `main.c` i `modbus.c`.

## Licencja

MIT License
