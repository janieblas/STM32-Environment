import can
from datetime import datetime

INTERFACE = 'can0'

def recibir_mensajes_stm32():
    print("=" * 70)
    print("  RECEPTOR CAN - Esperando mensajes del STM32")
    print("=" * 70)
    print(f"Interfaz: {INTERFACE}")
    print(f"Hora de inicio: {datetime.now().strftime('%H:%M:%S')}")
    print("=" * 70)
    print()
    
    try:
        # Conectar al bus CAN (sin filtros, recibir todo)
        bus = can.interface.Bus(channel=INTERFACE, bustype='socketcan')
        print("✅ Conectado al bus CAN\n")
        print("👂 Escuchando mensajes del STM32...")
        print("   Presiona Ctrl+C para detener\n")
        print("-" * 70)
        
        contador_mensajes = 0
        
        while True:
            # Recibir mensaje (timeout de 5 segundos)
            message = bus.recv(timeout=5.0)
            
            if message:
                contador_mensajes += 1
                
                # Formatear datos
                datos_hex = ' '.join(f'{b:02X}' for b in message.data)
                timestamp = datetime.now().strftime('%H:%M:%S.%f')[:-3]
                
                # Mostrar mensaje
                print(f"[{timestamp}] 📨 #{contador_mensajes:4d} | "
                      f"ID: 0x{message.arbitration_id:03X} | "
                      f"DLC: {message.dlc} | "
                      f"Data: [{datos_hex}]")
                
                # Decodificar si es el mensaje del STM32
                if message.arbitration_id == 0x123 and message.dlc == 8:
                    if (message.data[0] == 0xDE and 
                        message.data[1] == 0xAD and 
                        message.data[2] == 0xBE and 
                        message.data[3] == 0xEF):
                        contador_stm32 = message.data[7]
                        print(f"           └─> ✅ Mensaje del STM32 - Contador: {contador_stm32}")
                        print()
            else:
                # Timeout - no se recibió mensaje en 5 segundos
                print(f"[{datetime.now().strftime('%H:%M:%S')}] ⏱️  Timeout - Sin mensajes recibidos")
                
    except KeyboardInterrupt:
        print("\n\n" + "=" * 70)
        print("  RESUMEN")
        print("=" * 70)
        print(f"Mensajes recibidos: {contador_mensajes}")
        print("✅ Detenido por usuario")
        print("=" * 70)
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
        
    finally:
        bus.shutdown()
        print("\n✅ Bus CAN cerrado\n")

if __name__ == "__main__":
    recibir_mensajes_stm32()