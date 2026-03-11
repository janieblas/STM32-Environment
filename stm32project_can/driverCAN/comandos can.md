# Guía de Comandos CAN en Linux

Esta guía cubre los comandos esenciales para configurar y usar interfaces CAN en Linux con SocketCAN.

---

## 📋 Requisitos Previos

```bash
# Verificar que los módulos están cargados
lsmod | grep can

# Si no están cargados:
sudo modprobe can
sudo modprobe can_raw
sudo modprobe gs_usb
```

---

## 🔧 Configuración Básica de la Interfaz CAN

### Verificar Estado de la Interfaz

```bash
# Ver todas las interfaces de red
ip link show

# Ver solo can0
ip link show can0

# Ver detalles completos
ip -details link show can0

# Ver estadísticas
ip -statistics link show can0
```

### Apagar la Interfaz

```bash
sudo ip link set can0 down
```

> **Nota:** Debes apagar la interfaz antes de cambiar configuraciones.

### Configurar Bitrate

El bitrate debe coincidir con todos los dispositivos en tu red CAN.

```bash
# Configurar bitrate (interfaz debe estar DOWN)
sudo ip link set can0 type can bitrate 500000

# Bitrates comunes:
# 125000  (125 kbps) - CAN lento, redes industriales
# 250000  (250 kbps) - Automotriz, algunos vehículos
# 500000  (500 kbps) - Automotriz estándar, OBD-II
# 1000000 (1 Mbps)   - CAN de alta velocidad
```

### Levantar la Interfaz

```bash
sudo ip link set can0 up
```

### Secuencia Completa - Modo Normal

```bash
# 1. Apagar interfaz
sudo ip link set can0 down

# 2. Configurar bitrate y modo normal (sin loopback)
sudo ip link set can0 type can bitrate 500000 loopback off

# 3. Levantar interfaz
sudo ip link set can0 up

# 4. Verificar que está UP
ip link show can0
```

---

## 🔄 Modo Loopback

El modo loopback hace que los mensajes enviados se reciban de vuelta sin salir al bus físico.

### Activar Loopback

```bash
# Apagar interfaz
sudo ip link set can0 down

# Configurar con loopback
sudo ip link set can0 type can bitrate 500000 loopback on

# Levantar interfaz
sudo ip link set can0 up
```

### Desactivar Loopback

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000 loopback off
sudo ip link set can0 up
```

### ¿Cuándo usar loopback?

- ✅ **Desarrollo y pruebas** sin hardware adicional
- ✅ **Testing de software** antes de conectar al bus real
- ✅ **Debugging** de aplicaciones CAN
- ❌ **NO** para comunicación con dispositivos reales

---

## 🎛️ Configuraciones Avanzadas

### Listen-Only Mode (Solo Escuchar)

Útil para monitorear el bus sin interferir.

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000 listen-only on
sudo ip link set can0 up
```

> **Nota:** En este modo NO puedes enviar mensajes, solo recibir.

### Triple Sampling

Mejora la robustez en ambientes con ruido.

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000 triple-sampling on
sudo ip link set can0 up
```

### Restart Automático

Configura reinicio automático después de errores.

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000 restart-ms 100
sudo ip link set can0 up
```

> `restart-ms 100` = reiniciar automáticamente después de 100ms si entra en bus-off.

### One-Shot Mode

Para envíos sin retransmisión automática.

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000 one-shot on
sudo ip link set can0 up
```

### Configuración Combinada

Puedes combinar múltiples opciones:

```bash
sudo ip link set can0 down
sudo ip link set can0 type can \
    bitrate 500000 \
    loopback off \
    listen-only off \
    triple-sampling on \
    restart-ms 100
sudo ip link set can0 up
```

---

## 📊 Monitoreo y Diagnóstico

### Ver Estado Detallado

```bash
# Ver configuración completa
ip -details link show can0

# Ver estadísticas de tráfico
ip -statistics link show can0

# Monitoreo continuo (actualiza cada segundo)
watch -n 1 'ip -statistics link show can0'
```

### Información que verás:

```
can0: <NOARP,UP,LOWER_UP,ECHO> mtu 16 qdisc pfifo_fast state UP
    can state ERROR-ACTIVE restart-ms 0 
    bitrate 500000 sample-point 0.875
    tq 125 prop-seg 6 phase-seg1 7 phase-seg2 2 sjw 1 brp 6
    
    RX: bytes  packets  errors  dropped  missed   mcast
        12345    678       0        0        0        0
    TX: bytes  packets  errors  dropped  carrier collsns
        9876     543       0        0        0        0
