# Sistema Meteorológico IoT

Este proyecto consiste en un sistema meteorológico basado en IoT utilizando un ESP32 para capturar datos de temperatura, humedad, radiación solar, precipitación, y distancia. Los datos se publican en un broker MQTT y se almacenan en Firebase Firestore. Además, un servidor web en Express expone estos datos a través de una API REST.

## Componentes

- **ESP32**: Microcontrolador para la recolección de datos.
- **Sensores**:
  - DHT-11 (Temperatura y Humedad)
  - BH1750 (Radiación Solar)
  - HC-SR04 (Distancia)
  - YL-83 (Precipitación)
- **Broker MQTT**: Para la comunicación de datos.
- **Firestore**: Almacenamiento en la nube para los datos recolectados.

## Instalación

1. **ESP32**: 
   - Carga el código en el ESP32 utilizando el IDE de Arduino.
   - Conecta el dispositivo a la red WiFi y configura el broker MQTT.

2. **Node.js y MQTT**:
   - Instala las dependencias en el servidor backend:
     ```bash
     npm install mqtt express firebase-admin cors
     ```

3. **Conexión MQTT**:
   - Asegúrate de tener un broker MQTT en funcionamiento y configurado para el proyecto.

## Uso

1. **Servidor Web**: 
   - Accede a los datos de los sensores mediante la API REST en `http://localhost:3001/sensores`.

2. **MQTT**: 
   - Los datos de los sensores se publican en el tema `sensores`, los cuales pueden ser suscritos desde otros dispositivos.

## Contribuciones

Si deseas contribuir, por favor abre un "issue" o envía un "pull request".

## Licencia

Este proyecto está bajo la licencia MIT.
