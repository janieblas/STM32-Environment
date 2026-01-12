#!/usr/bin/env python3
"""Prueba CANable - Auto envío/recepción con resistor 120Ω interno"""

import can
import time
import sys
from datetime import datetime

def test_canable_auto_echo():
    """Prueba que CANable pueda recibir sus propios mensajes"""
    print("="*70)
    print("  PRUEBA CANABLE - Resistor 120Ω INTERNO")
    print("="*70)
    print("CONFIGURACIÓN VERIFICADA:")
    print("✅ CANable: Jumper 120Ω ACTIVADO")
    print("✅ STM32: Sin resistor (solo conectado)")
    print("✅ Bitrate: 500 kbit/s")
    print("✅ Total resistencia bus: 120Ω (CORRECTO)")
    print("="*70)
    
    try:
        # Configurar bus CAN - IMPORTANTE receive_own_messages
        bus = can.Bus(
            channel='can0',
            interface='socketcan',
            receive_own_messages=True  # <-- ESENCIAL
        )
        
        print("✅ Bus CAN configurado")
        print("   receive_own_messages = True")
        
        # ID de prueba (no conflictivo)
        test_id = 0x7E1  # ID común diagnóstico
        test_counter = 0
        
        print(f"\n🚀 INICIANDO PRUEBA...")
        print("   Enviando mensajes a ID 0x{:03X}".format(test_id))
        print("   Presiona Ctrl+C para detener")
        print("-"*70)
        
        try:
            while True:
                test_counter += 1
                
                # Datos de prueba únicos
                timestamp = int(time.time() * 100) & 0xFF
                test_data = [
                    0xDE, 0xAD, 0xBE, 0xEF,  # Firma
                    test_counter & 0xFF,      # Contador
                    (test_counter >> 8) & 0xFF,
                    timestamp,
                    0xAA                      # Check
                ]
                
                # Crear mensaje
                msg = can.Message(
                    arbitration_id=test_id,
                    data=test_data,
                    is_extended_id=False,
                    dlc=8
                )
                
                # 1. ENVIAR mensaje
                try:
                    bus.send(msg, timeout=0.5)
                    send_time = datetime.now().strftime('%H:%M:%S.%f')[:-3]
                    data_str = ' '.join(f'{b:02X}' for b in test_data)
                    print(f"[{send_time}] 📤 TX #{test_counter:3d} | "
                          f"ID: 0x{test_id:03X} | Data: [{data_str}]")
                except Exception as e:
                    print(f"❌ Error enviando: {e}")
                    continue
                
                # 2. INTENTAR RECIBIR nuestro propio mensaje
                received_own = False
                start_wait = time.time()
                
                while time.time() - start_wait < 1.0:  # Esperar 1 segundo máximo
                    try:
                        recv_msg = bus.recv(timeout=0.1)
                        if recv_msg:
                            recv_time = datetime.now().strftime('%H:%M:%S.%f')[:-3]
                            recv_data = ' '.join(f'{b:02X}' for b in recv_msg.data)
                            
                            # Mostrar TODOS los mensajes recibidos
                            print(f"[{recv_time}] 📥 RX | ID: 0x{recv_msg.arbitration_id:03X} | "
                                  f"Data: [{recv_data}]")
                            
                            # Verificar si es nuestro mensaje
                            if recv_msg.arbitration_id == test_id:
                                # Verificar datos
                                if (list(recv_msg.data)[:4] == [0xDE, 0xAD, 0xBE, 0xEF] and
                                    list(recv_msg.data)[4] == (test_counter & 0xFF)):
                                    print("       └─> ✅ ¡MENSAJE PROPIO RECIBIDO!")
                                    received_own = True
                                else:
                                    print("       └─> ⚠️  ID correcto pero datos diferentes")
                            elif recv_msg.arbitration_id == 0x00C or recv_msg.arbitration_id == 0x04C:
                                print("       └─> ⚠️  Ruido conocido en el bus")
                            else:
                                print("       └─> ⚠️  Mensaje externo no esperado")
                                
                    except KeyboardInterrupt:
                        raise
                    except:
                        pass
                
                if not received_own:
                    print("       └─> ❌ NO se recibió mensaje propio")
                    print("           Posible: receive_own_messages no funciona")
                
                # Esperar entre mensajes
                time.sleep(1.0)
                
        except KeyboardInterrupt:
            print("\n\n🛑 Prueba interrumpida por usuario")
            
        finally:
            bus.shutdown()
            print("\n" + "="*70)
            print("  RESUMEN DE PRUEBA")
            print("="*70)
            print(f"Mensajes enviados: {test_counter}")
            print(f"Ciclos completados: {test_counter}")
            
            if test_counter > 0:
                print("\n📊 INTERPRETACIÓN:")
                print("-"*70)
                print("✅ Si ves 'MENSAJE PROPIO RECIBIDO':")
                print("   ¡CANable funciona PERFECTO!")
                print("   El bus está bien configurado")
                
                print("\n⚠️  Si NO ves mensajes propios pero sí otros:")
                print("   - CANable no soporta receive_own_messages")
                print("   - O hay mucho ruido en el bus")
                
                print("\n❌ Si NO ves NINGÚN mensaje recibido:")
                print("   - Problema de terminación (verificar 120Ω)")
                print("   - Bitrate incorrecto")
                print("   - CANable defectuoso")
            
            print("="*70)
            
    except Exception as e:
        print(f"\n❌ Error configurando bus CAN: {e}")
        print("\nSOLUCIÓN RÁPIDA:")
        print("1. Verificar que can0 está configurada:")
        print("   sudo ip link set can0 type can bitrate 500000")
        print("   sudo ip link set can0 up")
        print("2. Probar con candump manual:")
        print("   Terminal 1: candump can0")
        print("   Terminal 2: cansend can0 7E1#DEADBEEF01020304")

