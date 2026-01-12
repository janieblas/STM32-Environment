#!/usr/bin/env python3
"""CAN Tester que NO reconfigura el bus existente"""

import can
import time
import os
import subprocess
from datetime import datetime

def verificar_configuracion_can():
    """Verifica que can0 esté bien configurada sin cambiarla"""
    print("🔍 Verificando configuración CAN...")
    
    # Verificar que can0 existe y está UP
    result = subprocess.run(['ip', 'link', 'show', 'can0'], 
                          capture_output=True, text=True)
    
    if result.returncode != 0:
        print("❌ can0 no existe. Configurar primero:")
        print("   sudo ip link set can0 type can bitrate 500000")
        print("   sudo ip link set can0 up")
        return False
    
    # Verificar estado
    if "state UP" not in result.stdout:
        print("⚠️  can0 está DOWN. Activando...")
        subprocess.run(['sudo', 'ip', 'link', 'set', 'can0', 'up'], check=True)
        time.sleep(0.5)
    
    # Verificar bitrate
    result = subprocess.run(['ip', '-details', 'link', 'show', 'can0'],
                          capture_output=True, text=True)
    
    print("✅ can0 configurada correctamente")
    print(f"   Estado: {'UP' if 'state UP' in result.stdout else 'DOWN'}")
    
    # Extraer bitrate
    import re
    match = re.search(r'bitrate (\d+)', result.stdout)
    if match:
        print(f"   Bitrate: {match.group(1)} bps")
    
    return True

def prueba_simple_envio():
    """Prueba simple de envío SIN reconfigurar"""
    print("\n🚀 Probando envío...")
    
    try:
        # Conexión SIMPLE sin parámetros que puedan reconfigurar
        bus = can.Bus(channel='can0', interface='socketcan', receive_own_messages=False)
        
        # Mensaje de prueba
        msg = can.Message(
            arbitration_id=0x555,  # Usar ID diferente para pruebas
            data=[0xDE, 0xAD, 0xBE, 0xEF, 1, 2, 3, 4],
            is_extended_id=False,
            dlc=8
        )
        
        print(f"Enviando: ID=0x{msg.arbitration_id:03X}, Data={msg.data.hex().upper()}")
        
        # Enviar
        bus.send(msg, timeout=0.5)
        print("✅ Mensaje enviado")
        
        # Esperar respuesta breve
        print("Escuchando 3 segundos...")
        start = time.time()
        
        while time.time() - start < 3:
            try:
                recv_msg = bus.recv(timeout=0.1)
                if recv_msg:
                    print(f"📥 RX: ID=0x{recv_msg.arbitration_id:03X}, "
                          f"Data={recv_msg.data.hex().upper()}")
                    
                    # Verificar si es del STM32
                    if (recv_msg.arbitration_id == 0x123 or 
                        recv_msg.arbitration_id == 0x555):
                        if len(recv_msg.data) >= 4:
                            if (recv_msg.data[0] == 0xDE and 
                                recv_msg.data[1] == 0xAD and
                                recv_msg.data[2] == 0xBE and 
                                recv_msg.data[3] == 0xEF):
                                print("🎯 ¡MENSAJE DEL STM32 DETECTADO!")
                                return True
            except KeyboardInterrupt:
                break
            except:
                pass
        
        print("❌ No se recibió respuesta del STM32")
        return False
        
    except Exception as e:
        print(f"❌ Error: {e}")
        return False
    finally:
        try:
            bus.shutdown()
        except:
            pass

def monitorear_antes_despues():
    """Monitoriza el bus antes y después de usar python-can"""
    print("\n📊 MONITOREO ANTES de usar python-can:")
    print("-" * 40)
    
    # Contar mensajes por 2 segundos
    subprocess.run(['timeout', '2', 'candump', 'can0'], 
                   capture_output=True)
    
    print("\n🔧 Ejecutando script python-can...")
    
    # Aquí ejecutarías tu prueba
    prueba_simple_envio()
    
    print("\n📊 MONITOREO DESPUÉS de usar python-can:")
    print("-" * 40)
    
    # Esperar un momento
    time.sleep(1)
    
    # Contar mensajes por 2 segundos
    result = subprocess.run(['timeout', '2', 'candump', 'can0'], 
                           capture_output=True, text=True)
    
    # Analizar resultado
    lineas = result.stdout.strip().split('\n')
    if lineas and lineas[0]:
        print(f"Se recibieron {len(lineas)} mensajes después:")
        for i, linea in enumerate(lineas[:5]):  # Mostrar primeros 5
            print(f"  {linea}")
        if len(lineas) > 5:
            print(f"  ... y {len(lineas)-5} más")
    else:
        print("✅ No hay ruido después del script")

def main():
    print("=" * 70)
    print("  DIAGNÓSTICO DE PROBLEMAS python-can")
    print("=" * 70)
    
    # Verificar configuración
    if not verificar_configuracion_can():
        return
    
    # Monitorizar antes/después
    monitorear_antes_despues()
    
    print("\n" + "=" * 70)
    print("  RECOMENDACIONES:")
    print("=" * 70)
    print("1. Usar candump/cansend en lugar de python-can para pruebas")
    print("2. Si debes usar python-can:")
    print("   - NO pases parámetros bitrate")
    print("   - Usa receive_own_messages=False")
    print("   - Configura can0 manualmente antes")
    print("3. Verificar versión de python-can:")
    print("   pip3 show python-can")
    
    # Probar con candump en paralelo
    print("\n🔍 Prueba en paralelo (2 terminales):")
    print("   Terminal 1: candump can0")
    print("   Terminal 2: cansend can0 555#DEADBEEF01020304")

if __name__ == "__main__":
    main()