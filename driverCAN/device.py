import can
import subprocess
import os
import sys

def verificar_interfaz_can(interfaz='can0'):
    """
    Verifica si la interfaz CAN existe y está configurada
    """
    print(f"🔍 Verificando interfaz CAN: {interfaz}")
    print("-" * 50)
    
    # 1. Verificar si la interfaz existe
    try:
        result = subprocess.run(
            ['ip', 'link', 'show', interfaz],
            capture_output=True,
            text=True,
            check=True
        )
        print(f"✅ Interfaz {interfaz} encontrada")
        print(result.stdout)
    except subprocess.CalledProcessError:
        print(f"❌ ERROR: La interfaz {interfaz} NO existe")
        print("\n💡 Posibles soluciones:")
        print("   1. Conecta el dispositivo CANable")
        print("   2. Verifica con: lsusb | grep -i can")
        print("   3. Carga módulos: sudo modprobe gs_usb")
        return False
    
    # 2. Verificar si está UP
    try:
        result = subprocess.run(
            ['ip', 'link', 'show', interfaz],
            capture_output=True,
            text=True,
            check=True
        )
        if 'state UP' in result.stdout:
            print(f"✅ Interfaz {interfaz} está activa (UP)")
        else:
            print(f"⚠️  Interfaz {interfaz} existe pero está DOWN")
            print("\n💡 Para activarla:")
            print(f"   sudo ip link set {interfaz} type can bitrate 500000")
            print(f"   sudo ip link set {interfaz} up")
            return False
    except subprocess.CalledProcessError:
        return False
    
    # 3. Verificar estadísticas
    try:
        result = subprocess.run(
            ['ip', '-details', '-statistics', 'link', 'show', interfaz],
            capture_output=True,
            text=True,
            check=True
        )
        print("\n📊 Detalles de la interfaz:")
        print(result.stdout)
    except subprocess.CalledProcessError:
        pass
    
    # 4. Intentar conectar con python-can
    try:
        print("\n🔌 Intentando conectar con python-can...")
        bus = can.interface.Bus(channel=interfaz, bustype='socketcan')
        print(f"✅ Conexión exitosa a {interfaz}")
        bus.shutdown()
        return True
    except Exception as e:
        print(f"❌ ERROR al conectar con python-can: {e}")
        return False

def verificar_modulos_kernel():
    """
    Verifica si los módulos del kernel están cargados
    """
    print("\n🔧 Verificando módulos del kernel...")
    print("-" * 50)
    
    modulos_necesarios = ['can', 'can_raw', 'gs_usb', 'can_dev']
    
    try:
        result = subprocess.run(
            ['lsmod'],
            capture_output=True,
            text=True,
            check=True
        )
        
        for modulo in modulos_necesarios:
            if modulo in result.stdout:
                print(f"✅ Módulo {modulo} cargado")
            else:
                print(f"⚠️  Módulo {modulo} NO cargado")
                print(f"   Carga con: sudo modprobe {modulo}")
                
    except subprocess.CalledProcessError as e:
        print(f"❌ Error verificando módulos: {e}")

def verificar_dispositivo_usb():
    """
    Verifica si el dispositivo USB CANable está conectado
    """
    print("\n🔌 Verificando dispositivo USB...")
    print("-" * 50)
    
    try:
        result = subprocess.run(
            ['lsusb'],
            capture_output=True,
            text=True,
            check=True
        )
        
        # Buscar dispositivos CAN comunes
        keywords = ['CAN', 'OpenMoko', 'Geschwister', '1d50:606f']
        
        encontrado = False
        for keyword in keywords:
            if keyword.lower() in result.stdout.lower():
                print(f"✅ Dispositivo CAN encontrado:")
                for linea in result.stdout.split('\n'):
                    if keyword.lower() in linea.lower():
                        print(f"   {linea}")
                encontrado = True
                break
        
        if not encontrado:
            print("❌ No se encontró dispositivo CAN en USB")
            print("\n💡 Verifica:")
            print("   1. Cable USB conectado")
            print("   2. Dispositivo encendido")
            print("   3. En VirtualBox: Devices → USB → Selecciona el CANable")
            
    except subprocess.CalledProcessError as e:
        print(f"❌ Error ejecutando lsusb: {e}")

def main():
    print("=" * 50)
    print("  VERIFICADOR DE DISPOSITIVO CAN")
    print("=" * 50)
    
    # Verificar si python-can está instalado
    try:
        import can
        print("✅ python-can instalado\n")
    except ImportError:
        print("❌ python-can NO está instalado")
        print("   Instala con: pip install python-can")
        sys.exit(1)
    
    # Ejecutar verificaciones
    verificar_dispositivo_usb()
    verificar_modulos_kernel()
    
    resultado = verificar_interfaz_can('can0')
    
    print("\n" + "=" * 50)
    if resultado:
        print("✅ TODO LISTO - El dispositivo CAN está funcionando")
        print("=" * 50)
        print("\n💡 Puedes usar:")
        print("   candump can0          # Para monitorear")
        print("   python tu_script.py   # Para usar python-can")
        return 0
    else:
        print("❌ HAY PROBLEMAS - Revisa los errores arriba")
        print("=" * 50)
        return 1

if __name__ == "__main__":
    sys.exit(main())