import mqtt from "mqtt";

import { routeToPage } from "./utils.js";
import { authStore, mqttStore, mqttSendData } from "./store.svelte.js";

let client : any;
let connectionTimeoutId : any;



// let direction;
// let steer;
// let speed;
// let steerAngle;

// let userData = {
//   "direction": direction,
//   "steer": steer,
//   "speed": speed,
//   "steerAngle": steerAngle,
// };

// JSON of the user data set
// let jsonUserData;



export function connect(clusterURL: string, username: string, password: string) {
  console.log("Connecting to MQTT broker...");
  authStore.attemptingMQTTConnection = true;

  if (clusterURL.startsWith("wss://")) {
    mqttStore.protocol = "WebSocket over TLS/SSL";
  } else {
    mqttStore.protocol = "Not WebSocket Secure";
  }

  // Generate random clientID
  if (mqttStore.clientID == "") {
    mqttStore.clientID = 'mqttjs_' + Math.random().toString(16).substr(2, 8);
  }

  client = mqtt.connect(clusterURL, {
    username: username,
    password: password,
    clientId: mqttStore.clientID,
    keepalive: 60,
    protocolId: 'MQTT',
    protocolVersion: 4,
    clean: true,
    reconnectPeriod: 1000,
    connectTimeout: 30 * 1000
  });

  client.on('connect', function () {
    console.log('Connected to the broker');
    authStore.connectedToMQTT = true;
    authStore.mqttSuccess = true;

    setTimeout(() => {
      authStore.attemptingMQTTConnection = false;
      routeToPage("dashboard");

      mqttStore.time = new Date();
      updateConnectionTime();
    }, 600)
  });

  client.on('close', function() {
    console.log("Disconnected from the broker");
    authStore.connectedToMQTT = false;
    routeToPage("login");
  });

  client.on('error', function (error : any) {
    console.log(error);
    authStore.mqttSuccess = false;
  });

  client.on('message', function (topic: string, message : any) {
    if (topic == "espData") {
      // Parse JSON data
      let recievedEspData = JSON.parse(message);
      console.log("ESP Data: ", recievedEspData);
    }

    if (topic === "pingResponse") {
      if (message.toString() === "1") {
        mqttStore.espConnected = true;
      }
    }
  });

  client.subscribe('userData');
  client.subscribe('espData');
  client.subscribe('pingResponse');

  // sendUserData();

  // Set an interval for continuesly checking every 5 seconds if the ESP is connected or not, like a ping
  setInterval(() => {
    client.publish('ping', 0);

    // If the ping has not changed in 5 seconds, then the esp is disconnected (or has troubles)
    if (mqttStore.espConnected) {
      mqttStore.espConnected = false;
    }
  }, 5000);

  // Update connection time every second
  function updateConnectionTime() {
    const now = new Date();
    const elapsedTime = now.getTime() - mqttStore.time.getTime();

    mqttStore.elapsedTime = Math.floor(elapsedTime / 1000);
    connectionTimeoutId = setTimeout(updateConnectionTime, 1000);
  }
}

export function sendMqtt() {
  const jsonMqttSend = JSON.stringify(mqttSendData);

  if (jsonMqttSend != mqttStore.latestMqttSend) {
    console.log("Sending MQTT user data:   " + jsonMqttSend);
    client.publish('userData', jsonMqttSend);
    mqttStore.latestMqttSend = jsonMqttSend;
  } else {
    console.log("MQTT user data is the same; Will not publish.");
  }
}

// Disconnect from the broker
export function disconnect() {
  console.log("Disconnecting...");

  client.end();

  // Stop connection time counting
  clearTimeout(connectionTimeoutId);
}