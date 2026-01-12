# Guía Completa: Configurar CANable en Linux usando VirtualBox

Esta guía explica cómo configurar un dispositivo CANable para trabajar con Linux a través de VirtualBox, evitando los problemas de drivers en Windows.

---

## 📋 Requisitos Previos

- CANable 2.0 (o similar adaptador CAN USB)
- Windows (host)
- VirtualBox con Ubuntu/Linux (guest)
- VirtualBox Extension Pack instalado
- Acceso a internet

---

## Parte 1: Preparación en Windows

### Paso 1: Actualizar Firmware del CANable

1. **Poner el CANable en modo bootloader:**
   - **Si tu CANable tiene un jumper:** Coloca el jumper en los pines de BOOT mientras conectas el dispositivo al USB
   - **Si tiene un botón:** Mantén presionado el botón BOOT mientras conectas el dispositivo
   - **Alternativa desde la página web:** Puedes usar el botón "Enter Bootloader" en la página web si el dispositivo ya está conectado

2. Abre tu navegador web (Chrome o Edge recomendado - necesita soporte WebUSB)

3. Ve a la página de actualización del firmware CANable 2.0:
   - URL: **https://canable.io/updater/canable2.html**

4. Sigue las instrucciones en pantalla:
   - Selecciona el puerto COM correspondiente
   - Haz click en "Update Firmware"

5. Espera a que termine la actualización (toma unos segundos)

6. **Quita el jumper** (si lo usaste)

7. Desconecta y reconecta el dispositivo - ahora tendrá el firmware candleLight más reciente

### Paso 2: Instalar Driver con Zadig

1. **Descarga Zadig:**
   - Ve a https://zadig.akeo.ie/
   - Descarga la última versión

2. **Ejecuta Zadig como Administrador:**
   - Click derecho en `zadig.exe` → "Ejecutar como administrador"

3. **Configura el driver:**
   - En el menú: `Options` → marca `List All Devices`
   - Selecciona tu CANable de la lista desplegable
   - En el selector de driver (recuadro verde con flechas):
     - Asegúrate que el driver de destino sea **libusbWin32** o **libusb-win32**
   - Click en `Replace Driver` o `Install Driver`
   - Espera a que termine la instalación

4. **Verifica la instalación:**
   - Abre el Administrador de Dispositivos
   - Busca tu dispositivo bajo "Universal Serial Bus devices" o "libusb-win32 devices"

> **Nota:** El driver `libusbWin32` es necesario para que VirtualBox pueda pasar el dispositivo correctamente a la máquina virtual Linux.

---

## Parte 2: Configuración de VirtualBox

### Paso 3: Configurar USB en VirtualBox

1. **Asegúrate que VirtualBox Extension Pack esté instalado:**
   - Abre VirtualBox
   - Ve a `Archivo` → `Herramientas` → `Extension Pack Manager`
   - Si no está instalado, descárgalo de https://www.virtualbox.org/

2. **Configura el controlador USB (con la VM apagada):**
   - Selecciona tu máquina virtual Ubuntu/Linux
   - Click en `Configuración` → `USB`
   - Controlador USB: selecciona **USB 3.0 (xHCI) Controller**
     - Esto mejora la compatibilidad aunque no siempre es estrictamente necesario

3. **Agrega un filtro USB (opcional pero recomendado):**
   - En la misma ventana de configuración USB
   - Click en el ícono "+" (agregar filtro USB)
   - Selecciona tu CANable de la lista
   - Click `OK`

---

## Parte 3: Configuración en Linux

### Paso 4: Conectar el Dispositivo a la VM

1. Inicia tu máquina virtual Linux
2. Con el CANable conectado al USB:
   - En el menú de VirtualBox: `Dispositivos` → `USB`
   - Marca tu dispositivo CANable (ejemplo: "OpenMoko, Inc. Geschwister Schneider CAN adapter")

### Paso 5: Verificar Detección del Dispositivo

Abre una terminal y ejecuta:

```bash
lsusb
```