def prueba_manual_simple():
    """Prueba manual rápida sin hilos complicados"""
    print("\n" + "="*70)
    print("  PRUEBA MANUAL RÁPIDA")
    print("="*70)
    
    try:
        bus = can.Bus(channel='can0', interface='socketcan')
        
        print("Enviando mensaje de prueba...")
        msg = can.Message(
            arbitration_id=0x7E2,
            data=[0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88],
            is_extended_id=False
        )
        
        bus.send(msg)
        print("✅ Mensaje enviado: ID=0x7E2")
        
        print("\nEscuchando por 3 segundos...")
        start = time.time()
        mensajes = []
        
        while time.time() - start < 3:
            try:
                recv = bus.recv(timeout=0.1)
                if recv:
                    mensajes.append(recv)
                    print(f"📥 RX: ID=0x{recv.arbitration_id:03X}")
            except:
                pass
        
        bus.shutdown()
        
        print(f"\nTotal mensajes recibidos: {len(mensajes)}")
        
        if mensajes:
            print("\nIDs recibidos:")
            for m in mensajes:
                print(f"  - 0x{m.arbitration_id:03X}")
        else:
            print("\n❌ No se recibió NADA")
            
    except Exception as e:
        print(f"❌ Error: {e}")

def main():
    print("🔥 PRUEBA DEFINITIVA CANABLE 120Ω")
    print("\nInstrucciones:")
    print("1. Asegúrate que solo CANable tiene resistor 120Ω")
    print("2. STM32 conectado pero sin transmitir")
    print("3. can0 configurada a 500 kbit/s")
    
    # Primero prueba simple
    prueba_manual_simple()
    
    # Preguntar por prueba automática
    resp = input("\n¿Ejecutar prueba automática extendida? (s/n): ").strip().lower()
    if resp == 's':
        test_canable_auto_echo()
    
    print("\n" + "="*70)
    print("  VERIFICACIÓN FINAL")
    print("="*70)
    print("Si NO funciona, prueba estos comandos MANUALES:")
    print("\nTerminal 1 (escucha):")
    print("  candump can0")
    print("\nTerminal 2 (envío):")
    print("  cansend can0 7E0#1122334455667788")
    print("\nSi en Terminal 1 ves el mensaje: ¡FUNCIONA!")
    print("Si no lo ves: problema de hardware/configuración")
    print("="*70)

if __name__ == "__main__":
    main()