```

**Estados CAN importantes:**
- `ERROR-ACTIVE` - Funcionando normalmente ✅
- `ERROR-WARNING` - Muchos errores, cuidado ⚠️
- `ERROR-PASSIVE` - Demasiados errores, limitado ⚠️
- `BUS-OFF` - Desconectado por errores ❌

### Ver Logs del Kernel

```bash
# Ver mensajes recientes del sistema CAN
dmesg | grep -i can

# Ver en tiempo real
dmesg -w | grep -i can

# Ver últimos 20 mensajes
dmesg | tail -20
```

---

## 🛠️ Herramientas de Línea de Comandos (can-utils)

### Instalar can-utils

```bash
sudo apt-get update
sudo apt-get install can-utils
```

### candump - Monitorear Tráfico

```bash
# Ver todos los mensajes
candump can0

# Con timestamp
candump can0 -t a

# Con colores
candump can0 -c

# Guardar a archivo
candump can0 -l

# Filtrar por ID
candump can0,123:7FF  # Solo ID 0x123
```

### cansend - Enviar Mensajes

```bash
# Formato: ID#DATA
cansend can0 123#DEADBEEF

# Con 8 bytes
cansend can0 123#1122334455667788

# ID extendido (29 bits)
cansend can0 12345678#AABBCCDD
```

### cangen - Generar Tráfico

```bash
# Generar mensajes aleatorios
cangen can0

# Intervalo específico (100ms)
cangen can0 -g 100

# ID específico
cangen can0 -I 123

# Número limitado de mensajes
cangen can0 -n 100
```

### cansniffer - Análisis Interactivo

```bash
# Monitor interactivo con cambios resaltados
cansniffer can0

# Con colores
cansniffer -c can0
```

### canplayer - Reproducir Logs

```bash
# Reproducir archivo de log
canplayer -I logfile.log

# Con velocidad ajustada
canplayer -I logfile.log -t
```

### cansequence - Test de Secuencia

```bash
# Enviar secuencia numerada
cansequence can0
```

---

## 🔍 Solución de Problemas

### Interfaz no aparece

```bash
# Verificar dispositivo USB
lsusb | grep -i can

# Cargar módulos manualmente
sudo modprobe gs_usb

# Ver mensajes de error
dmesg | tail -20
```

### No puede levantar la interfaz

```bash
# Verificar que está configurada
ip -details link show can0

# Reintentar secuencia completa
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up
```

### Muchos errores en el bus

```bash
# Ver estadísticas
ip -statistics link show can0

# Verificar:
# - Bitrate correcto (debe coincidir con todos los dispositivos)
# - Terminación de 120Ω en ambos extremos
# - Cables CAN-H y CAN-L correctos
# - Sin cables rotos o mal conectados
```

### Bus-Off

```bash
# Si can0 entra en BUS-OFF, reiniciar:
sudo ip link set can0 down
sudo ip link set can0 up

# O configurar restart automático:
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000 restart-ms 100
sudo ip link set can0 up
```

### Reset completo

```bash
# Apagar interfaz
sudo ip link set can0 down

# Remover módulos
sudo rmmod gs_usb
sudo rmmod can_dev

# Recargar módulos
sudo modprobe can
sudo modprobe can_raw
sudo modprobe gs_usb

# Configurar de nuevo
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up
```

---

## 📝 Scripts Útiles

### Script de Setup Rápido

Guarda como `setup_can.sh`:

```bash
#!/bin/bash

INTERFACE="can0"
BITRATE="500000"
MODE="normal"  # opciones: normal, loopback, listen-only

echo "Configurando $INTERFACE..."

# Apagar interfaz
sudo ip link set $INTERFACE down

# Configurar según modo
case $MODE in
    "normal")
        sudo ip link set $INTERFACE type can bitrate $BITRATE loopback off
        ;;
    "loopback")
        sudo ip link set $INTERFACE type can bitrate $BITRATE loopback on
        ;;
    "listen-only")
        sudo ip link set $INTERFACE type can bitrate $BITRATE listen-only on
        ;;
esac

# Levantar interfaz
sudo ip link set $INTERFACE up

# Verificar
if ip link show $INTERFACE | grep -q "state UP"; then
    echo "✅ $INTERFACE configurado correctamente"
    ip -details link show $INTERFACE
else
    echo "❌ Error al configurar $INTERFACE"
    exit 1
fi
```

Uso:
```bash
chmod +x setup_can.sh
./setup_can.sh
```

### Script de Monitoreo

Guarda como `monitor_can.sh`:

```bash
#!/bin/bash