Deberías ver una línea similar a:
```
Bus 001 Device 003: ID 1d50:606f OpenMoko, Inc. Geschwister Schneider CAN adapter
```

### Paso 6: Cargar Módulos del Kernel

Ejecuta los siguientes comandos:

```bash
sudo modprobe can
sudo modprobe can_raw
sudo modprobe gs_usb
```

Verifica que se cargaron correctamente:

```bash
lsmod | grep can
```

Deberías ver:
```
gs_usb
can_dev
can_raw
can
```

### Paso 7: Verificar la Interfaz CAN

Verifica que apareció la interfaz `can0`:

```bash
ip link show
```

Deberías ver algo como:
```
3: can0: <NOARP,ECHO> mtu 16 qdisc noop state DOWN mode DEFAULT group default qlen 10
    link/can
```

### Paso 8: Configurar y Activar la Interfaz CAN

1. **Configura el bitrate** (500 kbps en este ejemplo):
```bash
sudo ip link set can0 type can bitrate 500000
```

> **Nota:** Bitrates comunes: 125000, 250000, 500000, 1000000 (debe coincidir con tu red CAN)

2. **Levanta la interfaz:**
```bash
sudo ip link set can0 up
```

3. **Verifica que esté activa:**
```bash
ip link show can0
```

Deberías ver:
```
3: can0: <NOARP,UP,LOWER_UP,ECHO> mtu 16 qdisc pfifo_fast state UP mode DEFAULT group default qlen 10
    link/can
```

¡Nota el `state UP`!

---

## Parte 4: Usar el Dispositivo CAN

### Opción A: Herramientas de Línea de Comandos

1. **Instala can-utils:**
```bash
sudo apt-get update
sudo apt-get install can-utils
```

2. **Monitorear tráfico CAN:**
```bash
candump can0
```

3. **Enviar un mensaje:**
```bash
cansend can0 123#DEADBEEF
```

4. **Generar tráfico de prueba:**
```bash
cangen can0
```

### Opción B: Python con python-can

1. **Instala python-can:**
```bash
pip3 install python-can
```

O:
```bash
sudo apt-get install python3-can
```

2. **Script para recibir mensajes** (`recibir.py`):
```python
import can

bus = can.interface.Bus(channel='can0', bustype='socketcan')

print("👂 Escuchando mensajes CAN...")
print("Presiona Ctrl+C para detener\n")

try:
    while True:
        message = bus.recv(timeout=1.0)
        if message:
            print(f"📨 ID: 0x{message.arbitration_id:03X} | "
                  f"Data: {' '.join(f'{b:02X}' for b in message.data)}")
except KeyboardInterrupt:
    print("\n✅ Detenido")
finally:
    bus.shutdown()
```

3. **Script para enviar mensajes** (`enviar.py`):
```python
import can
import time

bus = can.interface.Bus(channel='can0', bustype='socketcan')

print("📤 Enviando mensajes CAN...")

try:
    for i in range(10):
        msg = can.Message(
            arbitration_id=0x123,
            data=[0x11, 0x22, 0x33, 0x44, i],
            is_extended_id=False
        )
        bus.send(msg)
        print(f"✉️  Mensaje {i+1} enviado: "
              f"ID=0x123, Data=[11 22 33 44 {i:02X}]")
        time.sleep(0.5)
except KeyboardInterrupt:
    print("\n⚠️  Interrumpido")
finally:
    bus.shutdown()
    print("✅ Cerrado")
```

4. **Ejecuta los scripts:**
```bash
python3 recibir.py
# o
python3 enviar.py
```

---

## 🔧 Script de Verificación

Guarda este script como `verificar_can.py` para diagnosticar problemas:

