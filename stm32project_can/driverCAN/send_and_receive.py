import can
import time
import threading

INTERFACE = 'can0'
TEST_ID = 0x123
TEST_DATA = [0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04]

mensaje_recibido = False

def recibir_mensajes(bus):
    global mensaje_recibido
    
    print("👂 Esperando recibir cualquier mensaje...\n")
    
    while not mensaje_recibido:
        try:
            message = bus.recv(timeout=5.0)
            if message:
                datos_hex = ' '.join(f'{b:02X}' for b in message.data)
                print(f"📨 RECIBIDO - ID: 0x{message.arbitration_id:03X} | Data: [{datos_hex}]")
                
                # Cerrar al recibir CUALQUIER mensaje
                print("   ✅ Mensaje recibido! Cerrando...\n")
                mensaje_recibido = True
                break
        except Exception as e:
            print(f"⚠️  Timeout: {e}")
            break

def main():
    global mensaje_recibido
    
    print("=" * 60)
    print("  TEST CAN - ENVIAR Y RECIBIR")
    print("=" * 60)
    print()
    
    try:
        bus = can.interface.Bus(channel=INTERFACE, bustype='socketcan')
        print("✅ Conectado al bus CAN\n")
        
        hilo_rx = threading.Thread(target=recibir_mensajes, args=(bus,))
        hilo_rx.start()
        
        time.sleep(0.5)
        
        # Enviar mensaje
        mensaje = can.Message(
            arbitration_id=TEST_ID,
            data=TEST_DATA,
            is_extended_id=False
        )
        
        bus.send(mensaje)
        datos_hex = ' '.join(f'{b:02X}' for b in TEST_DATA)
        print(f"📤 ENVIADO - ID: 0x{TEST_ID:03X} | Data: [{datos_hex}]\n")
        
        hilo_rx.join(timeout=6.0)
        
        if not mensaje_recibido:
            print("❌ No se recibió ningún mensaje\n")
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
    finally:
        bus.shutdown()
        print("✅ Cerrado\n")

if __name__ == "__main__":
    main()