INTERFACE="can0"

echo "Monitoreando $INTERFACE (Ctrl+C para detener)"
echo "=============================================="

# Función para mostrar estadísticas
show_stats() {
    clear
    echo "=== Estado de $INTERFACE ==="
    ip -details -statistics link show $INTERFACE
    echo ""
    echo "=== Mensajes CAN (últimos 10) ==="
}

# Mostrar estadísticas cada 2 segundos
while true; do
    show_stats
    sleep 2
done
```

Uso:
```bash
chmod +x monitor_can.sh
./monitor_can.sh
```

---

## 🎯 Casos de Uso Comunes

### Desarrollo Local (sin hardware CAN real)

```bash
# Configurar loopback
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000 loopback on
sudo ip link set can0 up

# Probar con cansend y candump en terminales separadas
# Terminal 1:
candump can0

# Terminal 2:
cansend can0 123#DEADBEEF
```

### Conectar a Red CAN Real

```bash
# Modo normal, bitrate 500k (típico automotriz)
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000 loopback off
sudo ip link set can0 up

# Monitorear tráfico
candump can0 -c
```

### Sniffing sin Interferir

```bash
# Listen-only mode
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000 listen-only on
sudo ip link set can0 up

# Solo recibir, no transmitir
candump can0 -c
```

### Debug de Red CAN con Problemas

```bash
# Configurar con reinicio automático y triple sampling
sudo ip link set can0 down
sudo ip link set can0 type can \
    bitrate 500000 \
    restart-ms 100 \
    triple-sampling on
sudo ip link set can0 up

# Monitorear errores
watch -n 1 'ip -statistics link show can0'
```

---

## 📚 Referencia Rápida de Comandos

```bash
# CONFIGURACIÓN BÁSICA
sudo ip link set can0 down                                    # Apagar
sudo ip link set can0 type can bitrate 500000                # Configurar bitrate
sudo ip link set can0 up                                     # Levantar

# MODOS ESPECIALES
sudo ip link set can0 type can bitrate 500000 loopback on           # Loopback
sudo ip link set can0 type can bitrate 500000 listen-only on        # Solo escuchar
sudo ip link set can0 type can bitrate 500000 triple-sampling on    # Triple sampling
sudo ip link set can0 type can bitrate 500000 restart-ms 100        # Auto-restart

# MONITOREO
ip link show can0                        # Estado básico
ip -details link show can0              # Estado detallado
ip -statistics link show can0           # Estadísticas

# HERRAMIENTAS can-utils
candump can0                            # Monitorear mensajes
candump can0 -c                         # Con colores
candump can0 -t a                       # Con timestamps
cansend can0 123#DEADBEEF              # Enviar mensaje
cangen can0                             # Generar tráfico
cansniffer can0                         # Monitor interactivo

# DIAGNÓSTICO
lsusb | grep -i can                     # Ver dispositivo USB
lsmod | grep can                        # Ver módulos cargados
dmesg | grep -i can                     # Ver logs del kernel
```

---

## ⚙️ Configuración Permanente (Opcional)

Para que can0 se configure automáticamente al arrancar:

### Usando systemd

Crea `/etc/systemd/system/can0.service`:

```ini
[Unit]
Description=CAN interface setup
After=network.target

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=/sbin/ip link set can0 type can bitrate 500000
ExecStart=/sbin/ip link set can0 up
ExecStop=/sbin/ip link set can0 down

[Install]
WantedBy=multi-user.target
```

Activar:
```bash
sudo systemctl enable can0.service
sudo systemctl start can0.service
```

---

## 🔗 Recursos Adicionales

- **Documentación oficial SocketCAN:** https://www.kernel.org/doc/html/latest/networking/can.html
- **can-utils GitHub:** https://github.com/linux-can/can-utils
- **CAN Bus Wikipedia:** https://en.wikipedia.org/wiki/CAN_bus
- **CANable Docs:** https://canable.io/

---

## ✅ Checklist de Configuración

Antes de usar CAN, verifica:

- [ ] Módulos del kernel cargados (`lsmod | grep can`)
- [ ] Dispositivo USB conectado y detectado (`lsusb`)
- [ ] Interfaz can0 existe (`ip link show can0`)
- [ ] Bitrate configurado correctamente
- [ ] Interfaz levantada (UP)
- [ ] Terminación de 120Ω en el bus (si usas modo normal)
- [ ] Cables CAN-H y CAN-L correctos

---

**¡Ya estás listo para trabajar con CAN en Linux!** 🚀