```python
import can
import subprocess
import sys

def verificar_dispositivo():
    print("=" * 50)
    print("  VERIFICADOR DE DISPOSITIVO CAN")
    print("=" * 50)
    
    # Verificar USB
    print("\n🔌 Verificando dispositivo USB...")
    result = subprocess.run(['lsusb'], capture_output=True, text=True)
    if '1d50:606f' in result.stdout or 'CAN' in result.stdout:
        print("✅ Dispositivo CAN encontrado")
    else:
        print("❌ Dispositivo CAN NO encontrado")
        return False
    
    # Verificar módulos
    print("\n🔧 Verificando módulos...")
    result = subprocess.run(['lsmod'], capture_output=True, text=True)
    for mod in ['can', 'can_raw', 'gs_usb']:
        if mod in result.stdout:
            print(f"✅ {mod} cargado")
        else:
            print(f"❌ {mod} NO cargado")
    
    # Verificar interfaz
    print("\n🔍 Verificando interfaz can0...")
    result = subprocess.run(['ip', 'link', 'show', 'can0'], 
                          capture_output=True, text=True)
    if result.returncode == 0:
        if 'state UP' in result.stdout:
            print("✅ can0 está UP y funcionando")
            return True
        else:
            print("⚠️  can0 existe pero está DOWN")
            return False
    else:
        print("❌ can0 NO existe")
        return False

if __name__ == "__main__":
    if verificar_dispositivo():
        print("\n✅ TODO LISTO")
        sys.exit(0)
    else:
        print("\n❌ HAY PROBLEMAS")
        sys.exit(1)
```

Ejecuta:
```bash
python3 verificar_can.py
```

---

## ⚙️ Automatización (Opcional)

### Script de inicio automático

Crea un script `setup_can.sh`:

```bash
#!/bin/bash
echo "Configurando CAN..."
sudo modprobe can
sudo modprobe can_raw
sudo modprobe gs_usb
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up
echo "✅ can0 configurado y activo"
```

Dale permisos de ejecución:
```bash
chmod +x setup_can.sh
```

Ejecútalo cuando necesites:
```bash
./setup_can.sh
```

---

## 🐛 Solución de Problemas

### El dispositivo no aparece en lsusb
- Verifica el cable USB
- En VirtualBox: `Dispositivos` → `USB` → selecciona el CANable
- Revisa que Extension Pack esté instalado

### can0 no aparece
```bash
sudo modprobe gs_usb
dmesg | tail -20  # Ver mensajes del kernel
```

### can0 existe pero no levanta
```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up
```

### Error "Network is down"
```bash
sudo ip link set can0 up
```

### Bitrate incorrecto
Cambia el bitrate según tu red CAN:
```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 250000  # Ejemplo: 250 kbps
sudo ip link set can0 up
```

---

## 📚 Comandos Útiles de Referencia

```bash
# Ver estado detallado
ip -details -statistics link show can0

# Apagar interfaz
sudo ip link set can0 down

# Ver mensajes del kernel
dmesg | grep -i can

# Ver módulos cargados
lsmod | grep can

# Estadísticas en tiempo real
watch -n 1 'ip -statistics link show can0'
```

---

## ✅ Ventajas de Usar Linux para CAN

- **Drivers nativos:** SocketCAN integrado en el kernel
- **Sin problemas de drivers:** No necesitas Zadig ni drivers especiales
- **Herramientas incluidas:** can-utils viene con muchas utilidades
- **Python-can funciona perfecto:** Sin complicaciones
- **Estabilidad:** Más confiable que Windows para CAN

---

## 📖 Recursos Adicionales

- **Documentación CANable:** https://canable.io/
- **Python-can docs:** https://python-can.readthedocs.io/
- **Linux SocketCAN:** https://www.kernel.org/doc/html/latest/networking/can.html
- **can-utils GitHub:** https://github.com/linux-can/can-utils

---

## 📝 Notas Finales

- El bitrate debe ser el mismo en todos los dispositivos de tu red CAN
- Los buses CAN requieren terminación de 120Ω en ambos extremos
- Para proyectos de producción, considera usar un cable CAN con blindaje apropiado
- Si trabajas con OBD-II, el bitrate estándar es 500 kbps

---

**¡Listo! Ahora tienes tu CANable funcionando en Linux.** 🎉