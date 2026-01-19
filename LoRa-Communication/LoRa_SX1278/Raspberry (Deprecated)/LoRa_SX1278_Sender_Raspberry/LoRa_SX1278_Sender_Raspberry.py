#!/usr/bin/env python3
import time
import struct
import RPi.GPIO as GPIO
from LoRa import LoRa

# Configuración de pines
CS_PIN = 8    # GPIO8 (Pin 24)
RST_PIN = 25  # GPIO25 (Pin 22)

# Inicializar LoRa
lora = LoRa(CS_PIN, RST_PIN)
lora.begin(433.0)  # 433 MHz

# Configurar parámetros
lora.set_spreading_factor(9)
lora.set_sync_word(0x22)

# Estructura de datos
PAYLOAD_FORMAT = "<hhff5s"

def send_data(num1, num2, num3, num4, text):
    # Empaquetar datos
    data = struct.pack(PAYLOAD_FORMAT, num1, num2, num3, num4, text.encode())
    
    # Enviar
    lora.send(data)
    print(f"Datos enviados: {len(data)} bytes")

# Ejemplo de uso
while True:
    send_data(1, 222, 3.3, 4.4, "qwert")
    time.sleep(5)