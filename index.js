const { db } = require("./firebase");
const { collection, addDoc } = require("firebase/firestore");

const express = require("express");
const mqtt = require("mqtt");
const cors = require("cors");

const app = express();
const port = 3001;

app.use(cors());

let sensorData = {
  temperatura: null,
  humedad: null,
  radiacionSolar: null,
  precipitacion: null,
};

// Conectar al broker MQTT
const client = mqtt.connect("mqtt://192.168.20.24:1883");

// Suscribirse al tema donde la ESP32 publica los datos
client.on("connect", () => {
  console.log("Conectado al broker MQTT");
  client.subscribe("sensores", (err) => {
    if (!err) {
      console.log("Suscrito al tema sensores");
    } else {
      console.error("Error al suscribirse:", err);
    }
  });
});

// Recibir los datos MQTT y actualizarlos en la variable global
client.on("message", async (topic, message) => {
  if (topic === "sensores") {
    try {
      const data = JSON.parse(message.toString());
      sensorData.temperatura = data.temperatura;
      sensorData.humedad = data.humedad;
      sensorData.radiacionSolar = data.radiacionSolar;
      sensorData.precipitacion = data.precipitacion/120.0;


      console.log("Datos recibidos:", sensorData);

      // Guardar los datos en Firestore
      await guardarDatosEnFirestore(sensorData);
    } catch (error) {
      console.error("Error al procesar los datos:", error);
    }
  }
});

// Función para guardar los datos en Firestore
async function guardarDatosEnFirestore(sensorData) {
  try {
    const coleccionRef = collection(db, 'sensores'); 
    const now = new Date();
    const nuevoDocumento = {
      ...sensorData,
      timestamp: now
    };
    const docRef = await addDoc(coleccionRef, nuevoDocumento);
    console.log('Datos guardados con ID: ', docRef.id);
  } catch (error) {
    console.error('Error al guardar datos en Firestore:', error);
  }
}

app.get("/sensores", (req, res) => {
  res.json(sensorData);
});

app.listen(port, () => {
  console.log(`Servidor escuchando en http://localhost:${port}`);
});
