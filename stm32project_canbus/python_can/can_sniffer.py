import can
import time
from datetime import datetime
from collections import defaultdict
import threading

INTERFACE = 'can0'

class CANSniffer:
    def __init__(self):
        self.mensajes_totales = 0
        self.ids_contador = defaultdict(int)
        self.ids_datos = defaultdict(list)
        self.inicio = time.time()
        self.ultimo_mensaje = None
        self.running = True
        
    def mostrar_estadisticas(self):
        """Muestra estadísticas en tiempo real cada 5 segundos"""
        while self.running:
            time.sleep(5)
            if self.mensajes_totales > 0:
                print("\n" + "="*70)
                print(f"📊 ESTADÍSTICAS - {datetime.now().strftime('%H:%M:%S')}")
                print("="*70)
                tiempo_total = time.time() - self.inicio
                tasa = self.mensajes_totales / tiempo_total if tiempo_total > 0 else 0
                print(f"Tiempo: {tiempo_total:.1f}s | Mensajes: {self.mensajes_totales} | Tasa: {tasa:.2f} msg/s")
                print(f"IDs únicos: {len(self.ids_contador)}")
                print("-"*70)
                print("Top 10 IDs más activos:")
                for can_id, count in sorted(self.ids_contador.items(), key=lambda x: x[1], reverse=True)[:10]:
                    porcentaje = (count / self.mensajes_totales) * 100
                    barra = "█" * int(porcentaje / 2)
                    print(f"  0x{can_id:03X}: {count:5d} msgs ({porcentaje:5.1f}%) {barra}")
                print("="*70 + "\n")

    def sniffer(self):
        print("=" * 70)
        print("  CAN BUS SNIFFER - Modo Normal")
        print("=" * 70)
        print(f"Interfaz: {INTERFACE}")
        print(f"Inicio: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print("=" * 70)
        print()
        
        try:
            # Conectar al bus CAN sin filtros
            bus = can.interface.Bus(channel=INTERFACE, bustype='socketcan')
            print("✅ Conectado al bus CAN\n")
            print("👂 Sniffing en progreso... (Ctrl+C para detener)\n")
            print("-" * 70)
            print(f"{'TIMESTAMP':<12} {'ID':<8} {'DLC':<5} {'DATA':<30} {'ASCII':<10}")
            print("-" * 70)
            
            # Iniciar hilo de estadísticas
            stats_thread = threading.Thread(target=self.mostrar_estadisticas, daemon=True)
            stats_thread.start()
            
            while self.running:
                # Recibir mensaje
                message = bus.recv(timeout=1.0)
                
                if message:
                    self.mensajes_totales += 1
                    self.ids_contador[message.arbitration_id] += 1
                    self.ultimo_mensaje = time.time()
                    
                    # Formatear timestamp
                    timestamp = datetime.now().strftime('%H:%M:%S.%f')[:-3]
                    
                    # Formatear datos en hexadecimal
                    datos_hex = ' '.join(f'{b:02X}' for b in message.data)
                    
                    # Intentar mostrar ASCII (solo caracteres imprimibles)
                    ascii_str = ''
                    for byte in message.data:
                        if 32 <= byte <= 126:  # Caracteres ASCII imprimibles
                            ascii_str += chr(byte)
                        else:
                            ascii_str += '.'
                    
                    # Determinar si es ID extendido
                    id_str = f"0x{message.arbitration_id:08X}" if message.is_extended_id else f"0x{message.arbitration_id:03X}"
                    id_type = "EXT" if message.is_extended_id else "STD"
                    
                    # Mostrar mensaje
                    print(f"{timestamp} {id_str:<8} {message.dlc:<5} {datos_hex:<30} {ascii_str:<10}")
                    
                    # Guardar datos para análisis (mantener últimos 5)
                    self.ids_datos[message.arbitration_id].append(message.data)
                    if len(self.ids_datos[message.arbitration_id]) > 5:
                        self.ids_datos[message.arbitration_id].pop(0)
                        
        except KeyboardInterrupt:
            print("\n\n" + "=" * 70)
            print("  REPORTE FINAL")
            print("=" * 70)
            self.running = False
            self.generar_reporte()
            
        except Exception as e:
            print(f"\n❌ Error: {e}")
            self.running = False
            
        finally:
            bus.shutdown()
            print("\n✅ Sniffer cerrado\n")
    
    def generar_reporte(self):
        """Genera reporte final al terminar"""
        tiempo_total = time.time() - self.inicio
        
        print(f"Tiempo total: {tiempo_total:.2f} segundos")
        print(f"Mensajes capturados: {self.mensajes_totales}")
        print(f"IDs únicos: {len(self.ids_contador)}")
        
        if self.mensajes_totales > 0:
            tasa_promedio = self.mensajes_totales / tiempo_total
            print(f"Tasa promedio: {tasa_promedio:.2f} mensajes/segundo")
            
            print("\n" + "-" * 70)
            print("TABLA DE IDs DETECTADOS:")
            print("-" * 70)
            print(f"{'ID':<12} {'COUNT':<10} {'%':<8} {'ÚLTIMO DATA':<35}")
            print("-" * 70)
            
            for can_id in sorted(self.ids_contador.keys()):
                count = self.ids_contador[can_id]
                porcentaje = (count / self.mensajes_totales) * 100
                
                # Último dato recibido
                if self.ids_datos[can_id]:
                    ultimo_dato = ' '.join(f'{b:02X}' for b in self.ids_datos[can_id][-1])
                else:
                    ultimo_dato = "N/A"
                
                print(f"0x{can_id:03X}      {count:<10} {porcentaje:6.2f}%  {ultimo_dato:<35}")
            
            print("-" * 70)
            
            # Detectar patrones
            print("\n🔍 ANÁLISIS DE PATRONES:")
            print("-" * 70)
            
            for can_id, datos_list in self.ids_datos.items():
                if len(datos_list) >= 3:
                    # Verificar si hay bytes que cambian constantemente
                    bytes_cambiantes = []
                    for i in range(len(datos_list[0])):
                        valores = [datos[i] for datos in datos_list if len(datos) > i]
                        if len(set(valores)) == len(valores):  # Todos diferentes
                            bytes_cambiantes.append(i)
                    
                    if bytes_cambiantes:
                        print(f"ID 0x{can_id:03X}: Bytes que cambian constantemente: {bytes_cambiantes}")
                        print(f"           Posible contador o sensor variable")
            
        print("=" * 70)

if __name__ == "__main__":
    sniffer = CANSniffer()
    sniffer.sniffer()