import can
import time
from collections import defaultdict
from datetime import datetime

INTERFACE = 'can0'
DURACION_MONITOREO = 10  # segundos

def monitorear_bus():
    print("=" * 70)
    print("  MONITOR DE BUS CAN - DETECCIÓN DE RUIDO")
    print("=" * 70)
    print(f"Interfaz: {INTERFACE}")
    print(f"Duración: {DURACION_MONITOREO} segundos")
    print(f"Hora de inicio: {datetime.now().strftime('%H:%M:%S')}")
    print("=" * 70)
    print()
    
    try:
        # Conectar sin filtros (recibir todo)
        bus = can.interface.Bus(channel=INTERFACE, bustype='socketcan')
        print("✅ Conectado al bus CAN\n")
        print("👂 Escuchando el bus...\n")
        
        # Contadores
        mensajes_totales = 0
        ids_unicos = defaultdict(int)
        inicio = time.time()
        ultimo_reporte = inicio
        
        while time.time() - inicio < DURACION_MONITOREO:
            # Recibir mensaje
            message = bus.recv(timeout=0.5)
            
            if message:
                mensajes_totales += 1
                ids_unicos[message.arbitration_id] += 1
                
                # Mostrar mensaje
                datos_hex = ' '.join(f'{b:02X}' for b in message.data)
                timestamp = time.time() - inicio
                print(f"[{timestamp:6.3f}s] ID: 0x{message.arbitration_id:03X} | "
                      f"DLC: {len(message.data)} | Data: [{datos_hex}]")
            
            # Reporte cada 2 segundos
            if time.time() - ultimo_reporte >= 2:
                tiempo_transcurrido = time.time() - inicio
                tasa = mensajes_totales / tiempo_transcurrido if tiempo_transcurrido > 0 else 0
                print(f"\n--- Reporte: {mensajes_totales} mensajes, "
                      f"{len(ids_unicos)} IDs únicos, "
                      f"{tasa:.1f} msg/s ---\n")
                ultimo_reporte = time.time()
        
        # Reporte final
        print("\n" + "=" * 70)
        print("  REPORTE FINAL")
        print("=" * 70)
        print(f"Duración total: {DURACION_MONITOREO} segundos")
        print(f"Mensajes recibidos: {mensajes_totales}")
        print(f"IDs únicos detectados: {len(ids_unicos)}")
        
        if mensajes_totales > 0:
            tasa_promedio = mensajes_totales / DURACION_MONITOREO
            print(f"Tasa promedio: {tasa_promedio:.2f} mensajes/segundo")
            
            print("\n📊 Distribución de IDs:")
            print("-" * 70)
            for can_id in sorted(ids_unicos.keys()):
                count = ids_unicos[can_id]
                porcentaje = (count / mensajes_totales) * 100
                barra = "█" * int(porcentaje / 2)
                print(f"ID 0x{can_id:03X}: {count:4d} msgs ({porcentaje:5.1f}%) {barra}")
        else:
            print("\n✅ BUS LIMPIO - No se detectaron mensajes")
            print("   El bus está silencioso, sin ruido ni tráfico.")
        
        # Análisis de ruido
        print("\n🔍 ANÁLISIS:")
        print("-" * 70)
        
        if mensajes_totales == 0:
            print("✅ EXCELENTE: Bus completamente limpio")
            print("   No hay ruido ni mensajes fantasma")
            print("   Listo para comunicación CAN")
            
        elif mensajes_totales < 5:
            print("✅ BUENO: Muy pocos mensajes detectados")
            print("   Posible ruido mínimo o mensajes legítimos")
            print("   Verificar si son esperados")
            
        elif len(ids_unicos) > 10:
            print("⚠️  ADVERTENCIA: Muchos IDs diferentes detectados")
            print("   Posible ruido eléctrico en el bus")
            print("   Verificar:")
            print("   - Terminación de 120Ω en ambos extremos")
            print("   - Calidad de cables y conexiones")
            print("   - Blindaje del cable CAN")
            
        elif mensajes_totales > 100:
            print("⚠️  ADVERTENCIA: Alto tráfico en el bus")
            print("   Puede ser:")
            print("   - Dispositivo activo transmitiendo")
            print("   - Ruido eléctrico significativo")
            print("   - Bus con actividad legítima")
        else:
            print("ℹ️  NORMAL: Actividad moderada detectada")
            print("   Revisar si los mensajes son esperados")
        
        print("=" * 70)
        
    except KeyboardInterrupt:
        print("\n\n⚠️  Monitoreo interrumpido por usuario")
    except Exception as e:
        print(f"\n❌ Error: {e}")
    finally:
        bus.shutdown()
        print("\n✅ Monitor cerrado\n")

if __name__ == "__main__":
    monitorear_bus()